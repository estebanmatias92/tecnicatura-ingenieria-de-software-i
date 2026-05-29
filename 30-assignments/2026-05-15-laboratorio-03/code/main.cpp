#include "include/ModuleManager.hpp"
#include "include/IGreeter.hpp"
#include <iostream>
#include <vector>

int main() 
{
    ModuleManager moduleManager;

    std::cout << "-> Iniciando aplicación y cargando módulos..." << std::endl;

    // Intentamos cargar la biblioteca compartida (el .so se añade automáticamente)
    // Asumimos que la biblioteca compilada está en la carpeta actual o en "./lib/"
    if (!moduleManager.loadModule("Greeter", "./libGreeter")) 
    {
        return EXIT_FAILURE;
    }

    std::cout << "-> Instanciando componente de forma segura..." << std::endl;
    
    // Obtenemos el shared_ptr seguro
    auto greeter = moduleManager.createInstance<IGreeter>("Greeter");

    if (greeter) 
    {
        // Usamos el componente respetando el ABI (buffers crudos)
        char buffer[256];
        greeter->greet("Gabriel", buffer, sizeof(buffer));
        
        std::cout << "\nResultado del Componente:\n";
        std::cout << "------------------------------------\n";
        std::cout << buffer << std::endl;
        std::cout << "------------------------------------\n\n";
    }

    std::cout << "-> Finalizando la aplicación. El recolector inteligente limpiará la memoria..." << std::endl;
    
    // Al salir del scope:
    // 1. 'greeter' (shared_ptr) se destruye.
    // 2. Se invoca el Deleter Personalizado.
    // 3. El Deleter invoca 'destroyComponent' en la DLL.
    // 4. El Deleter suelta su copia del std::shared_ptr<SharedLibrary>.
    // 5. 'moduleManager' se destruye, soltando la última referencia de la biblioteca.
    // 6. El destructor de SharedLibrary invoca dlclose().

    return EXIT_SUCCESS;
}