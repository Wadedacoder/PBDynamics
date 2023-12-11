#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions

// Import GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

class Camera
{
    public:
        Camera(glm::vec3 cameraPos, float Yaw, float Pitch, glm::vec3 cameraUp, bool Perspective = true);
        glm::mat4 getViewMatrix();
        glm::mat4 getProjectionMatrix(int height, int width);
        void processKeyboardInput(GLFWwindow* window);
        void processMouseInput(GLFWwindow* window, double xpos, double ypos);
        glm::vec3 getCameraDir();
        std::pair<glm::vec3, glm::vec3> getRayPair(float xpos, float ypos);

        
    
    private:
        // Camera View
        glm::vec3 cameraPos;
        glm::vec3 cameraDir; // Gaze direction
        glm::vec3 cameraUp;
        glm::vec3 cameraRight;
        glm::vec3 Up;
        glm::mat4 view;
        float cameraSpeed = 0.05f; // adjust accordingly
        float prevTime = 0.0f;

        // Camera Rotation
        float sensitivity = 0.3f;
        float yaw = 0.0f;
        float pitch = 0.0f;
        int xpos = 0;
        int ypos = 0;
        bool movable = true; // Whether the camera is movable or not

        void updateCameraDir();

        // Camera Projection
        bool Perspective = true;
        glm::mat4 projection;
};

#endif