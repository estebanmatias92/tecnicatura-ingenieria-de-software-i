/**
 * @file module_manager.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief C Plus Plus Component Model: Cargador de módulos, seguro en presencia de concurrencia.
 * @version 1
 * @date 2026-05-19
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */

#ifndef MODULE_MANAGER_HPP
#define MODULE_MANAGER_HPP

#include "shared_library.hpp"
#include "i_component.hpp"
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
        std::unordered_map<std::string, std::shared_ptr<SharedLibrary>> loaded_libraries_;
        
        /**
         * @brief Mutex para proteger el acceso concurrente al mapa de bibliotecas.
         */
        std::mutex map_mutex_;

        /**
         * @brief Extrae el nombre del archivo de una ruta completa.
         * @details Extrae "greeter" a partir de "./lib/greeter".
         */
        std::string extract_module_name(const std::string& path) const 
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
        void load_module(const std::string& path) 
        {
            std::string module_name = extract_module_name(path);

            try 
            {
                auto lib = std::make_shared<SharedLibrary>(path);
                
                // Bloqueo del mutex antes de modificar el mapa compartido
                std::lock_guard<std::mutex> lock(map_mutex_);
                loaded_libraries_[module_name] = lib;
            }
            catch (const std::exception& e)
            {
                // En lugar de retornar false o usar exit(), lanzamos una excepción controlada del Host
                throw std::runtime_error("ModuleManager Error Crítico al cargar " + module_name + ": " + e.what());
            }
        }

        /**
         * @brief Crea una instancia validando la versión del ABI.
         * @param module_name El nombre del archivo extraído de la ruta (ej. "Greeter").
         * @details Lanza std::runtime_error si hay discrepancias o errores internos,
         * garantizando que el Host no reciba jamás un puntero inválido o nullptr.
         */
        template<typename InterfaceType>
        std::shared_ptr<InterfaceType> create_instance(const std::string& module_name)
        {
            std::shared_ptr<SharedLibrary> lib;

            // Ámbito artificial para reducir el tiempo de bloqueo del mutex
            {
                std::lock_guard<std::mutex> lock(map_mutex_);
                auto it = loaded_libraries_.find(module_name);
                if (it == loaded_libraries_.end())
                {
                    throw std::runtime_error("ModuleManager Error: Módulo requerido no precargado -> " + module_name);
                }
                lib = it->second;
            }

            auto get_api_func = (get_api_version_func)lib->get_symbol("get_api_version");
            auto create_func = (create_component_func)lib->get_symbol("create_component");
            auto destroy_func = (destroy_component_func)lib->get_symbol("destroy_component");

            if (!get_api_func || !create_func || !destroy_func)
            {
                throw std::runtime_error("ModuleManager Error: Faltan símbolos requeridos en " + module_name);
            }

            /**
             * @brief Control de versiones estricto.
             * @details Comparamos la versión de la interfaz con la que compiló la biblioteca compartida
             * contra la versión actual que maneja el Host. Si difieren, abortamos por excepción.
             */
            int module_api_version = get_api_func();
            if (module_api_version != CURRENT_API_VERSION)
            {
                throw std::runtime_error("ModuleManager Error: Discrepancia de ABI en " + module_name + 
                                         ". Esperado: " + std::to_string(CURRENT_API_VERSION) + 
                                         ", Encontrado: " + std::to_string(module_api_version));
            }

            /**
             * @brief Invocamos a la biblioteca compartida mediante su función creadora
             * para crear el objeto en su propio heap
             */
            IComponent* raw_instance = create_func();
            if (!raw_instance) 
            {
                throw std::runtime_error("ModuleManager Error: La fábrica de la biblioteca devolvió una instancia nula.");
            }

            /**
             * @brief Casteamos a la interfaz solicitada
             * @details luego si este casteo falla (entrega un puntero nulo)
             * limpiamos para evitar fugas invocando a la función de destrucción del componente.
             */
            InterfaceType* casted_instance = dynamic_cast<InterfaceType*>(raw_instance);
            if (!casted_instance)
            {
                destroy_func(raw_instance);
                throw std::runtime_error("ModuleManager Error: El componente " + module_name + " no implementa la interfaz solicitada.");
            }

            /** 
             * @attention LA MAGIA Creamos un shared_ptr con un custom deleter.
             * @details Capturamos el puntero a la función de destrucción y el shared_ptr de la biblioteca.
             * Esto asegura que la biblioteca compartida no se descargue de memoria mientras la instancia exista.
             */
            auto deleter = [destroy_func, lib](InterfaceType* ptr)
            {
                destroy_func(ptr);
            };

            return std::shared_ptr<InterfaceType>(casted_instance, deleter);
        }
};

#endif // MODULE_MANAGER_HPP