/**
 * @file i_http_server.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief Interfaz de un componente de server HTTP
 * @version 0.1
 * @date 2026-06-16
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */
#ifndef I_HTTP_SERVER_HPP
#define I_HTTP_SERVER_HPP

#include "i_component.hpp"
#include "http_types.hpp" // Contiene HttpRequest, HttpResponse y HttpStatusCode
#include <functional>
#include <string>

using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief Interfaz para el componente servidor HTTP.
 */
class IHttpServer : public IComponent 
{
    public:
        virtual ~IHttpServer() noexcept = default;

        /**
         * @brief Inicia el servidor en un hilo en segundo plano.
         * @param port Puerto de escucha.
         * @return ComponentResult::SUCCESS si se inicia correctamente.
         */
        virtual ComponentResult start(int port) noexcept = 0;

        /**
         * @brief Detiene la ejecución del servidor y cierra los sockets.
         */
        virtual void stop() noexcept = 0;

        /**
         * @brief Registra un manejador para una ruta específica.
         */
        virtual void add_route(const std::string& method, const std::string& path, RouteHandler handler) = 0;

        // Fachadas semánticas (Sugar Syntax)
        virtual void get(const std::string& path, RouteHandler h) = 0;
        virtual void post(const std::string& path, RouteHandler h) = 0;
        virtual void put(const std::string& path, RouteHandler h) = 0;
        virtual void del(const std::string& path, RouteHandler h) = 0;
        virtual void patch(const std::string& path, RouteHandler h) = 0;
        virtual void serve_static(const std::string& route, const std::string& filepath, const std::string& content_type) = 0;
};

#endif // I_HTTP_SERVER_HPP