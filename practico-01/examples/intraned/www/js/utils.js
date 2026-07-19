// 1. Funciones utilitarias 
function crearElemento(tag, clase = "", texto = "") {
    const el = document.createElement(tag);
    if (clase) el.className = clase;
    if (texto) el.textContent = texto;
    return el;
}

function crearInput(labelTxt, id, type = 'text') {
    const div = crearElemento('div', 'campo-form');
    const label = crearElemento('label', '', labelTxt);
    const input = crearElemento('input');
    input.id = id;
    input.type = type;
    input.required = true;
    div.append(label, input);
    return div;
}
