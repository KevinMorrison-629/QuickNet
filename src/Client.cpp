#include "quicknet/components/Client.h"

#include <iostream>

namespace QNET
{
    /// @brief Attempts to connect to a server at the specified address.
    /// It parses the server address string, sets up connection options (including the global status callback),
    /// and initiates the connection.
    /// @param strServerAddress The IP address and port of the server (e.g., "127.0.0.1:27020").
    /// @return True if the connection attempt was initiated successfully, false if the interface is not available,
    /// the address is invalid, or the connection creation fails.
    bool Client::Connect(const std::string &strServerAddress)
    {
        if (!m_pInterface)
            return false;

        SteamNetworkingIPAddr serverAddr;
        const char *srvAddress = strServerAddress.c_str();
        if (!serverAddr.ParseString(srvAddress))
        {
            /// @brief Logs an error if the server address is invalid.
            std::cerr << "Invalid server address: " << strServerAddress << std::endl;
            return false;
        }

        // Set up configuration options for the connection.
        SteamNetworkingConfigValue_t opts[2];
        opts[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                       (void *)ConnectionManager::OnGlobalConnectionStatusChanged);
        opts[1].SetInt64(k_ESteamNetworkingConfig_ConnectionUserData,
                         (int64)this); // Pass this Client instance as user data.

        m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 2, opts);
        if (m_hConnection == k_HSteamNetConnection_Invalid)
        {
            /// @brief Logs an error if connection creation fails.
            std::cerr << "Failed to create connection." << std::endl;
            return false;
        }

        return true;
    }

    /// @brief Disconnects from the server.
    /// If connected, it closes the connection gracefully and marks the connection handle as invalid.
    void Client::Disconnect()
    {
        if (!m_pInterface || m_hConnection == k_HSteamNetConnection_Invalid)
            return;

        m_pInterface->CloseConnection(m_hConnection, 0, "Client disconnecting", true);
        m_hConnection = k_HSteamNetConnection_Invalid;
    }

    /// @brief Sends a message to the connected server.
    /// The message is sent reliably. Does nothing if not connected.
    /// @param strMessage The message content to send.
    void Client::SendMessageToServer(const std::string &strMessage)
    {
        if (!IsConnected())
            return;
        m_pInterface->SendMessageToConnection(m_hConnection, strMessage.c_str(), strMessage.length(),
                                              k_nSteamNetworkingSend_Reliable, nullptr);
    }

    /// @brief Checks if the client is currently connected to a server.
    /// A connection is considered active if its handle is not k_HSteamNetConnection_Invalid.
    /// @return True if connected, false otherwise.
    bool Client::IsConnected() const
    {
        // A connection is considered active if its handle is not invalid.
        return m_hConnection != k_HSteamNetConnection_Invalid;
    }

    /// @brief Handles connection status changes for the client.
    /// This method is called by the global connection status callback. It processes events
    /// like successful connection, disconnection by peer, or local problem detection.
    /// @param pInfo Pointer to the SteamNetConnectionStatusChangedCallback_t structure containing event details.
    void Client::HandleConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
    {
        // The client only cares about events for its single connection.
        if (pInfo->m_hConn != m_hConnection)
            return;

        switch (pInfo->m_info.m_eState)
        {
        case k_ESteamNetworkingConnectionState_Connected:
            /// @brief Logs successful connection to the server.
            std::cout << "Client: Successfully connected to server." << std::endl;
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            /// @brief Logs disconnection from the server and the reason.
            std::cout << "Client: Disconnected from server. Reason: " << pInfo->m_info.m_szEndDebug << std::endl;
            m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false); // Close the connection formally.
            m_hConnection = k_HSteamNetConnection_Invalid;                    // Mark as disconnected.
            break;
        }

        default:
            // Other states like 'Connecting' are transitional and don't need specific handling here.
            // k_ESteamNetworkingConnectionState_Connecting
            // k_ESteamNetworkingConnectionState_FindingRoute
            // etc.
            break;
        }
    }

    /// @brief Receives pending messages from the server.
    /// If connected, it polls for incoming messages. For each received message,
    /// if the OnMessageReceived callback is set, it's invoked with the message content.
    /// Messages are released after processing.
    void Client::ReceiveMessages()
    {
        if (!IsConnected())
            return;

        ISteamNetworkingMessage *pIncomingMsgs[16]; // Buffer for incoming messages.
        int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, pIncomingMsgs, 16);
        if (numMsgs > 0)
        {
            for (int i = 0; i < numMsgs; ++i)
            {
                if (pIncomingMsgs[i] && pIncomingMsgs[i]->m_cbSize > 0)
                {
                    std::vector<uint8_t> msg((const char *)pIncomingMsgs[i]->m_pData,
                                             (const char *)pIncomingMsgs[i]->m_pData + pIncomingMsgs[i]->m_cbSize);

                    // If the application has set a callback, use it.
                    if (OnMessageReceived)
                    {
                        /// @brief Invokes the application-defined callback for the received message.
                        OnMessageReceived(msg);
                    }

                    pIncomingMsgs[i]->Release(); // Release the message resource.
                }
            }
        }
    }
} // namespace QNET