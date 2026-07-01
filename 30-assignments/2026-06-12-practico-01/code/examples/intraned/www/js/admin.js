// admin.js

// 1. Verificación estricta de sesión
if (!localStorage.getItem('admin_token')) {
    window.location.href = '/login.html';
}

const app = document.getElementById('app-admin');

// 2. Construcción del Layout
const containerPrincipal = crearElemento('div', 'admin-container');
app.appendChild(containerPrincipal);

// --- SECCIÓN A: FORMULARIO DE CARGA ---
const seccionFormulario = crearElemento('section', 'card');
seccionFormulario.append(crearElemento('h2', '', 'Cargar Nuevo Recurso'));

const formUpload = crearElemento('form');
formUpload.append(
    crearInput('Título del Material:', 'titulo'),
    crearInput('Autor/a:', 'autor'),
    crearInput('Tema:', 'tema'),
    crearInput('Archivo (PDF/MP3/IMG):', 'archivo', 'file')
);

const btnSubmit = crearElemento('button', 'btn', 'Subir a la Isla');
btnSubmit.type = 'submit';

const statusMsg = crearElemento('p', 'status-msg');
statusMsg.style.marginTop = '15px';

formUpload.append(btnSubmit, statusMsg);
seccionFormulario.appendChild(formUpload);
containerPrincipal.appendChild(seccionFormulario);

// --- SECCIÓN B: LISTADO DE RECURSOS ---
const seccionRecursos = crearElemento('section', 'card');
seccionRecursos.style.marginTop = '20px';
seccionRecursos.append(crearElemento('h2', '', 'Recursos en el Servidor'));

const listaContenidos = crearElemento('div', 'lista-admin');
seccionRecursos.appendChild(listaContenidos);
containerPrincipal.appendChild(seccionRecursos);


// 4. Lógica de Negocio

// Función para obtener y renderizar los archivos alojados
async function cargarRecursosAdmin() {
    listaContenidos.replaceChildren(); // Limpieza eficiente del DOM
    listaContenidos.appendChild(crearElemento('p', '', 'Cargando recursos...'));

    try {
        const res = await fetch('/api/contenidos');
        if (!res.ok) throw new Error("Respuesta no satisfactoria del servidor");
        
        const data = await res.json();
        listaContenidos.replaceChildren(); // Volver a limpiar tras la carga

        if (!data.contenidos || data.contenidos.length === 0) {
            listaContenidos.appendChild(crearElemento('p', '', 'La base de datos está vacía.'));
            return;
        }

        // Renderizado del listado
        data.contenidos.forEach(item => {
            const fila = crearElemento('div', 'admin-resource-item');
            fila.style.borderBottom = "1px solid #ddd";
            fila.style.padding = "10px 0";
            fila.style.display = "flex";
            fila.style.justifyContent = "space-between";
            fila.style.alignItems = "center";
            
            const info = crearElemento('span', '', `${item.titulo} - ${item.autor} (${item.tema})`);
            
            // Contenedor para agrupar los botones
            const acciones = crearElemento('div');

            const enlace = crearElemento('a', 'btn', 'Abrir');
            enlace.href = `/recursos/${item.file}`;
            enlace.target = "_blank";
            enlace.style.padding = "4px 8px";
            enlace.style.fontSize = "0.85rem";

            const btnBorrar = crearElemento('button', 'btn', 'Borrar');
            btnBorrar.style.background = "#e74c3c";
            btnBorrar.style.marginLeft = "10px";
            btnBorrar.style.padding = "4px 8px";
            btnBorrar.style.fontSize = "0.85rem";
            btnBorrar.style.border = "none";
            btnBorrar.style.cursor = "pointer";
            
            // Asignación del evento con el ID del recurso extraído de la DB
            btnBorrar.onclick = () => eliminarRecurso(item.id);

            acciones.append(enlace, btnBorrar);
            fila.append(info, acciones);
            listaContenidos.appendChild(fila);
        });

    } catch (error) {
        console.error("Error técnico al cargar contenidos:", error);
        listaContenidos.replaceChildren(crearElemento('p', '', 'Error de conexión con la API de contenidos.'));
        listaContenidos.style.color = "#e74c3c";
    }
}

// Lógica de Negocio: Eliminación segura
async function eliminarRecurso(id) {
    if (!confirm("¿Atención: Esta acción eliminará permanentemente el archivo físico y su registro en la base de datos. ¿Desea continuar?")) {
        return;
    }

    try {
        const res = await fetch(`/api/contenidos?id=${id}`, {
            method: 'DELETE',
            headers: { 
                'Authorization': localStorage.getItem('admin_token') 
            }
        });

        if (res.ok) {
            alert("Recurso eliminado correctamente.");
            cargarRecursosAdmin(); // Refrescar el DOM visual
        } else {
            const err = await res.json();
            alert(`Error del servidor: ${err.error}`);
        }
    } catch (error) {
        alert("Excepción: Error de red al intentar borrar el recurso.");
        console.error(error);
    }
}

// Evento de subida de archivos
formUpload.addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const fileInput = document.getElementById('archivo');
    if (fileInput.files.length === 0) {
        statusMsg.textContent = "Requisito: Seleccione un archivo.";
        statusMsg.style.color = "#e74c3c";
        return;
    }

    btnSubmit.disabled = true;
    btnSubmit.textContent = "Procesando E/S...";
    statusMsg.textContent = "";

    const file = fileInput.files[0];
    
    // 1. Codificar los metadatos para que sean seguros en una URL
    const titulo = encodeURIComponent(document.getElementById('titulo').value);
    const autor = encodeURIComponent(document.getElementById('autor').value);
    const tema = encodeURIComponent(document.getElementById('tema').value);
    const filename = encodeURIComponent(file.name);

    // 2. Construir la URL con la Query String esperada por el backend C++
    const url = `/api/upload?titulo=${titulo}&autor=${autor}&tema=${tema}&filename=${filename}`;

    try {
        // 3. Enviar el archivo directamente (RAW Binary) en el body
        const res = await fetch(url, {
            method: 'POST',
            headers: { 
                'Authorization': localStorage.getItem('admin_token'),
                'Content-Type': 'application/octet-stream'
            },
            body: file 
        });

        if (res.ok) {
            statusMsg.textContent = "Escritura en disco y metadata exitosa.";
            statusMsg.style.color = "#2ecc71";
            formUpload.reset(); 
            cargarRecursosAdmin(); 
        } else {
            const err = await res.json();
            statusMsg.textContent = `Error del servidor: ${err.error || res.statusText}`;
            statusMsg.style.color = "#e74c3c";
        }
    } catch (error) {
        statusMsg.textContent = "Excepción: Error de red al intentar el POST.";
        statusMsg.style.color = "#e74c3c";
    } finally {
        btnSubmit.disabled = false;
        btnSubmit.textContent = "Subir a la Isla";
    }
});

// 5. Inicialización
cargarRecursosAdmin();