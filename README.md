# Yellow Mug - Overview

## Environment & Settings

- **Language:** C++23
- **Compiler Configuration:**
  - Clang/GCC (Linux) or MSVC (Windows).
  - C++23 standard required.
  - Modules enabled, [`std`](https://www.kitware.com/import-std-in-cmake-3-30/) module available.
- **Build System:** CMake (Version 3.28+) with Ninja generator.
- **External Dependencies:**
  - GLFW (Window management & OpenGL context)
  - Dear ImGui (Docking branch - UI rendering)
  - ImNodeFlow (Node graph interface)
  - imgui_zoomable_image (Image viewer widget)
  - stb_image (Image loading/decoding)
- **Custom Settings & Flags:**
  - Interprocedural Optimization (LTO) is enabled.
  - Release builds use `-ffast-math` and `-O3` (or `/O2` and `/fp:fast` on MSVC).
  - Dependencies are managed automatically via `FetchContent`.

## Build Instructions

The project relies on CMake Presets for simple and consistent builds:

```bash
# Configure the project using the release preset
cmake --preset release
# Alternatively use custom compiler
# cmake --preset release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

# Build the project
cmake --build build
```

Built executable should appear in the `build` directory as `yellow-mug` on Linux or `yellow-mug.exe` on Windows.
