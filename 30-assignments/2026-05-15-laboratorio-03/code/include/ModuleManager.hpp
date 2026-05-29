#ifndef MODULE_MANAGER_HPP
#define MODULE_MANAGER_HPP

#include "SharedLibrary.hpp"
#include "IComponent.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <iostream>

/**
 * @brief Gestor central de módulos que resuelve la instanciación segura.
 */
class ModuleManager 
{
    private:
        // Almacena las bibliotecas cargadas. Usamos shared_ptr para compartir la propiedad.
        std::unordered_map<std::string, std::shared_ptr<SharedLibrary>> loadedLibraries;

    public:
        /**
        * @brief Carga un módulo en memoria.
        * @param alias Nombre clave para referirse al módulo internamente.
        * @param path Ruta real del archivo (sin extensión).
        * @return true si se cargó correctamente, false en caso contrario.
        */
        bool loadModule(const std::string& alias, const std::string& path) {
            try 
            {
                auto lib = std::make_shared<SharedLibrary>(path);
                loadedLibraries[alias] = lib;
                return true;
            }
            catch (const std::exception& e)
            {
                std::cerr << "ModuleManager Error: " << e.what() << std::endl;
                return false;
            }
        }

        /**
        * @brief Crea una instancia de un componente y lo envuelve de forma segura.
        * @tparam InterfaceType Tipo de la interfaz esperada (ej. IGreeter).
        * @param alias El alias del módulo cargado.
        * @return std::shared_ptr a la interfaz, o nullptr en caso de error.
        */
        template<typename InterfaceType>
        std::shared_ptr<InterfaceType> createInstance(const std::string& alias)
        {
            auto it = loadedLibraries.find(alias);
            if (it == loadedLibraries.end())
            {
                std::cerr << "ModuleManager Error: Módulo no cargado -> " << alias << std::endl;
                return nullptr;
            }

            std::shared_ptr<SharedLibrary> lib = it->second;

            auto createFunc = (CreateComponentFunc)lib->getSymbol("createComponent");
            auto destroyFunc = (DestroyComponentFunc)lib->getSymbol("destroyComponent");

            if (!createFunc || !destroyFunc)
            {
                std::cerr << "ModuleManager Error: Faltan símbolos requeridos en el módulo." << std::endl;
                return nullptr;
            }

            // 1. Invocamos a la DLL para crear el objeto en su propio heap
            IComponent* rawInstance = createFunc();
            if (!rawInstance) return nullptr;

            // 2. Casteamos a la interfaz solicitada
            InterfaceType* castedInstance = dynamic_cast<InterfaceType*>(rawInstance);
            if (!castedInstance)
            {
                std::cerr << "ModuleManager Error: El componente no implementa la interfaz solicitada." << std::endl;
                destroyFunc(rawInstance); // Limpiamos para evitar fugas
                return nullptr;
            }

            // 3. LA MAGIA: Creamos un shared_ptr con un custom deleter.
            // Capturamos el puntero a la función de destrucción Y el shared_ptr de la biblioteca.
            // Esto asegura que la DLL no se descargue de memoria mientras la instancia exista.
            auto deleter = [destroyFunc, lib](InterfaceType* ptr)
            {
                destroyFunc(ptr);
            };

            return std::shared_ptr<InterfaceType>(castedInstance, deleter);
        }
};

#endif // MODULE_MANAGER_HPP