/**
 * @file shared_library.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief C Plus Plus Component Model: manejador RAII de Bibliotecas Compartidas
 * @version 1
 * @date 2026-05-19
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */
 
#ifndef SHARED_LIBRARY_HPP
#define SHARED_LIBRARY_HPP

#include <string>
#include <stdexcept>

// Detección de plataforma
#ifdef __unix__
    #include <dlfcn.h>
    const std::string LIB_EXTENSION = ".so";
#elif defined(_WIN32) || defined(WIN32)
    #include <windows.h>
    const std::string LIB_EXTENSION = ".dll";
#elif __APPLE__
    #include <dlfcn.h>
    const std::string LIB_EXTENSION = ".dylib";
#else
    #error "Plataforma no soportada"
#endif

/**
 * @brief Clase RAII para gestionar el ciclo de vida de una biblioteca dinámica.
 */
class SharedLibrary 
{
    private:
        void* handle_;
        std::string path_;

    public:
        /**
        * @brief Constructor que intenta cargar la biblioteca.
        * @param lib_path Ruta de la biblioteca (sin la extensión del SO).
        * @throws std::runtime_error Si la biblioteca no se puede cargar.
        */
        explicit SharedLibrary(const std::string& lib_path) : path_(lib_path + LIB_EXTENSION) 
        {
            #ifdef _WIN32
                handle_ = LoadLibrary(path_.c_str());
            #else
                // Limpiamos errores previos antes de invocar dlopen
                dlerror(); 
                handle_ = dlopen(path_.c_str(), RTLD_NOW | RTLD_LOCAL);
            #endif

                if (!handle_) 
                {
                    std::string error_details = "Error al cargar la biblioteca: " + path_;
                    #ifndef _WIN32
                        /** 
                        * @details dlopen proporciona la función dlerror() para obtener la cadena 
                        * exacta de texto que explica detalladamente por qué falló la operación 
                        * (por ejemplo: si el archivo no existe, si faltan permisos, o si hay un 
                        * símbolo no resuelto).La terminal indica explícitamente el símbolo faltante
                        * de hilos, evitando tener que adivinar la causa del puntero nulo.
                        */
                        const char* dl_err_str = dlerror();
                        if (dl_err_str) 
                        {
                            error_details += " (Detalle del SO: " + std::string(dl_err_str) + ")";
                        }
                    #endif
                    throw std::runtime_error(error_details);
                }
        }

        /**
        * @brief Destructor que libera la biblioteca de la memoria.
        */
        ~SharedLibrary()
        {
            if (handle_) 
            {
                #ifdef _WIN32
                    FreeLibrary((HINSTANCE)handle_);
                #else
                    dlclose(handle_);
                #endif
            }
        }

        // Prevenimos la copia para evitar cerrar el handle_ accidentalmente
        SharedLibrary(const SharedLibrary&) = delete;
        SharedLibrary& operator=(const SharedLibrary&) = delete;

        /**
        * @brief Obtiene un símbolo (función) exportado por la biblioteca.
        * @param symbol_name Nombre de la función exportada.
        * @return Puntero a la función, o nullptr si no se encuentra.
        */
        void* get_symbol(const std::string& symbol_name) 
        {
            #ifdef _WIN32
                return (void*)GetProcAddress((HINSTANCE)handle_, symbol_name.c_str());
            #else
                return dlsym(handle_, symbol_name.c_str());
            #endif
        }
};

#endif // SHARED_LIBRARY_HPP