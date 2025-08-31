#pragma once

#include "quicknet/components/ConnectionManager.h"

#include <functional>
#include <string>
#include <vector>

namespace QNET
{
    /// @brief Manages the server-side network operations, including listening for client connections.
    /// This class handles starting and stopping the server, broadcasting messages to clients,
    /// and managing connected clients. It inherits from ConnectionManager.
    class Server : public ConnectionManager
    {
    public:
        /// @brief Starts the server and begins listening for incoming connections on the specified port.
        /// @param nPort The port number to listen on.
        /// @return True if the server started successfully and is listening, false otherwise.
        bool Initialize(uint16 nPort);

        /// @brief Starts the server
        /// @details This is a blocking call that runs until Stop() is called.
        void Run();

        /// @brief Stops the server, disconnects all clients, and closes the listen socket.
        void Stop();

        /// @brief Broadcasts a reliable message to all connected clients.
        /// @param byteMessage The message content to broadcast.
        void BroadcastReliableMessage(const std::vector<uint8_t> &byteMessage);

        /// @brief Broadcasts an Unreliable message to all connected clients.
        /// @param byteMessage The message content to broadcast.
        void BroadcastUnreliableMessage(const std::vector<uint8_t> &byteMessage);

        /// @brief Receives and processes pending messages from all connected clients.
        /// This method should be called regularly to handle incoming data.
        void ReceiveMessages();

    public:
        /// @brief Callback function invoked when a message is received from a client.
        /// Assign a function to this member to handle incoming messages.
        /// The function should take a HSteamNetConnection (the client's handle) and a
        /// const std::string& (the message content) as parameters.
        std::function<void(HSteamNetConnection, const std::vector<uint8_t> &)> OnMessageReceived;

    protected:
        /// @brief Handles connection status changes for the server.
        /// Overrides the base class method to manage server-specific connection events,
        /// such as new client connections, disconnections, and connection acceptance.
        /// @param pInfo Pointer to the SteamNetConnectionStatusChangedCallback_t structure.
        virtual void HandleConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo) override;

    private:
        /// @brief Handle to the listen socket used by the server.
        /// k_HSteamListenSocket_Invalid if the server is not listening.
        HSteamListenSocket m_hListenSocket;

        /// @brief Vector storing the connection handles of all currently connected clients.
        std::vector<HSteamNetConnection> m_vecClients;

        /// @brief Flag indicating whether the ServerManager is currently running.
        bool m_isRunning = false;
    };
} // namespace QNET