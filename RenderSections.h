#pragma once
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/spline.hpp>

#include "LoadMesh.h"

//#include "ControllerSpline.h"

using namespace std;

struct SectionNode{
	vector<pair<int,SectionNode*>> v_next;

	int ID;
	int select_mode;
	//ControllerSpline Spline;
	vector<glm::mat4>* v_model_matrixs;
	vector<int>* v_model_IDs;
	vector<bool>* v_model_selected;

};

class RenderSections
{
	MeshData connectLines;
	std::vector<glm::vec3> connectLinesVertices;
	std::vector<glm::vec3> connectLinesColors;

public:

	SectionNode* head;
	SectionNode* cur; //current selectedNode
	int select_id = -1;

	bool b_selected_mode = true;
	bool b_push_mode = false;

	static int ID;

	void push_sectionNode(SectionNode* ptr);
	SectionNode* get_sectionNode(int ID);
	void draw_sections();
	void draw_sections_pick();

	//lines
	void init_lines();
	void update_lines();
	void draw_lines();

	RenderSections();
	~RenderSections();
};

