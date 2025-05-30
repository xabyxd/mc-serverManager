
# 📚 Guía de uso de la librería `generator`

Esta librería en C++ permite generar estructuras de directorios y archivos a partir de plantillas definidas en JSON.

---

## 📁 Estructura del proyecto

```
project/
├── main.cpp
├── generator/
│   ├── generator.hpp
│   ├── generator.cpp
│   └── templates/
│       ├── ejemplo.json
│       └── license_mit.txt
├── generator/
│   └── nlohmann/
│       └── json.hpp  <-- https://github.com/nlohmann/json
└── README_GENERATOR.md
```

---

## 🔧 Requisitos

- Compilador C++17 o superior
- `json.hpp` (incluido en `generator/nlohmann/json.hpp`, versión de [nlohmann/json](https://github.com/nlohmann/json))

---

## 🚀 Cómo compilar

### En Linux/macOS:

```bash
g++ -std=c++17 main.cpp generator/generator.cpp -o generador
./generador
```

### En Windows (con g++ de MinGW o WSL):

```bash
g++ -std=c++17 main.cpp generator/generator.cpp -o generador.exe
generador.exe
```

---

## 📦 Cómo usar la librería

### 1. Incluir la cabecera

```cpp
#include "generator/generator.hpp"
```

### 2. Cargar una plantilla desde JSON

```cpp
auto tpl = generator::Template::from_json("generator/templates/ejemplo.json");
```

### 3. Generar la estructura en un directorio

```cpp
tpl.generate("proyectos");
```

Esto creará una estructura como `proyectos/MiProyecto/`.

---

## 🧩 Ejemplo de archivo JSON (`ejemplo.json`)

```json
{
  "name": "MiProyecto",
  "directories": [
    "src",
    "include",
    "docs"
  ],
  "files": [
    {
      "path": "README.md",
      "content": "# MiProyecto\n\nEste es un proyecto base."
    },
    {
      "path": "src/main.cpp",
      "content": "// main\nint main() { return 0; }"
    },
    {
      "path": "LICENSE",
      "from_file": "generator/templates/license_mit.txt"
    }
  ]
}
```

---

## 📜 Alternativa: Crear plantillas en C++

```cpp
using namespace generator;

Template t("MiTemplate");
t.add_directory("src")
 .add_directory("include")
 .add_file("src/main.cpp", "// Hola mundo")
 .generate("proyectos");
```

---

## ❓¿Qué hace `.generate("proyectos")`?

Crea un directorio llamado `proyectos/MiTemplate/` con:

- Subdirectorios: `src`, `include`
- Archivos: `src/main.cpp` con contenido personalizado

---

## 📎 Licencia

Este proyecto usa licencia MIT. Puedes generar archivos de licencia fácilmente con `from_file`.

---

## 📬 Contacto

Para mejoras o sugerencias, crea un *pull request* o abre un *issue*.
