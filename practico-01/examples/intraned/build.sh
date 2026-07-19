#!/bin/bash

# Forzar que el script termine si algún comando falla
set -e

# Asegurar la existencia del directorio de destino para los módulos y la base de datos
mkdir -p lib
mkdir -p data

# 0. Compilar el HttpServerComponent (dependencia para este ejemplo).
# Agregamos -pthread porque HttpServerComponent utiliza hilos internamente.
g++ -std=c++17 -c -fPIC -I./include src/http_server_component.cpp -o http_server_component.o
g++ -std=c++17 -shared -pthread -o lib/http_server.so http_server_component.o

# 1. Compilar el motor SQLite en código objeto C (usando gcc, no g++)
gcc -c -fPIC vendor/sqlite/sqlite3.c -o sqlite3.o

# 2. Compilar el componente sqlite_component en C++
g++ -std=c++17 -c -fPIC -I./include -I./vendor/sqlite src/sqlite_component.cpp -o sqlite_component.o

# 3. Enlazar ambos objetos en una sola biblioteca dinámica maestra
g++ -std=c++17 -shared -o lib/sqlite_handler.so sqlite_component.o sqlite3.o

# 4. Compilar Host
g++ -std=c++17 -I./include -I./vendor/json main.cpp -o host.app -ldl -pthread

# 6. Limpieza opcional de objetos intermedios
rm *.o

# 7. Ejecutar la aplicación
./host.app