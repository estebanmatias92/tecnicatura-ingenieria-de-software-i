#ifndef IGREETER_HPP
#define IGREETER_HPP

#include "IComponent.hpp"
#include <cstddef> // Para size_t

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
        * @param outBuffer Puntero al buffer donde se escribirá el resultado.
        * @param bufferSize Tamaño máximo del buffer para evitar desbordamientos.
        */
        virtual void greet(const char* name, char* outBuffer, size_t bufferSize) = 0;
};

#endif // IGREETER_HPP