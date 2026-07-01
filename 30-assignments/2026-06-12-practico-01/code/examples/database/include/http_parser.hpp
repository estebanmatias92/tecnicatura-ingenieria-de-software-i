/**
 * @file http_parser.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief Utilidades de parseo de cabeceras, parámetros de consulta y decodificación.
 * @version 0.1
 * @date 2026-06-11
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 *
 */

#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include <string>
#include <unordered_map>
#include <sstream>
#include <algorithm>

class HttpParser 
{
    public:
        static std::string url_decode(const std::string& value) 
        {
            std::string result;
            result.reserve(value.length());
            for (std::size_t i = 0; i < value.length(); ++i) 
            {
                if (value[i] == '%') 
                {
                    if (i + 2 < value.length()) 
                    {
                        int hex_val;
                        std::istringstream hex_stream(value.substr(i + 1, 2));
                        if (hex_stream >> std::hex >> hex_val) 
                        {
                            result += static_cast<char>(hex_val);
                            i += 2;
                        }
                    }
                } 
                else if (value[i] == '+') { result += ' '; }
                else { result += value[i]; }
            }
            return result;
        }

        static std::unordered_map<std::string, std::string> parse_query_string(const std::string& query) {
            std::unordered_map<std::string, std::string> params;
            std::istringstream query_stream(query);
            std::string pair;
            while (std::getline(query_stream, pair, '&')) 
            {
                if (pair.empty()) continue;
                size_t eq_pos = pair.find('=');
                if (eq_pos != std::string::npos) 
                {
                    params[url_decode(pair.substr(0, eq_pos))] = url_decode(pair.substr(eq_pos + 1));
                } 
                else 
                {
                    params[url_decode(pair)] = "";
                }
            }
            return params;
        }

        /**
        * @brief Procesa el bloque de texto de cabeceras y las convierte en un mapa asociativo.
        * @param headers_block Fragmento de texto que contiene las lineas de cabeceras.
        */
        static std::unordered_map<std::string, std::string> parse_headers(const std::string& headers_block) {
            std::unordered_map<std::string, std::string> headers;
            std::istringstream stream(headers_block);
            std::string line;

            while (std::getline(stream, line) && line != "\r") 
            {
                if (!line.empty() && line.back() == '\r') 
                {
                    line.pop_back();
                }
                size_t colon_pos = line.find(':');
                if (colon_pos != std::string::npos) 
                {
                    std::string key = line.substr(0, colon_pos);
                    std::string value = line.substr(colon_pos + 1);
                    
                    // Trim basico de espacios sobrantes
                    value.erase(0, value.find_first_not_of(" "));
                    headers[key] = value;
                }
            }
            return headers;
        }
};

#endif