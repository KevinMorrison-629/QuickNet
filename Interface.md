# QuickNet Library Interface

This document outlines the public interface of the QuickNet library. It is intended for developers who want to integrate QuickNet into their projects.

## Overview

The QuickNet library provides a simple interface for creating client-server network applications using the SteamNetworkingSockets library. The main components are the `Client`, `Server`, and `HttpServer` classes.

## Threading Model

The library offers both blocking and non-blocking methods for handling network events.

- **`Server::Run()` & `HttpServer::Run()`**: These are **blocking** functions that enter an infinite loop to manage server operations. If you need your application's main thread to remain responsive, you should run the server in a separate thread. For example:
  ```cpp
  #include <thread>
  // ...
  QNET::Server server;
  // ... initialize server ...
  std::thread server_thread(&QNET::Server::Run, &server);
  // ... main thread continues ...
  server_thread.join();
  ```

- **`Poll()` and `ReceiveMessages()`**: These functions are **non-blocking**. They are designed to be called repeatedly in your main application loop to process network events without halting your program. This is the recommended approach for most client applications and for servers that need to perform other tasks on the main thread.

---

## `HttpServer` Class

Manages a simple, high-level HTTP server, built on top of `cpp-httplib`.

### Use Case

Use the `HttpServer` class to quickly create a web server for APIs or simple web pages. It handles the low-level HTTP protocol details, allowing you to focus on defining routes and handlers for `GET` and `POST` requests.

### Public Functions

- **`HttpServer()`**:
   - **Description**: Constructs the server instance. Initializes the underlying `cpp-httplib` server and sets up default logging and error handlers.

- **`void Get(const std::string& path, httplib::Server::Handler handler)`**:
   - **Description**: Registers a function to handle HTTP GET requests for a given URL path.
   - **Parameters**:
    - **path**: The URL path to handle (e.g., "/").
    - **handler**: A function (or lambda) that takes a `const httplib::Request&` and a `httplib::Response&` as parameters.

- **`void Post(const std::string& path, httplib::Server::Handler handler)`**:
   - **Description**: Registers a function to handle HTTP POST requests for a given URL path.
   - **Parameters**:
    - **path**: The URL path to handle (e.g., "/api/data").
    - **handler**: A function (or lambda) that takes a `const httplib::Request&` and a `httplib::Response&` as parameters.

- **`void Run(uint16_t port)`**:
   - **Description**: Starts the server and listens for connections on the specified port. This is a blocking call that will run until `Stop()` is called or the program is terminated.
   - **Parameters**:
    - **port**: The port number to listen on.

- **`void Stop()`**:
   - **Description**: Stops the server if it is currently running.


## `ConnectionManager` Class

This is the base class for both `Client` and `Server`. It handles the low-level network polling and provides core message-sending functionality.

### Use Case

The `ConnectionManager` provides the core polling and messaging mechanism that drives the networking for both clients and servers. It is not meant to be instantiated directly.

### Public Functions

- **`Poll()`**:
  - **Description**: Polls for network events. This method should be called regularly to process incoming messages and connection status changes.

- **`void SendReliableMessage(HSteamNetConnection hConn, const std::vector<uint8_t> &byteMessage)`**:
  - **Description**: Sends a reliable message to a specific connection (guarantees delivery and order).
  - **Parameters**:
    - **hConn**: The handle of the connection to send the message to.
    - **byteMessage**: The message content to send.

- **`void SendUnreliableMessage(HSteamNetConnection hConn, const std::vector<uint8_t> &byteMessage)`**:
  - **Description**: Sends an unreliable message to a specific connection (faster, but no delivery guarantees).
  - **Parameters**:
    - **hConn**: The handle of the connection to send the message to.
    - **byteMessage**: The message content to send.

---

## `Client` Class

Manages the client-side network connection to a server.

### Use Case

Use the `Client` class to connect to a server, send and receive messages, and manage the connection state.

### Public Functions

- **`bool Connect(const std::string &strServerAddress)`**:
  - **Description**: Attempts to connect to a server at the specified address.
  - **Parameters**:
    - `strServerAddress`: The IP address and port of the server (e.g., "127.0.0.1:27020").
  - **Returns**: `true` if the connection attempt was initiated successfully, `false` otherwise.

- **`void Disconnect()`**:
  - **Description**: Disconnects from the server.

- **`void SendReliableMessageToServer(const std::vector<uint8_t> &byteMessage)`**:
  - **Description**: Sends a reliable message to the connected server.
  - **Parameters**:
    - `byteMessage`: The message content to send.

- **`void SendUnreliableMessageToServer(const std::vector<uint8_t> &byteMessage)`**:
  - **Description**: Sends a unreliable message to the connected server.
  - **Parameters**:
    - `byteMessage`: The message content to send.

- **`void ReceiveMessages()`**:
  - **Description**: Receives pending messages from the server. Calls the `OnMessageReceived` callback for each message.

- **`bool IsConnected() const`**:
  - **Description**: Checks if the client is currently connected to a server.
  - **Returns**: `true` if connected, `false` otherwise.

### Public Variables

- **`std::function<void(const std::vector<uint8_t> &)> OnMessageReceived`**:
  - **Description**: A callback function that is invoked when a message is received from the server. Assign a function to this member to handle incoming messages.

---

## `Server` Class

Manages the server-side network operations, including listening for client connections.

### Use Case

Use the `Server` class to create a server that listens for incoming connections, broadcasts messages to clients, and manages connected clients.

### Public Functions

- **`bool Initialize(uint16 nPort)`**:
  - **Description**: Starts the server and begins listening for incoming connections on the specified port.
  - **Parameters**:
    - `nPort`: The port number to listen on.
  - **Returns**: `true` if the server started successfully and is listening, `false` otherwise.

- **`void Run()`**:
  - **Description**: This is a blocking call that runs the server until `Stop()` is called.

- **`void Stop()`**:
  - **Description**: Stops the server, disconnects all clients, and closes the listen socket.

- **`void BroadcastReliableMessage(const std::vector<uint8_t> &byteMessage)`**:
  - **Description**: Broadcasts a reliable message to all connected clients.
  - **Parameters**:
    - `byteMessage`: The message content to broadcast.

- **`void BroadcastUnreliableMessage(const std::vector<uint8_t> &byteMessage)`**:
  - **Description**: Broadcasts a unreliable message to all connected clients.
  - **Parameters**:
    - `byteMessage`: The message content to broadcast.

- **`void ReceiveMessages()`**:
  - **Description**: Receives and processes pending messages from all connected clients. This method should be called regularly to handle incoming data.

### Public Variables

- **`std::function<void(HSteamNetConnection, const std::vector<uint8_t> &)> OnMessageReceived`**:
  - **Description**: A callback function that is invoked when a message is received from a client. Assign a function to this member to handle incoming messages.
  - **Parameters**:
    - `HSteamNetConnection`: The handle of the client who sent the message.
    - `const std::vector<uint8_t> &`: The message content.
