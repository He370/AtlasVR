#include "RenderSections.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <vector>
#include <stack>
#include "AtlasVR.h"
#include "brain_hierarchy.h"

extern GLuint line_program;
extern GLuint program;
extern glm::mat4 PVvr_last;
extern GLuint model_uniform_buffer;
extern GLuint pass_uniform_buffer;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

int RenderSections::ID = 1;

void RenderSections::push_sectionNode(SectionNode* ptr)
{
	if (head==NULL) {
		head = ptr;
	}

	if (cur==NULL) {
		cur = ptr;
	}
	else {
		cur->v_next.push_back(pair<int, SectionNode*>(select_id,ptr));
		//cur = ptr;
		update_lines();
	}
}


SectionNode* RenderSections::get_sectionNode(int ID)
{
	SectionNode* temp = head;
	if (temp->ID == ID)
		return temp;
	std::stack<SectionNode* > s;
	for (auto next_pair : temp->v_next) {
		s.push(next_pair.second);
	}

	while (s.size()!=0) {
		temp = s.top();
		s.pop();
		if (temp->ID == ID)
			return temp;

		for (auto next_pair : temp->v_next) {
			s.push(next_pair.second);
		}
	}

	return NULL;
}

void RenderSections::draw_sections()
{
	if (head == NULL) return;

	std::stack<SectionNode* > s;
	s.push(head);

	while (s.size()!=0) {
		SectionNode* temp = s.top();
		s.pop();

		PassUniformData.PV = PVvr_last;
		glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);
		glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer);

		for (int i = 0; i<model_data.size(); i++)
		{
			if ((*temp->v_model_selected)[i] == true) {
				model_data[i].model_uniforms.M = (*temp->v_model_matrixs)[i];

				if (temp->select_mode == SelectMode::Hemisphere) {
					int h = model_data[i].hemisphere;
					model_data[i].model_uniforms.ka = HemisphereColors[h];
					model_data[i].model_uniforms.kd = model_data[i].model_uniforms.ka;
				}
				else if (temp->select_mode == SelectMode::Lobe) {
					int l = model_data[i].lobe;
					model_data[i].model_uniforms.ka = LobeColors[l];
					model_data[i].model_uniforms.kd = model_data[i].model_uniforms.ka;
				}
				else if (temp->select_mode == SelectMode::Region) {
					int region = model_data[i].region;
					model_data[i].model_uniforms.ka = RegionColors[region];
					model_data[i].model_uniforms.kd = model_data[i].model_uniforms.ka;
				}
				else {
					model_data[i].model_uniforms.ka = BrainColor;
					model_data[i].model_uniforms.kd = model_data[i].model_uniforms.ka;
				}

				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &model_data[i].model_uniforms);

				glBindVertexArray(model_data[i].mesh_data.mVao);
				glDrawElements(GL_TRIANGLES, model_data[i].mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
			}
		}

		set_select_mode_colors();

		for (auto next_pair : temp->v_next) {
			s.push(next_pair.second);
		}
	}
}

void RenderSections::draw_sections_pick()
{
	if (head == NULL) return;

	std::stack<SectionNode* > s;
	s.push(head);

	while (s.size() != 0) {
		SectionNode* temp = s.top();
		s.pop();

		//glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer);
		for (int i = 0; i<model_data.size(); i++)
		{
			if ((*temp->v_model_selected)[i] == true) {
				model_data[i].model_uniforms.M = (*temp->v_model_matrixs)[i];

				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &model_data[i].model_uniforms);

				int section_id_loc = glGetUniformLocation(program,"section_id");
				glUniform1i(section_id_loc,temp->ID);

				glBindVertexArray(model_data[i].mesh_data.mVao);
				glDrawElements(GL_TRIANGLES, model_data[i].mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
			}
		}

		for (auto next_pair : temp->v_next) {
			s.push(next_pair.second);
		}
	}


}

void RenderSections::init_lines(){
	connectLines.mScaleFactor = 1.0f;

	//shader attrib locations
	int pos_loc = -1;
	int tex_coord_loc = -1;
	int normal_loc = -1;
	int color_loc = -1;

	pos_loc = glGetAttribLocation(line_program, "pos_attrib");
	tex_coord_loc = glGetAttribLocation(line_program, "tex_coord_attrib");
	normal_loc = glGetAttribLocation(line_program, "normal_attrib");
	color_loc = glGetAttribLocation(line_program, "color_attrib");

	connectLines.mNumIndices = 0;

	glGenVertexArrays(1, &connectLines.mVao);
	glBindVertexArray(connectLines.mVao);

	//Buffer vertices
	glGenBuffers(1, &connectLines.mVboVerts);
	glBindBuffer(GL_ARRAY_BUFFER, connectLines.mVboVerts);
	glBufferData(GL_ARRAY_BUFFER, 2 * 2 * 116 * sizeof(glm::vec3), 0, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, 0);

	glEnableVertexAttribArray(color_loc);
	glVertexAttribPointer(color_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(2 * 116 * sizeof(glm::vec3)));

	// no normals
	connectLines.mVboNormals = -1;
	glDisableVertexAttribArray(normal_loc);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RenderSections::update_lines()
{
	if (head == NULL) return;
	if (head->v_next.size() == 0) return;

	std::stack<SectionNode* > s;
	s.push(head);

	while (s.size()!=0) {
		SectionNode* temp = s.top();
		s.pop();
		if (temp->v_next.size() == 0) continue;

		//glm::vec4 zero = glm::vec4(0, 0, 0, 1.0);

		for (auto nextNode : temp->v_next) {
			SectionNode* temp_next = nextNode.second;
			int select_id = nextNode.first;

			glm::vec4 pos_1;
			glm::vec4 pos_2;
			glm::vec4 line_color;
			glm::vec3 pos;

			if (temp->select_mode == SelectMode::Lobe) {
				pos = LobeAABB[(*temp->v_model_IDs)[select_id]].getCenter();
				line_color = LobeColors[(*temp->v_model_IDs)[select_id]];
			}
			else if (temp->select_mode == SelectMode::Hemisphere) {
				pos = HemisphereAABB[(*temp->v_model_IDs)[select_id]].getCenter();
				line_color = HemisphereColors[(*temp->v_model_IDs)[select_id]];
			}
			else if (temp->select_mode == SelectMode::Region) {
				pos = RegionAABB[(*temp->v_model_IDs)[select_id]].getCenter();
				line_color = RegionColors[(*temp->v_model_IDs)[select_id]];
			}
			else if (temp->select_mode == SelectMode::Brain) {
				pos = BrainAABB.getCenter();
				line_color = BrainColor;
			}

			glm::vec4 zero = glm::vec4(pos, 1.0f);
			glm::vec4 v1_temp = (*temp->v_model_matrixs)[select_id] * zero;
			glm::vec4 v2_temp = (*temp_next->v_model_matrixs)[select_id] * zero;

			connectLinesVertices.push_back(glm::vec3(v1_temp));
			connectLinesVertices.push_back(glm::vec3(v2_temp));
			connectLinesColors.push_back(glm::vec3(line_color));
			connectLinesColors.push_back(glm::vec3(line_color));
		}

		for (auto next_pair : temp->v_next) {
			s.push(next_pair.second);
		}
	}

	glBindVertexArray(connectLines.mVao);
	glBindBuffer(GL_ARRAY_BUFFER, connectLines.mVboVerts);
	glBufferData(GL_ARRAY_BUFFER, (connectLinesColors.size() + connectLinesVertices.size()) * sizeof(glm::vec3), 0, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, connectLinesVertices.size() * sizeof(glm::vec3), connectLinesVertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, connectLinesVertices.size() * sizeof(glm::vec3), connectLinesColors.size() * sizeof(glm::vec3), connectLinesColors.data());

	int pos_loc = glGetAttribLocation(line_program, "pos_attrib");
	int color_loc = glGetAttribLocation(line_program, "color_attrib");
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, 0);
	glVertexAttribPointer(color_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(connectLinesVertices.size() * sizeof(glm::vec3)));
	glBindVertexArray(0);
}

void RenderSections::draw_lines()
{
	if (connectLinesVertices.size() >= 2) {
		glBindVertexArray(connectLines.mVao);
		glDrawArrays(GL_LINES, 0, connectLinesVertices.size());
		glBindVertexArray(0);
	}
}

RenderSections::RenderSections()
{
}


RenderSections::~RenderSections()
{
}
