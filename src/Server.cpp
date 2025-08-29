#include "quicknet/components/Server.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

namespace QNET
{
    /// @brief Starts the server and begins listening for incoming connections on the specified port.
    /// Configures the listen socket with the global connection status callback and sets this Server
    /// instance as user data for the callback.
    /// @param nPort The port number to listen on.
    /// @return True if the server started successfully and the listen socket was created,
    /// false if the network interface is unavailable or socket creation fails.
    bool Server::Initialize(uint16 nPort)
    {
        if (!m_pInterface)
            return false;

        SteamNetworkingIPAddr serverAddr;
        serverAddr.Clear(); // Initialize to listen on all local addresses
        serverAddr.m_port = nPort;

        // Set up the configuration options for the listen socket.
        SteamNetworkingConfigValue_t opts[2];

        // Option 1: Set the callback function for connection status changes.
        // The static function ConnectionManager::OnGlobalConnectionStatusChanged will dispatch to our instance method.
        opts[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                       (void *)ConnectionManager::OnGlobalConnectionStatusChanged);

        // Option 2: Set the user data to be a pointer to this Server instance.
        // This is crucial for the static callback to find the correct instance to delegate to.
        opts[1].SetInt64(k_ESteamNetworkingConfig_ConnectionUserData, (int64)this);

        m_hListenSocket = m_pInterface->CreateListenSocketIP(serverAddr, 2, opts);
        if (m_hListenSocket == k_HSteamListenSocket_Invalid)
        {
            /// @brief Logs an error if listen socket creation fails.
            std::cerr << "Failed to create listen socket on port " << nPort << std::endl;
            return false;
        }
        /// @brief Logs successful server start and listening port.
        std::cout << "Server listening on port " << nPort << std::endl;

        return true;
    }

    void Server::Run()
    {
        m_isRunning = true;
        while (m_isRunning)
        {
            Poll();
            ReceiveMessages();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    /// @brief Stops the server.
    /// Closes all active client connections and then closes the listen socket.
    void Server::Stop()
    {
        m_isRunning = false;

        if (!m_pInterface)
            return;

        /// @brief Logs that the server is shutting down.
        std::cout << "Server shutting down..." << std::endl;
        // Close all active client connections.
        for (HSteamNetConnection conn : m_vecClients)
        {
            m_pInterface->CloseConnection(conn, 0, "Server shutting down", true);
        }
        m_vecClients.clear();

        // Close the listen socket.
        if (m_hListenSocket != k_HSteamListenSocket_Invalid)
        {
            m_pInterface->CloseListenSocket(m_hListenSocket);
            m_hListenSocket = k_HSteamListenSocket_Invalid;
        }
        /// @brief Logs that the server has stopped.
        std::cout << "Server stopped." << std::endl;
    }

    /// @brief Broadcasts a message to all currently connected clients.
    /// The message is sent reliably. Does nothing if the network interface is not available.
    /// @param strMessage The message content to broadcast.
    void Server::BroadcastMessage(const std::string &strMessage)
    {
        if (!m_pInterface)
            return;

        /// @brief Logs the message being broadcast.
        // std::cout << "Broadcasting message: " << strMessage << std::endl; // Optional: for debugging
        for (HSteamNetConnection hConn : m_vecClients)
        {
            m_pInterface->SendMessageToConnection(hConn, strMessage.c_str(), strMessage.length(),
                                                  k_nSteamNetworkingSend_Reliable, nullptr);
        }
    }

    /// @brief Handles connection status changes.
    /// This method is called by the global connection status callback. It manages new client connections
    /// (accepting them), and handles disconnections by removing clients from the active list.
    /// @param pInfo Pointer to the SteamNetConnectionStatusChangedCallback_t structure with event details.
    void Server::HandleConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
    {
        switch (pInfo->m_info.m_eState)
        {
        case k_ESteamNetworkingConnectionState_Connecting:
        {
            /// @brief Logs a connection request from a client.
            std::cout << "Server: Connection request from " << pInfo->m_info.m_szConnectionDescription << std::endl;
            // Attempt to accept the new connection.
            if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK)
            {
                // If acceptance fails, close the connection.
                m_pInterface->CloseConnection(pInfo->m_hConn, 0, "Failed to accept (server busy?)", false);
                /// @brief Logs failure to accept a connection.
                std::cout << "Server: Failed to accept connection from " << pInfo->m_info.m_szConnectionDescription
                          << std::endl;
            }
            else
            {
                /// @brief Logs successful acceptance of a connection.
                std::cout << "Server: Accepted connection from " << pInfo->m_info.m_szConnectionDescription << std::endl;
            }
            break;
        }

        case k_ESteamNetworkingConnectionState_Connected:
        {
            /// @brief Logs that a client has successfully connected and adds them to the client list.
            std::cout << "Server: Client connected. ID: " << pInfo->m_hConn << " ("
                      << pInfo->m_info.m_szConnectionDescription << ")" << std::endl;
            m_vecClients.push_back(pInfo->m_hConn);
            // You might want to send a welcome message or perform other setup here.
            break;
        }

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            /// @brief Logs that a client has disconnected and removes them from the client list.
            std::cout << "Server: Client disconnected. ID: " << pInfo->m_hConn << " ("
                      << pInfo->m_info.m_szConnectionDescription << "). Reason: " << pInfo->m_info.m_szEndDebug << std::endl;
            m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false); // Ensure connection is closed.

            // Remove the client from our active list.
            auto it = std::remove(m_vecClients.begin(), m_vecClients.end(), pInfo->m_hConn);
            if (it != m_vecClients.end())
            {
                m_vecClients.erase(it);
            }
            break;
        }
        // k_ESteamNetworkingConnectionState_None is typically for new connections before any state.
        // k_ESteamNetworkingConnectionState_FinWait and k_ESteamNetworkingConnectionState_Linger are part of graceful
        // shutdown, usually not needing direct handling here for server logic.
        default:
            // std::cout << "Server: Connection " << pInfo->m_hConn << " changed state to " << pInfo->m_info.m_eState <<
            // std::endl; // Optional: for debugging other states
            break;
        }
    }

    /// @brief Receives and processes messages from all connected clients.
    /// Iterates through each connected client and invokes the OnMessageReceived callback.
    void Server::ReceiveMessages()
    {
        if (!m_pInterface)
            return;

        for (HSteamNetConnection hConn : m_vecClients)
        {
            ISteamNetworkingMessage *pIncomingMsgs[16]; // Buffer for incoming messages for this client.
            int numMsgs = m_pInterface->ReceiveMessagesOnConnection(hConn, pIncomingMsgs, 16);
            if (numMsgs < 0)
            {
                // Error receiving messages for this connection
                // std::cerr << "Error receiving messages on connection " << hConn << std::endl; // Optional: for debugging
                continue;
            }

            for (int i = 0; i < numMsgs; ++i)
            {
                if (pIncomingMsgs[i] && pIncomingMsgs[i]->m_cbSize > 0)
                {
                    std::vector<uint8_t> msg((const char *)pIncomingMsgs[i]->m_pData,
                                             (const char *)pIncomingMsgs[i]->m_pData + pIncomingMsgs[i]->m_cbSize);

                    if (OnMessageReceived)
                    {
                        OnMessageReceived(hConn, msg);
                    }

                    pIncomingMsgs[i]->Release(); // Release the message resource.
                }
            }
        }
    }
} // namespace QNET