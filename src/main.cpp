#include "init.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "engine.hpp"
#include "particle.hpp"
#include "primitive.hpp"

#include "tiny_obj_loader.h"

const glm::vec3 clearColor = glm::vec3(0.541f, 0.898f, 1.0f);

// Window parameters
int height = 600;
int width = 800;

// Model, view, projection matrices
glm::mat4 model = glm::mat4(1.0f);

// Camera parameters
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // Camera position in world space
glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f); // Camera up vector
float Yaw = -90.0f; // Camera yaw
float Pitch = 0.0f; // Camera pitch
bool Perspective = true; // Camera projection type
Camera camera(cameraPos, Yaw, Pitch, Up, Perspective);
double lastX = width / 2.0f;
double lastY = height / 2.0f;
bool firstMouse = true;

// Physics parameters
float gravity = -.098f;
glm::mat4 geometricToPhysical{glm::mat4(1.0f)};

// Callback functions
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);


std::vector<Particle> particles;

// Define a Cube with 8 vertices
std::vector<float> cu_vertices = {
    // front
    -0.5f, -0.5f,  0.5f, // 0
    0.5f, -0.5f,  0.5f, // 1
    0.5f,  0.5f,  0.5f, // 2
    -0.5f,  0.5f,  0.5f, // 3
    // back
    -0.5f, -0.5f, -0.5f, // 4
    0.5f, -0.5f, -0.5f, // 5
    0.5f,  0.5f, -0.5f, // 6
    -0.5f,  0.5f, -0.5f, // 7
};

std::vector<glm::vec3> velocity_vector = {
    glm::vec3(0.0f, .2f, 0.0f), // 0
    glm::vec3(0.0f, .2f, 0.0f), // 1
    glm::vec3(0.0f, .2f, 0.0f), // 2
    glm::vec3(0.0f, .2f, 0.0f), // 3
    glm::vec3(0.0f, .2f, 0.0f), // 4
    glm::vec3(0.0f, .2f, 0.0f), // 5
    glm::vec3(0.0f, .2f, 0.0f), // 6
    glm::vec3(0.0f, .2f, 0.0f), // 7
};

std::vector<float> inverse_masses = {
    1.0f, // 0
    1.0f, // 1
    1.0f, // 2
    1.0f, // 3
    1.0f, // 4
    1.0f, // 5
    1.0f, // 6
    1.0f, // 7
};


int main(){

    // Load the obj file
// Load the obj file : cube.obj
    
    GLFWwindow* window = init::setupWindow(height, width);
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    // Set the mouse callback function
    glfwSetCursorPosCallback(window, mouse_callback);

    // Set the screen resizing callback function
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Define the vertices for a Ground Plane
    std::vector<float> g_vertices = {
        // front
        -0.5f, -0.5f,  0.5f, // 0
         0.5f, -0.5f,  0.5f, // 1
         0.5f,  0.5f,  0.5f, // 2
        -0.5f,  0.5f,  0.5f, // 3
        // back
        -0.5f, -0.5f, -0.5f, // 4
         0.5f, -0.5f, -0.5f, // 5
         0.5f,  0.5f, -0.5f, // 6
        -0.5f,  0.5f, -0.5f, // 7
    };

    // Define the indices for the cube
    std::vector<unsigned int> indices = {
        // front
        0, 1, 2,
        0, 2, 3,
        // right
        1, 5, 6,
        1, 6, 2,
        // back
        7, 6, 5,
        7, 5, 4,
        // left
        4, 0, 3,
        4, 3, 7,
        // bottom
        4, 5, 1,
        4, 1, 0,
        // top
        3, 2, 6,
        6, 7, 3
    };

// std::cout << "Loading the obj file" << std::endl;
//     std::string inputfile = "./models/cube.obj";
//     tinyobj::attrib_t attrib;
//     std::vector<tinyobj::shape_t> shapes;
//     std::string warn;
//     std::string err;
//     bool ret = tinyobj::LoadObj(&attrib, &shapes, NULL, &warn, &err, inputfile.c_str());
//     if (!warn.empty()) {
//         std::cout << warn << std::endl;
//     }
//     if (!err.empty()) {
//         std::cerr << err << std::endl;
//     }
//     if(!ret){
//         std::cout << "Failed to load the obj file" << std::endl;
//         return 1;
//     }
//     std::cout << "Loaded the obj file" << std::endl;

//     // Get the vertices and indices
//     std::vector<float> obj_vertices;
//     std::vector<unsigned int> obj_indices;
//     for (int j = 0; j < shapes[0].mesh.indices.size(); j++){
//         obj_indices.push_back(shapes[0].mesh.indices[j].vertex_index);
//     }
//     std::cout << "obj_indices.size(): " << obj_indices.size() << std::endl;
//     for (int i = 0; i < obj_indices.size(); i++){
//         obj_vertices.push_back(attrib.vertices[3 * obj_indices[i]]);
//         obj_vertices.push_back(attrib.vertices[3 * obj_indices[i] + 1]);
//         obj_vertices.push_back(attrib.vertices[3 * obj_indices[i] + 2]);
//     }

    // Translate the vertices to the +10 y
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::translate(rotation, glm::vec3(0.0f, 1.f, 0.f));
    rotation = glm::rotate(rotation, glm::radians(150.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // Apply the rotation to the vertices
    for (int i = 0; i < 8; i++){
        glm::vec4 temp = glm::vec4(cu_vertices[i * 3], cu_vertices[i * 3 + 1], cu_vertices[i * 3 + 2], 1.0f);
        temp = rotation * temp;
        cu_vertices[i * 3] = temp.x;
        cu_vertices[i * 3 + 1] = temp.y;
        cu_vertices[i * 3 + 2] = temp.z;
    }

    // std::cout << "obj_vertices.size(): " << cu_vertices.size() << std::endl;
    // Add the vertices to the particles
    for (int i = 0; i < cu_vertices.size(); i+=3){
        Particle p;
        p.position = glm::vec3(cu_vertices[i], cu_vertices[i+1], cu_vertices[i+2]);
        p.old_position = p.position;
        p.velocity = velocity_vector[i/3];
        p.inverse_mass = inverse_masses[i/3];
        particles.push_back(p);
    }

    // Make a primitive
    Primitive Cube = Primitive(particles, indices);

    // std::cout << "particles.size(): " << particles.size() << std::endl;

    // Generate VAO and VBO for the ground plane
    unsigned int g_VAO, g_VBO, g_EBO;
    glGenVertexArrays(1, &g_VAO);
    glGenBuffers(1, &g_VBO);
    glGenBuffers(1,&g_EBO);

    // Bind the VAO and VBO
    glBindVertexArray(g_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_VBO);

    // Copy the vertices data into the VBO
    glBufferData(GL_ARRAY_BUFFER, g_vertices.size() * sizeof(float), g_vertices.data(), GL_STATIC_DRAW);
    
    // Bind the EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_EBO);
    
    // Copy the indices data into the EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // Frametime
    float last_time = 0.0f;
    float deltaTime = 0.0f;

    // Add a shader
    Shader program("./shaders/shader.vert", "./shaders/shader.frag");
  
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    //Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);  
    int scale = 1;
    
    // Main loop
    while (!glfwWindowShouldClose(window))
    {   
        glfwPollEvents();

        // Process input
        camera.processKeyboardInput(window);

        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - last_time;
        last_time = currentFrame;

        Cube.update(deltaTime, glm::vec3(0.0f, gravity, 0.0f));
      
        // listen for a ctrl event
        if (io.KeyCtrl){
            std::cout << "ctrl pressed" << std::endl;
            //Close the window
            glfwSetWindowShouldClose(window, true);
        }

        // Render fps
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("FPS");
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::End();
        }
   

        // Rendering commands here
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Render the cube
        program.use();
        model = glm::mat4(1.0f);
        // model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        program.setMat4("model", model);
        program.setMat4("view", camera.getViewMatrix());
        program.setMat4("projection", camera.getProjectionMatrix(height, width));
        program.setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        Cube.render();

        //Make a ground plane with cube
        glBindVertexArray(g_VAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, .01f, 10.f));
        program.setMat4("model", model);
        program.setVec4("color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);


     
        // Render imgui
        ImGui::Render();
        // std::cout << "looping" << std::endl;
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
        
    }

    init::cleanup(window);
    return 0;
}


// Callback function for mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    if (firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;
    camera.processMouseInput(window, xoffset, yoffset);
}

// Callback function for screen resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    width = width;
    height = height;
    glViewport(0, 0, width, height);
}