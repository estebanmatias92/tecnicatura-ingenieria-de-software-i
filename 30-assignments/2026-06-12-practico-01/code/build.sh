#!/bin/bash
# Forzar que el script termine si algún comando falla
set -e

# Asegurar la existencia del directorio de destino para los módulos
mkdir -p lib

# 1. Compilar el Componente como una Biblioteca Compartida (Dynamic Shared Object)
# Usamos -fPIC (Position Independent Code) vital para bibliotecas compartidas en Linux
g++ -std=c++17 -c -fPIC -I./include src/greeter_component.cpp -o greeter_component.o
g++ -std=c++17 -shared -o lib/greeter.so greeter_component.o

# 2. Compilar el Ejecutable Principal
# Necesitamos enlazar la biblioteca -ldl para poder usar dlopen, dlclose, dlsym en Linux
g++ -std=c++17 -I./include main.cpp -o host.app -ldl

# 3. Limpieza opcional de objetos intermedios
rm *.o

# 4. Ejecutar la aplicación
./host.app