::1. Compilar el Componente como una Biblioteca Compartida (Dynamic Shared Object)
g++ -std=c++17 -c src\GreeterComponent.cpp -o GreeterComponent.o
g++ -std=c++17 -shared -o lib\Greeter.dll GreeterComponent.o

::2. Compilar el Ejecutable Principal
::Necesitamos enlazar la biblioteca -ldl para poder usar dlopen, dlclose, dlsym en Linux
g++ -std=c++17 main.cpp -o hostApp.bin -ldl

::3. Ejecutar la aplicación
hostApp.bin