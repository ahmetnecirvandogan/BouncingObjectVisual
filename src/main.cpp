#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Angel.h"
#include "PhysicsObject.h"
#include <vector>
#include <fstream>

//For background
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint program;
GLuint texture; // Texture ID
GLuint backgroundVAO, backgroundVBO; // VAO and VBO for background quad
double frameRate = 120;
double deltaTime = 1.0 / frameRate;
vec3 initialVelocity;

//For the Background
void loadTexture(const char* filename) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (nrChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else if (nrChannels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load texture" << std::endl;
        stbi_image_free(data);
    }
}float backgroundVertices[] = {
    // positions         // texture coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};
void setupBackground() {
    glGenVertexArrays(1, &backgroundVAO);
    glGenBuffers(1, &backgroundVBO);

    glBindVertexArray(backgroundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(backgroundVertices), backgroundVertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// About the scene
const int sceneWidth = 1200;
const int sceneHeight = 600;

// About the object
PhysicsObject bouncingObject;
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
std::vector<GLuint> indices_sphere; // Add indices vector

// setup for the bunny
std::vector<point4> vertices_bunny;
std::vector<GLuint> indices_bunny;
float bunnyScale = 0.02f; // Initial scale for the bunny

GLuint cubeVAO, cubeVBO, cubeIBO; // IBO for cube
GLuint sphereVAO, sphereVBO, sphereIBO; // IBO for sphere
GLuint bunnyVAO, bunnyVBO, bunnyIBO; // VAO, VBO, IBO for bunny

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
vec4 colorFirst(1.0, 0.0f, 0.0f, 1.0f);  // Red
vec4 colorSecond(0.6f, 0.3f, 0.0f, 1.0f);  // Brown
vec4 currentColor = colorFirst;
bool isRed = true;

GLuint vPosition;

// load the off file for the bunny
bool loadOFF(const std::string& filename, std::vector<point4>& vertices, std::vector<GLuint>& indices) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Can't open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    file >> line;
    if (line != "OFF") {
        std::cerr << "Not an OFF file\n";
        return false;
    }

    int numVertices, numFaces, numEdges;
    file >> numVertices >> numFaces >> numEdges;

    vertices.clear();
    indices.clear();

    for (int i = 0; i < numVertices; ++i) {
        float x, y, z;
        file >> x >> y >> z;
        vertices.emplace_back(x, y, z, 1.0f);
    }

    for (int i = 0; i < numFaces; ++i) {
        int n, v0, v1, v2;
        file >> n >> v0 >> v1 >> v2;
        if (n == 3) {
            indices.push_back(v0);
            indices.push_back(v1);
            indices.push_back(v2);
        }
        else {
            std::cerr << "Only triangle faces supported\n";
            return false;
        }
    }

    return true;
}

void bindObject(GLuint vPosition);
//----------------------------------------------------------------------------

// generation of the cube
int Index = 0;
void quad(int a, int b, int c, int d) {
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
void generateCube() {
    Index = 0;
    quad(1, 0, 3, 2);
    quad(2, 3, 7, 6);
    quad(3, 0, 4, 7);
    quad(6, 5, 1, 2);
    quad(4, 5, 6, 7);
    quad(5, 4, 0, 1);
}

// generation of the sphere
void generateSphere(float radius) {
    points_sphere.clear();
    indices_sphere.clear();

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

    for (int lat = 0; lat < latitudeBands; lat++) {
        for (int lon = 0; lon < longitudeBands; lon++) {
            int first = lat * (longitudeBands + 1) + lon;
            int second = first + longitudeBands + 1;

            indices_sphere.push_back(first);
            indices_sphere.push_back(second);
            indices_sphere.push_back(first + 1);

            indices_sphere.push_back(second);
            indices_sphere.push_back(second + 1);
            indices_sphere.push_back(first + 1);
        }
    }
}
//---------------------------------------------------------------------
//
// init
//

void init()
{
    //Background initialization
    loadTexture("toy-story-background.jpg");
    setupBackground();

    float objectSize = 0.5f;
    vec3 initPos = computeInitialPosition(objectSize);
    vec3 initVel(0.5f, 0.0f, 0.0f); // Set initial x-velocity here
    initialVelocity = initVel; // Store the initial velocity
    vec3 initAcc(0.0f);
    float initMass = 1.0f;
    bouncingObject = PhysicsObject(initPos, initVel, initAcc, initMass);
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
    projection = Ortho(left, right, bottom, top, -1.0, 1.0);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

//---------------------------------------------------------------------
//
// display
//
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int Axis = Yaxis;
GLfloat Theta[NumAxes] = { 0.0, 0.0, 0.0 };
float rotationSpeed = 50.0f;

void display(void) {

    // 1. Ensure Depth Testing is Enabled at the Start of Each Frame
    glEnable(GL_DEPTH_TEST);

    // 2. Set the Depth Function (Default is GL_LESS, but being explicit is good)
    glDepthFunc(GL_LESS);

    // 3. Clear Both Color and Depth Buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 4. Render Background
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLuint backgroundProgram = InitShader("background_vshader.glsl", "background_fshader.glsl");
    glUseProgram(backgroundProgram);
    glBindVertexArray(backgroundVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(backgroundProgram, "texture1"), 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // 5. Render the Physics Object (Cube, Sphere, or Bunny)
    glPolygonMode(GL_FRONT_AND_BACK, drawingMode);
    glUseProgram(program);
    bouncingObject.update(deltaTime);

    mat4 model_view = Translate(bouncingObject.position.x, bouncingObject.position.y, 0.1f) * 
        RotateY(Theta[Yaxis]) *
        RotateZ(Theta[Zaxis]);

    if (currentObject == CUBE) {
        float cubeScale = 1.0f;
        mat4 cube_model_view = model_view * Scale(cubeScale, cubeScale, cubeScale);
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, cube_model_view);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    }
    else if (currentObject == SPHERE) {
        float sphereScale = 0.48f;
        mat4 sphere_model_view = model_view * Scale(sphereScale, sphereScale, sphereScale);
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, sphere_model_view);
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIBO);
        glDrawElements(GL_TRIANGLES, indices_sphere.size(), GL_UNSIGNED_INT, 0);
    }
    else if (currentObject == BUNNY) {
        mat4 bunny_model_view = model_view * Scale(bunnyScale, bunnyScale, bunnyScale) * RotateZ(90) * RotateY(90);
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, bunny_model_view);
        glBindVertexArray(bunnyVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bunnyIBO);
        glDrawElements(GL_TRIANGLES, indices_bunny.size(), GL_UNSIGNED_INT, 0);
    }

    glFinish();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;
    switch (key) {
    case GLFW_KEY_ESCAPE: case GLFW_KEY_Q:
        exit(EXIT_SUCCESS);
        break;
    case GLFW_KEY_I:
    {
        float objectSize;
        if (currentObject == CUBE) {
            objectSize = 1.0f;
        }
        else if (currentObject == SPHERE) {
            objectSize = 0.48f;
        }
        else if (currentObject == BUNNY) {
            objectSize = bunnyScale;
        }


        vec3 pos = computeInitialPosition(objectSize);
        bouncingObject.position = pos;
        bouncingObject.velocity = initialVelocity; // Set velocity to the stored initial velocity
        break;
    }

    case GLFW_KEY_C:
        isRed = !isRed;
        currentColor = isRed ? colorFirst : colorSecond;
        glUseProgram(program);
        glUniform4fv(colorLocation, 1, currentColor);
        break;
    case GLFW_KEY_H:
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
            if (currentObject == CUBE) {
                currentObject = SPHERE;
            }
            else if (currentObject == SPHERE)
                currentObject = BUNNY;
            else
                currentObject = CUBE;

            bindObject(vPosition);
            glUseProgram(program);
            break;
        }
        case GLFW_MOUSE_BUTTON_LEFT:
        {
            drawingMode = (drawingMode == GL_LINE) ? GL_FILL : GL_LINE;
            glPolygonMode(GL_FRONT_AND_BACK, drawingMode);
            break;
        }
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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
        if (vao == sphereVAO) sphereIBO = ibo;
        if (vao == cubeVAO) cubeIBO = ibo;
        if (vao == bunnyVAO) bunnyIBO = ibo;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void bindObject(GLuint vPosition) {
    if (currentObject == CUBE) {
        generateCube();
        createAndBindBuffer(points_cube, sizeof(points_cube), cubeVAO, cubeVBO, vPosition);
    }
    else if (currentObject == SPHERE) {
        generateSphere(0.5f);
        createAndBindBuffer(points_sphere.data(), points_sphere.size() * sizeof(point4), sphereVAO, sphereVBO, vPosition, indices_sphere.data(), indices_sphere.size() * sizeof(GLuint));
    }
    else if (currentObject == BUNNY) {
        bool loaded = loadOFF("bunny.off", vertices_bunny, indices_bunny);
        std::cout << "Bunny loaded: " << loaded << ", "
            << vertices_bunny.size() << " vertices, "
            << indices_bunny.size() / 3 << " triangles\n";

        if (!loaded || vertices_bunny.empty()) return;

        // Center the bunny
        vec3 center(0.0f, 0.0f, 0.0f);
        for (const auto& v : vertices_bunny) {
            center.x += v.x;
            center.y += v.y;
            center.z += v.z;
        }
        if (!vertices_bunny.empty()) {
            center /= vertices_bunny.size();
            for (auto& v : vertices_bunny) {
                v.x -= center.x;
                v.y -= center.y;
                v.z -= center.z;
            }
        }

        glGenVertexArrays(1, &bunnyVAO);
        glBindVertexArray(bunnyVAO);

        glGenBuffers(1, &bunnyVBO);
        glBindBuffer(GL_ARRAY_BUFFER, bunnyVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices_bunny.size() * sizeof(point4), vertices_bunny.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(point4), (void*)0);
        glEnableVertexAttribArray(vPosition);

        glGenBuffers(1, &bunnyIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bunnyIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_bunny.size() * sizeof(GLuint), indices_bunny.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
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

    GLFWwindow* window = glfwCreateWindow(sceneWidth, sceneHeight, "Bouncing Objects", NULL, NULL);
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

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    init();

    double frameRate = 120.0;
    double currentTime, previousTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();


        double currentTime = glfwGetTime();
        double deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        Theta[Axis] += rotationSpeed * deltaTime;
        if (Theta[Axis] > 360.0f) Theta[Axis] -= 360.0f;
        else if (Theta[Axis] < 0.0f) Theta[Axis] += 360.0f;

        display();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}