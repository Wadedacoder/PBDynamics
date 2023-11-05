#include "init.hpp"
#include "shader.hpp"
#include "camera.hpp"

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
float gravity = .098f;
glm::mat4 geometricToPhysical{glm::mat4(1.0f)};

// Callback functions
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Update function for physics
void update(float deltaTime);

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
    5.0f, // 2
    5.0f, // 3
    1.0f, // 4
    1.0f, // 5
    1.0f, // 6
    1.0f, // 7
};

void solveStretchingConstraint(int ind1, int ind2, float length, float k);


int main(){

    // Rotate the cube by 45 degrees
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, glm::radians(150.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // Apply the rotation to the vertices
    for (int i = 0; i < 8; i++){
        glm::vec4 temp = glm::vec4(cu_vertices[i * 3], cu_vertices[i * 3 + 1], cu_vertices[i * 3 + 2], 1.0f);
        temp = rotation * temp;
        cu_vertices[i * 3] = temp.x;
        cu_vertices[i * 3 + 1] = temp.y;
        cu_vertices[i * 3 + 2] = temp.z;
    }

    // Compute the geometric to physical matrix
    // Let Bottom Left corner of the screen be (0, 0) and Top Right corner be (1, 1)
    geometricToPhysical = glm::mat4(1.0f);
    geometricToPhysical = glm::translate(geometricToPhysical, glm::vec3(1.0f, 1.0f, 1.0f));
    geometricToPhysical = glm::scale(geometricToPhysical, glm::vec3(.5f, .5f, .5f));


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
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
    };

    // Generate VAO and VBO for the cube
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1,&EBO);

    // Bind the VAO and VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Copy the vertices data into the VBO
    glBufferData(GL_ARRAY_BUFFER, cu_vertices.size() * sizeof(float), cu_vertices.data(), GL_STATIC_DRAW);

    // Bind the EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Copy the indices data into the EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

        // Update physics
        update(deltaTime);

        // Update the vertices for the cube
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, cu_vertices.size() * sizeof(float), cu_vertices.data(), GL_STATIC_DRAW);

      
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

        glBindVertexArray(VAO);

        // Render the cube
        program.use();
        model = glm::mat4(1.0f);
        // model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        program.setMat4("model", model);
        program.setMat4("view", camera.getViewMatrix());
        program.setMat4("projection", camera.getProjectionMatrix(height, width));
        program.setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        glDrawElements(GL_LINE_LOOP, indices.size(), GL_UNSIGNED_INT, 0);

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

// Update function for physics
void update(float deltaTime){
    // std::cout << "Updating physics" << std::endl;
    // Gonna do Position Based Dynamics
    // https://matthias-research.github.io/pages/publications/posBasedDyn.pdf
    // https://www.youtube.com/watch?v=7HKoqNJtMTQ
    // https://www.youtube.com/watch?v=Qupqu1xe7Io

    // For now, just gonna do a simple simulation of a cube falling on a plane
    // The cube will be a 1x1x1 cube with a mass of 1kg
    // The plane will be a 10x10 plane with a mass of 0kg

    // Update the velocity of the cube
    // std::cout << "Updating velocity" << std::endl;
    float a = deltaTime;
    int n_iter = 20;
    float k_f = 1.f;
    float k = 1 - pow(1 - k_f, 1.0f / n_iter);
            float diagonal = sqrt(2.0f);
    for(int i = 0; i < n_iter; i++){
        float deltaTime = a / n_iter;
        std::vector<float> cu_vertices_physical; // deep copy
        for(float i : cu_vertices){
            cu_vertices_physical.push_back(i);
        }
        for (int i = 0; i < 8; i++){
            velocity_vector[i].y -= gravity * deltaTime;
        }

        // Update the position of the cube
        // std::cout << "Updating position" << std::endl;
        for (int i = 0; i < 8; i++){
            cu_vertices[i * 3 + 1] += velocity_vector[i].y * deltaTime;
        }

        // Check for collision
        // std::cout << "Checking for collision" << std::endl;
        for (int i = 0; i < 8; i++){
            if (cu_vertices[i * 3 + 1] < -1.0f){
                std::cout << "Collision detected with vertice " << i << std::endl;
                cu_vertices[i * 3 + 1] = cu_vertices_physical[i * 3 + 1];
                // velocity_vector[i].y = 0.2f;
            }
        }
            // Add diagonal constraints 

        // Solve the stretching constraint between each triangle
        // std::cout << "Solving stretching constraint" << std::endl;
        // Solve the stretching constraint between each triangle
        // Front
            solveStretchingConstraint(0, 1, 1.0f, k);
            solveStretchingConstraint(1, 2, 1.0f, k);
            solveStretchingConstraint(2, 3, 1.0f, k);
            solveStretchingConstraint(3, 0, 1.0f, k);
            // Back
            solveStretchingConstraint(4, 5, 1.0f, k);
            solveStretchingConstraint(5, 6, 1.0f, k);
            solveStretchingConstraint(6, 7, 1.0f, k);
            solveStretchingConstraint(7, 4, 1.0f, k);
            // Left
            solveStretchingConstraint(4, 0, 1.0f, k);
            solveStretchingConstraint(7, 3, 1.0f, k);
            // Right
            solveStretchingConstraint(5, 1, 1.0f, k);
            solveStretchingConstraint(6, 2, 1.0f, k);


            // Front
            solveStretchingConstraint(0, 2, diagonal, k);
            // Back
            solveStretchingConstraint(1, 6, diagonal, k);
            // Left
            solveStretchingConstraint(4, 3, diagonal, k);
            // Right
            solveStretchingConstraint(5, 7, diagonal, k);
            // Top
            solveStretchingConstraint(2, 7, diagonal, k);
            // Bottom
            solveStretchingConstraint(0, 5, diagonal, k);
    
    // Compute velocity
    // std::cout << "Computing velocity" << std::endl;
        for (int i = 0; i < 8; i++){
            velocity_vector[i].x = (cu_vertices[i * 3] - cu_vertices_physical[i * 3]) / deltaTime;
            velocity_vector[i].y = (cu_vertices[i * 3 + 1] - cu_vertices_physical[i * 3 + 1]) / deltaTime;
            velocity_vector[i].z = (cu_vertices[i * 3 + 2] - cu_vertices_physical[i * 3 + 2]) / deltaTime;
        }
            for(int i = 0; i < 8; i++){
        std::cout << "Position of vertice " << i << ": (" << cu_vertices[i * 3] << ", " << cu_vertices[i * 3 + 1] << ", " << cu_vertices[i * 3 + 2] << ")\n";
    }
    }

}


void solveStretchingConstraint(int ind1, int ind2, float length, float k){
    glm::vec3 p0 = glm::vec3(cu_vertices[ind1 * 3], cu_vertices[ind1 * 3 + 1], cu_vertices[ind1 * 3 + 2]);
    glm::vec3 p1 = glm::vec3(cu_vertices[ind2 * 3], cu_vertices[ind2 * 3 + 1], cu_vertices[ind2 * 3 + 2]);
    float d = length;
    // The constraint is |p0 - p1| = d
    // We want to find the delta p0 and delta p1 that satisfies the constraint
    float len = glm::length(p0 - p1);
    float w0 = inverse_masses[ind1] / (inverse_masses[ind1] + inverse_masses[ind2]);
    float w1 = inverse_masses[ind2] / (inverse_masses[ind1] + inverse_masses[ind2]);
    glm::vec3 delta_p0 = -w0 * k *(len - d) * (p0 - p1) / len;
    glm::vec3 delta_p1 = w1 * k * (len - d) * (p0 - p1) / len;
    cu_vertices[ind1 * 3] += delta_p0.x;
    cu_vertices[ind1 * 3 + 1] += delta_p0.y;
    cu_vertices[ind1 * 3 + 2] += delta_p0.z;
    cu_vertices[ind2 * 3] += delta_p1.x;
    cu_vertices[ind2 * 3 + 1] += delta_p1.y;
    cu_vertices[ind2 * 3 + 2] += delta_p1.z;
}
