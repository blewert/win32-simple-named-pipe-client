// named-pipe-client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "NamedPipeClient.hpp"

using namespace win32Pipes;

int main()
{
    //Make a client
    NamedPipeClient client;

    //Set stuff up
    PipeConnectionOptions connectOptions;
    client.Connect(L"tmp-app.world", connectOptions);

    //Write something
    client.Send("{ \"type\": \"message\", \"data\": \"hi\" }");

    //Read something
    auto response = client.Receive();

    //Could read? Print it out
    if (std::get<bool>(response))
        std::cout << "Read message: " + std::get<std::string>(response) << std::endl;

    //Otherwise
    else
        std::cout << "Couldn't read message" << std::endl;

    //Close it up
    client.Close();
}
