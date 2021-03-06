#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Plane.h"
#include "Cube.h"
#include "ShadeBasic.h"

#define GLM_FORCE_RADIANS 

#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

using namespace glm;

GLuint matrix_loc;
GLuint projection_matrix_loc;
GLuint view_matrix_loc;
GLuint program;

//different boolean variables

bool show_line = false;
bool top_view = false;

const double kPI = 3.1415926535897932384626433832795;

mat4 view_matrix(1.0f);
mat4 projection_matrix(1.0f);
mat4 rotate_matrix(1.0f);
mat4 translate_matrix(1.0f);
mat4 scale_matrix(1.0f);
mat4 shear_matrix(1.0f);
mat4 model_matrix(1.0f);

//Add light components
vec4 light_position(5.0, 10.0, 5.0, 1.0);
vec3 light_ambient(0.3, 0.3, 0.3);
vec3 light_color(1.0, 1.0, 1.0);
vec3 material_color(0.9, 0.5, 0.3);
float shininess = 50.0f;

// uniform indices of light
GLuint ambient_loc;
GLuint light_source_loc;
GLuint light_position_loc;
GLuint material_color_loc;

GLfloat eye[3] = { 0.0f, 2.5f, 8.0f };
GLfloat center[3] = { 0.0f, 0.0f, 0.0f };
GLfloat cameraRotation[3] = { 0.0f, 0.0f, 0.0f };
GLfloat offset = 0.0;

GLfloat angle = 0.0f;

char* ReadFile(const char* filename);
GLuint initShaders(const char* v_shader, const char* f_shader);

void keyboard(unsigned char key, int x, int y);

char* ReadFile(const char* filename) {

	FILE* infile;
#ifdef WIN32
	fopen_s(&infile, filename, "rb");
#else
	infile = fopen(filename, "rb");
#endif


	if (!infile) {
		printf("Unable to open file %s\n", filename);
		return NULL;
	}

	fseek(infile, 0, SEEK_END);
	int len = ftell(infile);
	fseek(infile, 0, SEEK_SET);
	char* source = (char*)malloc(len + 1);
	fread(source, 1, len, infile);
	fclose(infile);
	source[len] = 0;
	return (source);

}

/*************************************************************/
/*************************************************************/

GLuint initShaders(const char* v_shader, const char* f_shader) {

	GLuint p = glCreateProgram();

	GLuint v = glCreateShader(GL_VERTEX_SHADER);
	GLuint f = glCreateShader(GL_FRAGMENT_SHADER);

	const char * vs = ReadFile(v_shader);
	const char * fs = ReadFile(f_shader);

	glShaderSource(v, 1, &vs, NULL);
	glShaderSource(f, 1, &fs, NULL);

	free((char*)vs);
	free((char*)fs);

	glCompileShader(v);

	GLint compiled;

	glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLsizei len;
		glGetShaderiv(v, GL_INFO_LOG_LENGTH, &len);

		char* log = (char*)malloc(len + 1);

		glGetShaderInfoLog(v, len, &len, log);

		printf("Vertex Shader compilation failed: %s\n", log);

		free(log);

	}

	glCompileShader(f);
	glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);

	if (!compiled) {

		GLsizei len;
		glGetShaderiv(f, GL_INFO_LOG_LENGTH, &len);
		char* log = (char*)malloc(len + 1);
		glGetShaderInfoLog(f, len, &len, log);
		printf("Vertex Shader compilation failed: %s\n", log);
		free(log);
	}

	glAttachShader(p, v);
	glAttachShader(p, f);
	glLinkProgram(p);
	GLint linked;

	glGetProgramiv(p, GL_LINK_STATUS, &linked);

	if (!linked) {

		GLsizei len;
		glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
		char* log = (char*)malloc(len + 1);
		glGetProgramInfoLog(p, len, &len, log);
		printf("Shader linking failed: %s\n", log);
		free(log);
	}

	glUseProgram(p);

	return p;

}

/*******************************************************/
void Initialize(void){
	// Create the program for rendering the model

	program = initShaders("shader_picnic_table.vs", "shader_picnic_table.fs");

	// attribute indices
	model_matrix = mat4(1.0f);
	view_matrix_loc = glGetUniformLocation(program, "view_matrix");
	matrix_loc = glGetUniformLocation(program, "model_matrix");
	projection_matrix_loc = glGetUniformLocation(program, "projection_matrix");
	
	createPlane();
	createCube();
	createPyramid();

	glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, (GLfloat*)&model_matrix[0]);
	projection_matrix = perspective(radians(90.0f), 1.0f, 1.0f, 20.0f);
	glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, (GLfloat*)&projection_matrix[0]);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Add uniform indices for light and material

	
	ambient_loc = glGetUniformLocation(program, "Ambient");
	glUniform3fv(ambient_loc, 1, (GLfloat*)&light_ambient[0]);

	
	material_color_loc = glGetUniformLocation(program, "MaterialColor");
	glUniform3fv(material_color_loc, 1, (GLfloat*)&material_color[0]);

	glUniform3fv(glGetUniformLocation(program, "LightColor"), 1, (GLfloat*)&light_color[0]);
	glUniform1f(glGetUniformLocation(program, "Shininess"), shininess);

}


void Display(void)
{
	// Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Setup view matrix
	if (top_view) {
		eye[0] = 0.0;
		eye[1] = 8.0;
		eye[2] = 0.01; // for some reason it doesn't show anything when set to zero

		cameraRotation[0] = -8.0 * cos(radians(angle));
		cameraRotation[1] = 0.0;
		cameraRotation[2] = -8.0 * sin(radians(angle));
	}
	else{
		eye[0] = 8.0 * cos(radians(angle));
		eye[1] = 2.75;
		eye[2] = 8.0 * sin(radians(angle));

		cameraRotation[0] = 0.0;
		cameraRotation[1] = 1.0;
		cameraRotation[2] = 0.0;
	}
	view_matrix = glm::lookAt(vec3(eye[0], eye[1], eye[2]), glm::vec3(center[0], center[1], center[2]), glm::vec3(cameraRotation[0], cameraRotation[1], cameraRotation[2]));
	glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, (GLfloat*)&view_matrix[0]);

	vec4 light_position_camera = view_matrix * light_position;
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, (GLfloat*)&light_position_camera[0]);

	if (show_line) // to show wires
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Draws the pole
	material_color = vec3(0.9, 0.5, 0.3);
	glUniform3fv(material_color_loc, 1, (GLfloat*)&material_color[0]);
	model_matrix = translate(mat4(1.0f), vec3(offset, 0.0, 0.0))*scale(mat4(1.0f), vec3(0.20, 8.0, 0.20));
	glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, (GLfloat*)&model_matrix[0]);
	drawCube();

	// Draws the ground
	model_matrix = translate(mat4(1.0f), vec3(offset, -4.0, 0.0));
	glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, (GLfloat*)&model_matrix[0]);
	material_color = vec3(0.0, 1.0, 0.0);
	glUniform3fv(material_color_loc, 1, (GLfloat*)&material_color[0]);
	drawPlane();

	// Draws the shade
	material_color = vec3(0.9, 0.5, 0.3);
	glUniform3fv(material_color_loc, 1, (GLfloat*)&material_color[0]);
	mat4 model_matrix = translate(mat4(1.0f), vec3(0.0, 3.5, 0.0)) * scale(mat4(1.0f), vec3(5.0, .75, 5.0));
	glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, (GLfloat*)&model_matrix[0]);
	drawPyramid();

	// Draw the table
	material_color = vec3(0.9, 0.5, 0.3);
	glUniform3fv(material_color_loc, 1, (GLfloat*)&material_color[0]);
	model_matrix = scale(mat4(1.0f), vec3(3.0, .25, 3.0));
	glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, (GLfloat*)&model_matrix[0]);
	drawCube();

	// draws four legs and chairs
	for(int x = 0; x < 360; x += 90)
	{
		// Draw table legs by drawing them around the table and then rotating them facing towards the table
		translate_matrix = translate(mat4(1.0f), vec3(1.2 * cos(radians(1.0f * x)), -2, 1.2 * sin(radians(1.0f * x))));
		shear_matrix = { .35, 0, 0, 0, -1.2, 4, 0, 0, 0, 0, .35, 0, 0, 0, 0, 1 };
		rotate_matrix = rotate(mat4(1.0f), radians(1.0f * x), vec3(0.0f, -1.0f, 0.0f));
		model_matrix = translate_matrix * scale(mat4(1.0f), vec3(1.0, 1.0, 1.0)) * rotate_matrix * shear_matrix ;
		glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, (GLfloat*)&model_matrix[0]);
		drawCube();

		// Draws the chairs
		translate_matrix = translate(mat4(1.0f), vec3(3.2 * cos(radians(1.0f * x)), -3, 3.2 * sin(radians(1.0f * x))));
		scale_matrix = scale(mat4(1.0f), vec3(2.2, 2.2, 2.2));
		model_matrix = translate_matrix * scale_matrix;
		glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, (GLfloat*)&model_matrix[0]);
		drawCube();
	}
	glutSwapBuffers();
}


void keyboard(unsigned char key, int x, int y){

	switch (key){
	case 'q':case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case 't':case 'T':
		top_view = !top_view;
		break;
	case 'w':case 'W':
		show_line = !show_line;
		break;
	}
	glutPostRedisplay();
}

void timer(int n) {

	angle += 5.0f;
	glutPostRedisplay();
	glutTimerFunc(100, timer, 1);

}
/*********/
int main(int argc, char** argv){

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowSize(800, 800);

	glutCreateWindow("Picnic Table");

	if (glewInit()) {
		printf("Unable to initialize GLEW ... exiting\n");
	}

	Initialize();
	printf("%s\n", glGetString(GL_VERSION));
	glutDisplayFunc(Display);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(100, timer, 1);
	glutMainLoop();
	
	return 0;
}

/*************/



