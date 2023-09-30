#include <math.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>            // Initialize with glewInit()

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions

namespace init{ 
int openGLInit();
std::string setGLSLVersion(int major, int minor);
void cleanup(GLFWwindow*);

GLFWwindow* setupWindow(int height , int width);

}

