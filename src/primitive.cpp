#include "primitive.hpp"

void Primitive::init(){
    // Initialize the vertices
    for(int i = 0; i < particles.size(); i++){
        vertices.push_back(particles[i].position.x);
        vertices.push_back(particles[i].position.y);
        vertices.push_back(particles[i].position.z);
    }

    // Initialize the constraints

    // Initialize stretching constraint (structural)
    for(int i = 0; i < indices.size(); i += 3){
        // Make three stretching constraints
        Triplet t1;
        t1.ind1 = indices[i];
        t1.ind2 = indices[i+1];
        t1.length = glm::length(particles[t1.ind1].position - particles[t1.ind2].position);
        addStretchingConstraint(t1);

        Triplet t2;
        t2.ind1 = indices[i+1];
        t2.ind2 = indices[i+2];
        t2.length = glm::length(particles[t2.ind1].position - particles[t2.ind2].position);
        addStretchingConstraint(t2);

        Triplet t3;
        t3.ind1 = indices[i+2];
        t3.ind2 = indices[i];
        t3.length = glm::length(particles[t3.ind1].position - particles[t3.ind2].position);
        addStretchingConstraint(t3);
    }

    // Initialize the VAO, VBO, EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind the VAO
    glBindVertexArray(VAO);
     
    // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Bind the EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Set the vertices
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_DYNAMIC_DRAW);

    // Set the indices
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_DYNAMIC_DRAW);

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
    
}

void Primitive::preUpdate(float delta_time, glm::vec3 ext_f){
    // Update the velocity
    for(int i = 0; i < particles.size(); i++){
        particles[i].velocity += delta_time * particles[i].inverse_mass * ext_f;
    }

    // Update the particles
    for(int i = 0; i < particles.size(); i++){
        setNewPos(i, particles[i].position + delta_time * particles[i].velocity);
        if(particles[i].position.y < -1.0f){
            particles[i].position.y = -1.0f;
            setNewPos(i, particles[i].position);
        }
    }
}

void Primitive::solve(){
    // Solve all stretching constraints
    for(auto constraint : stretching_constraints){
        // Solve Constraint
        Engine::solveStretchingConstraint(particles[constraint.ind1], particles[constraint.ind2], constraint.length, stretching_stiffness);
        // Update the vertex array
        setNewPos(constraint.ind1, particles[constraint.ind1].position);
        setNewPos(constraint.ind2, particles[constraint.ind2].position);
    }
}

void Primitive::postUpdate(float delta_time){
    // Recompute velocity
    for(Particle p : particles){
        p.velocity = (p.position - p.old_position) / delta_time;
        p.old_position = p.position;
    }

}



void Primitive::render(){
    

    // Bind the VAO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //Print the vertices
    // for(int i = 0; i < vertices.size(); i += 3){
    //     std::cout << "Vertex " << i/3 << " position: " << vertices[i] << ", " << vertices[i+1] << ", " << vertices[i+2] << std::endl;
    // }

    // Update the vertices
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    

    // Draw the primitive
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    // std::cout << "Drawing " << indices.size() << " indices" << std::endl;

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
    if(a < b){
        Triplet tmp = t;
        t.ind1 = tmp.ind2;
        t.ind2 = tmp.ind1;
    }
    stretching_constraints.insert(t);
}