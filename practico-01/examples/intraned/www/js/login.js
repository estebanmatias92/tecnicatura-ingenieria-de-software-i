const app = document.getElementById('app-login');

const form = crearElemento('form', 'card');
const titulo = crearElemento('h2', '', 'Acceso Administrador');

const inputPass = crearElemento('input');
inputPass.type = 'password';
inputPass.placeholder = 'Contraseña';
inputPass.id = 'pass';
inputPass.required = true;

const btnLogin = crearElemento('button', 'btn', 'Entrar');
btnLogin.type = 'submit';

form.append(titulo, inputPass, btnLogin);
app.appendChild(form);

form.addEventListener('submit', async (e) => {
    e.preventDefault();
    const password = document.getElementById('pass').value;

    const res = await fetch('/api/login', {
        method: 'POST',
        body: JSON.stringify({ password })
    });

    if (res.ok) {
        const data = await res.json();
        localStorage.setItem('admin_token', data.token);
        window.location.href = '/admin.html';
    } else {
        alert('Contraseña incorrecta');
    }
});