#include "../include/IGreeter.hpp"
#include <string>
#include <cstring>

/**
 * @brief Implementación concreta del componente Saludador.
 * @details Esta clase permanece oculta a la aplicación principal. 
 * Solo se expone a través de la interfaz IGreeter.
 */
class GreeterComponent : public IGreeter 
{
    private:
        std::string prefix;

    public:
        GreeterComponent() : prefix("Hello, ") {}
        ~GreeterComponent() override = default;

        void greet(const char* name, char* outBuffer, size_t bufferSize) override 
        {
            if (!name || !outBuffer || bufferSize == 0) return;

            std::string result = prefix + name + "!";
            
            // Copiamos de manera segura al buffer C-style
            strncpy(outBuffer, result.c_str(), bufferSize - 1);
            outBuffer[bufferSize - 1] = '\0'; // Asegurar la terminación nula
        }
};

// --- EXPORTACIÓN DE C-API ---

/**
 * @brief Crea una instancia del componente.
 * @details La memoria se reserva en el heap local de esta biblioteca.
 */
extern "C" IComponent* createComponent() 
{
    return new GreeterComponent();
}

/**
 * @brief Destruye una instancia del componente.
 * @details La memoria se libera en el heap local de esta biblioteca.
 */
extern "C" void destroyComponent(IComponent* instance) 
{
    delete instance;
}