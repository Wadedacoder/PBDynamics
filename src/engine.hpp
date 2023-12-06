#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <glm/glm.hpp>
#include "particle.hpp"

// Debugging
#include <iostream>


class Engine{
    public:
        Engine(){};
        
        // TODO: MOVE UPDATE FROM PRIMITIVE TO ENGINE
        // void Update(float dt, glm::vec3 ext_f, std::vector<Primitive>& particles); 

        static void solveStretchingConstraint(Particle& p0, Particle& p1, float rest_length, float stiffness);
};

#endif