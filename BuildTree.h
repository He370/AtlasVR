// T. McGraw, A. Guayaquil
// 02.2016
#pragma once
#include "Tree.h"
#include <fstream>
#include <ctime>

namespace gg
{
   bool node_comp(tree_node* a, tree_node* b);

	tree_node * build_brain_from_file(std::string);
	void test_print(tree_node *);
	
	void start_program();
	void end_program();

#if defined NO_USE
	bool areMatricesEqual(const glm::mat4 &, const glm::mat4 &);
	glm::mat4 generateRandomMatrix();

	tree_node * build_brain_hardcoded();
	void test_search(tree_node *);
#endif
}