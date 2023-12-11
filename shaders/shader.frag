#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;
uniform vec4 color;

void main()
{	

	vec3 lightPos = vec3(0.0, 0.0, 1.0);
	vec3 lightDir = normalize(lightPos - FragPos);

	// ambient
	float ambientStrength = 0.2;
	vec4 ambient = ambientStrength * color;
	
	// diffuse
	vec3 norm = normalize(Normal);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuse = diff * color;

	// specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(vec3(0.0, 0.0, 1.0));
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
	vec4 specular = specularStrength * spec * vec4(1.0, 1.0, 1.0, 1.0);

	FragColor = ambient + diffuse + specular;
	return;
	// FragColor = color;
	// FragColor = vec4(abs(Normal), 1.0);
	
};