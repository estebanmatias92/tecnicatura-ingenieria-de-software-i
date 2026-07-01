/**
 * @file http_types.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief Extension del sistema de tipos HTTP para soportar multiples verbos y datos binarios.
 * @version 0.1
 * @date 2026-06-11
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 *
 */

#ifndef HTTP_TYPES_HPP
#define HTTP_TYPES_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

/**
 * @enum HttpStatusCode
 * @brief Codigos de estado extendidos para operaciones RESTful CRUD.
 */
enum class HttpStatusCode 
{
    OK = 200,
    Created = 201,
    NoContent = 204,
    BadRequest = 400,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    InternalServerError = 500
};

inline std::string get_status_text(HttpStatusCode code) 
{
    switch (code) 
    {
        case HttpStatusCode::OK: return "200 OK";
        case HttpStatusCode::Created: return "201 Created";
        case HttpStatusCode::NoContent: return "204 No Content";
        case HttpStatusCode::BadRequest: return "400 Bad Request";
        case HttpStatusCode::Forbidden: return "403 Forbidden";
        case HttpStatusCode::NotFound: return "404 Not Found";
        case HttpStatusCode::MethodNotAllowed: return "405 Method Not Allowed";
        default: return "500 Internal Server Error";
    }
}

/**
 * @struct HttpRequest
 * @brief DTO evolutivo que soporta multiples verbos y payloads binarios (archivos).
 */
struct HttpRequest 
{
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> query_params;
    std::vector<char> body; ///< binario-safe para almacenamiento de archivos

    /**
     * @brief Recupera de forma segura un parametro de la URL.
     */
    std::string get_query_param(const std::string& key) const 
    {
        auto it = query_params.find(key);
        return (it != query_params.end()) ? it->second : "";
    }

    /**
     * @brief Recupera de forma segura el valor de una cabecera HTTP (case-insensitive idealmente).
     */
    std::string get_header(const std::string& key) const 
    {
        auto it = headers.find(key);
        return (it != headers.end()) ? it->second : "";
    }
};

/**
 * @class HttpResponse
 * @brief Constructor de respuestas HTTP con soporte de encadenamiento (Builder Pattern).
 */
class HttpResponse 
{
    private:
        HttpStatusCode status_code_{HttpStatusCode::OK};
        std::unordered_map<std::string, std::string> headers_;
        std::string body_;
        std::string file_path_;

    public:
        HttpResponse& set_status(HttpStatusCode code) { status_code_ = code; return *this; }
        HttpResponse& set_header(const std::string& key, const std::string& value) { headers_[key] = value; return *this; }
        
        HttpResponse& set_body(const std::string& content, const std::string& content_type = "text/plain") 
        {
            body_ = content;
            set_header("Content-Type", content_type + "; charset=UTF-8");
            set_header("Content-Length", std::to_string(body_.length()));
            return *this;
        }

        std::string to_string() const 
        {
            std::ostringstream response_stream;
            response_stream << "HTTP/1.1 " << get_status_text(status_code_) << "\r\n";
            for (const auto& [key, value] : headers_) 
            {
                response_stream << key << ": " << value << "\r\n";
            }
            response_stream << "Connection: close\r\n\r\n" << body_;
            return response_stream.str();
        }

        /**
        * @brief Get the status object
        * Retorno por valor: HttpStatusCode es un enum class. 
        * A nivel de compilador, un enum class se trata como un tipo de dato 
        * primitivo integral (por defecto, un int). Retornar este tipo de datos 
        * por valor es la operación más rápida posible, ya que el dato se copia 
        * directamente en los registros del procesador.
        *
        * Garantía de inmutabilidad (const): Al colocar const al final de la firma 
        * del método, le garantizas al compilador que invocar este getter no modificará
        * el estado interno de la instancia de HttpResponse. Esto permite que puedas 
        * llamar a este método incluso si el objeto se pasa como una referencia constante
        * en otras partes del código (const HttpResponse&).
        * @return HttpStatusCode 
        */
        HttpStatusCode get_status() const 
        {
            return status_code_;
        }
        
        HttpResponse& set_file(const std::string& filepath, const std::string& content_type, size_t file_size) 
        {
            file_path_ = filepath;
            set_header("Content-Type", content_type);
            set_header("Content-Length", std::to_string(file_size));
            return *this;
        }

        const std::string& get_file_path() const 
        {
            return file_path_;
        }
};

#endif