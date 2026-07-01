/**
 * @file main.cpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief Punto de entrada del servidor de recursos educativos Intraned
 * @version 1
 * @date 2026-06-09
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 */

#include "include/application.hpp"
#include <iostream>
#include <stdexcept>

int main() 
{
    try 
    {
        Application app;

        std::cout << "-> Inicializando componentes globales..." << std::endl;
        app.initialize();

        std::cout << "-> Ejecutando lógica de negocio..." << std::endl;
        return app.run();
    }
    catch (const std::exception& e) 
    {
        // Cualquier error crítico de infraestructura (Carga, ABI, Memoria) es capturado aquí
        std::cerr << "\n[FATAL] Ejecución abortada de forma segura por error de infraestructura:\n" 
                  << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}