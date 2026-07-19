# <img src="frontend/img/favicon-bw.png" width="45" height="45" align="center"> SampleVaultest

**SampleVaultest** estos son tests para el backend de una aplicación web diseñada para la gestión profesional de librerías de sonido. Permite a los productores musicales subir, categorizar, escuchar y organizar sus muestras de audio (samples) de forma privada y segura, todo bajo una arquitectura modular y eficiente.

---

## 🚀 Características Principales

* **Gestión de Samples:** Sube archivos de audio (MP3, WAV, OGG, FLAC) con metadatos personalizados (BPM, categoría).
* **Reproductor Integrado:** Escucha tus sonidos directamente desde la biblioteca mediante una interfaz minimalista.
* **Arquitectura Limpia:** Separación total de responsabilidades entre controladores, servicios y utilidades.
* **Frontend "Zero innerHTML":** Manipulación del DOM 100% nativa para máxima seguridad y rendimiento.
* **Sostenibilidad de Software:** Código optimizado siguiendo principios de eficiencia en el consumo de recursos (inspirado en el modelo **GREENSOFT**).

## 🛠️ Tecnologías Utilizadas

### Backend
* **Node.js & Express:** Servidor robusto y escalable.
* **Multer:** Gestión eficiente de subida de archivos binarios.
* **MySQL/MariaDB:** Persistencia de metadatos de forma relacional.

### Frontend
* **Vanilla JavaScript:** Lógica pura sin dependencias de frameworks pesados.
* **W3.CSS:** Framework CSS ligero para una interfaz moderna y responsiva.
* **Web Components (Custom):** Manejo dinámico de la UI mediante inyección de nodos nativos.

---

### Inicialización del Backend:
Entra en la carpeta del servidor e instala las dependencias necesarias:
```bash
cd backend
npm init -y  # Crea el archivo package.json por defecto
```
### Instalación de módulos:
Ejecuta este comando para instalar todas las bibliotecas que configuramos en los scripts:
```bash
npm install express mysql2 cors multer jsonwebtoken bcrypt dotenv
```
### Agregar archivo .env en el directorio backend/ con estas variables como ejemplo:
```text
PORT=3000
DB_HOST=localhost
DB_USER=samplevaultest
DB_PASS=samplevaultest
DB_NAME=samplevaultest
JWT_SECRET=tu_clave_secreta_super_segura
NODE_ENV=testing
```
## 📂 Estructura del Proyecto

```text
├── backend/
│   ├── config/          # Configuración de Multer y DB.
│   ├── controllers/     # Lógica de negocio.
│   ├── repositories/    # Acceso a datos (SQL).
│   ├── routes/          # Rutas de acceso a módulos del backend.
│   ├── uploads/         # Almacenamiento físico de sonidos.
│   └── utils/           # Helpers de sistema de archivos (fileHelper).
├── frontend/
│   ├── css/             # Estilos personalizados.
│   ├── html/            # Archivos de maquetación estáticos.
│   ├── img/             # Activos visuales (favicon.png).
│   └── js/
│       ├── tests/  # Scripts necesarios para crear el frontend de los tests.
├── test-samples/ #samples de prueba para subir.