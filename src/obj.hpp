#include <vector>


std::pair<std::vector<float>, std::vector<unsigned int>>& createCloth(float width, float height, int num_particles_width, int num_particles_height){
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	float rest_length_horizontal = width / (num_particles_width - 1);
	float rest_length_vertical = height / (num_particles_height - 1);
	for(int i = 0; i < num_particles_height; i++){
		for(int j = 0; j < num_particles_width; j++){
			vertices.push_back(j * rest_length_horizontal);
			vertices.push_back(i * rest_length_vertical);
			vertices.push_back(0.f);
		}
	}
	for(int i = 0; i < num_particles_height - 1; i++){
		for(int j = 0; j < num_particles_width - 1; j++){
			indices.push_back(i * num_particles_width + j);
			indices.push_back(i * num_particles_width + j + 1);
			indices.push_back((i + 1) * num_particles_width + j);
			indices.push_back((i + 1) * num_particles_width + j);
			indices.push_back(i * num_particles_width + j + 1);
			indices.push_back((i + 1) * num_particles_width + j + 1);
		}
	}
	std::pair<std::vector<float>, std::vector<unsigned int>>* p = new std::pair<std::vector<float>, std::vector<unsigned int>>(vertices, indices);
	return *p;
}