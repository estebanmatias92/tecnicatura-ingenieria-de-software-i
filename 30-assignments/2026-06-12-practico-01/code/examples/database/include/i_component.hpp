/**
 * @file i_component.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief C Plus Plus Component Model
 * @version 1
 * @date 2026-05-19
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */

#ifndef ICOMPONENT_HPP
#define ICOMPONENT_HPP

/**
 * @brief Enumeración para códigos de estado de las operaciones a través del ABI.
 * @details Al evitar excepciones, requerimos un mecanismo clásico de retorno
 * de errores (tipo HRESULT en COM) para informar al Host sobre el estado.
 */
enum class ComponentResult : int 
{
    SUCCESS = 0,
    ERROR_INVALID_ARGUMENT = 1,
    ERROR_INTERNAL = 2
};

/**
 * @brief Versión actual del contrato ABI. 
 * @details Si se modifican las interfaces virtuales (agregando o quitando métodos),
 * este número DEBE incrementarse para evitar violaciones de segmento.
 */
constexpr int CURRENT_API_VERSION = 1;

/**
 * @brief Interfaz base para todos los componentes.
 */
class IComponent 
{
    public:
        /**
         * @brief Destructor virtual.
         * @details El uso de 'noexcept' es obligatorio aquí. Evita que una excepción 
         * lanzada durante la destrucción intente cruzar el límite del módulo.
         */
        virtual ~IComponent() noexcept = default;
};

/**
 * @brief Tipos de punteros a función exportados (C-API).
 * @details Todas las funciones que cruzan el límite de la biblioteca compartida
 * están marcadas con 'noexcept' garantizando que el compilador aborte el 
 * programa internamente  si ocurre un error no manejado, en lugar de corromper 
 * la pila del Host.
 */
extern "C" 
{
    typedef int (*get_api_version_func)() noexcept;
    typedef IComponent* (*create_component_func)() noexcept;
    typedef void (*destroy_component_func)(IComponent*) noexcept;
}

#endif // ICOMPONENT_HPP