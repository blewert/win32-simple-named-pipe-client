
# Win32 Simple Named Pipe Client

## Quickstart
1. Download the latest release from the [Releases](https://github.com/blewert/win32-simple-named-pipe-client/releases) page, choosing the `NamedPipeClient.hpp` file.
2. Include it in your project.
3. Enter `using namespace win32Pipes;` at the top of your source file
4. You're good to go! See the examples below for an idea of how to use the class.

## What is this?
This is a header-only Win32 named pipe client for sending or receiving simple strings (like JSON) or byte data. This project was initially created to interface with an NodeJS IPC server using named pipes. If you are looking for some C++ that fits in well with `node-ipc`, then you're in luck!

Please note that the project is currently in pre-release, and as such, may not function correctly in some areas. There is also not much documentation, as the class was thrown together as part of an existing project. I thought it would find utility for other folks, as it appears documentation on using named pipes via the Windows API is sparse.

## Version history
| Version | Date | Notes |
|---------|------|-------|
| `v1.0.0-rc.1` | 17/09/23 | Initial pre-release, copied over from existing project |
## Example code (sending data)
```cpp
//At the top of your source file
using namespace win32Pipes;

//...
//later:

//Make a client
NamedPipeClient client;

//Set stuff up, use default connection settings 
PipeConnectionOptions connectOptions;
client.Connect(L"tmp-app.world", connectOptions);

//At this point, the client is trying to (or has already connected) to 
//the pipe at \\.\pipe\tmp-app.world. If you are using node-ipc, then this
//is the pipe path when you specify "world"

//Write something
client.Send("{ \"type\": \"message\", \"data\": \"hi\" }");
```

## Example code (receiving data)
```cpp
//At the top of your source file
using namespace win32Pipes;

//...
//later:

//Make a client
NamedPipeClient client;

//When the PipeConnectionOptions struct is omitted, default values are loaded instead
client.Connect(L"tmp-app.world");

//Read something
PipeStatusResponse response = client.Receive();

//Was it successful? If so, print out the message
if(std::get<bool>(response))
    printf("Successfully read message: %s\n", std::get<std::string>().c_str());
```

## Example code (sending/receiving)
```cpp
//At the top of your source file
using namespace win32Pipes;

//...
//later:

//Make a client and connect it
NamedPipeClient client;
client.Connect(L"tmp-app.world");

//Write some data
client.Send("{ \"type\": \"message\", \"data\": \"hi\" }");

//Wait for a response. A PipeStatusResponse object is actually
//an std::tuple, so we can use C++17 structured bindings!
const auto [ success, message ] = client.Receive();

//Print it out if successful
if(success)
    printf("Read message: %s\n", message.c_str());
```

## The `PipeConnectionOptions` struct
The `PipeConnectionOptions` struct can be passed to specify parameters to use during connection.

### `PipeConnectionOptions.prependPrefix`, default: `true`
Whether or not the pipe prefix (by default `\\.\pipe\`) should be prepended before the pipe name. If you wish to specify your pipe's path explicitly, set this to `false`.


### `PipeConnectionOptions.delimiter`, default: `\f`
The delimiter to scan for when reading messages from the pipe. You can set this to `NULL` if you don't want to use a delimiter, or some other character value (such as `\0`) is used to delimit messages.

### `PipeConnectionOptions.openSleepTicks`, default: `250`
The number of ticks to sleep for between reconnection attempts.

### `PipeConnectionOptions.pipeWaitTime`, default: `1000 * 15` (15 seconds)
The number of ticks to wait for if a pipe is busy.

### `PipeConnectionOptions.maxAttempts`, default: `10`
The maximum number of attempts to try connecting to the pipe, before giving up.

### `PipeConnectionOptions.mode`, default: `PipeMode::Byte`
The type of pipe connection to make. By default, this uses `PIPE_READMODE_BYTE`.

**Warning:** Using `PipeMode::Message`` has not yet been extensively tested. If you plan on sending simple strings which are delimited by a character (like `node-ipc`` does), it is advised that you do not change this parameter.

# License
Distributed under the MIT License. See `LICENSE` for more information.