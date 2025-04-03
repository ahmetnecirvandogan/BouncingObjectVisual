#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Angel.h"
#include "PhysicsObject.h"
#include <vector>


GLuint program;

// About the scene
const int sceneWidth = 1200;
const int sceneHeight = 600;

// About the object
PhysicsObject bouncingObject;
float xSpeed = 0.2;
vec3 computeInitialPosition(float objectSize);

//drawing mode
GLenum drawingMode = GL_FILL;

// object mode
enum ObjectMode {
    CUBE,
    SPHERE,
    BUNNY
};

ObjectMode currentObject = CUBE;

// setup for cube
typedef vec4  point4;
const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
point4 points_cube[NumVertices];


// setup for sphere
const int latitudeBands = 50; //Divides the sphere along the vertical axis
const int longitudeBands = 50; //Divides the sphere along the horizontal axis

std::vector<point4> points_sphere;
std::vector<GLuint> indices; // Add indices vector

GLuint cubeVAO, cubeVBO, cubeIBO; // IBO for cube (if needed)
GLuint sphereVAO, sphereVBO, sphereIBO; // IBO for sphere


// Vertices of the cube
point4 cube_vertices[8] = {
    point4(-0.25, -0.25,  0.25, 1.0),
    point4(-0.25,  0.25,  0.25, 1.0),
    point4(0.25,  0.25,  0.25, 1.0),
    point4(0.25, -0.25,  0.25, 1.0),
    point4(-0.25, -0.25, -0.25, 1.0),
    point4(-0.25,  0.25, -0.25, 1.0),
    point4(0.25,  0.25, -0.25, 1.0),
    point4(0.25, -0.25, -0.25, 1.0)
};



// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;

// Color uniform location
GLuint colorLocation;
vec4 colorFirst(1.0, 0.0f, 0.0f, 1.0f);
vec4 colorSecond(0.0f, 0.0f, 1.0f, 1.0f);
vec4 currentColor = colorFirst;
bool isRed = true;

GLuint vPosition;

void bindObject(GLuint vPosition);
//----------------------------------------------------------------------------

// generation of the cube

// quad generates two triangles for each face and assigns colors to the vertices
int Index = 0;

void quad(int a, int b, int c, int d)
{
    if (Index + 6 > NumVertices) {
        std::cerr << "Error: Index out of bounds! Current Index: " << Index << std::endl;
        return;
    }
    points_cube[Index] = cube_vertices[a]; Index++;
    points_cube[Index] = cube_vertices[b]; Index++;
    points_cube[Index] = cube_vertices[c]; Index++;
    points_cube[Index] = cube_vertices[a]; Index++;
    points_cube[Index] = cube_vertices[c]; Index++;
    points_cube[Index] = cube_vertices[d]; Index++;
    
   
}

// generate 12 triangles: 36 vertices and 36 colors

void generateCube()
{
    quad(1, 0, 3, 2);
    quad(2, 3, 7, 6);
    quad(3, 0, 4, 7);
    quad(6, 5, 1, 2);
    quad(4, 5, 6, 7);
    quad(5, 4, 0, 1);
}

// generation of the sphere


void generateSphere(float radius) {
    points_sphere.clear(); // Clear previous data
    indices.clear();

    for (int lat = 0; lat <= latitudeBands; lat++) {
        float theta = lat * M_PI / latitudeBands;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= longitudeBands; lon++) {
            float phi = lon * 2 * M_PI / longitudeBands;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            points_sphere.push_back(point4(radius * x, radius * y, radius * z, 1.0));
        }
    }

    for (int lat = 0; lat < latitudeBands; lat++) { // Corrected loop
        for (int lon = 0; lon < longitudeBands; lon++) {
            int first = lat * (longitudeBands + 1) + lon;
            int second = first + longitudeBands + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}
//---------------------------------------------------------------------
//
// init
//

void init()
{
    float objectSize = 0.5f;
    vec3 initPos = computeInitialPosition(objectSize);
    bouncingObject.position = initPos;

    // Load shaders and use the resulting shader program
    program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // bind the object to the shader
    vPosition = glGetAttribLocation(program, "vPosition");
    bindObject(vPosition);  // pass it in

    // Retrieve color uniform variable locations
    colorLocation = glGetUniformLocation(program, "color");
    glUniform4fv(colorLocation, 1, currentColor);

    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation(program, "ModelView");
    Projection = glGetUniformLocation(program, "Projection");

    // Set projection matrix
    mat4  projection;

    float aspect = (float)sceneWidth / (float)sceneHeight;
    float viewHeight = 2.0f;
    float viewWidth = viewHeight * aspect;
    float top = viewHeight / 2.0f;
    float bottom = -top;
    float right = viewWidth / 2.0f;
    float left = -right;
    projection = Ortho(left, right, bottom, top, -1.0, 1.0); // Ortho(): user-defined function in mat.h
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection); // Send projection matrix to shader

   


    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

//---------------------------------------------------------------------
//
// display
//

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double frameRate = 120;
    double deltaTime = 1.0 / frameRate;
    bouncingObject.velocity.x = xSpeed;
    bouncingObject.update(deltaTime);
    mat4 model_view = (Translate(bouncingObject.position) * Scale(1.0, 1.0, 1.0));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

    if (currentObject == CUBE) {
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    } else if (currentObject == SPHERE) {
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIBO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); // Use glDrawElements
    }

    glFinish();
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // not to be triggered in key release situation
    if (action != GLFW_PRESS) return;
    switch (key) {
    case GLFW_KEY_ESCAPE: case GLFW_KEY_Q:
        exit(EXIT_SUCCESS);
        break;
    case GLFW_KEY_I:
    {
        float objectSize = 0.5f;
        vec3 pos = computeInitialPosition(objectSize);
        bouncingObject.position = pos;
        vec3 zerovec(0.0f);
        bouncingObject.velocity = zerovec;
        break;
    }

    case GLFW_KEY_C:
        // switch the color.
        isRed = !isRed;
        currentColor = isRed ? colorFirst : colorSecond;
        glUseProgram(program); // just in case
        glUniform4fv(colorLocation, 1, currentColor);
        break;
    case GLFW_KEY_H:
        // display the help commands
        std::cout << "i -- initialize the pose (top left corner of the window)\nc-- switch between two colors(of your choice), which is used to draw lines or triangles.\nh -- help; print explanation of your input control(simply to the command line)\nq -- quit(exit) the program" << std::endl;
        break;

    }
    
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        switch (button) {
       
        case GLFW_MOUSE_BUTTON_RIGHT:
        {
            // switch object type
            currentObject = (currentObject == CUBE) ? SPHERE : CUBE;
            
            
            
            bindObject(vPosition);
            glUseProgram(program);
            break;
        }
       
        case GLFW_MOUSE_BUTTON_LEFT:
        {
            // switch drawing type
            drawingMode = (drawingMode == GL_LINE) ? GL_FILL : GL_LINE;
            glPolygonMode(GL_FRONT_AND_BACK, drawingMode);
            //std::cout << "Drawing mode: " << (drawingMode == GL_LINE ? "Wireframe" : "Solid") << std::endl;

        }
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    init();

    double frameRate = 120, currentTime, previousTime = 0.0;
    while (!glfwWindowShouldClose(window))
    {
        
        
        glfwPollEvents();
        currentTime = glfwGetTime();
        if (currentTime - previousTime >= 1 / frameRate) {
            previousTime = currentTime;
            
        }
        display();
        glfwSwapBuffers(window);
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
    
    
}



// Simulate the movement again
vec3 computeInitialPosition(float objectSize)
{


    float aspect = (float)sceneWidth / (float)sceneHeight;
    float viewHeight = 2.0f;
    float viewWidth = viewHeight * aspect;
    float top = viewHeight / 2.0f;
    float bottom = -top;
    float right = viewWidth / 2.0f;
    float left = -right;


    float halfObjectSize = objectSize / 2.0f;


    vec3 initialPosition = vec3(left + halfObjectSize, top - halfObjectSize, 0.0f);
    return initialPosition;
}

void createAndBindBuffer(const void* data, size_t dataSize, GLuint& vao, GLuint& vbo, GLuint vPosition, const GLuint* indicesData = nullptr, size_t indicesSize = 0) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, dataSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, data);

    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(vPosition);

    if (indicesData) {
        GLuint ibo;
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indicesData, GL_STATIC_DRAW);
        if(vao==sphereVAO) sphereIBO = ibo;
        if(vao==cubeVAO) cubeIBO = ibo;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void bindObject(GLuint vPosition) {
    if (currentObject == CUBE) {
        generateCube();
        createAndBindBuffer(points_cube, sizeof(points_cube), cubeVAO, cubeVBO, vPosition); // No indices for cube in this case
    } else if (currentObject == SPHERE) {
        generateSphere(0.5f);
        createAndBindBuffer(points_sphere.data(), points_sphere.size() * sizeof(point4), sphereVAO, sphereVBO, vPosition, indices.data(), indices.size() * sizeof(GLuint)); // Added indices for sphere
    }
}

