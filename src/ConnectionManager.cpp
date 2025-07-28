#include "quicknet/components/ConnectionManager.h"

#include <iostream>

namespace QNET
{
    /// @brief Static callback function for global connection status changes.
    /// This function is registered with SteamNetworkingSockets and dispatches events
    /// to the appropriate ConnectionManager instance by casting m_nUserData.
    /// @param pInfo Pointer to the SteamNetConnectionStatusChangedCallback_t structure.
    void ConnectionManager::OnGlobalConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
    {
        // The user data we set during connection/listen is a pointer to our ConnectionManager instance.
        ConnectionManager *manager = (ConnectionManager *)pInfo->m_info.m_nUserData;
        if (manager)
        {
            /// @brief Calls the instance-specific handler for connection status changes.
            manager->HandleConnectionStatusChanged(pInfo);
        }
    }

    /// @brief Constructor for ConnectionManager.
    /// Initializes the GameNetworkingSockets library. If initialization fails,
    /// an error message is printed to std::cerr. It also acquires the
    /// ISteamNetworkingSockets interface.
    ConnectionManager::ConnectionManager() : m_pInterface(nullptr)
    {
        // Initialize the GameNetworkingSockets library.
        SteamDatagramErrMsg errMsg;
        if (!GameNetworkingSockets_Init(nullptr, errMsg))
        {
            /// @brief Logs a fatal error if GameNetworkingSockets_Init fails.
            std::cerr << "FATAL: GameNetworkingSockets_Init failed. " << errMsg << std::endl;
        }
        else
        {
            m_pInterface = SteamNetworkingSockets();
        }
    }

    /// @brief Destructor for ConnectionManager.
    /// Shuts down the GameNetworkingSockets library.
    ConnectionManager::~ConnectionManager()
    {
        // Shutdown the library.
        GameNetworkingSockets_Kill();
    }

    /// @brief Polls for network events by running callbacks.
    /// If the network interface is not initialized, this function does nothing.
    /// This method is crucial for processing network messages and status updates.
    void ConnectionManager::Poll()
    {
        if (!m_pInterface)
            return;
        // This is the heart of the manager. It triggers all callbacks for connection
        // status changes, which are then handled by the derived classes.
        m_pInterface->RunCallbacks();
    }
}