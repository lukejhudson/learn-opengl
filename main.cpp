#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>

#include "shader.h"
#include "stb_image.h"
#include "camera.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadTexture(char const *fileName);

// Camera
static Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
// Positions of mouse last frame, initilised to centre of screen
static float lastX = 800.0f / 2.0f;
static float lastY = 600.0f / 2.0f;
// Skip first frame so camera doesn't jump when mouse is pulled into centre of screen
static bool firstMouse = true;

// Timing - keeps track of time between frames, so faster computers don't move faster than slower computers
static float deltaTime = 0.0f; // Time between current frame and last frame
static float lastFrame = 0.0f;

// Lighting
static glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main() {
    /* ----- GLFW & GLAD INIT ----- */
    // Initialise GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    /* Telling GLFW explicitly that we want to use the core-profile means we'll
     * get access to a smaller subset of OpenGL features (without backwards-compatible
     * features we no longer need) */
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window object
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Tell GLFW to make the context of our window the main context on the current thread
    glfwMakeContextCurrent(window);
    // Register resize function so GLFW calls it when the window is resized
    // Must register callback functions after we've created the window and before the game loop is initiated
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD manages function pointers for OpenGL so we want to initialize GLAD before we call any OpenGL function:
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    /* ----- CONFIGURE OPENGL STATE ----- */
    // Store all fragments in a depth buffer (that must be cleared after each frame)
    // that calculates whether a fragment is closest to the camera and only renders it if it is
    glEnable(GL_DEPTH_TEST);

    /* ----- SHADERS ----- */
    Shader shaderProgram("shader.vert", "shader.frag"); // Regular lighting shader
    Shader lampShader("lamp.vert", "lamp.frag"); // Shaders for lamp (pure white, unaffected by lighting)

    /* ----- VERTEX DATA AND BUFFERS ----- */
    /* OpenGL only processes 3D coordinates when they're in a specific range between -1.0 and 1.0 on all 3 axes (x, y and z)
     * All coordinates within this so called normalized device coordinates range will end up visible on your screen (and all coordinates outside this region won't) */
    // To render a single triangle we want to specify a total of three vertices with each vertex having a 3D position
    // (0, 0, 0) is in the centre of the screen
    // All z-coordinates are 0 --> triangle appears 2D
    float vertices[] = {
        // positions          // texture coords // normal directions
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f, 0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f, 0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f, 0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f, 0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f, 0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f, 0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f, 0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f, 0.0f,
    };
    // World space positions of each of the 10 cubes
    glm::vec3 cubePositions[] = {
      glm::vec3( 0.0f,  0.0f,  0.0f),
      glm::vec3( 2.0f,  5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f),
      glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3( 2.4f, -0.4f, -3.5f),
      glm::vec3(-1.7f,  3.0f, -7.5f),
      glm::vec3( 1.3f, -2.0f, -2.5f),
      glm::vec3( 1.5f,  2.0f, -2.5f),
      glm::vec3( 1.5f,  0.2f, -1.5f),
      glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    // Positions of the point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };

    // Break rectangle into two triangles
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    // Vertex buffer object (VBO)
    // Vertex array object (VAO) - Stores vertex attribute calls --> Only have to configure vertex attribute pointers once
    // Element buffer object (EBO) - Create shapes by listing stored coordinates, rather than repeating coordinates over and over
    // (e.g. 2 triangles that form a rectangle = 6 vertices total, but only 4 different vertices)
    unsigned int VBO, VAO, EBO;

    glGenVertexArrays(1, &VAO);
    // Generate a unique ID for the object
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind the VAO first, then bind and set vertex buffer(s), and then configure vertex attributes(s)
    glBindVertexArray(VAO);

    // Bind the newly created buffer to the GL_ARRAY_BUFFER
    // From this point on any buffer calls we make (on the GL_ARRAY_BUFFER target) will be used to configure the currently bound buffer, which is VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Copy the previously defined vertex data into the buffer's memory
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Storing elements of arrays here
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Tell OpenGL how to interpret the vertex buffer data
    // index, size, type, normalized, stride, offset
    // index      - Which vertex attribute we want to configure
    // size       - Each vertex has 3 values (vec3)
    // type       - The vertices are floats
    // normalized - Whether data needs to be clamped to range of -1 to 1
    // stride     - Space between consecutive vertex attributes
    // offset     - Offset of where the position data begins in the buffer (no offset in this case)
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
    // Enables the vertex attribute; 0 = location of vertex attribute
    //glEnableVertexAttribArray(0);

    // Position attribute
    // 8 * sizeof(float) bytes between each vertex coord
    // Position data starts at the beginning of the vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0)); // Vertex coords
    glEnableVertexAttribArray(0);
    // Texture coord attribute (2D coordinate, 2 floats)
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
    //glEnableVertexAttribArray(2);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float))); // Texture coords
    glEnableVertexAttribArray(1);
    // Normal direction attribute (3D direction vector, 3 floats)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(5 * sizeof(float))); // Normal directions
    glEnableVertexAttribArray(2);

    /* ----- LIGHTING ----- */
    // Configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    // We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it;
    // The VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Take vertex attributes from vertices array, same as for cubes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0)); // Vertex coords
    glEnableVertexAttribArray(0);

    /* ----- TEXTURES ----- */
    unsigned int diffuseMap = loadTexture("container2.png");
    unsigned int specularMap = loadTexture("container2_specular.png");


    // Note that this is allowed - the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    //glBindBuffer(GL_ARRAY_BUFFER, 0);


    // Activate our shader
    shaderProgram.use();
    // Tell OpenGL for each sampler to which texture unit it belongs to (only has to be done once)
    shaderProgram.setInt("material.diffuse", 0);
    shaderProgram.setInt("material.specular", 1);
/*
   Distance	Constant	Linear	Quadratic
    7       1.0         0.7     1.8
    13      1.0         0.35	0.44
    20      1.0         0.22	0.20
    32      1.0         0.14	0.07
    50      1.0         0.09	0.032
    65      1.0         0.07	0.017
    100     1.0         0.045	0.0075
    160     1.0         0.027	0.0028
    200     1.0         0.022	0.0019
    325     1.0         0.014	0.0007
    600     1.0         0.007	0.0002
    3250	1.0         0.0014	0.000007
*/
    shaderProgram.setFloat("light.constant", 1.0f);
    shaderProgram.setFloat("light.linear", 0.022f);
    shaderProgram.setFloat("light.quadratic", 0.0019f);

    /* ----- LIGHTING PROPERTIES ----- */
    // Directional light
    shaderProgram.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    shaderProgram.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    shaderProgram.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shaderProgram.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // Point light 1
    shaderProgram.setVec3("pointLights[0].position", pointLightPositions[0]);
    shaderProgram.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    shaderProgram.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shaderProgram.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shaderProgram.setFloat("pointLights[0].constant", 1.0f);
    shaderProgram.setFloat("pointLights[0].linear", 0.09f);
    shaderProgram.setFloat("pointLights[0].quadratic", 0.032f);
    // Point light 2
    shaderProgram.setVec3("pointLights[1].position", pointLightPositions[1]);
    shaderProgram.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    shaderProgram.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
    shaderProgram.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    shaderProgram.setFloat("pointLights[1].constant", 1.0f);
    shaderProgram.setFloat("pointLights[1].linear", 0.09f);
    shaderProgram.setFloat("pointLights[1].quadratic", 0.032f);
    // Point light 3
    shaderProgram.setVec3("pointLights[2].position", pointLightPositions[2]);
    shaderProgram.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
    shaderProgram.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
    shaderProgram.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    shaderProgram.setFloat("pointLights[2].constant", 1.0f);
    shaderProgram.setFloat("pointLights[2].linear", 0.09f);
    shaderProgram.setFloat("pointLights[2].quadratic", 0.032f);
    // Point light 4
    shaderProgram.setVec3("pointLights[3].position", pointLightPositions[3]);
    shaderProgram.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
    shaderProgram.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
    shaderProgram.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
    shaderProgram.setFloat("pointLights[3].constant", 1.0f);
    shaderProgram.setFloat("pointLights[3].linear", 0.09f);
    shaderProgram.setFloat("pointLights[3].quadratic", 0.032f);


    /* ----- RENDER LOOP ----- */
    while (!glfwWindowShouldClose(window)) { // Should the window be closed?
        // Calculate frame timings
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check for user inputs
        processInput(window);

        // Rendering commands
        // Clear the screen otherwise we would still see the results from the previous iteration
        // Set the colour we want to use
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // Clear the window with the above colour, and clears the depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Green value updated between 0 and 1 as time varies
        float timeValue = static_cast<float>(glfwGetTime());

        // Make lamp float around in a cirlce
        lightPos.x = sinf(timeValue) * 5;
        lightPos.z = cosf(timeValue) * 5;

        // Find and update uniform value in shader to update colour selection
        shaderProgram.use(); // Activate shader before setting objects
        shaderProgram.setVec3("viewPos", camera.Position);
//        shaderProgram.setVec3("light.position", lightPos);
        shaderProgram.setVec3("light.position",  camera.Position);
        shaderProgram.setVec3("light.direction", camera.Front);
        shaderProgram.setFloat("light.cutOff",   glm::cos(glm::radians(12.5f))); // cos since dot product produces the cosine of the angle between the two vectors, and cos^-1 is expensive
        shaderProgram.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f))); // Add some blur to the edge of the flashlight

        // Spot light properties
        shaderProgram.setVec3("spotLight.position", camera.Position);
        shaderProgram.setVec3("spotLight.direction", camera.Front);
        shaderProgram.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        shaderProgram.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        shaderProgram.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        shaderProgram.setFloat("spotLight.constant", 1.0f);
        shaderProgram.setFloat("spotLight.linear", 0.09f);
        shaderProgram.setFloat("spotLight.quadratic", 0.032f);
        shaderProgram.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        shaderProgram.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // Material properties
//        shaderProgram.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
//        shaderProgram.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
//        shaderProgram.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        shaderProgram.setFloat("material.shininess", 32.0f);

        // Transformations
        // Matrices to translate each object's local space coordinates to clip space coordinates
        //glm::mat4 model = glm::mat4(1.0f); // Local space to world space (single object to world of objects)
        glm::mat4 view = glm::mat4(1.0f); // World space to view space (apply camera view)
        glm::mat4 projection = glm::mat4(1.0f); // View space to clip space (apply perspective)

        // Move the camera backwards (actually move all objects away from camera)
        //view = glm::translate(view, glm::vec3(sinf(timeValue*0.5f), sinf(timeValue*2), -3.0f));
        //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        //view = glm::rotate(view, sinf(timeValue*1.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        view = camera.GetViewMatrix();

        // Create camera perspective (doesn't have to be done every loop since it very rarely changes)
        // fov, aspect ratio, distance to near plane (minimum distance), distance to far plane (maximum distance)
        projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);

        // Send matrices to shaders
        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("projection", projection); // Doesn't have to be set every loop

        // Get VBO's vertex data
        glBindVertexArray(VAO); // Don't really have to bind every time since only one VAO
        // Draw a triangle using all 3 vertices
        //glDrawArrays(GL_TRIANGLES, 0, 3);

        // Draw two triangles that make up a rectangle
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, reinterpret_cast<void*>(0));

//        // Draw top triangle in wireframe mode
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe mode
//        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
//        // Draw bottom triangle in fill mode
//        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, reinterpret_cast<void*>(3 * sizeof(unsigned int)));

        // Draw rectangle made from 2 triangles
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
        //glDrawArrays(GL_TRIANGLES, 0, 36);

        // Bind textures
        glActiveTexture(GL_TEXTURE0); // Activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1); // Activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, specularMap);

        // Draw all 10 cubes
        for (unsigned int i = 0; i < 10; i++) {
            // Create individual model matrix for each cube
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, timeValue * i, glm::vec3(1.0f, 0.3f, 0.5f));
            shaderProgram.setMat4("model", model);
            // Draw cube
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Draw the lamp object
        lampShader.use();
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);

        glBindVertexArray(lightVAO);
        for (unsigned int i = 0; i < 4; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make lamp smaller
            lampShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //glBindVertexArray(0); // Don't need to unbind every time

        // Display the current frame in the window
        glfwSwapBuffers(window);
        // Check if any IO events have triggered and act on them
        glfwPollEvents();
    }

    // Clean up all the resources and properly exit the application
    glfwTerminate();
    return 0;
}

// GLFW: Whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Adjust the size of the viewport with the given new sizes
    glViewport(0, 0, width, height);
}

// Process all keyboard input: Query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window) {
    // Check if ESCAPE key has been pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        // Esc --> Close window
        // (Set flag and the window will close in the next render loop)
        glfwSetWindowShouldClose(window, true);
    // WASD camera controls
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::UP, deltaTime);
}

// Process mouse movement; called whenever the mouse moves
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    float xposf = static_cast<float>(xpos);
    float yposf = static_cast<float>(ypos);

    // First frame --> Skip camera movement to avoid camera jumping
    if (firstMouse) {
        lastX = xposf;
        lastY = yposf;
        firstMouse = false;
    }
    float xoffset = xposf - lastX;
    float yoffset = lastY - yposf; // Reversed since y-coordinates go from bottom to top

    lastX = xposf;
    lastY = yposf;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Process mouse scrolls; called whenever the use scrolls the mouse wheel
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const *fileName) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Set the texture wrapping/filtering options (on the currently bound texture object)
    // Wrap modes for x and y (or s and t) axes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Minification and magnification filtering modes (how the colours of each pixel are chosen when the object size and texture size differ)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load and generate the texture
    int width, height, nrChannels;
    // Read texture image
    stbi_set_flip_vertically_on_load(true); // Flip images the correct way
    std::string path = "../learn-opengl/textures/";
    path.append(fileName);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        // Store texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        // Generate mipmaps - smaller versions of the texture for viewing small objects (avoids artifacts and is more efficient)
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture: " << path << std::endl;
    }
    // No longer need image data
    stbi_image_free(data);

    return textureID;
}






























