<!-- Copied from: -->
<!-- https://github.com/othneildrew/Best-README-Template/blob/master/BLANK_README.md -->

[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]

<br/>
<div align="center">
<h3 align="center">Win32 Simple Named-Pipe Client</h3>

  <p align="center">
    A simple to use, simple to implement named-pipe client, using the Win32 API.
    <br />
    <br />
    <a href="https://github.com/blewert/win32-simple-named-pipe-client/issues">Report Bug</a>
    ·
    <a href="https://github.com/blewert/win32-simple-named-pipe-client/issues">Request Feature</a>
  </p>
</div>

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

## Example code
### Sending data
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

### Receiving data
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

### Send/receive
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

## License
Distributed under the MIT License. See `LICENSE.txt` for more information.

## Acknowledgements
* [othneildrew](https://raw.githubusercontent.com/othneildrew) for their fantastic [README template](https://raw.githubusercontent.com/othneildrew/Best-README-Template/). This repository uses a modified version

<!-- Copied from: -->
<!-- https://github.com/othneildrew/Best-README-Template/blob/master/BLANK_README.md -->

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/blewert/win32-simple-named-pipe-client.svg?style=for-the-badge
[contributors-url]: https://github.com/blewert/win32-simple-named-pipe-client/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/blewert/win32-simple-named-pipe-client.svg?style=for-the-badge
[forks-url]: https://github.com/blewert/win32-simple-named-pipe-client/network/members
[stars-shield]: https://img.shields.io/github/stars/blewert/win32-simple-named-pipe-client.svg?style=for-the-badge
[stars-url]: https://github.com/blewert/win32-simple-named-pipe-client/stargazers
[issues-shield]: https://img.shields.io/github/issues/blewert/win32-simple-named-pipe-client.svg?style=for-the-badge
[issues-url]: https://github.com/blewert/win32-simple-named-pipe-client/issues
[license-shield]: https://img.shields.io/github/license/blewert/win32-simple-named-pipe-client.svg?style=for-the-badge
[license-url]: https://github.com/blewert/win32-simple-named-pipe-client/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/linkedin_username
[product-screenshot]: images/screenshot.png
[Next.js]: https://img.shields.io/badge/next.js-000000?style=for-the-badge&logo=nextdotjs&logoColor=white
[Next-url]: https://nextjs.org/
[React.js]: https://img.shields.io/badge/React-20232A?style=for-the-badge&logo=react&logoColor=61DAFB
[React-url]: https://reactjs.org/
[Vue.js]: https://img.shields.io/badge/Vue.js-35495E?style=for-the-badge&logo=vuedotjs&logoColor=4FC08D
[Vue-url]: https://vuejs.org/
[Angular.io]: https://img.shields.io/badge/Angular-DD0031?style=for-the-badge&logo=angular&logoColor=white
[Angular-url]: https://angular.io/
[Svelte.dev]: https://img.shields.io/badge/Svelte-4A4A55?style=for-the-badge&logo=svelte&logoColor=FF3E00
[Svelte-url]: https://svelte.dev/
[Laravel.com]: https://img.shields.io/badge/Laravel-FF2D20?style=for-the-badge&logo=laravel&logoColor=white
[Laravel-url]: https://laravel.com
[Bootstrap.com]: https://img.shields.io/badge/Bootstrap-563D7C?style=for-the-badge&logo=bootstrap&logoColor=white
[Bootstrap-url]: https://getbootstrap.com
[JQuery.com]: https://img.shields.io/badge/jQuery-0769AD?style=for-the-badge&logo=jquery&logoColor=white
[JQuery-url]: https://jquery.com 