# OpenGL Raycast Engine

A simple educational raycasting engine written in C++ using OpenGL and GLFW. This project demonstrates the fundamentals of 2D raycasting, grid-based collision, and basic 3D projection using modern OpenGL.

## Features
- 2D grid map rendering
- Player movement and rotation
- Raycasting for wall detection
- 3D projection view (classic Wolfenstein-style)
- Adjustable number of rays (slices)
- Clean, well-commented code for learning and extension

## Controls
- **W/A/S/D**: Move player forward/left/back/right
- **Left/Right Arrow**: Rotate player
- **ESC**: Exit

## Building & Running

### Prerequisites
- C++17 or newer
- [GLFW](https://www.glfw.org/)
- [GLAD](https://glad.dav1d.de/) (OpenGL loader)
- CMake (recommended)

### Build Instructions
1. Clone the repository:
   ```sh
   git clone https://github.com/PezKid/opengl-raycast.git
   cd opengl-raycast
   ```
2. Generate build files and build:
   ```sh
   mkdir build && cd build
   cmake ..
   make
   ```
3. Run the executable:
   ```sh
   ./opengl_raycast
   ```

## Project Structure
- `src/` - Main source code
- `include/` - Header files (GLFW, GLAD, KHR)
- `CMakeLists.txt` - Build configuration
- `build/` - Build output (after compilation)

## Customization
- Map layout and wall types can be edited in `src/main.cpp` (`mapArray`)
- Rendering and projection logic is modular and easy to extend
- Add your own textures, colors, or features for experimentation

## License
MIT License. See [LICENSE](LICENSE) for details.

## Credits
- Inspired by classic raycasting engines (Wolfenstein 3D, Doom)
- Uses [GLFW](https://www.glfw.org/) and [GLAD](https://glad.dav1d.de/)

---

Feel free to fork, modify, and use this project for your own learning or creative projects!
