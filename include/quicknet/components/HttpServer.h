#pragma once

#include "httplib.h"

#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace QNET
{
    /// @brief Manages a simple, high-level HTTP server.
    /// @details This class provides a wrapper around the cpp-httplib library
    /// to simplify the creation of HTTP endpoints for web services or APIs.
    class HttpServer
    {
    public:
        /// @brief Constructs an HttpServer instance.
        /// @details Initializes the underlying httplib::Server and sets up default
        /// handlers for logging requests and formatting error responses.
        HttpServer();

        /// @brief Destructor for the HttpServer.
        /// @details Ensures the server is stopped cleanly upon object destruction.
        ~HttpServer();

        // Prevent copying and assignment
        HttpServer(const HttpServer &) = delete;
        HttpServer &operator=(const HttpServer &) = delete;

        /// @brief Registers a handler for HTTP GET requests on a specific path.
        /// @param path The URL path to handle (e.g., "/").
        /// @param handler The function to execute when a request matches the path.
        void Get(const std::string &path, httplib::Server::Handler handler);

        /// @brief Registers a handler for HTTP POST requests on a specific path.
        /// @param path The URL path to handle (e.g., "/api/submit").
        /// @param handler The function to execute when a request matches the path.
        void Post(const std::string &path, httplib::Server::Handler handler);

        /// @brief Starts the server and listens for connections on the specified port.
        /// @details This is a blocking call that will run until Stop() is called or the program is terminated.
        /// @param port The port number to listen on.
        void Run(uint16_t port);

        /// @brief Stops the HTTP server if it is running.
        void Stop();

    private:
        /// @brief Logs an error message to the standard error stream.
        /// @param msg The message to log.
        void log_message(const std::string &msg);

        /// @brief The underlying httplib server instance.
        /// @details std::unique_ptr is used to manage its lifetime.
        std::unique_ptr<httplib::Server> m_server;
    };
} // namespace QNET