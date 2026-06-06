/**
 * @file ModuleManager.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief C Plus Plus Component Model
 * @version 1
 * @date 2026-05-19
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */

#ifndef MODULE_MANAGER_HPP
#define MODULE_MANAGER_HPP

#include "SharedLibrary.hpp"
#include "IComponent.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <iostream>
#include <mutex> // Para std::mutex y std::lock_guard
#include <stdexcept> // Para std::runtime_error

/**
 * @brief Gestor central de módulos que resuelve la instanciación segura.
 * @details Mantiene una frontera estricta de infraestructura. No debe incorporar 
 * lógica de negocio ni conocer los métodos específicos de las interfaces derivadas.
 */
class ModuleManager 
{
    private:
        std::unordered_map<std::string, std::shared_ptr<SharedLibrary>> loadedLibraries;
        
        /**
         * @brief Mutex para proteger el acceso concurrente al mapa de bibliotecas.
         */
        std::mutex mapMutex;

        /**
         * @brief Extrae el nombre del archivo de una ruta completa.
         * @details Extrae "Greeter" a partir de "./lib/Greeter".
         */
        std::string extractModuleName(const std::string& path) const 
        {
            size_t pos = path.find_last_of("/\\");
            return (pos == std::string::npos) ? path : path.substr(pos + 1);
        }

    public:
        /**
         * @brief Carga un módulo en memoria usando la ruta proporcionada.
         * @param path Ruta real del archivo (sin extensión).
         * @details Lanza std::runtime_error si el módulo no puede ser cargado,
         * deteniendo la ejecución en cascada de dependencias mediante RAII.
         */
        void loadModule(const std::string& path) 
        {
            std::string moduleName = extractModuleName(path);

            try 
            {
                auto lib = std::make_shared<SharedLibrary>(path);
                
                // Bloqueo del mutex antes de modificar el mapa compartido
                std::lock_guard<std::mutex> lock(mapMutex);
                loadedLibraries[moduleName] = lib;
            }
            catch (const std::exception& e)
            {
                // En lugar de retornar false o usar exit(), lanzamos una excepción controlada del Host
                throw std::runtime_error("ModuleManager Error Crítico al cargar " + moduleName + ": " + e.what());
            }
        }

        /**
         * @brief Crea una instancia validando la versión del ABI.
         * @param moduleName El nombre del archivo extraído de la ruta (ej. "Greeter").
         * @details Lanza std::runtime_error si hay discrepancias o errores internos,
         * garantizando que el Host no reciba jamás un puntero inválido o nullptr.
         */
        template<typename InterfaceType>
        std::shared_ptr<InterfaceType> createInstance(const std::string& moduleName)
        {
            std::shared_ptr<SharedLibrary> lib;

            // Ámbito artificial para reducir el tiempo de bloqueo del mutex
            {
                std::lock_guard<std::mutex> lock(mapMutex);
                auto it = loadedLibraries.find(moduleName);
                if (it == loadedLibraries.end())
                {
                    throw std::runtime_error("ModuleManager Error: Módulo requerido no precargado -> " + moduleName);
                }
                lib = it->second;
            }

            auto getApiFunc = (GetApiVersionFunc)lib->getSymbol("getApiVersion");
            auto createFunc = (CreateComponentFunc)lib->getSymbol("createComponent");
            auto destroyFunc = (DestroyComponentFunc)lib->getSymbol("destroyComponent");

            if (!getApiFunc || !createFunc || !destroyFunc)
            {
                throw std::runtime_error("ModuleManager Error: Faltan símbolos requeridos en " + moduleName);
            }

            /**
             * @brief Control de versiones estricto.
             * @details Comparamos la versión de la interfaz con la que compiló la DLL
             * contra la versión actual que maneja el Host. Si difieren, abortamos por excepción.
             */
            int moduleApiVersion = getApiFunc();
            if (moduleApiVersion != CURRENT_API_VERSION)
            {
                throw std::runtime_error("ModuleManager Error: Discrepancia de ABI en " + moduleName + 
                                         ". Esperado: " + std::to_string(CURRENT_API_VERSION) + 
                                         ", Encontrado: " + std::to_string(moduleApiVersion));
            }

            /**
             * @brief Invocamos a la DLL mediante su función creadora
             * para crear el objeto en su propio heap
             */
            IComponent* rawInstance = createFunc();
            if (!rawInstance) 
            {
                throw std::runtime_error("ModuleManager Error: La fábrica de la DLL devolvió una instancia nula.");
            }

            /**
             * @brief Casteamos a la interfaz solicitada
             * @details luego si este casteo falla (entrega un puntero nulo)
             * limpiamos para evitar fugas invocando a la función de destrucción del componente.
             */
            InterfaceType* castedInstance = dynamic_cast<InterfaceType*>(rawInstance);
            if (!castedInstance)
            {
                destroyFunc(rawInstance);
                throw std::runtime_error("ModuleManager Error: El componente " + moduleName + " no implementa la interfaz solicitada.");
            }

            /** 
             * @attention LA MAGIA Creamos un shared_ptr con un custom deleter.
             * @details Capturamos el puntero a la función de destrucción y el shared_ptr de la biblioteca.
             * Esto asegura que la DLL no se descargue de memoria mientras la instancia exista.
             */
            auto deleter = [destroyFunc, lib](InterfaceType* ptr)
            {
                destroyFunc(ptr);
            };

            return std::shared_ptr<InterfaceType>(castedInstance, deleter);
        }
};

#endif // MODULE_MANAGER_HPP