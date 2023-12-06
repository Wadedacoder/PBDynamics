#include "init.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "engine.hpp"
#include "particle.hpp"
#include "primitive.hpp"
#include "obj.hpp"

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
float delta_time = 0.0166f; // 60 fps
glm::mat4 geometricToPhysical{glm::mat4(1.0f)};

// Callback functions
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);


Primitive makeCube(glm::mat4 inital_trans, float inverse_mass = 1.0f, float stretching_coeff = 1.0f){

    // Get cube vertices
    std::vector<float> vertices = {
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

    // Apply the transformation to the vertices
    for(int i = 0; i < vertices.size(); i+=3){
        glm::vec4 temp = glm::vec4(vertices[i], vertices[i+1], vertices[i+2], 1.0f);
        temp = inital_trans * temp;
        vertices[i] = temp.x;
        vertices[i+1] = temp.y;
        vertices[i+2] = temp.z;
    }

    // Create the particles
    std::vector<Particle> particles;
    for (int i = 0; i < vertices.size(); i+=3){
        Particle p;
        p.position = glm::vec3(vertices[i], vertices[i+1], vertices[i+2]);
        p.old_position = p.position;
        p.velocity = glm::vec3(0.0f, 0.f, 0.0f);
        p.inverse_mass = inverse_mass;
        particles.push_back(p);

    }

    // Make a primitive
    Primitive Cube = Primitive(particles, indices, stretching_coeff, true, 0.0f, false);

    return Cube;
}

int main(){


    
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

// std::cout << "Loading the obj file" << std::endl;
//     std::string inputfile = "./models/bunny.obj";
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

//     // Make the vertices
//     std::vector<float> obj_vertices;
//     for (int i = 0; i < attrib.vertices.size(); i++){
//         obj_vertices.push_back(attrib.vertices[i]);
//     }
//     // Make the indices
//     std::vector<unsigned int> obj_indices;
//     for (int i = 0; i < shapes[0].mesh.indices.size(); i++){
//         obj_indices.push_back(shapes[0].mesh.indices[i].vertex_index);
//     }

//     // Make the box from obj.hpp
//     std::vector<Particle> particles;
//     for(int i = 0; i < obj_vertices.size(); i+=3){
//         Particle p;
//         p.position = glm::vec3(obj_vertices[i], obj_vertices[i+1], obj_vertices[i+2]);
//         p.old_position = p.position;
//         p.velocity = glm::vec3(0.0f, 0.f, 0.0f);
//         p.inverse_mass = 1.f;
//         particles.push_back(p);
//     }

//     Primitive Box = Primitive(particles, obj_indices, 1.f);

    // Make the cloth from obj.hpp
    std::vector<Particle> particles_cloth;
    for(int i = 0; i < cloth_vertices.size(); i+=3){
        Particle p;
        p.position = glm::vec3(cloth_vertices[i], cloth_vertices[i+1], cloth_vertices[i+2]);
        p.old_position = p.position;
        p.velocity = glm::vec3(0.1f, 0.f, 0.0f);
        p.inverse_mass = 1.f;
        particles_cloth.push_back(p);
    }

    // Fix the top left and top right corners
    particles_cloth[0].inverse_mass = 0.0f;
    particles_cloth[0].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    // particles_cloth[9].inverse_mass = 0.0f;

    // Find the top right corner 
    int top_right = 0;
    for(int i = 0; i < particles_cloth.size(); i++){
        if(particles_cloth[i].position.x > particles_cloth[top_right].position.x){
            top_right = i;
        }
        if(particles_cloth[i].position.x == particles_cloth[top_right].position.x && particles_cloth[i].position.z > particles_cloth[top_right].position.z){
            top_right = i;
        }
    }
    particles_cloth[top_right].inverse_mass = 0.0f;
    particles_cloth[top_right].velocity = glm::vec3(0.0f, 0.0f, 0.0f);

    Primitive Cloth = Primitive(particles_cloth, cloth_indices, .3f);

    // Translate the vertices to the +10 y
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::translate(rotation, glm::vec3(0.0f, 1.f, 0.f));
    rotation = glm::rotate(rotation, glm::radians(150.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // Make a primitive
    // Primitive Cube = Primitive(particles, indices);
    Primitive Cube = makeCube(rotation, 1.0f, 1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, .01f, 10.f));
    Primitive Ground = makeCube(model, 0.0f, 0.0f);
    // std::cout << "particles.size(): " << particles.size() << std::endl;

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
    
    bool start_sim = false;

    int substeps = 5;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {   
        glfwPollEvents();

        // If the spacebar is pressed, start the simulation
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
            start_sim = true;
            std::cout << "Spacebar pressed" << std::endl;
        }

        // Process input
        camera.processKeyboardInput(window);

        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = 0.0166;
        last_time = currentFrame;

        // Add substeps
        if(start_sim){
            float substep_dt = deltaTime / substeps;
            for (int i = 0; i < substeps; i++){
                // Cube.update(substep_dt, glm::vec3(0.0f, gravity, 0.0f));
                // Ground.update(substep_dt, glm::vec3(0.0f, gravity, 0.0f));
                Cloth.update(substep_dt, glm::vec3(0.0f, gravity, 0.0f));
                // Box.update(substep_dt, glm::vec3(0.0f, gravity, 0.0f));
            }
            // Cube.update(deltaTime, glm::vec3(0.0f, gravity, 0.0f));
        }
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

                // Add a slider for substeps
            // ImGui::Begin("Substeps");
            ImGui::SliderInt("Substeps", &substeps, 1, 10);
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
        // Cube.render();
        Cloth.render();
        // Box.render();

        //Make a ground plane with cube
        // glBindVertexArray(g_VAO);
        model = glm::mat4(1.0f);

        program.setMat4("model", model);
        program.setVec4("color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        // glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        Ground.render();

     
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