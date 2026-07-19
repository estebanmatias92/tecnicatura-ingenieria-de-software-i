/**
 * @file GreeterComponent.cpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief C Plus Plus Component Model
 * @version 1
 * @date 2026-05-19
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */

#include "../include/IGreeter.hpp"
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
        ComponentResult greet(const char* name, char* outBuffer, size_t bufferSize) noexcept override 
        {
            if (!name || !outBuffer || bufferSize == 0) 
            {
                return ComponentResult::ERROR_INVALID_ARGUMENT;
            }

            try 
            {
                std::string result = prefix + name + "!";
                
                if (result.length() >= bufferSize) 
                {
                    return ComponentResult::ERROR_INVALID_ARGUMENT;
                }

                strncpy(outBuffer, result.c_str(), bufferSize - 1);
                outBuffer[bufferSize - 1] = '\0'; 
                
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
 * @brief Expone la versión con la que fue compilada esta DLL.
 */
extern "C" int getApiVersion() noexcept 
{
    return CURRENT_API_VERSION;
}

extern "C" IComponent* createComponent() noexcept 
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

extern "C" void destroyComponent(IComponent* instance) noexcept 
{
    // delete en C++ ya maneja internamente la comprobación de nullptr
    delete instance;
}