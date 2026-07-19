/**
 * @file http_server_component.cpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief Implementación concreta del servidor HTTP con soporte Zero-Copy.
 * @version 0.1
 * @date 2026-06-16
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */
#include "http_server_component.hpp"

HttpServerComponent::~HttpServerComponent() noexcept 
{
    stop();
}

ComponentResult HttpServerComponent::start(int port) noexcept 
{
    if (is_running_) return ComponentResult::ERROR_INTERNAL;

    try 
    {
        is_running_ = true;
        // Lanzamos el servidor en un hilo independiente para liberar al Host
        server_thread_ = std::thread(&HttpServerComponent::listen_and_serve, this, port);
        return ComponentResult::SUCCESS;
    } 
    catch (...) 
    {
        is_running_ = false;
        return ComponentResult::ERROR_INTERNAL;
    }
}

void HttpServerComponent::stop() noexcept 
{
    if (is_running_) 
    {
        is_running_ = false;
        if (server_fd_ >= 0) 
        {
            // Cerrar el socket principal fuerza a accept() a desbloquearse y retornar error
            shutdown(server_fd_, SHUT_RDWR);
            close(server_fd_);
            server_fd_ = -1;
        }
        if (server_thread_.joinable()) 
        {
            server_thread_.join();
        }
    }
}

void HttpServerComponent::add_route(const std::string& method, const std::string& path, RouteHandler handler) 
{
    routes_[method][path] = std::move(handler);
}

void HttpServerComponent::serve_static(const std::string& route, const std::string& filepath, const std::string& content_type) 
{
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file) 
    {
        get(route, [](const HttpRequest&) { return HttpResponse().set_status(HttpStatusCode::NotFound); });
        return;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    get(route, [content, content_type](const HttpRequest&) 
    {
        return HttpResponse().set_status(HttpStatusCode::OK).set_body(content, content_type);
    });
}

void HttpServerComponent::listen_and_serve(int port) 
{
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == -1) return;

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) return;
    if (listen(server_fd_, 10) < 0) return;

    std::cout << "[HttpServer] Iniciado en puerto " << port << std::endl;

    while (is_running_) 
    {
        struct sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        
        // accept se bloqueará aquí hasta recibir una conexión o hasta que stop() cierre server_fd_
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &len);
        
        if (!is_running_) break; // Salida segura si se invocó stop()

        if (client_fd >= 0) 
        {
            std::thread(&HttpServerComponent::process_client, this, client_fd).detach();
        }
    }
    std::cout << "[HttpServer] Detenido de forma segura." << std::endl;
}

void HttpServerComponent::process_client(int client_fd) 
{
    SocketRAII client_socket(client_fd);
    std::vector<char> stream_buffer;
    char request_chunk[1024];
    ssize_t bytes_read = 0;
    size_t header_end_pos = std::string::npos;

    /// ETAPA 1: Leer del socket hasta encontrar el fin de las cabeceras (\r\n\r\n)
    while ((bytes_read = read(client_socket.get(), request_chunk, sizeof(request_chunk))) > 0) 
    {
        stream_buffer.insert(stream_buffer.end(), request_chunk, request_chunk + bytes_read);
        
        std::string current_str(stream_buffer.begin(), stream_buffer.end());
        header_end_pos = current_str.find("\r\n\r\n");
        if (header_end_pos != std::string::npos) 
        {
            break;
        }
    }

    if (header_end_pos == std::string::npos) return; // Peticion invalida

    /// Extraer e interpretar la linea de control y cabeceras
    std::string raw_content(stream_buffer.begin(), stream_buffer.end());
    std::string headers_part = raw_content.substr(0, header_end_pos);
    
    std::istringstream header_stream(headers_part);
    HttpRequest req;
    std::string raw_uri;
    header_stream >> req.method >> raw_uri;

    // Parsear cabeceras completas
    std::string remaining_headers;
    std::getline(header_stream, remaining_headers); // Consumir resto de la primera linea
    req.headers = HttpParser::parse_headers(headers_part.substr(headers_part.find("\r\n") + 2));

    // Parser de URI para Query String
    size_t q_pos = raw_uri.find('?');
    if (q_pos != std::string::npos) 
    {
        req.path = raw_uri.substr(0, q_pos);
        req.query_params = HttpParser::parse_query_string(raw_uri.substr(q_pos + 1));
    } 
    else 
    {
        req.path = raw_uri;
    }

    /// ETAPA 2: Leer el cuerpo (Body / Archivo) basandose estrictamente en Content-Length
    std::string content_length_str = req.get_header("Content-Length");
    size_t content_length = content_length_str.empty() ? 0 : std::stoul(content_length_str);

    size_t bytes_already_read = stream_buffer.size() - (header_end_pos + 4);
    
    // Inicializar el cuerpo con los bytes que pasaron de largo en la primera lectura
    req.body.insert(req.body.end(), stream_buffer.begin() + header_end_pos + 4, stream_buffer.end());

    // Continuar leyendo del socket si falta contenido por recibir
    if (bytes_already_read < content_length) 
    {
        size_t missing_bytes = content_length - bytes_already_read;
        std::vector<char> body_chunk(1024);
        while (missing_bytes > 0 && (bytes_read = read(client_socket.get(), body_chunk.data(), std::min(body_chunk.size(), missing_bytes))) > 0) 
        {
            req.body.insert(req.body.end(), body_chunk.begin(), body_chunk.begin() + bytes_read);
            missing_bytes -= bytes_read;
        }
    }

    /// ETAPA 3: Enrutamiento semántico por Verbo y Path (Con soporte para Comodines)
    HttpResponse res;
    auto verb_it = routes_.find(req.method);
    if (verb_it != routes_.end()) 
    {
        auto path_it = verb_it->second.find(req.path);
        if (path_it != verb_it->second.end()) 
        {
            res = path_it->second(req); // Ejecución del handler especializado (Coincidencia Exacta)
        } 
        else 
        {
            // Fallback para evaluación de prefijos (Rutas dinámicas / Comodines)
            bool route_found = false;
            for (const auto& [registered_path, handler] : verb_it->second) 
            {
                if (!registered_path.empty() && registered_path.back() == '*') 
                {
                    // Extraer el prefijo (Ej: "/recursos/*" -> "/recursos/")
                    std::string prefix = registered_path.substr(0, registered_path.length() - 1);
                    
                    // Si la ruta solicitada comienza exactamente con este prefijo
                    if (req.path.find(prefix) == 0) 
                    {
                        res = handler(req);
                        route_found = true;
                        break;
                    }
                }
            }

            if (!route_found) 
            {
                res.set_status(HttpStatusCode::NotFound).set_body("Ruta no encontrada.", "text/plain");
            }
        }
    } 
    else 
    {
        res.set_status(HttpStatusCode::MethodNotAllowed).set_body("Metodo HTTP no soportado.", "text/plain");
    }

    /// ETAPA 4: Envío de Cabeceras (y cuerpo si está en memoria)
    std::string response_str = res.to_string();
    send(client_socket.get(), response_str.c_str(), response_str.length(), 0);

    /// ETAPA 5: Envío de Cuerpo Externo (Zero-Copy Transfer)
    if (!res.get_file_path().empty()) 
    {
        // Ruta de alta eficiencia. Se delega la transferencia al kernel.
        int file_fd = open(res.get_file_path().c_str(), O_RDONLY);
        if (file_fd >= 0) 
        {
            struct stat stat_buf;
            fstat(file_fd, &stat_buf);
            off_t offset = 0;
            
            ssize_t bytes_sent = sendfile(client_socket.get(), file_fd, &offset, stat_buf.st_size);
            
            if (bytes_sent == -1) 
            {
                std::cerr << "[Error] Fallo en sendfile para: " << res.get_file_path() << "\n";
            }
            close(file_fd);
        }
    }

    /// ETAPA 6: Logueo concurrente seguro de la transacción
    std::lock_guard<std::mutex> lock(console_mutex_);
    std::cout << "[" << req.method << "] " << req.path 
              << " -> " << get_status_text(res.get_status()) 
              << (res.get_file_path().empty() ? "" : " [Zero-Copy]") << "\n";
}

// --- EXPORTACIÓN DE C-API ---

extern "C" int get_api_version() noexcept 
{
    return CURRENT_API_VERSION;
}

extern "C" IComponent* create_component() noexcept 
{
    try 
    {
        return new HttpServerComponent();
    }
    catch (...) 
    {
        return nullptr;
    }
}

extern "C" void destroy_component(IComponent* instance) noexcept 
{
    delete instance;
}