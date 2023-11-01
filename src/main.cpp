#include "init.hpp"
#include "shader.hpp"

const glm::vec3 clearColor = glm::vec3(0.541f, 0.898f, 1.0f);

int main(){
    // Make a quick test to see if init.hpp and init.cpp are working
    int height = 600;
    int width = 800;
    GLFWwindow* window = init::setupWindow(height, width);
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);


    // Define a triangle
    std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f, // left
         0.5f, -0.5f, 0.0f, // right
         0.0f,  0.5f, 0.0f  // top
    };

    // Generate VAO and VBO for the triangle
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind the VAO and VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Copy the vertices data into the VBO
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // Add a shader
    Shader program("./shaders/shader.vs", "./shaders/shader.frag");

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // listen for a ctrl event
        if (io.KeyCtrl){
            std::cout << "ctrl pressed" << std::endl;
            //Close the window
            glfwSetWindowShouldClose(window, true);
        }
        program.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);


        // std::cout << "looping" << std::endl;

		glfwSwapBuffers(window);
        
    }

    init::cleanup(window);
    return 0;
}