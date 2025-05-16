# 🏀 Bouncing Object Simulation - COMP 410/510 Programming Assignment #1

**Witness the wonders of physics in motion! This OpenGL application simulates a bouncing ball (or cube) with user-controlled parameters and interactive elements.**

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT) ![Bouncing Ball Demo](path/to/your/cool_bouncing_ball_demo.gif) ## ✨ Project Overview

This OpenGL program simulates a bouncing ball (or cube) under the influence of gravity. The object bounces horizontally across the screen, and the user can interactively control various aspects of the simulation. This project is designed as an introduction to event-driven graphics programming using shader-based OpenGL and GLFW. [cite: 1]

## 🚀 Key Features

* **Simulates Bouncing Motion:** Accurately simulates the motion of a ball (or cube) bouncing under gravity, including decreasing velocity after each bounce. [cite: 1]
* **Object Selection:** Users can switch between drawing a cube or a sphere using the right mouse button. [cite: 1]
* **Drawing Mode Control:** Users can toggle between wireframe and solid drawing modes using the left mouse button. [cite: 1]
* **Color Switching:** Users can switch between two colors (of their choice) for the object using the 'c' key. [cite: 1]
* **Pose Initialization:** Users can reset the object's position to the top-left corner of the window using the 'i' key. [cite: 1]
* **Help Functionality:** Pressing the 'h' key displays a command-line help message explaining the input controls. [cite: 1]
* **Quit Option:** Users can exit the program by pressing the 'q' key. [cite: 1]
* **Reshape Handling:** The program correctly handles window reshape events to maintain proper display. [cite: 1]
* **Vertex Shader Transformations:** Object transformations and projection are implemented within the vertex shader. [cite: 1]
* **Orthographic Projection:** Uses default orthographic projection. [cite: 1]
* **Modern OpenGL:** Implemented using shader-based OpenGL, avoiding deprecated functions. [cite: 1]

## 🛠️ Getting Started - Let's Get Bouncing!

### Prerequisites

* A C/C++ compiler (e.g., g++, clang, Visual Studio).
* OpenGL libraries and headers.
* GLFW library.
* The provided `InitShader.cpp` and `mat.h` files (if applicable).

### Installation

1.  Clone the repository (if applicable):

    ```bash
    git clone [https://github.com/yourusername/yourrepo.git](https://github.com/yourusername/yourrepo.git)
    cd yourrepo
    ```
2.  Place the `InitShader.cpp` and `mat.h` files in your project directory (if applicable).

### Compilation

Compile the program using your C++ compiler, linking with the OpenGL and GLFW libraries. Example using g++:

```bash
g++ -o bouncing_object main.cpp InitShader.cpp -lGL -lglfw  # Adjust linker flags as needed
