#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <string>

class Particle{
    public:

    glm::vec3 position;
    glm::vec3 old_position;
    glm::vec3 velocity;
    glm::vec3 normal;
    float inverse_mass;

    Particle(glm::vec3 position, glm::vec3 velocity, float inverse_mass): position(position), old_position(position), velocity(velocity), inverse_mass(inverse_mass){};
    Particle(glm::vec3 position, glm::vec3 velocity): position(position), velocity(velocity), old_position(position), inverse_mass(1.0f){};
    Particle(glm::vec3 position): position(position), velocity(glm::vec3(0.0f)), old_position(position), inverse_mass(1.0f){};
    Particle(){}

    // String Particledebug(){return }

    float getMass(){
        if(inverse_mass == 0.0f){
            return 1e10; // Infinite mass
        }
        return 1.0f/inverse_mass;
    }
    


    private:
};

#endif