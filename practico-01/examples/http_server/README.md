# <img src="logo/logo.png" width="100" height="100" align="center"> CPP HTTP Server Component

## Componente Servidor Web de ejemplo para estudiantes de ingeniería de software

## Estructura de directorios
* lib/ contiene los binarios dinámicos de infraestructura 
* www/ contiene los recursos estáticos de entrega de red. 
* Esta separación limpia permite que el Host permanezca agnóstico a la lógica visual, mientras que el componente http_server solo tiene que apuntar al directorio ./www/ para resolver las peticiones entrantes.
```
├── include/
│   ├── application.hpp
│   ├── i_component.hpp
│   ├── i_http_server.hpp
│   ├── http_types.hpp
│   ├── http_parser.hpp
│   ├── module_manager.hpp
│   └── shared_library.hpp
├── src/
│   └── http_server_component.cpp
├── lib/
│   └── http_server.so  # Biblioteca compartida compilada.
└── www/                # <--- Directorio de activos Web Estáticos.
    ├── index.html
    ├── style.css
    └── script.js
├── build.sh            # Script de compilación y ejecución.
├── main.cpp            # Punto de entrada (Host).
```