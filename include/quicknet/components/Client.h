#pragma once

#include "quicknet/components/ConnectionManager.h"

#include <steam/steamnetworkingsockets.h>
#include <string>

namespace QNET
{
    /// @brief Manages the client-side network connection to a server.
    /// This class handles connecting to a server, sending and receiving messages,
    /// and managing the connection state. It inherits from ConnectionManager.
    class Client : public ConnectionManager
    {
    public:
        /// @brief Attempts to connect to a server at the specified address.
        /// @param strServerAddress The IP address and port of the server (e.g., "127.0.0.1:27020").
        /// @return True if the connection attempt was initiated successfully, false otherwise.
        bool Connect(const std::string &strServerAddress);

        /// @brief Disconnects from the server.
        void Disconnect();

        /// @brief Sends a message to the connected server.
        /// The message is sent reliably.
        /// @param strMessage The message content to send.
        void SendMessageToServer(const std::string &strMessage);

        /// @brief Receives pending messages from the server.
        /// Calls the OnMessageReceived callback for each message.
        void ReceiveMessages();

        /// @brief Checks if the client is currently connected to a server.
        /// @return True if connected, false otherwise.
        bool IsConnected() const;

    public:
        /// @brief Callback function invoked when a message is received from the server.
        /// Assign a function to this member to handle incoming messages.
        /// The function should take a const std::string& (the message content) as a parameter.
        std::function<void(const std::string &)> OnMessageReceived;

    protected:
        /// @brief Handles connection status changes for the client.
        /// Overrides the base class method to manage client-specific connection states.
        /// @param pInfo Pointer to the SteamNetConnectionStatusChangedCallback_t structure.
        virtual void HandleConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo) override;

    private:
        /// @brief Handle to the current connection to the server.
        /// k_HSteamNetConnection_Invalid if not connected.
        HSteamNetConnection m_hConnection;
    };
} // namespace QNET