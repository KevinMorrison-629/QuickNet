#include "quicknet/quicknet.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

int main()
{
    // --- Game Client/Server Demonstration ---
    std::cout << "--- Starting QuickNet Game Client/Server Test ---\n";
    {
        // Define network configuration
        const uint16_t PORT = 27020;
        const std::string SERVER_ADDRESS = "127.0.0.1:" + std::to_string(PORT);

        // Create manager instances
        auto serverManager = std::make_unique<QNET::Server>();
        auto clientManager = std::make_unique<QNET::Client>();

        // --- 1. Set up the Server ---
        // The server's callback prints messages it receives from any client.
        serverManager->OnMessageReceived = [](HSteamNetConnection hConn, const std::vector<uint8_t> &byteMsg)
        {
            std::string msg((const char *)byteMsg.data(), byteMsg.size());
            std::cout << "✅ [Server] Received from client " << hConn << ": '" << msg << "'\n";
        };

        // Initialize the server
        if (!serverManager->Initialize(PORT))
        {
            std::cerr << "Server initialization failed.\n";
            return 1;
        }

        // The ServerManager::Run() method is blocking, so we run it in a separate thread.
        std::thread serverThread([&]() { serverManager->Run(); });
        std::cout << "🚀 Server is running in a separate thread.\n";

        // Give the server a moment to start up before the client tries to connect.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // --- 2. Set up the Client ---
        // The client's callback prints messages it receives from the server.
        clientManager->OnMessageReceived = [](const std::vector<uint8_t> &byteMsg)
        {
            std::string msg((const char *)byteMsg.data(), byteMsg.size());
            std::cout << "📨 [Client] Received from server: '" << msg << "'\n";
        };

        // Connect the client to the server
        if (!clientManager->Connect(SERVER_ADDRESS))
        {
            std::cerr << "Client connection failed.\n";
            serverManager->Stop();
            serverThread.join();
            return 1;
        }
        std::cout << "🤝 Client is attempting to connect to " << SERVER_ADDRESS << "\n";

        // --- 3. Main Demonstration Loop ---
        // We'll run this for a few seconds, exchanging messages.
        std::cout << "\n--- Starting 5-second message exchange ---\n";
        for (int i = 0; i < 5; ++i)
        {
            // The client must manually poll for events and messages.
            clientManager->Poll();
            clientManager->ReceiveMessages();

            if (clientManager->IsConnected())
            {
                // Send a message from client to server
                std::string clientMessage = "Hello server! This is message #" + std::to_string(i + 1);
                std::vector<uint8_t> byteMsg(clientMessage.begin(), clientMessage.end());
                clientManager->SendReliableMessageToServer(byteMsg);
            }

            // The server broadcasts a message to all connected clients
            std::string serverMessage = "Public announcement #" + std::to_string(i + 1);
            std::vector<uint8_t> byteMsg(serverMessage.begin(), serverMessage.end());
            serverManager->BroadcastReliableMessage(byteMsg);

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout << "--- Message exchange finished ---\n\n";

        // --- 4. Shutdown ---
        std::cout << "🛑 Shutting down Game Server & Client.\n";
        serverManager->Stop();       // Signal the server to stop its loop
        clientManager->Disconnect(); // Disconnect the client
        serverThread.join();         // Wait for the server thread to finish execution
    }
    std::cout << "--- Game Client/Server Test Finished ---\n";

    // --- 5. Start HttpServer ---
    std::cout << "\n--- Starting QuickNet HTTP Server Test ---\n";
    try
    {
        QNET::HttpServer http_server;

        // Define a GET handler for the root path
        http_server.Get("/",
                        [](const httplib::Request &, httplib::Response &res)
                        {
                            std::string html_content = R"(
                <!DOCTYPE html><html lang="en">
                <head><title>QuickNet</title></head>
                <body><h1>Welcome!</h1><p>Served by QNET::HttpServer.</p></body>
                </html>
            )";
                            res.set_content(html_content, "text/html");
                        });

        // Define a POST handler
        http_server.Post("/api/echo",
                         [](const httplib::Request &req, httplib::Response &res)
                         {
                             std::cout << "🌐 [HttpServer] Received POST data: '" << req.body << "'\n";
                             res.set_content("Echoing your post: " + req.body, "text/plain");
                         });

        std::cout << "🚀 HTTP Server is starting. Listening on http://localhost:8080\n";
        std::cout << "   (Use Ctrl+C to stop the server)\n";

        // This is a blocking call and will run until the program is terminated.
        http_server.Run(8080);
    }
    catch (const std::exception &e)
    {
        std::cerr << "HTTP Server encountered a critical error: " << e.what() << '\n';
        return 1;
    }

    std::cout << "👋 Demonstration complete.\n";
    return 0;
}