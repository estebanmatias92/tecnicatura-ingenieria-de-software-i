let recursosCache = [];

async function cargarRecursos() {
    try {
        const res = await fetch('/api/contenidos');
        const data = await res.json();
        recursosCache = data.contenidos || [];
        renderizar(recursosCache);
    } catch (error) {
        console.error("Error al conectar con el servidor C++:", error);
    }
}

function renderizar(lista) {
    const contenedor = document.getElementById('contenedor-recursos');
    contenedor.replaceChildren(); // Limpia el contenedor de forma eficiente

    if (lista.length === 0) {
        contenedor.appendChild(crearElemento("p", "", "No se encontraron materiales educativos."));
        return;
    }

    lista.forEach(item => {
        const card = crearElemento("article", "card");

        const titulo = crearElemento("h3", "", item.titulo);
        
        const infoAutor = crearElemento("p");
        const boldAutor = crearElemento("strong", "", "Autor: ");
        infoAutor.append(boldAutor, item.autor);

        const infoTema = crearElemento("p");
        const boldTema = crearElemento("strong", "", "Tema: ");
        infoTema.append(boldTema, item.tema);

        const enlace = crearElemento("a", "btn", "Abrir Material");
        enlace.href = `/recursos/${item.file}`;
        enlace.target = "_blank";

        // Ensamblaje del DOM
        card.append(titulo, infoAutor, infoTema, enlace);
        contenedor.appendChild(card);
    });
}

document.getElementById('buscador').addEventListener('input', (e) => {
    const term = e.target.value.toLowerCase();
    const filtrados = recursosCache.filter(r => 
        r.titulo.toLowerCase().includes(term) ||
        r.autor.toLowerCase().includes(term) ||
        r.tema.toLowerCase().includes(term)
    );
    renderizar(filtrados);
});

cargarRecursos();