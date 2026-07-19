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

##Para ejecutar un script en Bash simulando que te encuentras 
##en otro directorio (sin afectar la ruta de tu script principal), 
##debes usar una subshell envolviendo los comandos entre paréntesis.

# Limpieza de database example
(cd ./examples/database/ && ./clean.sh)
# Limpieza de http_server example
(cd ./examples/http_server/ && ./clean.sh)
# Limpieza de intraned example
(cd ./examples/intraned && ./clean.sh)


# Limpieza de ejemplo de componente Greeter
borrar_si_existe "./lib/greeter.so"
borrar_si_existe "./host.app"
