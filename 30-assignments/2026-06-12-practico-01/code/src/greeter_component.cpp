/**
 * @file greeter_component.cpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief C Plus Plus Component Model
 * @version 1
 * @date 2026-05-19
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */

#include "../include/i_greeter.hpp"
#include <string>
#include <cstring>
#include <exception>

/**
 * @brief Implementación concreta que respeta el límite sin excepciones.
 */
class GreeterComponent : public IGreeter 
{
    private:
        std::string prefix;

    public:
        GreeterComponent() : prefix("Hello, ") {}
        ~GreeterComponent() noexcept override = default;

        /**
         * @brief Implementación envuelta en try-catch.
         * @details Atrapa cualquier excepción C++ (como std::bad_alloc en caso
         * de falta de memoria al concatenar std::string) impidiendo que cruce el ABI.
         */
        ComponentResult greet(const char* name, char* out_buffer, size_t buffer_size) noexcept override 
        {
            if (!name || !out_buffer || buffer_size == 0) 
            {
                return ComponentResult::ERROR_INVALID_ARGUMENT;
            }

            try 
            {
                std::string result = prefix + name + "!";
                
                if (result.length() >= buffer_size) 
                {
                    return ComponentResult::ERROR_INVALID_ARGUMENT;
                }

                strncpy(out_buffer, result.c_str(), buffer_size - 1);
                out_buffer[buffer_size - 1] = '\0'; 
                
                return ComponentResult::SUCCESS;
            }
            catch (...) 
            {
                // Un bloque catch-all asegura que NINGUNA excepción escape hacia el Host.
                return ComponentResult::ERROR_INTERNAL;
            }
        }
};

// --- EXPORTACIÓN DE C-API ---

/**
 * @brief Expone la versión con la que fue compilada esta biblioteca compartida.
 */
extern "C" int get_api_version() noexcept 
{
    return CURRENT_API_VERSION;
}

extern "C" IComponent* create_component() noexcept 
{
    try 
    {
        return new GreeterComponent();
    }
    catch (...) 
    {
        return nullptr;
    }
}

extern "C" void destroy_component(IComponent* instance) noexcept 
{
    // delete en C++ ya maneja internamente la comprobación de nullptr
    delete instance;
}