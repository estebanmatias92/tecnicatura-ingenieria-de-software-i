/**
 * @file Application.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief C Plus Plus Component Model
 * @version 1
 * @date 2026-05-19
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "ModuleManager.hpp"
#include "IGreeter.hpp"

/**
 * @brief Clase Orquestadora de la lógica de negocio de la aplicación.
 * @details Absorbe la complejidad de la ejecución, aislando al main de las
 * verificaciones lógicas concretas y flujos de datos intermedios.
 */
class Application 
{
    private:
        ModuleManager moduleManager;

    public:
        Application() = default;
        ~Application() = default;

        /**
         * @brief Inicializa las dependencias e infraestructura del sistema.
         */
        void initialize() 
        {
            // La infraestructura reporta errores hacia arriba vía excepciones
            moduleManager.loadModule("./lib/Greeter");
        }

        /**
         * @brief Ejecuta el ciclo de vida principal o los comandos de negocio.
         * @return Código de salida del proceso (EXIT_SUCCESS o EXIT_FAILURE).
         */
        int run() 
        {
            // El mánager garantiza que la instancia es válida o lanza excepción previa
            auto greeter = moduleManager.createInstance<IGreeter>("Greeter");

            char buffer[256];
            
            /**
             * Verificación basada en el código de retorno del componente.
             * Al estar encapsulado aquí, el cliente del mánager (main) permanece limpio.
             */
            ComponentResult result = greeter->greet("Matias", buffer, sizeof(buffer));
            
            if (result == ComponentResult::SUCCESS) 
            {
                std::cout << "Resultado: " << buffer << std::endl;
                return EXIT_SUCCESS;
            } 
            else 
            {
                std::cerr << "Error de Negocio: El componente falló con código: " 
                          << static_cast<int>(result) << std::endl;
                return EXIT_FAILURE;
            }
        }
};

#endif // APPLICATION_HPP
