#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Angel.h"
#include "PhysicsObject.h"

//
//  Display a rotating cube
//

PhysicsObject bouncingObject;


typedef vec4  color4;
typedef vec4  point4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4(-0.5, -0.5,  0.5, 1.0),
    point4(-0.5,  0.5,  0.5, 1.0),
    point4(0.5,  0.5,  0.5, 1.0),
    point4(0.5, -0.5,  0.5, 1.0),
    point4(-0.5, -0.5, -0.5, 1.0),
    point4(-0.5,  0.5, -0.5, 1.0),
    point4(0.5,  0.5, -0.5, 1.0),
    point4(0.5, -0.5, -0.5, 1.0)
};

// RGBA olors
color4 vertex_colors[8] = {
    color4(0.0, 0.0, 0.0, 1.0),  // black
    color4(1.0, 0.0, 0.0, 1.0),  // red
    color4(1.0, 1.0, 0.0, 1.0),  // yellow
    color4(0.0, 1.0, 0.0, 1.0),  // green
    color4(0.0, 0.0, 1.0, 1.0),  // blue
    color4(1.0, 0.0, 1.0, 1.0),  // magenta
    color4(1.0, 1.0, 1.0, 1.0),  // white
    color4(0.0, 1.0, 1.0, 1.0)   // cyan
};

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors to the vertices
int Index = 0;

void quad(int a, int b, int c, int d)
{
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[b]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[d]; points[Index] = vertices[d]; Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors

void colorcube()
{
    quad(1, 0, 3, 2);
    quad(2, 3, 7, 6);
    quad(3, 0, 4, 7);
    quad(6, 5, 1, 2);
    quad(4, 5, 6, 7);
    quad(5, 4, 0, 1);
}

//---------------------------------------------------------------------
//
// init
//

void init()
{
    // Load shaders and use the resulting shader program
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    colorcube(); // create the cube in terms of 6 faces each of which is made of two triangles

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation(program, "ModelView");
    Projection = glGetUniformLocation(program, "Projection");

    // Set projection matrix
    mat4  projection;
    projection = Ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0); // Ortho(): user-defined function in mat.h
    // Ortho() returns a 4x4 projection matrix, specifying the view volume (here you can change the view volume)
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection); // Send projection matrix to shader

    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

//---------------------------------------------------------------------
//
// display
//

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear both frame (color) buffer and depth buffer

    //  Generate the model-view matrix, which specifies a transformation to rotate, scale and then translate all the vertices.
        // RotateX, RotateY and RotateZ return 4x4 matrices for rotation around X, Y and Z axes, respectively. The order of transformation matrices matters.
        // Scale returns a 4x4 matrix to scale the geometry by the given factors along each axis.
        // Translate returns a 4x4 matrix to move the geometry by the given displacement vector.
    double frameRate = 120;
    double deltaTime = 1.0 / frameRate;
    bouncingObject.velocity.x = 0.5;
    bouncingObject.update(deltaTime);
    mat4  model_view = (Translate(bouncingObject.position) * Scale(0.2, 0.2, 0.2));  // Scale(), Translate(), RotateX(), RotateY(), RotateZ(): user-defined functions in mat.h

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // model_view matrix to shader
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    glFinish();
}

// Specify what to do when a keyboard event happens, i.e., when the user presses or releases a key
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key) {
    case GLFW_KEY_ESCAPE: case GLFW_KEY_Q:
        exit(EXIT_SUCCESS);
        break;
    }
}

// Specify what to do when a mouse event happens
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        switch (button) {
        case GLFW_MOUSE_BUTTON_RIGHT:    Axis = Xaxis;  break;
        case GLFW_MOUSE_BUTTON_MIDDLE:  Axis = Yaxis;  break;
        case GLFW_MOUSE_BUTTON_LEFT:   Axis = Zaxis;  break;
        }
    }
}

// Change the amount of rotation (from scratch) around the current axis of rotation
void update(void)
{
    Theta[Axis] += 4.0;

    if (Theta[Axis] > 360.0) {
        Theta[Axis] -= 360.0;
    }
}

//---------------------------------------------------------------------
//
// main
//

int main()
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1024, 1024, "Spin Cube", NULL, NULL);
    glfwMakeContextCurrent(window);


    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }


    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    //Specify which events to recognize and the callback functions to handle them
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    init();

    double frameRate = 120, currentTime, previousTime = 0.0;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        currentTime = glfwGetTime();
        if (currentTime - previousTime >= 1 / frameRate) {
            previousTime = currentTime;
            update();
        }

        display();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}




