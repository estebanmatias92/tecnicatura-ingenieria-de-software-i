/**
 * @file i_greeter.hpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief C Plus Plus Component Model: Interfaz del componente Greeter (saludador).
 * @version 1
 * @date 2026-05-19
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */

#ifndef IGREETER_HPP
#define IGREETER_HPP

#include "i_component.hpp"
#include <cstddef>

/**
 * @brief Interfaz específica para el componente Saludador.
 */
class IGreeter : public IComponent 
{
    public:
        /**
         * @brief Genera un saludo personalizado.
         * @details Se utilizan tipos compatibles con C (const char*, char*) para
         * evitar cruzar el límite del ABI con std::string.
         *
         * @param name Nombre de la persona a saludar.
         * @param out_buffer Puntero al buffer donde se escribirá el resultado.
         * @param buffer_size Tamaño máximo del buffer para evitar desbordamientos.
         * @details Retorna ComponentResult en lugar de void para el manejo de errores.
         * Marcado estrictamente como 'noexcept'.
         */
        virtual ComponentResult greet(const char* name, char* out_buffer, size_t buffer_size) noexcept = 0;
};

#endif // IGREETER_HPP