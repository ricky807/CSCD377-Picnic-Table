#version 430 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec3 normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 Ambient;
uniform vec3 MaterialColor;

uniform vec3 LightColor;
uniform vec4 LightPosition;
uniform float Shininess;

out vec4 finalColor;


void main(void)
{

	vec4 eyeSpacePosition = view_matrix*model_matrix*position;
	vec3 eyeSpaceNormal = mat3(view_matrix*model_matrix)*normal;

	vec3 N = normalize(eyeSpaceNormal);
	vec3 L = normalize(LightPosition.xyz - eyeSpacePosition.xyz);
	vec3 diffuse =  LightColor * (max(dot(L,N), 0.0));
	vec4 finalColor = vec4((Ambient+diffuse)*vec3(color) + specular, 1.0);

	color = finalColor;

	color = min(vec4(MaterialColor*Ambient, 1.0), vec4(1.0));

	gl_Position = projection_matrix*view_matrix*model_matrix*position;

}

