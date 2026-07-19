/**
 * @file http_server_component.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief HTTP Server Component Example
 * @version 0.1
 * @date 2026-06-11
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */
#ifndef HTTP_SERVER_COMPONENT_HPP
#define HTTP_SERVER_COMPONENT_HPP

#include "i_http_server.hpp"
#include "http_parser.hpp"
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>

/**
 * @brief 
 * cabeceras POSIX requeridas e interceptar 
 * el flujo de la respuesta para ejecutar 
 * sendfile si el objeto HttpResponse 
 * contiene una ruta de archivo.
 */
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>

class SocketRAII 
{
    private:
        int fd_;
    public:
        explicit SocketRAII(int file_descriptor) : fd_(file_descriptor) {}
        ~SocketRAII() { if (fd_ >= 0) close(fd_); }
        int get() const { return fd_; }
        void invalidate() { fd_ = -1; } // Evita cierre doble si se cierra manualmente
};

/**
 * @brief Implementación concreta del servidor HTTP.
 */
class HttpServerComponent : public IHttpServer 
{
    private:
        std::unordered_map<std::string, std::unordered_map<std::string, RouteHandler>> routes_;
        std::mutex console_mutex_;
        
        // Control de concurrencia y ciclo de vida
        std::atomic<bool> is_running_{false};
        std::thread server_thread_;
        int server_fd_{-1};

        void process_client(int client_fd);
        void listen_and_serve(int port);

    public:
        HttpServerComponent() = default;
        ~HttpServerComponent() noexcept override;

        ComponentResult start(int port) noexcept override;
        void stop() noexcept override;
        
        void add_route(const std::string& method, const std::string& path, RouteHandler handler) override;
        void get(const std::string& path, RouteHandler h) override { add_route("GET", path, h); }
        void post(const std::string& path, RouteHandler h) override { add_route("POST", path, h); }
        void put(const std::string& path, RouteHandler h) override { add_route("PUT", path, h); }
        void del(const std::string& path, RouteHandler h) override { add_route("DELETE", path, h); }
        void patch(const std::string& path, RouteHandler h) override { add_route("PATCH", path, h); }
        void serve_static(const std::string& route, const std::string& filepath, const std::string& content_type) override;
};

#endif // HTTP_SERVER_COMPONENT_HPP