#include "engine.hpp"

void Engine::solveStretchingConstraint(Particle& p0, Particle& p1, float rest_length, float stiffness){
    // Calculate the distance between the two particles
    // std::cout << "Particle " << 0 << " position: " << p0.position.x << ", " << p0.position.y << ", " << p0.position.z << std::endl;

    float d = rest_length;
    float k = stiffness;
    // The constraint is |p0 - p1| = d
    // We want to find the delta p0 and delta p1 that satisfies the constraint
    float len = glm::length(p0.position - p1.position);
    if(len == 0) len = 0.0001f;
    float w0 = p0.inverse_mass / (p0.inverse_mass + p1.inverse_mass);
    float w1 = p1.inverse_mass / (p0.inverse_mass + p1.inverse_mass);
    glm::vec3 delta_p0 = -(w0 * k *(len - d)  / len ) * (p0.position - p1.position);
    glm::vec3 delta_p1 = (w1 * k *(len - d)  / len ) * (p0.position - p1.position);
    p0.position += delta_p0;
    p1.position += delta_p1;

}