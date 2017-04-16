// T. McGraw, A. Guayaquil
// 02.2016
#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>
namespace gg
{
	struct tree_node
	{
		tree_node(const std::string &);
      tree_node(const std::string &, int);
		tree_node * parent;
		std::string name;
		glm::mat4 M;
      int id;

		std::vector<tree_node *> children;
		
		void add_child(tree_node *);

		void walk_depth_last() const;
		void walk_depth_first() const;
		void search_model_matrix_last(glm::mat4 &, std::string, std::string, std::string) const;
	};

   int count_nodes_at_depth(int depth, tree_node* node);
   void push_nodes_at_depth(std::vector<tree_node*>& v, int depth, tree_node* node);
}