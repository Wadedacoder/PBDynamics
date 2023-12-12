#include "primitive.hpp"

void Primitive::init(){

    std::cout << "Number of particles: " << particles.size() << std::endl;
    std::cout << "Number of indices: " << indices.size() << std::endl;

    std::cout << "Initializing the primitive" << std::endl;
    // Initialize the vertices
    for(int i = 0; i < particles.size(); i++){
        vertices.push_back(particles[i].position.x);
        vertices.push_back(particles[i].position.y);
        vertices.push_back(particles[i].position.z);

        // std::cout << "Particle " << i << " position: " << particles[i].position.x << ", " << particles[i].position.y << ", " << particles[i].position.z << std::endl;
    }


    // Initialize the constraints
    std::cout << "Initializing the constraints" << std::endl;
    
    if(haveStreching){
        generateStretchingConstraints();
    }

    if(haveBending){
        generateBendingConstraints();
        //TODO
    }
    
    // // Print the constraints
    // for(auto constraint : stretching_constraints){
    //     std::cout << "Constraint: " << constraint.ind1 << ", " << constraint.ind2 << ", " << constraint.length << std::endl;
    // }

    // Print the geometry
    // for(int i = 0; i < indices.size(); i += 3){
    //     std::cout << "Triangle " << i/3 << ": " << indices[i] << ", " << indices[i+1] << ", " << indices[i+2] << std::endl;
    // }



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

    //Compute the normals
    computeNormals();

    // Merge the vertices and normals list
    std::vector<float> vertices_and_normals;
    for(int i = 0; i < particles.size(); i++){
        vertices_and_normals.push_back(particles[i].position.x);
        vertices_and_normals.push_back(particles[i].position.y);
        vertices_and_normals.push_back(particles[i].position.z);
        vertices_and_normals.push_back(particles[i].normal.x);
        vertices_and_normals.push_back(particles[i].normal.y);
        vertices_and_normals.push_back(particles[i].normal.z);
    }

    // Set the vertices
    glBufferData(GL_ARRAY_BUFFER, vertices_and_normals.size() * sizeof(float), vertices_and_normals.data(), GL_DYNAMIC_DRAW);

    // // Set the indices
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_DYNAMIC_DRAW);

    std::cout << "Done initializing the primitive" << std::endl;
    // Set the vertex attributes pointers
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    // Enable the vertex attributes pointers
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

}

void Primitive::generateStretchingConstraints(){
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

}

void Primitive::generateBendingConstraints(){
    /*
    Algorithm:
    Find Triangle Neighbors:
    Create a Edge List
    Define a globalEdgeNum for each edge in the mesh.
    Add each edge to the edge list
    Sort the edge list by the globalEdgeNum
    Each id match has a triangle neighbor
    to find the triangle and the vertex, use the globalEdgeNum
    */
    std::vector<std::vector<unsigned int>> edge_list;
    int global_edge_num = 0;
    for(int i = 0; i < indices.size(); i += 3){
        // Make three stretching constraints
        global_edge_num = i;
        std::vector<unsigned int> t1;
        t1.push_back(std::min(indices[i], indices[i+1]));
        t1.push_back(std::max(indices[i], indices[i+1]));
        t1.push_back(global_edge_num);
        // std::cout << "Adding constraint: " << t1.ind1 << ", " << t1.ind2 << ", " << t1.length << std::endl;
        edge_list.push_back(t1);

        std::vector<unsigned int> t2;
        t2.push_back(std::min(indices[i+1], indices[i+2]));
        t2.push_back(std::max(indices[i+1], indices[i+2]));
        t2.push_back(global_edge_num + 1);
        // std::cout << "Adding constraint: " << t2.ind1 << ", " << t2.ind2 << ", " << t2.length << std::endl;
        edge_list.push_back(t2);

        std::vector<unsigned int> t3;
        t3.push_back(std::min(indices[i+2], indices[i]));
        t3.push_back(std::max(indices[i+2], indices[i]));
        t3.push_back(global_edge_num + 2);
        // std::cout << "Adding constraint: " << t3.ind1 << ", " << t3.ind2 << ", " << t3.length << std::endl;
        edge_list.push_back(t3);
    }

    // Sort the edge list
    std::sort(edge_list.begin(), edge_list.end());

    // Find the triangle neighbors
    for(int i = 1; i < edge_list.size(); i++){
        std::cout << "Edge " << i << ": " << edge_list[i][0] << ", " << edge_list[i][1] << ", " << edge_list[i][2] << std::endl;
        if(edge_list[i][0] == edge_list[i-1][0] && edge_list[i][1] == edge_list[i-1][1]){
            // We have a triangle neighbor
            // Find the triangle and the vertex
            int triangle1 = edge_list[i][2] / 3;
            int triangle2 = edge_list[i-1][2] / 3;
            int vertex1 = edge_list[i][2] % 3;
            int vertex2 = edge_list[i-1][2] % 3;
            // std::cout << "Triangle " << triangle1 << " and " << triangle2 << " are neighbors" << std::endl;
            // std::cout << "Vertex " << vertex1 << " and " << vertex2 << " are neighbors" << std::endl;
            // Add the bending constraint
            // Find the two unique vertices not in the edge
            int v3 = indices[triangle1 + 3 - vertex1 - vertex2];
            int v4 = indices[triangle2 + 3 - vertex1 - vertex2];
            // Add the bending constraint
            Triplet t;
            t.ind1 = v3;
            t.ind2 = v4;
            t.length = glm::length(particles[t.ind1].position - particles[t.ind2].position);

            addStretchingConstraint(t);

            std::cout << "Adding constraint: " << t.ind1 << ", " << t.ind2 << ", " << t.length << std::endl;
        }
    }

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
    for (int i = 0; i < particles.size(); i++){
        // if(particles[i].inverse_mass == 0.f) continue;
        particles[i].velocity += ext_f * delta_time * particles[i].inverse_mass;
    }
    
    // Update the position of the cube
    // std::cout << "Updating position" << std::endl;
    for (int i = 0; i < particles.size(); i++){
        particles[i].old_position = particles[i].position;
        particles[i].position += particles[i].velocity * delta_time;
    }
    
    //Check for ground collision
    for (int i = 0; i < particles.size(); i++){
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
    for (int i = 0; i < particles.size(); i++){
        particles[i].velocity = (particles[i].position - particles[i].old_position) / delta_time;
    }

    // Print the positions
    // for(int i = 0; i < particles.size(); i++){
    //     std::cout << "Particle " << i << " position: " << particles[i].position.x << ", " << particles[i].position.y << ", " << particles[i].position.z << std::endl;
    // }

}



void Primitive::render(){
    
    // Match the vertices with the particles
    for(int i = 0; i < particles.size(); i++){
        vertices[i*3] = particles[i].position.x;
        vertices[i*3 + 1] = particles[i].position.y;
        vertices[i*3 + 2] = particles[i].position.z;
    }

    // Compute the normals
    computeNormals();

    // Merge the vertices and normals list
    std::vector<float> vertices_and_normals;
    for(int i = 0; i < particles.size(); i++){
        vertices_and_normals.push_back(particles[i].position.x);
        vertices_and_normals.push_back(particles[i].position.y);
        vertices_and_normals.push_back(particles[i].position.z);
        vertices_and_normals.push_back(particles[i].normal.x);
        vertices_and_normals.push_back(particles[i].normal.y);
        vertices_and_normals.push_back(particles[i].normal.z);
    }


    // Bind the VAO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Update the vertices
    glBufferData(GL_ARRAY_BUFFER, vertices_and_normals.size() * sizeof(float), vertices_and_normals.data(), GL_DYNAMIC_DRAW);
    
    // Draw the primitive
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // glDrawElements(GL_LINE_LOOP, indices.size(), GL_UNSIGNED_INT, 0);


}

void Primitive::computeNormals(){
    // Recompute the normals using https://iquilezles.org/articles/normals/

    // Reset the normals
    for(int i = 0; i < particles.size(); i++){
        particles[i].normal = glm::vec3(0.0f);
    }

    // Sum the face normals
    for(int i = 0; i < indices.size(); i += 3){
        glm::vec3 v1 = particles[indices[i]].position;
        glm::vec3 v2 = particles[indices[i+1]].position;
        glm::vec3 v3 = particles[indices[i+2]].position;
        glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);
        particles[indices[i]].normal += normal;
        particles[indices[i+1]].normal += normal;
        particles[indices[i+2]].normal += normal;
    }

    // Normalize the normals
    for(int i = 0; i < particles.size(); i++){
        particles[i].normal = glm::normalize(particles[i].normal);
    }

    // Print the normals
    // for(int i = 0; i < particles.size(); i++){
    //     std::cout << "Particle " << i << " normal: " << particles[i].normal.x << ", " << particles[i].normal.y << ", " << particles[i].normal.z << std::endl;
    // }
    
}

void Primitive::setNewPos(unsigned int indice, glm::vec3 new_position){
    particles[indice].position = new_position;
    vertices[indice*3] = new_position.x;
    vertices[indice*3 + 1] = new_position.y;
    vertices[indice*3 + 2] = new_position.z;
    std::cout << "Vertex " << indice << " position: " << vertices[indice*3] << ", " << vertices[indice*3 + 1] << ", " << vertices[indice*3 + 2] << std::endl;
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

int Primitive::closestParticle(glm::vec3 ray_origin, glm::vec3 ray_dir){
    // Check for triangles intersection
    // Using Ray-Triangle Intersection

    // print the ray
    std::cout << "Ray origin: " << ray_origin.x << ", " << ray_origin.y << ", " << ray_origin.z << std::endl;
    std::cout << "Ray direction: " << ray_dir.x << ", " << ray_dir.y << ", " << ray_dir.z << std::endl;

    int closest_indice = -1;
    float closest_t = 1e10;
    for(int i = 0; i < indices.size(); i += 3){
        // Make three stretching constraints
        glm::vec3 v1 = particles[indices[i]].position;
        glm::vec3 v2 = particles[indices[i+1]].position;
        glm::vec3 v3 = particles[indices[i+2]].position;
        glm::vec3 e1 = v2 - v1;
        glm::vec3 e2 = v3 - v1;
        glm::vec3 s = ray_origin - v1;
        glm::vec3 d = ray_dir;
        glm::vec3 cross = glm::cross(d, e2);
        float denominator = glm::dot(-d, cross);
        if(denominator == 0.f) continue;
        float t = glm::dot(s, cross) / denominator;
        glm::vec3 q = glm::cross(s, e1);
        float gamma = glm::dot(-d, q) / denominator;
        float beta = glm::dot(s, q) / denominator;
        std::cout << "t: " << t << ", gamma: " << gamma << ", beta: " << beta << std::endl;
        if(t < 0.f) continue;
        if(gamma < 0.f || gamma > 1.f) continue;
        if(beta < 0.f || beta > 1.f - gamma) continue;
        if(t < closest_t){
            closest_t = t;
            closest_indice = indices[i];
        }
    }
    return closest_indice;
    
}

