#include "extern/imgui-docking/imgui.h"
#include "extern/imgui-docking/backends/imgui_impl_glfw.h"
#include "extern/imgui-docking/backends/imgui_impl_opengl3.h"
#include "Perlin.h"
#include "TextureGenerator.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// User input handling methods
void keyInput(GLFWwindow *window);

// Window and texture dimensions
const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 800, TEXTURE_WIDTH = 800, TEXTURE_HEIGHT = 800, TEXTURE_DEPTH = 1;

// Shader/buffer variables
unsigned int shaderProgram, VAO, VBO;

// Camera and control variables
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);  // Camera origin position (a vector in world space)
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);    // Camera front for moving foward (convention is move along -z-axis)
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);        // Camera up for moving upwards (convention is move along y-axis)
float previousX = (float)WINDOW_WIDTH / 2.0;             // To track previous mouse X position, initialized to center of horizontal axis
float previousY = (float)WINDOW_HEIGHT / 2.0;            // To track previous mouse Y position, initialized to center of vertical axis
float deltaTime = 0.0f;                                  // To track time between current frame and last frame for camera movement and fog animation
float previousFrame = 0.0f;                              // To store time of last frame 
float currentFrame = 0.0f;                               // To store time of current frame

// Fog variables (for user)
float density = 0.3f;
float fogSize = 0.1f;
float fogColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float geoColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
int selectedOctave = 0;
const char* octaveLabels[] = {"0", "4", "8", "16", "32"};
int octaveSteps[] = { 0, 4, 8, 16, 32 };
bool animationFlag = true;

// TextureGenerator pointer to access texture generation methods
TextureGenerator* Texture;

// Variables to store the textures in
unsigned int baseTexture, noiseTexture0, noiseTexture1, noiseTexture2, noiseTexture3;

// Function to read a file into a string
string readFile(const char* filePath) {
    string fileContent;
    ifstream fileStream(filePath, ios::in);

    if(!fileStream.is_open()) {
        cerr << "Error. Couldn't read file at " << filePath << ". File doesn't exist or incorrect path." << std::endl;
        return "";
    }

    string line = "";
    while(!fileStream.eof()) {
        getline(fileStream, line);
        fileContent.append(line + "\n");
    }

    fileStream.close();
    return fileContent;
}

// Sets up shaders, including the vertex shader, the fragment shader and the shader program that links the two
void shaders() {
    // Read in the glsl files for vertex and fragment shader, storing the strings
    const string vertexShaderSource = readFile("../shaders/vertexShader.glsl");
    const string fragmentShaderSource = readFile("../shaders/fragmentShader.glsl");

    // Vertex shader setup
    const char* vertexShaderSourcePtr = vertexShaderSource.c_str();        // Create a pointer that points to the contents of the vertex shader source code
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);          // Create vertex shader program
    glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, NULL);         // Load vertex shader source code into the vertex shader object
    glCompileShader(vertexShader);                                         // Compile shader source code

    // Fragment shader setup
    const char* fragmentShaderSourcePtr = fragmentShaderSource.c_str();    // Create a pointer that points to the contents of the fragment shader source code
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);      // Create fragment shader program
    glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, NULL);     // Load fragment shader source code into the fragment shader object
    glCompileShader(fragmentShader);                                       // Compile fragment source code

    // Shader setup, which combines vertex and fragment shaders
    shaderProgram = glCreateProgram();                                // Create a new shader program
    glAttachShader(shaderProgram, vertexShader);                      // Attach vertex shader
    glAttachShader(shaderProgram, fragmentShader);                    // Attach fragment shader
    glLinkProgram(shaderProgram);                                     // Link the vertex and fragment shaders
}

// Sets up OpenGL buffer objects: VAO (vertex array object), VBO (vertex buffer object), EBO (element buffer object), texCoordVBO (vertex buffer object for texture)
void bufferObjects() {
    // Set up vertex data for a cube
    float vertices[] = {
        // Vertices           // Texture coordinates
        // Back wall
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        
        // Front wall
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        // Left wall
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        // Right wall
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        // Bottom
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        // Top
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f

        // Note: Texture coordinates are not used in final rendering, but are kept to make it easy to add a texture in the future
    };

    // Set up buffers objects for vertex and index data
    glGenVertexArrays(1, &VAO);     // Generate a new VAO, which stores the configuration for how vertex data is stored/accessed
    glGenBuffers(1, &VBO);          // Generate a new VBO, which stores the actual vertex data

    glBindVertexArray(VAO);                                                          // Bind the VAO to the OpenGL context
    glBindBuffer(GL_ARRAY_BUFFER, VBO);                                              // Bind the VBO to the GL_ARRAY_BUFFER 
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);       // Copy the vertex data into it
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);    // Position attribute
    glEnableVertexAttribArray(0); // Enable the vertex attribute at index 0 (the verex position attribute aPos) to be used in the vertex shader during rendering

    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // Texture coordinate attribute
    // glEnableVertexAttribArray(1);  // Enable the vertex attribute at index 1 (the texture coordinate attribute textCoord) to be used in the vertex shader during rendering
}

// Sets up the textures
void textures() {
    // For blending, as Perlin noise has alpha channel for overlapping textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Tell OpenGL which shader program to use
    glUseProgram(shaderProgram);

    // BASE TEXTURE (2D)
    const float r = 0.0f, g = 0.0f, b = 200.0f; // Blue
    unsigned char* baseData = Texture->generateSolidTexture(TEXTURE_WIDTH, TEXTURE_HEIGHT, r, g, b); // Generate the solid-colored texture, storing a pointer to it in baseData
    glGenTextures(1, &baseTexture);                                   // Generate a new texture object, storing its identifier in noiseTexture0
    glBindTexture(GL_TEXTURE_2D, baseTexture);                        // Bind the texture object to the texture target GL_TEXTURE_2D so that any subsequent texture commends will be applied to this texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);     // Set texture wrapping parameters, indicating that the texture should be repeated when the texture coordinates extend beyond the range [0,1]
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Load the texture filtering parameters for the texture, indicating that linear interpolation should be used to filter the texture   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, baseData); // Set the texture unit index to the uniform variable
    glGenerateMipmap(GL_TEXTURE_2D);                                  // Generate a set of mipmaps (smaller version of texture) for performance improvement. Graphics hardware selects an appropriate level of detail based on viewer distance

    // NOISE TEXTURE 0 - 4 noise octaves
    const int octaves0 = 4;
    unsigned char* noiseData0 = Texture->generatePerlinTexture(TEXTURE_WIDTH, TEXTURE_HEIGHT, octaves0); // Generate the Perlin noise texture, storing a pointer to it in noiseTexture0
    glGenTextures(1, &noiseTexture0);                                   // Generate a new texture object, storing its identifier in noiseTexture0
    glBindTexture(GL_TEXTURE_3D, noiseTexture0);                        // Bind the texture object to the texture target GL_TEXTURE_2D so that any subsequent texture commends will be applied to this texture
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture wrapping parameters, indicating that the texture should be repeated when the texture coordinates extend beyond the range [0,1]
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // Load the texture filtering parameters for the texture, indicating that linear interpolation should be used to filter the texture   
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_DEPTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, noiseData0);  // Load the texture data
    glGenerateMipmap(GL_TEXTURE_3D);                                    // Generate a set of mipmaps (smaller version of texture) for performance improvement. Graphics hardware selects an appropriate level of detail based on viewer distance

    // NOISE TEXTURE 1 - 8 noise octaves
    const int octaves1 = 8;
    unsigned char* noiseData1 = Texture->generatePerlinTexture(TEXTURE_WIDTH, TEXTURE_HEIGHT, octaves1);
    glGenTextures(1, &noiseTexture1);
    glBindTexture(GL_TEXTURE_3D, noiseTexture1);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_DEPTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, noiseData1);
    glGenerateMipmap(GL_TEXTURE_3D);

    // NOISE TEXTURE 2 - 16 noise octaves
    const int octaves2 = 16;
    unsigned char* noiseData2 = Texture->generatePerlinTexture(TEXTURE_WIDTH, TEXTURE_HEIGHT, octaves2);
    glGenTextures(1, &noiseTexture2);
    glBindTexture(GL_TEXTURE_3D, noiseTexture2);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_DEPTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, noiseData2);
    glGenerateMipmap(GL_TEXTURE_3D);

    // NOISE TEXTURE 3 - 32 noise octaves
    const int octaves3 = 32;
    unsigned char* noiseData3 = Texture->generatePerlinTexture(TEXTURE_WIDTH, TEXTURE_HEIGHT, octaves3);
    glGenTextures(1, &noiseTexture3);
    glBindTexture(GL_TEXTURE_3D, noiseTexture3);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_DEPTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, noiseData3);
    glGenerateMipmap(GL_TEXTURE_3D);

    // Pass the textures to the fragment shader
    glUniform1i(glGetUniformLocation(shaderProgram, "baseTexture"), 0);   // Bind to TEXTURE0
    glUniform1i(glGetUniformLocation(shaderProgram, "noiseTexture0"), 1); // Bind to TEXTURE1
    glUniform1i(glGetUniformLocation(shaderProgram, "noiseTexture1"), 2); // Bind to TEXTURE2
    glUniform1i(glGetUniformLocation(shaderProgram, "noiseTexture2"), 3); // Bind to TEXTURE3
    glUniform1i(glGetUniformLocation(shaderProgram, "noiseTexture3"), 4); // Bind to TEXTURE4

    // Activate and bind the Perlin noise textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, noiseTexture0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, noiseTexture1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, noiseTexture2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, noiseTexture3);
    glActiveTexture(GL_TEXTURE4);
}

int main()
{
    // Initialize GLFW - GLFW used to open a window and connect to your OpenGL context
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "COMP 371 Project - Real-time Rendering of Heterogeneous Fog", NULL, NULL);
    if (!window)
    {
        cerr << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    // Make OpenGL context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        cerr << "Failed to initialize GLEW" << endl;
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // IMGUI SETUP
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    // Set up shaders: the vertex shader, the fragment shader and the shader program that links the two
    shaders();

    // Set up buffer objects: VAO, VBO
    bufferObjects();

    // Set up textures: base texture (solid color) and Perlin noise textures
    textures();

    // Enable depth testing for proper cube drawing (no see-through surfaces)
    glEnable(GL_DEPTH_TEST);

    // Define projection matrix outside of main loop as it never changes
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    unsigned int projectionLoc  = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Compute the new deltaTime value within the current frame, used for camera movements
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - previousFrame;
        previousFrame = currentFrame;

        // Handle keyboard input
        keyInput(window);

        // Create an IMGUI frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Clear the viewport color and depth buffer
        glClearColor(0.0, 0.0, 0.0, 0.0);                   // Set background color of rendering window to black
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the rendering window with the current clear color (black) and depth

        // Tell OpenGL to use shaderProgram for rendering
        glUseProgram(shaderProgram);

        // Create matrices/vectors and prepare to pass to shaders by getting appropriate uniform locations
        glm::mat4 model = glm::mat4(1.0f);  // Each cube has its own model matrix for appropriate scaling and translation
        glm::mat4 view = glm::mat4(1.0f);   // Shared by all geometry
        unsigned int viewLoc  = glGetUniformLocation(shaderProgram, "view");
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int animationLoc = glGetUniformLocation(shaderProgram, "animation");

        // Background boolean set to true as we're drawing front cubes
        bool background = false;
        glUniform1i(glGetUniformLocation(shaderProgram, "background"), background);

        // Define view matrix (camera position) and pass to shader 
        view  = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Define animation vector to pass to shader. The animation matrix is shared by all geometry in the scene, except the background
        glm::vec4 animation = glm::vec4((sin(currentFrame) * 0.02f),(cos(currentFrame) * 0.01f),(cos(currentFrame) * 0.009f),(sin(currentFrame) * 0.01f));
        glUniform4fv(animationLoc, 1, glm::value_ptr(animation));

        // CUBE0: Define model matrix and draw the cube
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        model = glm::translate(model, glm::vec3(-0.95f, 0.0f, -1.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6*6);

        // CUBE1: Define model matrix and draw the cube
        model = glm::scale(model, glm::vec3(0.9, 0.9, 0.9));
        model = glm::translate(model, glm::vec3(1.15f, 0.0f, -1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6*6);

        // CUBE2: Define model matrix and draw the cube
        model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
        model = glm::translate(model, glm::vec3(1.45f, 0.0f, -1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6*6);

        // CUBE3: Define model matrix and draw the cube
        model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
        model = glm::translate(model, glm::vec3(1.95f, 0.0f, -1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6*6);

        // Update background boolean and pass to shader, as we're now drawing the background plane
        background = true;
        glUniform1i(glGetUniformLocation(shaderProgram, "background"), background);

        // Update the model matrix for the background plane
        glm::mat4 modelBackground = glm::mat4(1.0f); 
        modelBackground = glm::scale(modelBackground, glm::vec3(15.0f, 15.0f, 1.0f));
        modelBackground = glm::translate(modelBackground, glm::vec3(0.0f, 0.0f, -14.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBackground));

        // Update the view matrix for the background plane (to keep it static by prevent it from moving with the camera)
        glm::mat4 viewBackground = glm::mat4(1.0f);
        viewBackground  = glm::translate(viewBackground, glm::vec3(0.0f, 0.0f, -3.0f)); 
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewBackground));
        
        // Update the animation matrix to reduce the animation intensity/speed
        glm::vec4 animationBackground = glm::vec4((sin(currentFrame) * 0.009f),(cos(currentFrame) * 0.01f),(cos(currentFrame) * 0.008f),(sin(currentFrame) * 0.009f));
        glUniform4fv(animationLoc, 1, glm::value_ptr(animationBackground));
        glDrawArrays(GL_TRIANGLES, 0, 6*2);

        // Render GUI for user controls
        ImGui::Begin("Controls");
        ImGui::SliderFloat("Fog Density", &density, 0.07f, 0.7f);
        ImGui::ColorEdit4("Fog Color", fogColor);
        ImGui::SliderFloat("Fog Size", &fogSize, 0.05f, 0.15);
        ImGui::ColorEdit4("Geometry Color", geoColor);
        ImGui::Combo("Number of Octaves", &selectedOctave, octaveLabels, IM_ARRAYSIZE(octaveLabels));
        int octaveVal =  octaveSteps[selectedOctave];
        ImGui::Checkbox("Animate", &animationFlag);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Pass variables to shaders
        glUniform1f(glGetUniformLocation(shaderProgram, "density"), density);
        glUniform1f(glGetUniformLocation(shaderProgram, "fogSize"), fogSize);
        glUniform4f(glGetUniformLocation(shaderProgram, "fogColor"), fogColor[0], fogColor[1], fogColor[2], fogColor[3]);
        glUniform4f(glGetUniformLocation(shaderProgram, "geoColor"), geoColor[0], geoColor[1], geoColor[2], geoColor[3]);
        glUniform1i(glGetUniformLocation(shaderProgram, "numOctaves"), octaveVal);
        glUniform1i(glGetUniformLocation(shaderProgram, "animationFlag"), animationFlag);

        // Swap buffers the back and front buffers
        glfwSwapBuffers(window);

        // Process all pending events (e.g. keyboard input)
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &noiseTexture0);
    glDeleteTextures(1, &noiseTexture1);
    glDeleteTextures(1, &noiseTexture2);
    glDeleteTextures(1, &noiseTexture3);
    glDeleteProgram(shaderProgram);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

// Used for processing key presses. Handles translation
void keyInput(GLFWwindow *window)
{
    // Camera moves at constant speed of 2.5 units per second
    float cameraSpeed = 2.5f * deltaTime;

    // W key and UP ARROW key shift camera backwards (so further from objects in scene)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * cameraFront;

    // S key and DOWN ARROW key shift camera forwards (so closer to objects in scene) 
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cameraPosition += cameraSpeed * cameraFront;

    // A key and LEFT ARROW key shift camera to the left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // D key and RIGHT ARROW key shift camera to the right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // ESC key press closes window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}