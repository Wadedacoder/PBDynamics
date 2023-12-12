#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;
uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;
uniform vec3 lightPos;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform vec3 viewPos;
uniform float shininess;

void main()
{	

	// vec3 lightPos = vec3(0.0, 0.0, 1.0);
	vec3 lightDir = normalize(lightPos - FragPos);

	// ambient
	// float ambientStrength = 0.2;
	vec4 ambient = ambientStrength * ambientColor;
	
	// diffuse
	vec3 norm = normalize(Normal);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuse = diffuseStrength * diff * diffuseColor;

	// specular
	// float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec4 specular = specularStrength * spec * specularColor;
	FragColor = ambient + diffuse + specular;
	return;
	// FragColor = color;
	// FragColor = vec4(abs(Normal), 1.0);
	
};