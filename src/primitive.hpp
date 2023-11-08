#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <glm/glm.hpp>
#include <vector>
#include <set>
#include "particle.hpp"
#include "engine.hpp"

#include <GL/glew.h>            // Initialize with glewInit()

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions

struct Triplet{
    // int indices
    unsigned int ind1;
    unsigned int ind2;
    float length;

    // Less than operator
    bool operator<(const Triplet& t) const{
        return !((ind1 > t.ind1) || (ind1 == t.ind1 && ind2 > t.ind2));
    }
};


class Primitive {
    public:
    Primitive(std::vector<Particle>& particles, std::vector<unsigned int>& indices): particles(particles), indices(indices){
        init();
    };

    ~Primitive(){
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    };

    void preUpdate(float delta_time, glm::vec3 ext_f); // dt is the time step in seconds; ext_f is the external force
    void solve();  // Solve the constraints
    void postUpdate(float delta_time); // dt is the time step in seconds
    void update(float delta_time, glm::vec3 ext_f); // Need to put this in engine

    void render();


    private:
        void setNewPos(unsigned int indice, glm::vec3 new_position);
        void init();
        void addStretchingConstraint(Triplet t);

        u_int32_t VAO;
        u_int32_t VBO;
        u_int32_t EBO;
        
        std::vector<Particle> particles;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;


        // Constraints
        std::set<Triplet> stretching_constraints;
        float stretching_stiffness = 0.3f;


};
#endif