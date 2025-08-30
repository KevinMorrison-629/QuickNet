# QuickNet

A C++ library for creating simple and fast client-server applications, built on top of Valve's GameNetworkingSockets. It provides a straightforward, high-level interface for C++ applications.

---

## Features

-   Modern C++17 interface.
-   Simple, high-level abstractions for `Client` and `Server`.
-   Callback-based message handling (`OnMessageReceived`).
-   Send messages to all clients (`BroadcastMessage`) or a specific client (`SendMessageToClient`).
-   Built on the reliable and performant GameNetworkingSockets library.

---

## Getting Started

### Prerequisites

-   A C++17 compliant compiler (e.g., GCC, Clang, MSVC).
-   CMake (version 3.20 or later).
-   `vcpkg` for dependency management.

### Build Instructions

1.  **Clone the repository:**
    ```bash
    git clone <repository-url>
    cd QuickNet
    ```

2.  **Install dependencies using vcpkg:**
    This project uses `vcpkg` to manage dependencies. The required `gamenetworkingsockets` dependency will be installed automatically if you have `vcpkg` integrated with your shell. If not, you can install it manually:
    ```bash
    vcpkg install gamenetworkingsockets
    ```

3.  **Configure and build with CMake:**
    You must provide the path to the `vcpkg.cmake` toolchain file when configuring the project. This is already defined in CMakePresets.json.
    ```bash
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
    cmake --build build
    ```

### Integration with your project

To use QuickNet in your own CMake project, you can include it as a submodule.

1.  **Add QuickNet as a submodule:**
    ```bash
    git submodule add <repository-url> extern/quicknet
    ```

2.  **Update your `CMakeLists.txt`:**
    ```cmake
    # Add the submodule directory
    add_subdirectory(extern/quicknet)

    # ... later in your CMakeLists.txt ...

    # Link against the quicknet library
    target_link_libraries(your_executable_or_library PRIVATE quicknet)
    ```

Make sure you also configure your main project with the `vcpkg.cmake` toolchain file so that the dependencies are resolved correctly.

---

## Example Usage

Here is a complete example of how to use QuickNet.

### Server

This example shows how to start a server, listen for messages, and broadcast them to all connected clients.

```cpp
#include <quicknet/quicknet.h>
#include <iostream>
#include <vector>
#include <string>

int main() {
    QNET::Server server;

    // Set a callback for when messages are received
    server.OnMessageReceived = [&](HSteamNetConnection hConn, const std::vector<uint8_t>& msg) {
        std::string message(msg.begin(), msg.end());
        std::cout << "Server received: " << message << " from client " << hConn << std::endl;
        
        // Echo the message back to the client that sent it
        std::string echo_msg = "Server echoes: " + message;
        server.SendMessageToClient(hConn, std::vector<uint8_t>(echo_msg.begin(), echo_msg.end()));
    };

    // Initialize the server on port 27020
    if (server.Initialize(27020)) {
        std::cout << "Server listening on port 27020" << std::endl;
        // Run the server (this is a blocking call)
        server.Run();
    } else {
        std::cerr << "Could not start server" << std::endl;
        return 1;
    }

    return 0;
}
```

### Client

This example shows how to connect to a server, send a message, and receive responses.

```cpp
#include <quicknet/quicknet.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

int main() {
    QNET::Client client;

    // Set a callback for when messages are received
    client.OnMessageReceived = [](const std::vector<uint8_t>& msg) {
        std::cout << "Client received: " << std::string(msg.begin(), msg.end()) << std::endl;
    };

    // Connect to the server
    if (client.Connect("127.0.0.1:27020")) {
        // Wait for the connection to be established
        while (!client.IsConnected()) {
            client.Poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::cout << "Client connected to server." << std::endl;

        // Send a message
        std::string message = "Hello, server!";
        client.SendMessageToServer(std::vector<uint8_t>(message.begin(), message.end()));

        // Poll for a few seconds to receive messages
        for (int i = 0; i < 5; ++i) {
            client.Poll();
            client.ReceiveMessages();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        client.Disconnect();
        std::cout << "Client disconnected." << std::endl;

    } else {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }

    return 0;
}
```
