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

        static void solveStretchingConstraint(Particle& p0, Particle& p1, float rest_length, float stiffness);
};

#endif