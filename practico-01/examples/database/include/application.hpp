/**
 * @file application.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief Aplicación ejemplo de manejador de Base de Datos
 * @version 1
 * @date 2026-06-09
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "module_manager.hpp"
#include "i_http_server.hpp"
#include "i_database.hpp"
#include <iostream>
#include <memory>
#include <string>

class Application 
{
    private:
        ModuleManager module_manager_;
        std::shared_ptr<IHttpServer> server_;
        std::shared_ptr<IDatabase> db_;

        /**
         * @brief Serializador JSON crudo para no invadir con dependencias de terceros.
         */
        std::string serialize_to_json(const ResultSet& rs) const
        {
            std::string json = "[\n";
            for (size_t i = 0; i < rs.size(); ++i) 
            {
                json += "  {";
                size_t j = 0;
                for (const auto& [key, value] : rs[i]) 
                {
                    json += "\"" + key + "\": \"" + value + "\"";
                    if (++j < rs[i].size()) json += ", ";
                }
                json += "}";
                if (i < rs.size() - 1) json += ",\n";
            }
            json += "\n]";
            return json;
        }

        void configure_database()
        {
            // Crear tabla de usuarios si no existe
            std::string ddl = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, email TEXT NOT NULL);";
            db_->execute(ddl, {});

            // Insertar datos de prueba seguros contra inyección
            db_->execute("INSERT INTO users (name, email) SELECT ?, ? WHERE NOT EXISTS (SELECT 1 FROM users WHERE email = ?);", 
                         {"Carolina", "caro@example.com", "caro@example.com"});
        }

        void configure_routes() 
        {
            // GET: Consulta a la DB, empaquetado a JSON y envío al cliente
            server_->get("/api/users", [this](const HttpRequest& req) 
            {
                ResultSet rs;
                
                // Si viene el parametro de busqueda, filtramos
                std::string filter = req.get_query_param("search");
                if (!filter.empty())
                {
                    // Sentencia preparada segura usando '%' para el operador LIKE
                    db_->query("SELECT * FROM users WHERE name LIKE ?;", {"%" + filter + "%"}, rs);
                }
                else
                {
                    db_->query("SELECT * FROM users;", {}, rs);
                }

                std::string json_body = serialize_to_json(rs);

                return HttpResponse().set_status(HttpStatusCode::OK)
                                     .set_body(json_body, "application/json");
            });
            
            // POST: Inserción directa desde HTTP a SQLite de forma segura
            server_->post("/api/users", [this](const HttpRequest& req) 
            {
                std::string name = req.get_query_param("name");
                std::string email = req.get_query_param("email");
                
                if (name.empty() || email.empty()) 
                {
                    return HttpResponse().set_status(HttpStatusCode::BadRequest)
                                         .set_body("Faltan parametros (name, email).", "text/plain");
                }

                ComponentResult res = db_->execute("INSERT INTO users (name, email) VALUES (?, ?);", {name, email});
                
                if (res == ComponentResult::SUCCESS) 
                {
                    return HttpResponse().set_status(HttpStatusCode::Created).set_body("Usuario creado exitosamente.", "text/plain");
                } 
                else 
                {
                    return HttpResponse().set_status(HttpStatusCode::InternalServerError).set_body("Fallo de escritura DB.", "text/plain");
                }
            });
        }

    public:
        Application() = default;
        ~Application() = default;

        void initialize() 
        {
            // 1. Cargar infraestructura binaria
            module_manager_.load_module("./lib/sqlite_handler");
            module_manager_.load_module("./lib/http_server");
            
            // 2. Instanciar Polimórficamente
            db_ = module_manager_.create_instance<IDatabase>("sqlite_handler");
            server_ = module_manager_.create_instance<IHttpServer>("http_server");
            
            // 3. Inicializar Conexión Base de Datos
            if (db_->connect("./data/app_database.db") != ComponentResult::SUCCESS)
            {
                throw std::runtime_error("No se pudo conectar a la base de datos.");
            }

            configure_database();
            configure_routes();
        }

        int run() 
        {
            if (server_->start(3333) != ComponentResult::SUCCESS) 
            {
                std::cerr << "Error de Negocio: El componente de servidor no fue instanciado." << std::endl;
                return EXIT_FAILURE;
            }

            std::cout << "\n>> API REST Activa. Intenta navegar a http://localhost:3333/api/users <<\n" << std::endl;

            std::cout << "\n>> Presiona ENTER para detener el servidor de forma segura <<\n" << std::endl;
            std::cin.get(); 

            std::cout << "-> Deteniendo servicios..." << std::endl;
            server_->stop();

            return EXIT_SUCCESS;
        }
};

#endif // APPLICATION_HPP