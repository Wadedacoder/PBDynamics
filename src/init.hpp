#include <math.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Import GLM
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>            // Initialize with glewInit()

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions

namespace init{ 
int openGLInit();
std::string setGLSLVersion(int major, int minor);
void cleanup(GLFWwindow*);

GLFWwindow* setupWindow(int height , int width);

}

