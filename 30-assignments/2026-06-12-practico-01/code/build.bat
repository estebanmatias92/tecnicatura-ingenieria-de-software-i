::1. Compilar el Componente como una Biblioteca Compartida (Dynamic Shared Object)
g++ -c src\greeter_component.cpp -o greeter_component.o
g++ -std=c++17 -shared -o lib\greeter.so greeter_component.o

::2. Compilar el Ejecutable Principal
::Necesitamos enlazar la biblioteca -ldl para poder usar dlopen, dlclose, dlsym en Linux
g++ main.cpp -o host -ldl

::3. Ejecutar la aplicación
host.exe