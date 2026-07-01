document.addEventListener('DOMContentLoaded', () => {
    const terminal = document.getElementById('terminalSalida');
    const inputNombre = document.getElementById('inputNombre');
    const inputFile = document.getElementById('inputFile');

    // Referencias a botones GET
    const btnSaludar = document.getElementById('btnSaludar');
    const btnGetRecurso = document.getElementById('btnGetRecurso');
    
    // Referencias a botones de Mutación
    const btnPut = document.getElementById('btnPut');
    const btnPatch = document.getElementById('btnPatch');
    const btnDelete = document.getElementById('btnDelete');
    
    // Referencias a operaciones de Archivo y Consola
    const btnUpload = document.getElementById('btnUpload');
    const btnLimpiar = document.getElementById('btnLimpiar');

    /**
     * @brief Escribe un mensaje en la terminal virtual usando creación estricta de nodos.
     * @param {string} mensaje El texto a mostrar.
     * @param {number|null} statusCode El código HTTP recibido.
     * @param {boolean} esError Indica si el texto debe representarse como error crítico.
     */
    function logToTerminal(mensaje, statusCode = null, esError = false) {
        const divEntrada = document.createElement('div');
        divEntrada.className = 'log-entry';
        
        if (esError) {
            divEntrada.classList.add('log-error');
        }

        if (statusCode !== null) {
            const spanStatus = document.createElement('span');
            spanStatus.className = 'log-status';
            spanStatus.textContent = `[${statusCode}]`;
            divEntrada.appendChild(spanStatus);
        }

        const nodoTexto = document.createTextNode(mensaje);
        divEntrada.appendChild(nodoTexto);
        
        terminal.insertBefore(divEntrada, terminal.firstChild);
    }

    /**
     * @brief Ejecuta una petición HTTP asíncrona genérica.
     * @param {string} url La ruta de destino.
     * @param {string} method El verbo HTTP (GET por defecto).
     * @param {BodyInit|null} body El payload a enviar (texto, Blob, File, etc.).
     */
    async function realizarPeticion(url, method = 'GET', body = null) {
        try {
            const fetchOptions = { method: method };
            if (body !== null) {
                fetchOptions.body = body;
            }

            const respuesta = await fetch(url, fetchOptions);
            
            // Los estados 204 (No Content) carecen de cuerpo, extraer texto arrojaría error si no se controla.
            let texto = "";
            if (respuesta.status !== 204) {
                texto = await respuesta.text();
            } else {
                texto = "Operación exitosa (Sin contenido devuelto).";
            }
            
            const esErrorHttp = !respuesta.ok; 
            logToTerminal(texto, respuesta.status, esErrorHttp);
            
        } catch (error) {
            logToTerminal(`Excepción de red al intentar alcanzar ${url} mediante ${method}`, null, true);
        }
    }

    /**
     * @brief Gestiona la lectura del archivo desde el input y su transmisión binaria.
     */
    async function handleFileUpload() {
        if (inputFile.files.length === 0) {
            logToTerminal("No se ha seleccionado ningún archivo para subir.", null, true);
            return;
        }

        const archivo = inputFile.files[0];
        
        // Deshabilitar botón durante la subida para prevenir concurrencia accidental
        btnUpload.disabled = true;
        logToTerminal(`Iniciando subida de: ${archivo.name} (${archivo.size} bytes)...`);

        // Codificamos el nombre del archivo en la URL como Query Parameter
        const targetUrl = `/api/upload?archivo=${encodeURIComponent(archivo.name)}`;
        
        // Transmisión binaria directa pasando el objeto File
        await realizarPeticion(targetUrl, 'POST', archivo);

        btnUpload.disabled = false;
        inputFile.value = ''; // Limpiar el input tras la subida
    }

    // --- Asignación de Eventos ---

    btnSaludar.addEventListener('click', () => {
        const nombre = inputNombre.value.trim();
        const urlSegura = nombre === '' ? '/api/saludar' : `/api/saludar?nombre=${encodeURIComponent(nombre)}`;
        realizarPeticion(urlSegura, 'GET');
    });

    btnGetRecurso.addEventListener('click', () => realizarPeticion('/api/recurso', 'GET'));
    btnPut.addEventListener('click', () => realizarPeticion('/api/recurso', 'PUT'));
    btnPatch.addEventListener('click', () => realizarPeticion('/api/recurso', 'PATCH'));
    btnDelete.addEventListener('click', () => realizarPeticion('/api/recurso', 'DELETE'));
    
    btnUpload.addEventListener('click', handleFileUpload);

    btnLimpiar.addEventListener('click', () => {
        while (terminal.firstChild) {
            terminal.removeChild(terminal.firstChild);
        }
    });
});