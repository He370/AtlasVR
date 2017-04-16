#pragma once
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
//#include "AtlasVR.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

extern GLuint image_program;

class HelpImage
{
	GLuint vao;
	GLuint vbo;

	GLuint texture_id_left_1;
	GLuint texture_id_left_2;
	GLuint texture_id_right_1;
	GLuint texture_id_right_2;

	glm::mat4 S = glm::scale(glm::vec3(1.0f / 500.0f));
	glm::mat4 T_left = glm::translate(glm::vec3(-0.2f, 0.0f, 0.0f));
	glm::mat4 T_right = glm::translate(glm::vec3(0.2f, 0.0f, 0.0f));
	glm::mat4 R = glm::rotate(glm::mat4(), -float(3.1415/2.0), glm::vec3(1.0, 0.0, 0.0));

public:

	glm::mat4 M_left_controller;
	glm::mat4 M_right_controller;

	glm::mat4 P;
	glm::mat4 V;

	void InitHelpImage();
	void drawImage(int index);
	void drawHelpImages();

	HelpImage();
	~HelpImage();
};

