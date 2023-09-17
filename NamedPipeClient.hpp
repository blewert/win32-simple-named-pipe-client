#pragma once

#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <tchar.h>
#include <tuple>
#include <fstream>

#define __NS win32Pipes

namespace __NS
{
	typedef std::tuple<bool, std::string> PipeStatusResponse;

	static const unsigned int NPC_DEFAULT_PIPE_TRYOPEN_SLEEP_TICKS = 250;
	static const unsigned int NPC_DEFAULT_PIPE_TRYOPEN_TIMEOUT = 1000 * 15; // Wait for 15 seconds by default
	static const unsigned int NPC_DEFAULT_PIPE_NUM_ATTEMPTS = 10; // How many times to try before giving up?

	enum class PipeMode
	{
		Byte, Message
	};

	struct PipeConnectionOptions
	{
		bool prependPrefix = true;
		char delimiter = '\f';
		unsigned int openSleepTicks = NPC_DEFAULT_PIPE_TRYOPEN_SLEEP_TICKS;
		unsigned int pipeWaitTime = NPC_DEFAULT_PIPE_TRYOPEN_TIMEOUT;
		unsigned int maxAttempts = NPC_DEFAULT_PIPE_NUM_ATTEMPTS;
		PipeMode mode = PipeMode::Byte;
	};

	class NamedPipeClient
	{
	public:
		NamedPipeClient(NamedPipeClient& pipe);
		NamedPipeClient(void);
		~NamedPipeClient();

		bool Connect(const std::wstring& pipeName, const PipeConnectionOptions& options);
		bool Connect(const std::wstring& pipeName);
		void Close(void);

		PipeStatusResponse Send(const std::string& text);
		PipeStatusResponse Receive();
		
		std::wstring GetPipeName(void) const;
		std::wstring GetPipePath(void) const;
		std::wstring GetPipePrefix(void) const;

		void SetPipePrefix(const std::wstring& pipePrefix);

	private:

		HANDLE pipeHandle = INVALID_HANDLE_VALUE;

		bool pipeOpen = false;

		std::wstring pipeName = L"";
		std::wstring pipePrefix = L"\\\\.\\pipe\\";

		bool Open(void);
		PipeStatusResponse TryOpen(void);

		unsigned int attempts = 0;

		PipeConnectionOptions connectionOptions;
	};


	std::string GetLastErrorAsString()
	{
		//Get the error message ID, if any.
		DWORD errorMessageID = GetLastError();
		if (errorMessageID == 0) {
			return std::string(); //No error message has been recorded
		}

		LPSTR messageBuffer = nullptr;

		//Ask Win32 to give us the string version of that message ID.
		//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		//Copy the error message into a std::string.
		std::string message(messageBuffer, size);

		//Free the Win32's string's buffer.
		LocalFree(messageBuffer);

		return message + " (" + std::to_string(errorMessageID) + ")";
	}

	NamedPipeClient::NamedPipeClient(NamedPipeClient& pipe)
	{
		//Set pipe name
		this->pipeName = pipe.GetPipeName();
	}

	NamedPipeClient::NamedPipeClient(void) {};

	NamedPipeClient::~NamedPipeClient()
	{
		this->Close();
	}


	PipeStatusResponse NamedPipeClient::TryOpen(void)
	{
		//Parameters for the pipe
		LPSECURITY_ATTRIBUTES pipeSecurityAttrs = NULL;
		DWORD pipeAccess = GENERIC_READ | GENERIC_WRITE;
		DWORD pipeSharing = 0;
		DWORD pipeCreationDispostion = OPEN_EXISTING;
		DWORD pipeDefaultAttrs = 0;
		HANDLE pipeTemplateFile = NULL;

		//Get the pipe name as a wchar_t*
		std::wstring pipePath = this->GetPipePath();
		const wchar_t* const pipeNameBuf = pipePath.c_str();

		//Try to connect to the pipe
		this->pipeHandle = CreateFileW(pipeNameBuf, pipeAccess, pipeSharing, pipeSecurityAttrs, pipeCreationDispostion, pipeDefaultAttrs, pipeTemplateFile);

		//Invalid handle?
		if (this->pipeHandle == INVALID_HANDLE_VALUE)
			return PipeStatusResponse{ false, "Pipe handle was invalid when connecting." };

		//Did we get an error?
		if (GetLastError() && GetLastError() != ERROR_PIPE_BUSY)
			return PipeStatusResponse{ false, "Unknown error occurred during pipe connection, code " + GetLastErrorAsString() + "!" };

		//Wait for a bit
		if (!WaitNamedPipeW(pipeNameBuf, this->connectionOptions.pipeWaitTime))
			return PipeStatusResponse{ false, "Pipe is still busy after " + std::to_string(this->connectionOptions.pipeWaitTime) + " ticks" };

		//Otherwise.. we're okay
		return PipeStatusResponse{ true, "Pipe connected" };
	}

	bool NamedPipeClient::Open(void)
	{
		while (!this->pipeOpen)
		{
			Sleep(connectionOptions.openSleepTicks);

			const PipeStatusResponse response = this->TryOpen();
			this->pipeOpen = std::get<bool>(response);

			if (++attempts >= connectionOptions.maxAttempts)
				return false;

			if (!this->pipeOpen)
			{
				fprintf(stderr, "Error connecting to pipe: %s\n", std::get<std::string>(response).c_str());
				continue;
			}

			break;
		}

		fprintf(stdout, "Pipe connected with handle %d\n", (int)this->pipeHandle);
		return true;
	}

	PipeStatusResponse NamedPipeClient::Receive(void)
	{
		//Not open? Open it up
		if (!pipeOpen)
		{
			//Try open the pipe
			bool status = this->Open();

			//Failed? Get out of here
			if (!status)
				return { false, "Failed to open the pipe" };
		}

		//------------------

		//Set mode
		const DWORD pipeMode = (this->connectionOptions.mode == PipeMode::Byte) ? (PIPE_READMODE_BYTE) : (PIPE_READMODE_MESSAGE);
		bool success = SetNamedPipeHandleState(this->pipeHandle, (LPDWORD)&pipeMode, NULL, NULL);

		//No success? Get out of here
		if (!success)
			return { false, "Failed to set pipe handle state; " + GetLastErrorAsString() };

		//Otherwise..
		//------------------ 

		//The character buffer to save into
		char pipeMsgCharBuffer[512] = { 0 };
		DWORD pipeNumBytesToRead = 512 * sizeof(char);
		DWORD pipeNumBytesRead = 0;

		//The string to build
		std::string buildStr = "";

		while (success)
		{
			//Read number of bytes
			success = ReadFile(this->pipeHandle, pipeMsgCharBuffer, pipeNumBytesToRead, &pipeNumBytesRead, NULL);

			//Error for reasons unknown
			if (!success && GetLastError() != ERROR_MORE_DATA)
				return { false, "Encountered error whilst reading pipe data; " + GetLastErrorAsString() };

			//Otherwise! Append to string
			int i = 0;

			for (i = 0; i < pipeNumBytesRead && pipeMsgCharBuffer[i] != connectionOptions.delimiter; i++)
				buildStr += pipeMsgCharBuffer[i];

			//Check if hit delimiter; if so, return result
			if (pipeMsgCharBuffer[i] == connectionOptions.delimiter)
				return { true, buildStr };
		}

		//By default, show what was read so far
		return { true, buildStr };
	}

	PipeStatusResponse NamedPipeClient::Send(const std::string& text)
	{
		//Not open? Open it up
		if (!pipeOpen)
		{
			//Try open the pipe
			bool status = this->Open();

			//Failed? Get out of here
			if (!status)
				return { false, "Failed to open the pipe" };
		}

		//------------------

		//Set mode
		const DWORD pipeMode = (this->connectionOptions.mode == PipeMode::Byte) ? (PIPE_READMODE_BYTE) : (PIPE_READMODE_MESSAGE);
		const bool success = SetNamedPipeHandleState(this->pipeHandle, (LPDWORD)&pipeMode, NULL, NULL);

		//No success? Get out of here
		if (!success)
			return { false, "Failed to set pipe handle state; " + GetLastErrorAsString() };

		//Otherwise..
		//------------------ 

		//Append a character
		std::string messageToSend = text;
		messageToSend += this->connectionOptions.delimiter;

		//Build buffer and other variables
		const char* pipeWriteBuffer = messageToSend.data();
		const unsigned int pipeWriteBufferSize = messageToSend.size();
		unsigned int pipeBytesWritten = 0;

		//Try write
		bool writeSuccess = WriteFile(this->pipeHandle, pipeWriteBuffer, pipeWriteBufferSize, (LPDWORD)&pipeBytesWritten, NULL);

		if (!writeSuccess)
			return { false, "Failed to write file; " + GetLastErrorAsString() };

		return { true, "Success, bytes written: " + std::to_string(pipeBytesWritten) };
	}

	inline bool NamedPipeClient::Connect(const std::wstring& pipeName)
	{
		//Set up pipename and connection options
		this->pipeName = pipeName;
		this->connectionOptions = PipeConnectionOptions { };

		//Open the pipe
		return this->Open();
	}

	inline bool NamedPipeClient::Connect(const std::wstring& pipeName, const PipeConnectionOptions& options)
	{
		//Set up pipename and connection options
		this->pipeName = pipeName;
		this->connectionOptions = options;

		//Open the pipe
		return this->Open();
	}

	void NamedPipeClient::Close(void)
	{
		if (this->pipeHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(this->pipeHandle);
			this->pipeHandle = INVALID_HANDLE_VALUE;
		}
	}

	std::wstring NamedPipeClient::GetPipeName(void) const
	{
		return this->pipeName;
	}

	std::wstring NamedPipeClient::GetPipePath(void) const
	{
		if (this->connectionOptions.prependPrefix)
			return this->pipePrefix + this->pipeName;
		else
			return this->pipeName;
	}

	std::wstring NamedPipeClient::GetPipePrefix(void) const
	{
		return this->pipePrefix;
	}

	void NamedPipeClient::SetPipePrefix(const std::wstring& pipePrefix)
	{
		this->pipePrefix = pipePrefix;
	}
}