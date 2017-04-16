#include "HelpImage.h"

#include <windows.h>

#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "LoadTexture.h"


void HelpImage::InitHelpImage()
{

	static const std::string texture1_name = "./images/1.png";
	static const std::string texture2_name = "./images/2.png";
	static const std::string texture3_name = "./images/3.png";
	static const std::string texture4_name = "./images/4.png";

	texture_id_left_1 = LoadTexture(texture1_name.c_str());
	texture_id_left_2 = LoadTexture(texture2_name.c_str());
	texture_id_right_1 = LoadTexture(texture3_name.c_str());
	texture_id_right_2 = LoadTexture(texture4_name.c_str());


	int pos_loc = glGetAttribLocation(image_program, "pos_attrib");
	int tex_coord_loc = glGetAttribLocation(image_program, "tex_coord_attrib");
	//int normal_loc = glGetAttribLocation(image_program, "normal_attrib");

	float vertices[] = {
		//  Position       Texcoords
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Top-left
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // Bottom-left
		1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  // Bottom-right
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f // Top-right

	};

	// Create vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create vertex buffer
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	int x = sizeof(*vertices);
	int y = sizeof(vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Specify vertex attributes

	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(tex_coord_loc);
	glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const GLvoid*)(3 * sizeof(GLfloat)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void HelpImage::drawHelpImages() {
	drawImage(1);
	drawImage(2);
	drawImage(3);
	drawImage(4);
}



void HelpImage::drawImage(int index)
{
	int pvm_loc = glGetUniformLocation(image_program, "PVM");

	glm::mat4 PVM;
	switch (index) {
	case 1:
		PVM = P*V*M_left_controller*T_left*S*R;
		break;
	case 2:
		PVM = P*V*M_left_controller*T_right*S*R;
		break;
	case 3:
		PVM = P*V*M_right_controller*T_left*S*R;
		break;
	case 4:
		PVM = P*V*M_right_controller*T_right*S*R;
		break;
	}


	glUniformMatrix4fv(pvm_loc, 1, false, glm::value_ptr(PVM));

	glActiveTexture(GL_TEXTURE0);

	switch (index) {
	case 1:
		glBindTexture(GL_TEXTURE_2D, texture_id_left_1);
		break;
	case 2:
		glBindTexture(GL_TEXTURE_2D, texture_id_left_2);
		break;
	case 3:
		glBindTexture(GL_TEXTURE_2D, texture_id_right_1);
		break;
	case 4:
		glBindTexture(GL_TEXTURE_2D, texture_id_right_2);
		break;
	}

	int tex_loc = glGetUniformLocation(image_program, "diffuseColor");
	if (tex_loc != -1)
	{
		glUniform1i(tex_loc, 0); // we bound our texture to texture unit 0
	}

	glBindVertexArray(vao);
	//glDrawElements(GL_TRIANGLE_FAN, 3 , GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

HelpImage::HelpImage()
{
}


HelpImage::~HelpImage()
{
}
