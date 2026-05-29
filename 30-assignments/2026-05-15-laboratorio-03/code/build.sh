# 1. Compilar el Componente como una Biblioteca Compartida (Dynamic Shared Object)
# Usamos -fPIC (Position Independent Code) vital para bibliotecas compartidas en Linux
g++ -c -fPIC src/GreeterComponent.cpp -o GreeterComponent.o
g++ -shared -o libGreeter.so GreeterComponent.o

# 2. Compilar el Ejecutable Principal
# Necesitamos enlazar la biblioteca -ldl para poder usar dlopen, dlclose, dlsym en Linux
g++ main.cpp -o app_main -ldl

# 3. Ejecutar la aplicación
# (Asegúrate de que libGreeter.so esté en el mismo directorio donde ejecutas app_main)
./app_main