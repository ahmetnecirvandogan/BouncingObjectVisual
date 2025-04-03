
// THIS CODE TRYING TO HANDLE BOTH SPHERE AND CUBE OBJECTS THEIR POSITION IS THE SAME

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Angel.h"
#include "PhysicsObject.h"

// About the scene
const int sceneWidth = 1200;
const int sceneHeight = 600;


typedef vec4  color4;
typedef vec4  point4;


// About the object
PhysicsObject bouncingObjectCube;
PhysicsObject bouncingObjectSphere;



//Sphere Properties
const int latitudeBands = 30; //Divides the sphere along the vertical axis
const int longitudeBands = 30; //Divides the sphere along the horizontal axis

std::vector<point4> points;
std::vector<color4> colors;
std::vector<GLuint> indices;

GLuint sphereVAO, sphereVBO;

void generateSphere(float radius) {
    for (int lat = 0; lat <= latitudeBands; lat++) {
        float theta = lat * M_PI / latitudeBands;  // Latitude angle
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= longitudeBands; lon++) {
            float phi = lon * 2 * M_PI / longitudeBands;  // Longitude angle
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            points.push_back(point4(radius * x, radius * y, radius * z, 1.0));

            // Set color to yellow for all vertices
            colors.push_back(color4(1.0, 1.0, 0.0, 1.0));  // Yellow color
        }
    }

    // Now we need to generate the indices for the triangles (proper sphere mesh)
    for (int lat = 0; lat < latitudeBands - 1; lat++) {
        for (int lon = 0; lon < longitudeBands - 1; lon++) {
            int first = lat * (longitudeBands + 1) + lon;
            int second = first + longitudeBands + 1;

            // Two triangles for each quad face of the sphere
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}





const int NumVertices_Cube = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points_cube[NumVertices_Cube];
color4 colors_cube[NumVertices_Cube];

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
    colors_cube[Index] = vertex_colors[a]; points_cube[Index] = vertices[a]; Index++;
    colors_cube[Index] = vertex_colors[b]; points_cube[Index] = vertices[b]; Index++;
    colors_cube[Index] = vertex_colors[c]; points_cube[Index] = vertices[c]; Index++;
    colors_cube[Index] = vertex_colors[a]; points_cube[Index] = vertices[a]; Index++;
    colors_cube[Index] = vertex_colors[c]; points_cube[Index] = vertices[c]; Index++;
    colors_cube[Index] = vertex_colors[d]; points_cube[Index] = vertices[d]; Index++;
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
    
    
    generateSphere(0.5f);
    
    // Generate VAO & VBO for sphere
    glGenVertexArrays(1, &sphereVAO);
    glBindVertexArray(sphereVAO);
    
    glGenBuffers(1, &sphereVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(point4) + colors.size() * sizeof(color4), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(point4), points.data());
    glBufferSubData(GL_ARRAY_BUFFER, points.size() * sizeof(point4), colors.size() * sizeof(color4), colors.data());
    
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    

    
    

    colorcube(); // create the cube in terms of 6 faces each of which is made of two triangles

    // Create a vertex array object
    GLuint cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    // Create and initialize a buffer object
    GLuint cubeVBO;
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW);

    
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices_Cube);
    glBindVertexArray(0);

 
 
 
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
    float aspect = (float)sceneWidth / (float)sceneHeight;
    float viewHeight = 2.0f;
    float viewWidth = viewHeight * aspect;
    // we do ths not to have the aspect same as the scene
    float top = viewHeight / 2.0f;     // = 1.0
    float bottom = -top;               // = -1.0
    float right = viewWidth / 2.0f;    // = aspect
    float left = -right;               // = -aspect

  
    projection = Ortho(left, right, bottom, top, -10.0, 10.0); // Ortho(): user-defined function in mat.h
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection); // Send projection matrix to shader

    // Create and bind the index buffer
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    

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
    bouncingObjectCube.velocity.x = 0.5;
    bouncingObjectCube.update(deltaTime);
    
    bouncingObjectSphere.velocity.x = 0.5;
    bouncingObjectSphere.update(deltaTime);
    
    std::cout << "Cube position: " << bouncingObjectCube.position << std::endl;
    std::cout << "Sphere position: " << bouncingObjectSphere.position << std::endl;


    
    // Render Cube
    mat4 cube_model = Translate(bouncingObjectCube.position) * Scale(0.2, 0.2, 0.2);
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, cube_model);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices_Cube);
     

    // Render Sphere
    mat4 sphere_model = Translate(bouncingObjectSphere.position) * Scale(0.3, 0.3, 0.3);
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, sphere_model);

    // Draw using the index buffer
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
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

    GLFWwindow* window = glfwCreateWindow(sceneWidth, sceneHeight, "Spin Cube", NULL, NULL);
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

 



// ONLY THE SPHERE (WORKING PROPERLY)


/*

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "Angel.h"
#include "PhysicsObject.h"

const int sceneWidth = 1200;
const int sceneHeight = 600;
const int latitudeBands = 20;
const int longitudeBands = 20;

PhysicsObject bouncingObject;
typedef vec4 color4;
typedef vec4 point4;

std::vector<point4> points;
std::vector<color4> colors;
std::vector<GLuint> indices;

void generateSphere(float radius) {
    for (int lat = 0; lat <= latitudeBands; lat++) {
        float theta = lat * M_PI / latitudeBands;  // Latitude angle
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= longitudeBands; lon++) {
            float phi = lon * 2 * M_PI / longitudeBands;  // Longitude angle
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            points.push_back(point4(radius * x, radius * y, radius * z, 1.0));

            // Set color to yellow for all vertices
            colors.push_back(color4(1.0, 1.0, 0.0, 1.0));  // Yellow color
        }
    }

    // Now we need to generate the indices for the triangles (proper sphere mesh)
    for (int lat = 0; lat < latitudeBands - 1; lat++) {
        for (int lon = 0; lon < longitudeBands - 1; lon++) {
            int first = lat * (longitudeBands + 1) + lon;
            int second = first + longitudeBands + 1;

            // Two triangles for each quad face of the sphere
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}



GLuint ModelView, Projection;
void init() {
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);
    generateSphere(0.5f);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(point4) + colors.size() * sizeof(color4), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(point4), points.data());
    glBufferSubData(GL_ARRAY_BUFFER, points.size() * sizeof(point4), colors.size() * sizeof(color4), colors.data());

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(points.size() * sizeof(point4)));

    ModelView = glGetUniformLocation(program, "ModelView");
    Projection = glGetUniformLocation(program, "Projection");

    mat4 projection = Ortho(-1.2, 1.2, -0.6, 0.6, -1.0, 1.0);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

    // Create and bind the index buffer
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);  // Set background to black for better contrast
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bouncingObject.update(1.0 / 120);
    mat4 model_view = Translate(bouncingObject.position) * Scale(0.2, 0.2, 0.2);
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
    
    // Draw using the index buffer
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glFinish();
}


int main() {
    if (!glfwInit()) exit(EXIT_FAILURE);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(sceneWidth, sceneHeight, "Bouncing Sphere", NULL, NULL);
    glfwMakeContextCurrent(window);
    
    if (!window) { glfwTerminate(); exit(EXIT_FAILURE); }
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "GLEW initialization failed" << std::endl; exit(EXIT_FAILURE); }
    
    init();
    
    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
        display();
        glfwSwapBuffers(window);
        
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


*/
