#version 330 core

out vec4 FragColor;

bool intersect(vec3 r)
{
	if(r.x < 0.0f || r.x > 0.5f)
		return true;
	return false;
}

void main()
{	
	vec3 r = vec3(gl_FragCoord.x / 800.0f, gl_FragCoord.y / 600.0f, 0.0f);
	// if(intersect(r))
	// 	FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	// else
	// 	FragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	FragColor = vec4(r, 1.0f);
	
};