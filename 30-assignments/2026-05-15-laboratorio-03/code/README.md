# Laboratorio 03: Sistema de Carga Dinámica de Módulos en C++

Demostración educativa de un sistema de plugins/módulos en C++ utilizando
bibliotecas compartidas (`.so` / `.dll` / `.dylib`) cargadas en tiempo de
ejecución.

## Conceptos demostrados

- **Polimorfismo dinámico** con interfaces abstractas (`IComponent`, `IGreeter`)
- **RAII** aplicado al ciclo de vida de bibliotecas compartidas (`SharedLibrary`)
- **Carga dinámica** en runtime con `dlopen`/`dlsym` (Unix) y
  `LoadLibrary`/`GetProcAddress` (Windows)
- **Custom deleter** en `std::shared_ptr` para destrucción segura a través del
  límite del módulo
- **ABI segura** mediante buffers estilo C (`char*`) en lugar de `std::string`
  a través de bibliotecas compartidas
- **Patrón Factory** con `createComponent()` / `destroyComponent()` exportados
  como C-API (`extern "C"`)
- **Verificación de interfaz en runtime** con `dynamic_cast`

## Estructura del proyecto

```
code/
├── README.md
├── main.cpp                      # Punto de entrada: carga el módulo y usa el componente
├── build.sh                      # Script de compilación para Linux
├── build.bat                     # Script de compilación para Windows
├── flake.nix                     # Entorno de desarrollo reproducido con Nix
├── .envrc                        # Activación automática con direnv
├── .clang-format                 # Configuración de formato de código
├── .vscode/
│   ├── tasks.json                # Tareas de VS Code (build, run, clean)
│   └── launch.json               # Configuración de debug con gdb
├── include/
│   ├── IComponent.hpp            # Interfaz base + typedefs de punteros a función C-API
│   ├── IGreeter.hpp              # Interfaz derivada: greet()
│   ├── ModuleManager.hpp         # Gestor de módulos: carga, creación y deleter personalizado
│   └── SharedLibrary.hpp         # RAII wrapper de dlopen/dlclose (multiplataforma)
└── src/
    └── GreeterComponent.cpp      # Implementación concreta compilada como .so/.dll
```

## Requisitos previos

- Compilador `g++` con soporte para C++11 (o superior)
- `libdl` en Linux (incluida en `glibc`, generalmente ya instalada)

## Entorno de desarrollo reproducido con Nix + direnv

Este proyecto incluye un entorno de desarrollo reproducido vía **Nix flakes**
que provee todas las herramientas necesarias (`g++`, `gdb`, `valgrind`,
 `clangd`, `clang-format`, etc.) de forma aislada.

### Prerrequisitos

| Herramienta | Versión requerida | Instalación |
|---|---|---|
| [Nix](https://nixos.org/download) | ≥ 2.19 (flakes enabled) | `curl -fsSL https://nixos.org/nix/install | sh` |
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
> ```bash
> direnv reload
> ```

### Herramientas incluidas

| Categoría | Herramientas |
|---|---|
| Compilación | `g++` (GCC), `clang++`, `make`, `cmake` |
| Debug | `gdb`, `valgrind` |
| LSP & Linting | `clangd`, `clang-tidy`, `clang-format` |

## Compilar y ejecutar

> Si usás **direnv + Nix**, al entrar al directorio todas las herramientas
> (`g++`, `gdb`, `valgrind`, etc.) ya están en tu `PATH`. No necesitás
> instalar nada manualmente.

### Linux

```bash
# 1. Compilar el componente como biblioteca compartida
g++ -c -fPIC src/GreeterComponent.cpp -o GreeterComponent.o
g++ -shared -o libGreeter.so GreeterComponent.o

# 2. Compilar el ejecutable principal
g++ main.cpp -o app_main -ldl

# 3. Ejecutar
./app_main
```

O directamente:

```bash
chmod +x build.sh && ./build.sh
```

### Windows (MinGW)

```bat
g++ -c src\GreeterComponent.cpp -o GreeterComponent.o
g++ -shared -o libGreeter.dll GreeterComponent.o
g++ main.cpp -o app_main -ldl
app_main.exe
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
   - Obtiene los símbolos `createComponent` y `destroyComponent` mediante
     `dlsym`.
   - Invoca `createComponent()` (dentro de la DLL) para construir el objeto en
     el heap de la biblioteca.
   - Castea con `dynamic_cast` a la interfaz solicitada; si falla, destruye el
     objeto y retorna `nullptr`.
   - Envuelve el puntero en un `std::shared_ptr` con un **custom deleter** que
     captura la función `destroyComponent` y una copia del `shared_ptr<SharedLibrary>`.

3. El **custom deleter** garantiza que:
   - `destroyComponent()` se invoque en el heap correcto (el de la DLL).
   - La biblioteca compartida permanezca cargada mientras exista alguna
     instancia del componente.

4. Al salir del `main()`, el `shared_ptr` ejecuta el deleter, y luego el
   `ModuleManager` libera su referencia, permitiendo que `SharedLibrary` cierre
   la biblioteca con `dlclose`.

## Notas de diseño

| Decisión | Motivo |
|----------|--------|
| Buffers `char*` en la interfaz | `std::string` no tiene un ABI estable entre distintos compiladores o versiones; los tipos C son seguros a través de límites de módulos. |
| `extern "C"` en `createComponent` / `destroyComponent` | El *name mangling* de C++ es incompatible entre compiladores; `extern "C"` produce símbolos con nombre predecible. |
| RAII para `SharedLibrary` | Asegura que `dlclose` se llame incluso si hay excepciones; evita fugas de recursos. |
| Custom deleter en `shared_ptr` | El `delete` estándar usaría el heap equivocado. |

## Base de una arquitectura de componentes

Este proyecto ilustra los **mecanismos fundamentales** sobre los que se
sostiene una arquitectura de componentes:

- **Separación de interfaz e implementación** — el cliente programa contra
  `IGreeter` sin conocer `GreeterComponent`.
- **Despliegue independiente** — el componente se compila como una biblioteca
  separada y se carga en tiempo de ejecución.
- **Ciclo de vida gestionado** — el `ModuleManager` orquesta creación y
  destrucción, y el custom deleter garantiza limpieza en el heap correcto.
- **ABI estable** — las fronteras entre módulos se cruzan con tipos C
  (`extern "C"`, `char*`), no con tipos C++ que varían entre compiladores.
- **Factory pattern** — puntos de extensión bien definidos
  (`createComponent`/`destroyComponent`) para incorporar nuevos componentes.

Sobre esta base se pueden construir modelos más completos (OSGi, COM,
sistemas de plugins) agregando contenedor formal, registro de servicios,
inyección de dependencias, configuración externa, etc. Este proyecto se
queda en la **capa de infraestructura** para que resulte didáctica.

## Asignatura

Proyecto desarrollado para **Ingeniería de Software I** de la
**Tecnicatura Superior en Sistemas**.
