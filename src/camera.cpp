#include "camera.hpp"

Camera::Camera(glm::vec3 cameraPos, float Yaw, float Pitch, glm::vec3 Up, bool Perspective){
    this->cameraPos = {cameraPos};
    this->Up = {Up};
    this->Perspective = Perspective;
    this->yaw = Yaw;
    this->pitch = Pitch;
    this->updateCameraDir();
}

glm::mat4 Camera::getViewMatrix(){
    // If Center is cameraDir then the camera is looking at a specific point rather than in a specific direction
    // std::cout << "Camera Direction: (" << this->cameraDir.x << " " << this->cameraDir.y << " " << this->cameraDir.z << ")\n";
    // std::cout << "Camera Position: (" << this->cameraPos.x << " " << this->cameraPos.y << " " << this->cameraPos.z << ")\n";
    return glm::lookAt(this->cameraPos, this->cameraPos + this->cameraDir, this->cameraUp);
}

glm::mat4 Camera::getProjectionMatrix(int height, int width){
    if (this->Perspective){
        return glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    }
    else{
        return glm::ortho(0.0f, (float)width, 0.0f, (float)height, 0.1f, 100.0f);
    }
}

void Camera::processKeyboardInput(GLFWwindow* window){
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - this->prevTime;
    this->prevTime = currentTime;
    float cameraSpeed = 1.f * deltaTime; // adjust accordingly
    // Camera movement
    // Move along the camera direction
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        this->cameraPos += cameraSpeed * this->cameraDir;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        this->cameraPos -= cameraSpeed * this->cameraDir;
    }
    // Move orthogonal to the camera direction
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        this->cameraPos -= cameraSpeed * this->cameraRight;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        this->cameraPos += cameraSpeed * this->cameraRight;
    }

    // Camera rotation
    // Rotate around the camera direction using mouse
}

void Camera::processMouseInput(GLFWwindow* window, double xpos, double ypos){
        
        xpos *= this->sensitivity;
        ypos *= this->sensitivity;

        this->yaw += xpos;
        this->pitch -= ypos;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (this->pitch > 89.0f){
            this->pitch = 89.0f;
        }
        if (this->pitch < -89.0f){
            this->pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Euler angles
        this->updateCameraDir();

}

void Camera::updateCameraDir(){
    glm::vec3 front;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    front.y = sin(glm::radians(this->pitch));
    front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    this->cameraDir = glm::normalize(front);
    this->cameraRight = glm::normalize(glm::cross(this->cameraDir, worldUp));
    this->cameraUp = glm::normalize(glm::cross(cameraRight, this->cameraDir));
}