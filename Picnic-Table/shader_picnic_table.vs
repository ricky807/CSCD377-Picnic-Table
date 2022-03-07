#version 430 core

layout (location = 0) in vec4 position;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 Ambient;
uniform vec3 MaterialColor;

out vec4 color;


void main(void)
{

	color = min(vec4(MaterialColor*Ambient, 1.0), vec4(1.0));

	gl_Position = projection_matrix*view_matrix*model_matrix*position;

}

