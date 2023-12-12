#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <glm/glm.hpp>
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
        if(ind1 == t.ind1) return ind2 < t.ind2;
        return ind1 < t.ind1;
    }
};


class Primitive {
    public:
    Primitive(std::vector<Particle>& particles, std::vector<unsigned int>& indices, float stretching_stiffness = .1f, bool haveStretching = true, float bending_stiffness = .1f, bool haveBending = true) : stretching_stiffness(stretching_stiffness), bending_stiffness(bending_stiffness), haveStreching(haveStretching), haveBending(haveBending){
        for(int i = 0; i < particles.size(); i++){
            this->particles.push_back(particles[i]);
        }
        for(int i = 0; i < indices.size(); i++){
            this->indices.push_back(indices[i]);
        }
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
    void setNewPos(unsigned int indice, glm::vec3 new_position);
    void setInverseMass(unsigned int indice, float inverse_mass){
        this->particles[indice].inverse_mass = std::max({0.f,inverse_mass});} 
    int closestParticle(glm::vec3 ray_origin, glm::vec3 ray_dir); // Return the indice of the closest particle to the ray

    void render();

    bool haveStreching = false;
    float stretching_stiffness = .1f;

    bool haveBending = false;
    float bending_stiffness = .1f;

    // Lighting
    glm::vec4 ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    glm::vec4 diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    glm::vec4 specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float shininess = 32.0f;
    float ambient_strength = 0.1f;
    float diffuse_strength = 0.5f;
    float specular_strength = 1.0f;


    private:
        void init();
        void generateStretchingConstraints();
        void addStretchingConstraint(Triplet t);
        void generateBendingConstraints();
        // void addBendingConstraint(Triplet t);

        // Lighting
        void computeNormals();

        
        std::vector<Particle> particles;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // Constraints
        std::set<Triplet> stretching_constraints;
        std::set<Triplet> bending_constraints;

        // OpenGL
        u_int32_t VAO;
        u_int32_t VBO;
        u_int32_t EBO;


};
#endif