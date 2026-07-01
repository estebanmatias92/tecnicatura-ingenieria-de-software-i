/**
 * @file application.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief Intraned Application: Servidor de Contenidos Educativos utilizando componentes.
 * @version 1
 * @date 2026-05-19
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */
#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "module_manager.hpp"
#include "i_http_server.hpp"
#include "i_database.hpp"
#include "json.hpp" // Header-only seguro
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sys/stat.h>

#include <cstdio> // Para dar soporte a la función std::remove()

using json = nlohmann::json;

class Application 
{
    private:
        ModuleManager module_manager_;
        std::shared_ptr<IHttpServer> server_;
        std::shared_ptr<IDatabase> db_;

        const std::string ADMIN_TOKEN = "educacion_libre_2026";
        const std::string UPLOAD_DIR = "./uploads/";

        void asegurar_directorio(const std::string& path) 
        {
            struct stat info;
            if (stat(path.c_str(), &info) != 0) 
            {
                mkdir(path.c_str(), 0777);
            }
        }

        void configure_database()
        {
            // Migración desde JSON a esquema relacional (Sustituye a fileHelper.cpp)
            std::string ddl = "CREATE TABLE IF NOT EXISTS recursos ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "titulo TEXT NOT NULL, "
                              "autor TEXT NOT NULL, "
                              "tema TEXT NOT NULL, "
                              "filename TEXT NOT NULL);";
            db_->execute(ddl, {});
        }

        void configure_routes() 
        {
            // --- 1. ENTREGA DE ACTIVOS ESTÁTICOS (Frontend) ---
            server_->serve_static("/", "www/index.html", "text/html");
            server_->serve_static("/admin.html", "www/admin.html", "text/html");
            server_->serve_static("/login.html", "www/login.html", "text/html");
            server_->serve_static("/css/style.css", "www/css/style.css", "text/css");
            server_->serve_static("/js/admin.js", "www/js/admin.js", "application/javascript");
            server_->serve_static("/js/login.js", "www/js/login.js", "application/javascript");
            server_->serve_static("/js/public.js", "www/js/public.js", "application/javascript");
            server_->serve_static("/js/utils.js", "www/js/utils.js", "application/javascript");

            // --- 2. API: AUTENTICACIÓN ---
            server_->post("/api/login", [this](const HttpRequest& req) 
            {
                try 
                {
                    std::string body_str(req.body.begin(), req.body.end());
                    auto j_input = json::parse(body_str);
                    
                    if (j_input.value("password", "") == "admin123") 
                    {
                        return HttpResponse().set_status(HttpStatusCode::OK)
                                             .set_body("{\"token\": \"" + ADMIN_TOKEN + "\"}", "application/json");
                    } 
                    return HttpResponse().set_status(HttpStatusCode::Forbidden).set_body("{\"error\": \"No autorizado\"}", "application/json");
                } 
                catch (...) 
                {
                    return HttpResponse().set_status(HttpStatusCode::BadRequest).set_body("{\"error\": \"JSON invalido\"}", "application/json");
                }
            });

            // --- 3. API: LECTURA DE RECURSOS (Actualizado con ID) ---
            server_->get("/api/contenidos", [this](const HttpRequest&) 
            {
                ResultSet rs;
                // Agregamos 'id' a la proyección de la consulta
                db_->query("SELECT id, titulo, autor, tema, filename as file FROM recursos;", {}, rs);

                json db_json = {{"contenidos", json::array()}};
                for (const auto& row : rs) 
                {
                    db_json["contenidos"].push_back({
                        {"id", row.at("id")}, // Exponemos el ID para el frontend
                        {"titulo", row.at("titulo")},
                        {"autor", row.at("autor")},
                        {"tema", row.at("tema")},
                        {"file", row.at("file")}
                    });
                }
                
                return HttpResponse().set_status(HttpStatusCode::OK).set_body(db_json.dump(), "application/json");
            });

            // --- 4. API: ESCRITURA Y ALMACENAMIENTO FÍSICO ---
            // Nota: Se espera que el cliente envíe los metadatos por URL y el archivo crudo en el body.
            server_->post("/api/upload", [this](const HttpRequest& req) 
            {
                if (req.get_header("Authorization") != ADMIN_TOKEN) 
                {
                    return HttpResponse().set_status(HttpStatusCode::Forbidden).set_body("{\"error\": \"Token invalido\"}", "application/json");
                }

                std::string titulo = req.get_query_param("titulo");
                std::string autor = req.get_query_param("autor");
                std::string tema = req.get_query_param("tema");
                std::string filename = req.get_query_param("filename");

                if (filename.empty() || req.body.empty()) 
                {
                    return HttpResponse().set_status(HttpStatusCode::BadRequest).set_body("{\"error\": \"Faltan datos o archivo vacio\"}", "application/json");
                }

                // 4.1. Escritura en disco (RAII)
                std::string path = UPLOAD_DIR + filename;
                std::ofstream ofs(path, std::ios::binary);
                
                if (!ofs) 
                {
                    return HttpResponse().set_status(HttpStatusCode::InternalServerError).set_body("{\"error\": \"Error de E/S en disco\"}", "application/json");
                }
                
                ofs.write(req.body.data(), req.body.size());
                ofs.close();

                // 4.2. Registro en SQLite
                std::string sql = "INSERT INTO recursos (titulo, autor, tema, filename) VALUES (?, ?, ?, ?);";
                if (db_->execute(sql, {titulo, autor, tema, filename}) == ComponentResult::SUCCESS)
                {
                    return HttpResponse().set_status(HttpStatusCode::Created).set_body("{\"status\": \"success\"}", "application/json");
                }

                return HttpResponse().set_status(HttpStatusCode::InternalServerError).set_body("{\"error\": \"Error en base de datos\"}", "application/json");
            });

            // --- 4.5 API: BORRADO DE RECURSOS (Físico y Lógico) ---
            server_->del("/api/contenidos", [this](const HttpRequest& req) 
            {
                // 1. Control de Autorización
                if (req.get_header("Authorization") != ADMIN_TOKEN) 
                {
                    return HttpResponse().set_status(HttpStatusCode::Forbidden).set_body("{\"error\": \"Token invalido\"}", "application/json");
                }

                std::string id_str = req.get_query_param("id");
                if (id_str.empty()) 
                {
                    return HttpResponse().set_status(HttpStatusCode::BadRequest).set_body("{\"error\": \"Se requiere el ID del recurso\"}", "application/json");
                }

                // 2. Extracción segura del nombre de archivo desde la DB (Previene Path Traversal)
                ResultSet rs;
                db_->query("SELECT filename FROM recursos WHERE id = ?;", {id_str}, rs);
                if (rs.empty()) 
                {
                    return HttpResponse().set_status(HttpStatusCode::NotFound).set_body("{\"error\": \"Recurso no encontrado\"}", "application/json");
                }

                std::string filename = rs[0].at("filename");
                std::string filepath = UPLOAD_DIR + filename;

                // 3. Borrado físico del disco (Si falla, se ignora, el archivo pudo haber sido borrado manualmente)
                std::remove(filepath.c_str());

                // 4. Borrado lógico de la base de datos
                if (db_->execute("DELETE FROM recursos WHERE id = ?;", {id_str}) == ComponentResult::SUCCESS)
                {
                    return HttpResponse().set_status(HttpStatusCode::OK).set_body("{\"status\": \"deleted\"}", "application/json");
                }

                return HttpResponse().set_status(HttpStatusCode::InternalServerError).set_body("{\"error\": \"Error al borrar registro en DB\"}", "application/json");
            });

            // --- 5. ENTREGA DINÁMICA DE ARCHIVOS (Zero-Copy y Seguridad) ---
            server_->get("/recursos/*", [this](const HttpRequest& req) 
            {
                std::string prefix = "/recursos/";
                std::string filename = req.path.substr(prefix.length());

                // PREVENCIÓN CRÍTICA DE PATH TRAVERSAL
                // Bloquea cualquier intento de retroceder directorios ("..") o inyectar subrutas ("/")
                if (filename.empty() || filename.find("..") != std::string::npos || filename.find('/') != std::string::npos) 
                {
                    return HttpResponse().set_status(HttpStatusCode::Forbidden).set_body("Acceso denegado.", "text/plain");
                }

                std::string filepath = UPLOAD_DIR + filename;
                
                // Obtener información del archivo desde el sistema operativo sin cargarlo en memoria
                struct stat file_stat;
                if (stat(filepath.c_str(), &file_stat) != 0) 
                {
                    return HttpResponse().set_status(HttpStatusCode::NotFound).set_body("Archivo no encontrado en la isla.", "text/plain");
                }

                // Determinación del tipo MIME fundamental para el cliente
                std::string content_type = "application/octet-stream";
                if (filename.find(".pdf") != std::string::npos) content_type = "application/pdf";
                else if (filename.find(".mp3") != std::string::npos) content_type = "audio/mpeg";
                else if (filename.find(".png") != std::string::npos) content_type = "image/png";
                else if (filename.find(".jpg") != std::string::npos || filename.find(".jpeg") != std::string::npos) content_type = "image/jpeg";
                else if (filename.find(".epub") != std::string::npos) content_type = "application/epub+zip";

                // Se instruye a la infraestructura a realizar una transferencia delegada
                return HttpResponse().set_status(HttpStatusCode::OK).set_file(filepath, content_type, file_stat.st_size);
            });
        }

    public:
        Application() = default;
        ~Application() = default;

        void initialize() 
        {
            asegurar_directorio(UPLOAD_DIR);
            
            module_manager_.load_module("./lib/sqlite_handler");
            module_manager_.load_module("./lib/http_server");
            
            db_ = module_manager_.create_instance<IDatabase>("sqlite_handler");
            server_ = module_manager_.create_instance<IHttpServer>("http_server");
            
            if (db_->connect("./data/intraned.db") != ComponentResult::SUCCESS)
            {
                throw std::runtime_error("Fallo critico: No se pudo enlazar SQLite.");
            }

            configure_database();
            configure_routes();
        }

        int run() 
        {
            if (server_->start(8080) != ComponentResult::SUCCESS) 
            {
                return EXIT_FAILURE;
            }

            std::cout << "Intraned operando en Isla Local (http://localhost:8080)" << std::endl;
            std::cin.get(); 

            server_->stop();
            return EXIT_SUCCESS;
        }
};

#endif