# <img src="logo/logo_sin_fondo.png" width="100" height="37" align="center"> CPP Component Model

1. **Infraestructura (`SharedLibrary`, `ModuleManager`):** Encargada exclusivamente del ciclo de vida binario, mapeo de símbolos y validación estricta de versiones del ABI. Opera en el lado del Host y utiliza excepciones de C++ para notificaciones de fallos críticos críticas.
2. **Orquestación / Aplicación (`Application`):** Capa intermedia que consume la infraestructura y encapsula la lógica de negocio, protegiendo al `main()` de flujos de control complejos de bajo nivel.
3. **Componentes / Plugins (`IComponent`, `IGreeter`, `GreeterComponent`):** Implementaciones concretas distribuidas en binarios independientes que cruzan la frontera del ABI de forma segura.

---

## ⧉ Decisiones Técnicas Clave

### 1. Frontera del ABI Segura (C-API y `noexcept`)

Para evitar la incompatibilidad de nombres (*name mangling*) entre diferentes compiladores o versiones de la biblioteca estándar, toda exportación en el binario dinámico se realiza mediante una interfaz puramente en C utilizando `extern "C"`.

Adicionalmente, se marca cada función exportada con `noexcept`. Esto garantiza que si una excepción C++ no controlada intenta cruzar el límite del componente, el compilador invocará inmediatamente `std::terminate()`, impidiendo la corrupción impredecible de la pila (*stack corruption*) del Host.

### 2. Eliminación de Excepciones Cruzadas y Tipos C++ Estándar

La interfaz de negocio (`IGreeter`) evita pasar objetos complejos como `std::string` a través del ABI, ya que su diseño en memoria varía según el compilador. En su lugar, se utilizan buffers clásicos de C (`const char*`, `char*`, `size_t`).
Toda la lógica interna del componente se envuelve en bloques `try-catch (...)` genéricos, transformando cualquier fallo interno en códigos de retorno controlados extraídos del enum nativo `ComponentResult`.

### 3. Gestión Automática de Memoria Cruzada (RAII con Custom Deleters)

Para prevenir el clásico error de liberar memoria en el Host que fue reservada dentro de un módulo dinámico (con diferentes *heaps* de asignación), el ciclo de vida se controla mediante `std::shared_ptr`.
`ModuleManager` inyecta un *deleter personalizado* en el puntero inteligente que retiene una referencia al binario `SharedLibrary` cargado en memoria y despacha la destrucción explícita llamando a la función exportada `destroyComponent` del módulo. El binario dinámico nunca se descargará con `dlclose()` mientras exista una instancia activa en el Host.

### 4. Seguridad de Hilos y Concurrencia Primitiva

El mapa interno de bibliotecas cargadas (`loadedLibraries`) dentro del `ModuleManager` se encuentra protegido de accesos concurrentes asíncronos mediante exclusión mutua con `std::mutex` y bloqueos estructurados de tipo `std::lock_guard`. Esto garantiza la estabilidad del sistema si múltiples hilos del Host intentan instanciar o cargar componentes de forma simultánea.

### 5. Rechazo a `std::exit` en Infraestructura (Propagación vía Excepciones)

Se rechazó el uso de cortes abruptos mediante `std::exit(EXIT_FAILURE)` dentro del mánager de módulos. Invocar `std::exit` aborta el programa omitiendo el desenredo de la pila (*stack unwinding*), impidiendo la ejecución de destructores locales activos y dejando descriptores o recursos abiertos. En su lugar, los fallos de infraestructura se elevan limpiamente al Host mediante `std::runtime_error`, permitiendo una finalización ordenada y segura.

---

## ⧉ Componentes del Proyecto

* **`main.cpp`**: Punto de entrada declarativo y limpio de validaciones estructuradas. Atrapa errores fatales globales de infraestructura.
* **`IComponent.hpp`**: Define el contrato del ciclo de vida base del componente, la versión del ABI (`CURRENT_API_VERSION`) y los tipos de punteros a función de la C-API.
* **`IGreeter.hpp`**: Interfaz de negocio pura compatible con el ABI para la funcionalidad de saludo.
* **`SharedLibrary.hpp`**: Encapsulación RAII multiplataforma para las llamadas del sistema nativas (`dlopen`/`LoadLibrary`, `dlclose`/`FreeLibrary`).
* **`ModuleManager.hpp`**: Factoría genérica encargada de validar la compatibilidad binaria del ABI y resolver instancias polimórficas de forma segura.
* **`Application.hpp`**: Orquestador de la lógica de negocio del Host.
* **`GreeterComponent.cpp`**: Implementación de la funcionalidad del plugin y exportación explícita de las funciones factoría de la C-API.

## ⧉ Estructura de directorios

```text
├── include/
│   ├── Application.hpp
│   ├── IComponent.hpp
│   ├── IGreeter.hpp
│   ├── ModuleManager.hpp
│   ├── SharedLibrary.hpp
├── lib/
├── logo/
├── src/
│   ├── GreeterComponent.cpp
├── build.sh
├── build.bat
├── flake.nix
└── main.cpp
```

## Requisitos previos

* Compilador `g++` con soporte para C++11 (o superior)
* `libdl` en Linux (incluida en `glibc`, generalmente ya instalada)

## Entorno de desarrollo reproducido con Nix + direnv

Este proyecto incluye un entorno de desarrollo reproducido vía **Nix flakes**
que provee todas las herramientas necesarias (`g++`, `gdb`, `valgrind`,
 `clangd`, `clang-format`, etc.) de forma aislada.

### Prerrequisitos

| Herramienta | Versión requerida | Instalación |
|---|---|---|
| [Nix](https://nixos.org/download) | ≥ 2.19 (flakes enabled) | `curl -fsSL <https://nixos.org/nix/install> | sh` |
| [direnv](https://direnv.net) | ≥ 2.30 | `nix profile install nixpkgs#direnv` |
| [nix-direnv](https://github.com/nix-community/nix-direnv) | cualquier versión | Ver abajo |

### Hookear direnv en tu shell

Agregá esto a tu `~/.bashrc`, `~/.zshrc` o `~/.config/fish/config.fish`:

```bash
# Bash / Zsh
eval "$(direnv hook bash)"  # o zsh
```

```bash
# Fish
direnv hook fish | source
```

Luego instalá **nix-direnv** para que `use flake` use caché persistente:

```bash
mkdir -p ~/.config/direnv
echo 'source $HOME/.nix-profile/share/nix-direnv/direnvrc' > ~/.config/direnv/direnvrc
```

O si preferís usarlo como plugin de Nix:

```bash
nix profile install nixpkgs#nix-direnv
```

### Activar el entorno

```bash
cd code/
direnv allow      # Confiá en el .envrc del proyecto
```

Esto descarga y cachea todas las herramientas en el store de Nix.
A partir de ahora cada vez que entrés a `code/` el entorno se activa solo.

> 💡 Para recargar el entorno después de cambios en `flake.nix`:
>
> ```bash
> direnv reload
> ```

## ⧉ Compilación

Asegúrate de compilar utilizando el estándar C++17 o superior para soportar de forma nativa las características estructurales del código:

> Si usás **direnv + Nix**, al entrar al directorio todas las herramientas
> (`g++`, `gdb`, `valgrind`, etc.) ya están en tu `PATH`. No necesitás
> instalar nada manualmente.

### Linux

```bash
# 1. Compilar el Componente como una Biblioteca Compartida (Dynamic Shared Object)
# Usamos -fPIC (Position Independent Code) vital para bibliotecas compartidas en Linux
g++ -std=c++17 -c -fPIC src/GreeterComponent.cpp -o GreeterComponent.o
g++ -std=c++17 -shared -o lib/Greeter.so GreeterComponent.o

# 2. Compilar el Ejecutable Principal
# Necesitamos enlazar la biblioteca -ldl para poder usar dlopen, dlclose, dlsym en Linux
g++ -std=c++17 main.cpp -o hostApp.bin -ldl

# 3. Ejecutar la aplicación
./hostApp.bin
```

O directamente:

```bash
chmod +x build.sh && ./build.sh
```

### Windows (MinGW)

```bat
g++ -std=c++17 -c src\GreeterComponent.cpp -o GreeterComponent.o
g++ -std=c++17 -shared -o lib\Greeter.dll GreeterComponent.o
g++ -std=c++17 main.cpp -o hostApp.bin -ldl
hostApp.bin
```

O directamente:

```bat
build.bat
```

## Cómo funciona

1. **`ModuleManager::loadModule()`** crea un `SharedLibrary` que abre la
   biblioteca compartida con `dlopen` (RAII: se cierra automáticamente en el
   destructor).

2. **`ModuleManager::createInstance<IGreeter>()`**:
   * Obtiene los símbolos `createComponent` y `destroyComponent` mediante
     `dlsym`.
   * Invoca `createComponent()` (dentro de la DLL) para construir el objeto en
     el heap de la biblioteca.
   * Castea con `dynamic_cast` a la interfaz solicitada; si falla, destruye el
     objeto y retorna `nullptr`.
   * Envuelve el puntero en un `std::shared_ptr` con un **custom deleter** que
     captura la función `destroyComponent` y una copia del `shared_ptr<SharedLibrary>`.

3. El **custom deleter** garantiza que:
   * `destroyComponent()` se invoque en el heap correcto (el de la DLL).
   * La biblioteca compartida permanezca cargada mientras exista alguna
     instancia del componente.

4. Al salir del `main()`, el `shared_ptr` ejecuta el deleter, y luego el
   `ModuleManager` libera su referencia, permitiendo que `SharedLibrary` cierre
   la biblioteca con `dlclose`.

## Base de una arquitectura de componentes

Este proyecto ilustra los **mecanismos fundamentales** sobre los que se
sostiene una arquitectura de componentes:

* **Separación de interfaz e implementación** — el cliente programa contra
  `IGreeter` sin conocer `GreeterComponent`.
* **Despliegue independiente** — el componente se compila como una biblioteca
  separada y se carga en tiempo de ejecución.
* **Ciclo de vida gestionado** — el `ModuleManager` orquesta creación y
  destrucción, y el custom deleter garantiza limpieza en el heap correcto.
* **ABI estable** — las fronteras entre módulos se cruzan con tipos C
  (`extern "C"`, `char*`), no con tipos C++ que varían entre compiladores.
* **Factory pattern** — puntos de extensión bien definidos
  (`createComponent`/`destroyComponent`) para incorporar nuevos componentes.

Sobre esta base se pueden construir modelos más completos (OSGi, COM,
sistemas de plugins) agregando contenedor formal, registro de servicios,
inyección de dependencias, configuración externa, etc. Este proyecto se
queda en la **capa de infraestructura** para que resulte didáctica.

## Asignatura

Proyecto desarrollado para **Ingeniería de Software I** de la
**Tecnicatura Superior en Sistemas**.
