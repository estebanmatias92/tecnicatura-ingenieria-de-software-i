/**
 * @file SharedLibrary.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief C Plus Plus Component Model
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
        void* handle;
        std::string path;

    public:
        /**
        * @brief Constructor que intenta cargar la biblioteca.
        * @param libPath Ruta de la biblioteca (sin la extensión del SO).
        * @throws std::runtime_error Si la biblioteca no se puede cargar.
        */
        explicit SharedLibrary(const std::string& libPath) : path(libPath + LIB_EXTENSION) 
        {
            #ifdef _WIN32
                handle = LoadLibrary(path.c_str());
            #else
                handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
            #endif
                if (!handle) 
                {
                    throw std::runtime_error("Error al cargar la biblioteca: " + path);
                }
        }

        /**
        * @brief Destructor que libera la biblioteca de la memoria.
        */
        ~SharedLibrary()
        {
            if (handle) 
            {
                #ifdef _WIN32
                    FreeLibrary((HINSTANCE)handle);
                #else
                    dlclose(handle);
                #endif
            }
        }

        // Prevenimos la copia para evitar cerrar el handle accidentalmente
        SharedLibrary(const SharedLibrary&) = delete;
        SharedLibrary& operator=(const SharedLibrary&) = delete;

        /**
        * @brief Obtiene un símbolo (función) exportado por la biblioteca.
        * @param symbolName Nombre de la función exportada.
        * @return Puntero a la función, o nullptr si no se encuentra.
        */
        void* getSymbol(const std::string& symbolName) 
        {
            #ifdef _WIN32
                return (void*)GetProcAddress((HINSTANCE)handle, symbolName.c_str());
            #else
                return dlsym(handle, symbolName.c_str());
            #endif
        }
};

#endif // SHARED_LIBRARY_HPP