#include "primitive.hpp"

void Primitive::init(){

    std::cout << "Initializing the primitive" << std::endl;
    // Initialize the vertices
    for(int i = 0; i < particles.size(); i++){
        vertices.push_back(particles[i].position.x);
        vertices.push_back(particles[i].position.y);
        vertices.push_back(particles[i].position.z);
    }

    // Initialize the constraints
    std::cout << "Initializing the constraints" << std::endl;
    // Initialize stretching constraint (structural)
    for(int i = 0; i < indices.size(); i += 3){
        // Make three stretching constraints
        Triplet t1;
        t1.ind1 = indices[i];
        t1.ind2 = indices[i+1];
        t1.length = glm::length(particles[t1.ind1].position - particles[t1.ind2].position);
        // std::cout << "Adding constraint: " << t1.ind1 << ", " << t1.ind2 << ", " << t1.length << std::endl;
        addStretchingConstraint(t1);

        Triplet t2;
        t2.ind1 = indices[i+1];
        t2.ind2 = indices[i+2];
        t2.length = glm::length(particles[t2.ind1].position - particles[t2.ind2].position);
        // std::cout << "Adding constraint: " << t2.ind1 << ", " << t2.ind2 << ", " << t2.length << std::endl;
        addStretchingConstraint(t2);

        Triplet t3;
        t3.ind1 = indices[i+2];
        t3.ind2 = indices[i];
        t3.length = glm::length(particles[t3.ind1].position - particles[t3.ind2].position);
        // std::cout << "Adding constraint: " << t3.ind1 << ", " << t3.ind2 << ", " << t3.length << std::endl;
        addStretchingConstraint(t3);
    }


    // Initialize the VAO, VBO, EBO
    // unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // std::cout << "HEIN";
    // Bind the VAO
    glBindVertexArray(VAO);
     
    // // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // // Bind the EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    std::cout << "Buffering the data" << std::endl;
    // Set the vertices
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    // // Set the indices
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_DYNAMIC_DRAW);

    std::cout << "Done initializing the primitive" << std::endl;
    // Set the vertex attributes pointers
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Enable the vertex attributes pointers
    glEnableVertexAttribArray(0);

}

void Primitive::update(float delta_time, glm::vec3 ext_f){
    //TODO
    preUpdate(delta_time, ext_f);
    solve();
    postUpdate(delta_time);

    // Print the positions
    // for(int i = 0; i < particles.size(); i++){
    //     std::cout << "Particle " << i << " position: " << particles[i].position.x << ", " << particles[i].position.y << ", " << particles[i].position.z << std::endl;
    // }
    
}

void Primitive::preUpdate(float delta_time, glm::vec3 ext_f){
    for (int i = 0; i < 8; i++){
        // if(particles[i].inverse_mass == 0.f) continue;
        particles[i].velocity += ext_f * delta_time * particles[i].inverse_mass;
    }
    
    // Update the position of the cube
    // std::cout << "Updating position" << std::endl;
    for (int i = 0; i < 8; i++){
        particles[i].old_position = particles[i].position;
        particles[i].position += particles[i].velocity * delta_time;
    }
    
    //Check for ground collision
    for (int i = 0; i < 8; i++){
        if(particles[i].position.y <= -1.0f){
            particles[i].position.y = -1.0f;
        }
    }
}

void Primitive::solve(){

    for(auto constraint : stretching_constraints){
        // Solve Constraint
        Engine::solveStretchingConstraint(particles[constraint.ind1], particles[constraint.ind2], constraint.length, stretching_stiffness);
    }
}

void Primitive::postUpdate(float delta_time){
    // Recompute velocity
    for (int i = 0; i < 8; i++){
        particles[i].velocity = (particles[i].position - particles[i].old_position) / delta_time;
    }

}



void Primitive::render(){
    
    // Match the vertices with the particles
    for(int i = 0; i < particles.size(); i++){
        vertices[i*3] = particles[i].position.x;
        vertices[i*3 + 1] = particles[i].position.y;
        vertices[i*3 + 2] = particles[i].position.z;
    }

    // Bind the VAO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Update the vertices
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Draw the primitive
    glDrawElements(GL_LINE_LOOP, indices.size(), GL_UNSIGNED_INT, 0);

}

void Primitive::setNewPos(unsigned int indice, glm::vec3 new_position){
    particles[indice].position = new_position;
    vertices[indice*3] = new_position.x;
    vertices[indice*3 + 1] = new_position.y;
    vertices[indice*3 + 2] = new_position.z;
    // std::cout << "Vertex " << indice << " position: " << vertices[indice*3] << ", " << vertices[indice*3 + 1] << ", " << vertices[indice*3 + 2] << std::endl;
}

void Primitive::addStretchingConstraint(Triplet t){
    // indice1 > indice2
    int a = t.ind1;
    int b = t.ind2; 
    if(a > b){
        Triplet tmp = t;
        t.ind1 = tmp.ind2;
        t.ind2 = tmp.ind1;
    }
    stretching_constraints.insert(t);
}