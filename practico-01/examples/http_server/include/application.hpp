/**
 * @file application.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief Orquestador de la aplicación HTTP REST utilizando el modelo de componentes.
 * @version 1
 * @date 2026-06-09
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "module_manager.hpp"
#include "i_http_server.hpp"
#include <iostream>
#include <fstream>
#include <memory>
#include <string>

/**
 * @brief Clase Orquestadora de la lógica de negocio de la aplicación.
 * @details Absorbe la complejidad de la ejecución, aislando al main de las
 * verificaciones lógicas concretas y flujos de datos intermedios.
 */
class Application 
{
    private:
        ModuleManager module_manager_;
        std::shared_ptr<IHttpServer> server_;

        /**
         * @brief Define y registra las rutas de la API REST del servidor.
         */
        void configure_routes() 
        {
            if (!server_) return;

            // Servir archivos estáticos para la interfaz de usuario
            server_->serve_static("/", "www/index.html", "text/html");
            server_->serve_static("/style.css", "www/style.css", "text/css");
            server_->serve_static("/script.js", "www/script.js", "application/javascript");

            // ==========================================
            // CONFIGURACIÓN DE ENDPOINTS CRUD
            // ==========================================

            // GET: Saludo con parámetros de consulta
            server_->add_route("GET", "/api/saludar", [](const HttpRequest& req) 
            {
                std::string nombre = req.get_query_param("nombre");
                if (nombre.empty()) 
                {
                    nombre = "Invitado Anónimo";
                }

                HttpResponse res;
                return res.set_status(HttpStatusCode::OK)
                          .set_body("Servidor C++ dice: ¡Hola, " + nombre + "!", "text/plain");
            });

            // GET: Operación de Lectura
            server_->get("/api/recurso", [](const HttpRequest&) 
            {
                return HttpResponse().set_status(HttpStatusCode::OK)
                                     .set_body("{ \"status\": \"Lectura exitosa (GET)\" }", "application/json");
            });

            // POST: Operación de Creación / Subida de Archivos Binarios
            server_->post("/api/upload", [](const HttpRequest& req) 
            {
                std::string filename = req.get_query_param("archivo");
                if (filename.empty()) filename = "upload_anonimo.bin";

                if (req.body.empty()) 
                {
                    return HttpResponse().set_status(HttpStatusCode::BadRequest)
                                         .set_body("Error: El cuerpo del archivo está vacío.", "text/plain");
                }

                // Escritura binaria segura RAII en disco
                std::ofstream output_file(filename, std::ios::out | std::ios::binary);
                if (!output_file) 
                {
                    return HttpResponse().set_status(HttpStatusCode::InternalServerError)
                                         .set_body("Error interno escribiendo en disco.", "text/plain");
                }

                output_file.write(req.body.data(), req.body.size());
                
                return HttpResponse().set_status(HttpStatusCode::Created)
                                     .set_body("Archivo '" + filename + "' subido correctamente. Tamaño: " + 
                                               std::to_string(req.body.size()) + " bytes.", "text/plain");
            });

            // PUT: Operación de Actualización Completa
            server_->add_route("PUT", "/api/recurso", [](const HttpRequest&) 
            {
                return HttpResponse().set_status(HttpStatusCode::OK)
                                     .set_body("Recurso modificado por completo (PUT).", "text/plain");
            });

            // PATCH: Operación de Actualización Parcial
            server_->add_route("PATCH", "/api/recurso", [](const HttpRequest&) 
            {
                return HttpResponse().set_status(HttpStatusCode::OK)
                                     .set_body("Propiedad modificada parcialmente (PATCH).", "text/plain");
            });

            // DELETE: Operación de Borrado (Sintaxis C-API limpia)
            server_->add_route("DELETE", "/api/recurso", [](const HttpRequest&) 
            {
                return HttpResponse().set_status(HttpStatusCode::NoContent);
            });
        }

    public:
        Application() = default;
        ~Application() = default;

        /**
         * @brief Inicializa las dependencias e infraestructura del sistema.
         */
        void initialize() 
        {
            // Carga el componente dinámico del servidor web (ej: libhttp_server.so o http_server.dll)
            module_manager_.load_module("./lib/http_server");
            
            // Instancia el componente a través de su interfaz abstracta
            server_ = module_manager_.create_instance<IHttpServer>("http_server");
            
            // Configura los endpoints de nuestro servicio
            configure_routes();
        }

        /**
         * @brief Ejecuta el ciclo de vida principal o los comandos de negocio.
         * @return Código de salida del proceso (EXIT_SUCCESS o EXIT_FAILURE).
         */
        int run() 
        {
            if (!server_) 
            {
                std::cerr << "Error de Negocio: El componente de servidor no fue instanciado." << std::endl;
                return EXIT_FAILURE;
            }

            // Iniciamos el servidor en el puerto especificado de forma asíncrona
            constexpr int port = 3333;
            ComponentResult result = server_->start(port);
            
            if (result != ComponentResult::SUCCESS) 
            {
                std::cerr << "Error de Negocio: No se pudo arrancar el servidor en el puerto " << port << std::endl;
                return EXIT_FAILURE;
            }

            std::cout << "\n>> Presiona ENTER para detener el servidor de forma segura <<\n" << std::endl;
            std::cin.get(); 

            std::cout << "-> Deteniendo servicios..." << std::endl;
            server_->stop();

            return EXIT_SUCCESS;
        }
};

#endif // APPLICATION_HPP