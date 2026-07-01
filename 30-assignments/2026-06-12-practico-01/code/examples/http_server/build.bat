::1. Compilar el Componente como una Biblioteca Compartida (Dynamic Shared Object)
g++ -std=c++17 -c src\http_server_component.cpp -o http_server_component.o
g++ -std=c++17 -shared -pthread -o lib\http_server.so http_server_component.o

::2. Compilar el Ejecutable Principal
::Necesitamos enlazar la biblioteca -ldl para poder usar dlopen, dlclose, dlsym en Linux
g++ main.cpp -o host.exe -ldl

::3. Ejecutar la aplicación
host.exe