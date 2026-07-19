# <img src="./www/img/favicon.png" width="45" height="45" align="center"> IntranEd

**IntranEd** es un Servidor de Contenidos Educativos diseñado para operar como una "isla de conocimiento" en zonas con conectividad limitada. No solo es técnicamente sólido por su bajo consumo de recursos, sino que resuelve un problema real de soberanía digital y democratización del acceso a la información.

Al utilizar C++ para el núcleo y un modelo de componentes dinámicos, se garantiza que el servidor pueda ejecutarse de manera eficiente incluso en hardware recuperado o netbooks educativas con apenas 2GB de RAM, cumpliendo estrictamente con los objetivos del modelo **GREENSOFT**.

## Credenciales de administrador: `admin123`
---

## ⧉ Impacto y Sostenibilidad (EdTech)

Este proyecto está diseñado bajo los principios de **Desarrollo Sostenible de Software**:
- **Bajo Consumo:** Optimizado para hardware de bajos recursos. Al evitar el renderizado del lado del servidor (SSR) y delegar la vista al cliente, se maximiza la eficiencia energética del procesador anfitrión.
- **Offline-First:** Funciona de forma autónoma en intranets locales. Aloja libros (PDF/EPUB), audios y partituras para su descarga y visualización sin depender de la nube.
- **Soberanía Digital:** Gestión local, privada y segura de materiales de estudio en escuelas o centros culturales, eludiendo los costos de una conexión a internet externa.

---

## ⧉ Arquitectura y Tecnologías Utilizadas

El sistema abandona el uso de dependencias monolíticas de terceros en favor de un **Modelo de Componentes en C++** interactuando a través de una frontera binaria (ABI) segura.

### Infraestructura Backend (C++17)
* **Application Host:** Orquestador minimalista que gestiona el ciclo de vida del sistema mediante RAII y punteros inteligentes.
* **Componente Servidor HTTP (`libhttp_server.so`):** Módulo dinámico propio, concurrente y asíncrono. Soporta operaciones RESTful CRUD y despacho de archivos binarios puros.
* **Componente Base de Datos (`libsqlite_handler.so`):** Integración con SQLite3 mediante sentencias preparadas nativas, eliminando la necesidad de un motor de base de datos externo ejecutándose en segundo plano.
* **JSON for Modern C++:** Única dependencia externa (*header-only*) utilizada exclusivamente para la serialización de respuestas de red. https://github.com/nlohmann/json

### Frontend
* **Vanilla JavaScript:** Lógica de cliente pura para la interacción con la API REST, prescindiendo de frameworks pesados.
* **HTML5 & CSS3:** Maquetación semántica y diseño responsivo de carga instantánea.

## Instalación y Compilación
1. Asegúrese de tener instalado el compilador `g++` (soporte para C++17).

2. Otorgue permisos de ejecución al script de construcción:
   ```bash
   chmod +x build.sh
   ```
3. Ejecute el script para compilar los componentes y el binario principal:
```bash
   ./build.sh
```

4. Inicie el servidor educativo::
```bash
   ./host.app 
```

5. Abrir en un navegador web la url:
```text
   http://localhost:8080/
```
## ⧉ Documentación de la API REST

El componente `HttpServer` expone los siguientes puntos de enlace (*endpoints*) para la interacción programática con la plataforma:

### 1. Autenticación
* **Ruta:** `POST /api/login`
* **Descripción:** Valida las credenciales de administración y retorna un token de sesión.
* **Cuerpo de la Petición (JSON):**
  ```json
  {
     "password": "admin_password"
  }

* **Respuesta Exitosa (200 OK)**
  ```json
   {
      "token": "educacion_libre_2026"
   }

### 2. Consulta de Recursos
* **Ruta:** `GET /api/contenidos`
* **Descripción:** Retorna el catálogo completo de materiales educativos registrados en la base de datos local.
* **Respuesta Exitosa (200 OK)**
  ```json
   {
   "contenidos": [
      {
         "titulo": "Álgebra Lineal",
         "autor": "Grossman",
         "tema": "Matemática",
         "file": "algebra.pdf"
      }
   ]
   }

### 3. Carga de Material Educativo (Ruta Protegida)
* **Ruta:** `POST /api/upload`
* **Descripción:** Sube un nuevo archivo al servidor y registra sus metadatos. El archivo físico debe enviarse en crudo (raw binary) en el cuerpo de la petición.
* **Cabeceras requeridas:** `Authorization: <token_de_administracion>`
* **Parámetros de URL (Query String):** `?titulo=<texto>&autor=<texto>&tema=<texto>&filename=<nombre_del_archivo.ext>`
* **Cuerpo de la Petición:** Secuencia binaria del archivo.
* **Respuesta Exitosa (201 Created)**
  ```json
   {
      "status": "success"
   }

### 4. Descarga de Material Educativo
* **Ruta:** `GET /recursos`
* **Descripción:** Despacha el contenido binario de un recurso educativo específico almacenado en el servidor.
* **Parámetros de URL (Query String):** `?f=<nombre_del_archivo.ext>`
* **Cuerpo de la Petición (JSON):** Cuerpo de la Petición: Secuencia binaria del archivo.
* **Respuesta Exitosa (200 OK)** Flujo binario (application/octet-stream o tipo MIME correspondiente) del archivo solicitado.

### 5. Borrado de Material Educativo (Ruta Protegida)
* **Ruta:** `DELETE /api/contenidos`
* **Descripción:** Elimina de forma permanente un recurso educativo. Realiza tanto el borrado lógico del registro en la base de datos SQLite como la eliminación física del archivo binario alojado en el disco.
* **Cabeceras Requeridas:**
  * `Authorization: <token_de_administracion>`
* **Parámetros de URL (Query String):** `?id=<id_numerico_del_recurso>`
* **Respuesta Exitosa (200 OK):**
  ```json
  {
     "status": "deleted"
  }
---

## ⧉ Estructura del Proyecto

```text
├── build.sh                 # Script de compilación automatizada
├── main.cpp                 # Punto de entrada y captura de fallos de infraestructura
├── include/
│   ├── application.hpp      # Orquestador lógico y enrutador de la API
│   ├── module_manager.hpp   # Cargador dinámico de bibliotecas compartidas
│   └── i_*.hpp              # Contratos e interfaces virtuales puras
├── src/
│   └── *_component.cpp      # Código fuente de los módulos dinámicos
├── lib/                     # Destino de los binarios dinámicos (.so / .dll)
├── data/                    
│   └── intraned.db          # Base de datos relacional local (SQLite)
├── uploads/                 # Almacenamiento físico aislado de los recursos educativos
├── vendor/                  # Bibliotecas de terceros
└── www/                     # Directorio de entrega de activos estáticos web