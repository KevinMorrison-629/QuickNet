#include "quicknet/components/HttpServer.h"

namespace QNET
{
    HttpServer::HttpServer()
    {
        m_server = std::make_unique<httplib::Server>();

        // Set up a default error handler
        m_server->set_error_handler(
            [](const Request &, Response &res)
            {
                const char *fmt = "<h1>Error %d</h1><p>%s</p>";
                char buf[BUFSIZ];
                snprintf(buf, sizeof(buf), fmt, res.status, httplib::status_message(res.status));
                res.set_content(buf, "text/html");
            });

        // Set up a default logger to print requests to the console
        m_server->set_logger(
            [](const Request &req, const Response &res)
            { std::cout << req.method << " " << req.remote_addr << " " << req.path << " -> " << res.status << std::endl; });

        // Set up CORS headers by default for web development
        m_server->set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Methods", "POST, GET, OPTIONS"},
            {"Access-Control-Allow-Headers", "Content-Type"},
        });
        m_server->Options(".*",
                          [](const Request &, Response &res)
                          {
                              res.status = 204; // No Content
                          });
    }

    HttpServer::~HttpServer() { Stop(); }

    void HttpServer::Get(const std::string &path, Handler handler)
    {
        if (m_server)
        {
            m_server->Get(path.c_str(), handler);
        }
    }

    void HttpServer::Post(const std::string &path, Handler handler)
    {
        if (m_server)
        {
            m_server->Post(path.c_str(), handler);
        }
    }

    bool HttpServer::ServeStaticFiles(const std::string &mount_point, const std::string &dir_path)
    {
        auto res = m_server->set_mount_point(mount_point.c_str(), dir_path.c_str());
        if (!res)
        {
            std::cerr << "Error: The directory '" << dir_path << "' for static files could not be found." << std::endl;
            return false;
        }
        std::cout << "Serving static files from '" << dir_path << "' at URL '" << mount_point << "'." << std::endl;
        return true;
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