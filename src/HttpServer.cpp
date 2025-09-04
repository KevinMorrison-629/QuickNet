#include "quicknet/components/HttpServer.h"

namespace QNET
{
    HttpServer::HttpServer()
    {
        m_server = std::make_unique<httplib::Server>();

        // Set up a default error handler
        m_server->set_error_handler(
            [](const httplib::Request &, httplib::Response &res)
            {
                const char *fmt = "<h1>Error %d</h1><p>%s</p>";
                char buf[BUFSIZ];
                snprintf(buf, sizeof(buf), fmt, res.status, httplib::status_message(res.status));
                res.set_content(buf, "text/html");
            });

        // Set up a default logger to print requests to the console
        m_server->set_logger(
            [](const httplib::Request &req, const httplib::Response &res)
            { std::cout << req.method << " " << req.remote_addr << " " << req.path << " -> " << res.status << std::endl; });
    }

    HttpServer::~HttpServer() { Stop(); }

    void HttpServer::Get(const std::string &path, httplib::Server::Handler handler)
    {
        if (m_server)
        {
            m_server->Get(path.c_str(), handler);
        }
    }

    void HttpServer::Post(const std::string &path, httplib::Server::Handler handler)
    {
        if (m_server)
        {
            m_server->Post(path.c_str(), handler);
        }
    }

    void HttpServer::Run(uint16_t port)
    {
        std::cout << "HTTP Server starting on port " << port << "..." << std::endl;
        if (!m_server->listen("0.0.0.0", port))
        {
            log_message("Failed to bind to port " + std::to_string(port));
            throw std::runtime_error("Server could not listen on the specified port.");
        }
    }

    void HttpServer::Stop()
    {
        if (m_server && m_server->is_running())
        {
            m_server->stop();
            std::cout << "HTTP Server stopped." << std::endl;
        }
    }

    void HttpServer::log_message(const std::string &msg) { std::cerr << "ERROR: " << msg << std::endl; }
} // namespace QNET