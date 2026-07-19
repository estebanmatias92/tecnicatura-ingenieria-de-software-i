#!/bin/bash

# Forzar que el script termine si algún comando falla
set -e

# Función para borrar si existe
borrar_si_existe() {
    local ruta="$1"
    if [ -f "$ruta" ]; then
        rm -v "$ruta"
    else
        echo "El archivo no existe o no es válido: $ruta"
    fi
}

# Limpieza de archivos y recursos
# Itera sobre todos los elementos no ocultos del directorio actual
for archivo in ./uploads/*; do
    # Verifica que sea un archivo regular antes de llamar a la función
    if [ -f "$archivo" ]; then
        borrar_si_existe "$archivo"
    fi
done

# Limpieza de archivos .o
for archivo in *.o; do
    # Verifica que sea un archivo regular antes de llamar a la función
    if [ -f "$archivo" ]; then
        borrar_si_existe "$archivo"
    fi
done

# Limpieza de base de datos
borrar_si_existe "./data/intraned.db"
borrar_si_existe "./data/intraned.db-shm"
borrar_si_existe "./data/intraned.db-wal"

# Limpieza de bibliotecas
borrar_si_existe  "./lib/http_server.so"
borrar_si_existe  "./lib/sqlite_handler.so"

# Limpieza de aplicación
borrar_si_existe  "host.app"
