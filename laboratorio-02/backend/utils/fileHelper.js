/**
*    Project     : Sample Vault
*    Author      : Tecnologías Informáticas B - Facultad de Ingeniería - UNMdP
*    License     : http://www.gnu.org/licenses/gpl.txt  GNU GPL 3.0
*    Date        : Marzo 2026
*/

/**
 * Node.js fs (File System) es un módulo nativo integrado que permite 
 * interactuar con el sistema de archivos del sistema operativo 
 * (leer, escribir, crear, borrar directorios y archivos). 
 * Ofrece métodos síncronos y asíncronos (más eficientes para 
 * alto rendimiento) sin necesidad de instalaciones adicionales.
 */
const fs = require('fs'); // para manejar el filesystem al borrar archivos

/**
 * El módulo path en Node.js es una utilidad integrada que permite 
 * manejar y transformar rutas de archivos y directorios de forma 
 * segura y consistente entre diferentes sistemas operativos 
 * (Windows, Linux, macOS). Facilita la creación de rutas absolutas 
 * y relativas sin preocuparse por los separadores de barras (/ o \). 
 */
const path = require('path'); // para el manejo de rutas

/**
 * Utilidad para gestionar archivos físicos
 */
const fileHelper = 
{
    /**
     * Elimina un archivo del disco de forma segura
     * @param {string} relativePath - Ruta guardada en la DB (ej: /uploads/audio.mp3)
     */
    deleteFile(relativePath) 
    {
        if (!relativePath) return;

        /**
         * path.join maneja perfectamente la barra inicial si existe
         * process.cwd() devuelve el directorio del proceso de Node.js
         * usando path.join con el directorio del proceso + la ruta relativa
         * construimos la ruta absoluta para poder borrar el archivo.
         */
        const absolutePath = path.join(process.cwd(), relativePath);

        try 
        {
            /**
             * fs.unlinkSync: Para operaciones de borrado dentro de un controlador
             * a veces es preferible la versión síncrona para evitar problemas 
             * de "condición de carrera" (que el código siga antes de que el 
             * archivo se borre realmente).
             */
            if (fs.existsSync(absolutePath))
            {
                fs.unlinkSync(absolutePath);
                console.log(`✅ Archivo eliminado: ${relativePath}`);
                return true;
            }
            console.warn(`⚠️ Archivo no encontrado: ${absolutePath}`);
        } catch (err) {
            console.error(`❌ Error al borrar archivo: ${err.message}`);
        }
        return false;
    }
};

module.exports = fileHelper;