// T. McGraw, A. Guayaquil
// 02.2016
#include "Tree.h"
namespace gg
{
	void tree_node::search_model_matrix_last(glm::mat4 & model_matrix, std::string hemisphere, std::string lobe, std::string section) const
	{
		for (auto & n : children) n->search_model_matrix_last(model_matrix, hemisphere, lobe, section);
		
		/// Traverse from bottom, looking when one of the tags is satisfied carry the M matrix
		if (name == (section + lobe + hemisphere + "brain") ||
			name == (lobe + hemisphere + "brain") || 
			name == (hemisphere + "brain")
			)
		{
			model_matrix = (M * model_matrix);
		}
	}

	tree_node::tree_node(const std::string & str)
	{
		name = str;
		M = glm::mat4(1.0f);
		parent = nullptr;
      id = 0;
	}

   tree_node::tree_node(const std::string & str, int n)
	{
		name = str;
		M = glm::mat4(1.0f);
		parent = nullptr;
      id = n;
	}

	void tree_node::add_child(tree_node * child)
	{
		child->parent = this;
		
		/// Concatenation, remove comment
		//child->name += child->parent->name;
		
		children.push_back(child);
	}

	void tree_node::walk_depth_first() const
	{
		std::cout << name << std::endl;
		for (auto & n : children) n->walk_depth_first();
	}

	void tree_node::walk_depth_last() const
	{
		for (auto & n : children) n->walk_depth_last();
		std::cout << name << std::endl;
	}


   int count_nodes_at_depth(int depth, tree_node* node)
   {
      if(depth == 0)
      {
         if(node == nullptr)
         {
            return 0;
         }
         else
         {
            return 1;
         }
      }

      int count = 0;
      for (auto & n : node->children)
      {
         count += count_nodes_at_depth(depth-1, n);
      }
      return count;
   }

   void push_nodes_at_depth(std::vector<tree_node*>& v, int depth, tree_node* node)
   {
      if(depth == 0)
      {
         if(node == nullptr)
         {
            return;
         }
         else
         {
            v.push_back(node);
            return;
         }
      }

      for (auto & n : node->children)
      {
         push_nodes_at_depth(v, depth-1, n);
      }

   }

}