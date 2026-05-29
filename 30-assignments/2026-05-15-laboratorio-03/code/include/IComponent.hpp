#ifndef ICOMPONENT_HPP
#define ICOMPONENT_HPP

/**
 * @brief Interfaz base para todos los componentes.
 * @details Garantiza que cualquier componente exponga un destructor virtual,
 * permitiendo una limpieza segura de los recursos polimórficos.
 */
class IComponent 
{
    public:
        virtual ~IComponent() = default;
};

/**
 * @brief Tipos de punteros a función esperados de cualquier módulo dinámico.
 * Todo módulo debe exportar una función "createComponent" y una "destroyComponent".
 */
extern "C" 
{
    typedef IComponent* (*CreateComponentFunc)();
    typedef void (*DestroyComponentFunc)(IComponent*);
}

#endif // ICOMPONENT_HPP