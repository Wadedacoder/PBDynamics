#include "init.hpp"


namespace{
    // Private functions for init.cpp
    void glfw_error_callback(int error, const char* description)
    {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    // End OpenGL Context
    void cleanupImgui(){
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    // Start GLFW Window
    int setupImgui(GLFWwindow* window, std::string& glsl_version){
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // ImGuiIO& io = ImGui::GetIO(); (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version.c_str());

        return 0;
    }
    
}

int init::openGLInit(){
    // Setup window
    // glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize OpenGL/GLEW loader!\n");
        return -1;
    }
    // Enable multisampling
    glEnable(GL_MULTISAMPLE);

    // Enable Blending
    glEnable(GL_BLEND);
    return 0;
    
}

void init::cleanup(GLFWwindow* window){
    cleanupImgui();
    glfwDestroyWindow(window);
    glfwTerminate();
}

std::string init::setGLSLVersion(int major, int minor){
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    std::string glsl_version = "#version " + std::to_string(major) + std::to_string(minor) + "0 core";
    if(major >= 3){
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        if(minor >= 2 || major > 3)glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
    }
    return glsl_version;
}

GLFWwindow* init::setupWindow(int height , int width){
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()){
        exit(0);
    }
    std::string glsl_version = setGLSLVersion(3, 3);
    GLFWwindow* window = glfwCreateWindow(width, height, "PBDynamics", NULL, NULL);
    if (window == NULL){
        fprintf(stderr, "Failed to create GLFW window\n");
        exit(0);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    int status = init::openGLInit();
    if(!status){
        std::cout << "Initialized OpenGL succesfully " << std::endl;
    }
    std::cout<< "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    setupImgui(window, glsl_version);

    return window;

}



