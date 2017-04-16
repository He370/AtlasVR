// T. McGraw, A. Guayaquil
// 02.2016

#include <sstream>
#include "BuildTree.h"

namespace gg
{
	bool checkStringFinding(std::size_t position)
	{
		return (position != std::string::npos);
	}

   bool node_comp(tree_node* a, tree_node* b)
   {
      return (a->id < b->id);
   }

	tree_node * build_brain_from_file(std::string file_name)
	{
		/// Build root
		tree_node * root = new tree_node("Whole Brain", 0);

		/// Add hemispheres
		root->add_child(new tree_node("Left Hemisphere", 0));
      root->add_child(new tree_node("Bilateral", 1));
		root->add_child(new tree_node("Right Hemisphere", 2));
		

		/// Add lobes (0 - left, 1 - bilateral, 2 - right)
		for (int i = 0; i < 3; i++)
		{
			root->children[i]->add_child(new tree_node("Posterior", 0 + 7*i));
			root->children[i]->add_child(new tree_node("Occipital", 1 + 7*i));
			root->children[i]->add_child(new tree_node("Parietal", 2 + 7*i));
			root->children[i]->add_child(new tree_node("Temporal", 3 + 7*i));
			root->children[i]->add_child(new tree_node("Frontal", 4 + 7*i));
			root->children[i]->add_child(new tree_node("Central", 5 + 7*i));
			root->children[i]->add_child(new tree_node("Insula", 6 + 7*i));
		}

		/// Read file and assign the AAL regions
		std::string line;
		std::ifstream input_file("./Atlas/" + file_name);
		if (input_file.is_open())
		{
			while (std::getline(input_file, line))
			{
				/// Search first and last space in the string to catch the textID of the AAL region
				std::size_t blankSpaceFirst = line.find_first_of(' ');
				std::size_t blankSpaceLast = line.find_last_of(' ');

				std::string AALregion = line.substr(blankSpaceFirst+1, (blankSpaceLast - blankSpaceFirst-1));

				std::size_t AALregionStringSize = AALregion.size();
				std::string AALregionNoLeftRight = AALregion.substr(0, AALregionStringSize - 2);
				std::string AALregionLeftOrRight = AALregion.substr(AALregionStringSize - 1, AALregionStringSize);

            int regionId = -1;
            std::istringstream(line) >> regionId;


				/// Choose a region
				int idHemisphere;
				if (AALregionLeftOrRight == "L")
				{
					idHemisphere = 0; // Left
				}
				else if (AALregionLeftOrRight == "R")
				{
					idHemisphere = 2; // Right
				}
				else
				{
               AALregionNoLeftRight = AALregion;
					idHemisphere = 1; // Bilateral
				}

				/// Order of lobes
				/// 0 - posterior
				/// 1 - occipital
				/// 2 - parietal
				/// 3 - temporal
				/// 4 - frontal
				/// 5 - central
				/// 6 - insula
				/// For mapping check paper of Dr. McGraw
				int idLobe;
				if (
					checkStringFinding(AALregionNoLeftRight.find("Paracentral")) ||
					checkStringFinding(AALregionNoLeftRight.find("Precentral")) ||
					checkStringFinding(AALregionNoLeftRight.find("Olfactory")) ||
					checkStringFinding(AALregionNoLeftRight.find("Rolandic")) ||
					checkStringFinding(AALregionNoLeftRight.find("Frontal")) ||
					checkStringFinding(AALregionNoLeftRight.find("Rectus")) ||
					checkStringFinding(AALregionNoLeftRight.find("Supp"))
					)
				{
					idLobe = 4;
				}
				else if (
					checkStringFinding(AALregionNoLeftRight.find("Cingulum")) ||
					checkStringFinding(AALregionNoLeftRight.find("Insula"))
					)
				{
					idLobe = 6;
				}
				else if (
					checkStringFinding(AALregionNoLeftRight.find("ParaHippocampal")) ||
					checkStringFinding(AALregionNoLeftRight.find("Hippocampus")) ||
					checkStringFinding(AALregionNoLeftRight.find("Temporal")) ||
					checkStringFinding(AALregionNoLeftRight.find("Amygdala")) ||
					checkStringFinding(AALregionNoLeftRight.find("Fusiform")) ||
					checkStringFinding(AALregionNoLeftRight.find("Heschl"))
					)
				{
					idLobe = 3;
				}
				else if (
					checkStringFinding(AALregionNoLeftRight.find("Calcarine")) ||
					checkStringFinding(AALregionNoLeftRight.find("Occipital")) ||
					checkStringFinding(AALregionNoLeftRight.find("Lingual")) ||
					checkStringFinding(AALregionNoLeftRight.find("Cuneus"))
					)
				{
					idLobe = 1;
				}
				else if (
					checkStringFinding(AALregionNoLeftRight.find("SupraMarginal")) ||
					checkStringFinding(AALregionNoLeftRight.find("Postcentral")) ||
					checkStringFinding(AALregionNoLeftRight.find("Precuneus")) ||
					checkStringFinding(AALregionNoLeftRight.find("Parietal")) ||
					checkStringFinding(AALregionNoLeftRight.find("Angular"))
					)
				{
					idLobe = 2;
				}
				else if (
					checkStringFinding(AALregionNoLeftRight.find("Thalamus")) ||
					checkStringFinding(AALregionNoLeftRight.find("Pallidum")) ||
					checkStringFinding(AALregionNoLeftRight.find("Putamen")) ||
					checkStringFinding(AALregionNoLeftRight.find("Caudate"))
					)
				{
					idLobe = 5;

				}
				else if (
					checkStringFinding(AALregionNoLeftRight.find("Cerebelum")) ||
					checkStringFinding(AALregionNoLeftRight.find("Vermis"))
					)
				{
					idLobe = 0;
				}
				else
				{
					idLobe = -1;
					std::cout << "Lobe not mapped" << AALregionNoLeftRight << "... continuing next line\n";
					continue;
				}


				root->children[idHemisphere]->children[idLobe]->add_child(new tree_node(AALregionNoLeftRight, regionId-1));
			}
			input_file.close();
		}
		else
		{
			std::cout << "unable to open file" << file_name << "\n";
		}

		return root;
	}

	void test_print(tree_node * root)
	{
		root->walk_depth_first();
	}

	void start_program()
	{
		srand((unsigned int)time(NULL));
	}

	void end_program()
	{
		std::cout << "Finish code, stroke a key\n";
		std::cin.get();
	}

#if defined NO_USE
	/// labels from the website http://doc.pmod.com/pneuro/6750.htm
	const std::string AALregions_occipital[] = { "V1", "Q", "LING", "O1", "O2", "O3" };
	const std::string AALregions_temporal[] = { "HIP", "PHIP", "AMYG", "FUSI" };
	const std::string AALregions_insula[] = { "IN", "ACIN", "MCIN", "PCIN" };
	const std::string lobes[] = { "occipital", "temporal", "insula" };
	#define NUMBER_AALREGIONS_OCCIPITAL 6
	#define NUMBER_AALREGIONS_TEMPORAL 4
	#define NUMBER_AALREGIONS_INSULA 4
	#define NUMBER_LOBES 3

	glm::mat4 generateRandomMatrix()
	{
		glm::mat4 matrix_return;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				matrix_return[i][j] = (float(std::rand() % 10) + 1.0f);
			}
		}

		return matrix_return;
	}

	bool areMatricesEqual(const glm::mat4 & A, const glm::mat4 & B)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (std::abs(A[i][j] - B[i][j]) > std::numeric_limits<float>::epsilon())
				{
					return false;
				}
			}
		}
		return true;
	}

	void test_search(tree_node * root)
	{
		glm::mat4 model_matrix;
		glm::mat4 M1, M2, M3;

		/// Reset to identity
		model_matrix = glm::mat4(1.0f);

		/// Set the three matrices we want to test
		/// left -> 0
		/// left -> occipital -> 0, 0
		/// left -> occipital -> O2 -> 0, 0, 4
		M1 = generateRandomMatrix();
		M2 = generateRandomMatrix();
		M3 = generateRandomMatrix();
		root->children[0]->M = M1;
		root->children[0]->children[0]->M = M2;
		root->children[0]->children[0]->children[4]->M = M3;

		root->search_model_matrix_last(model_matrix, "left", "occipital", "O2");
		std::cout << "model matrix for the brain -> left -> occipital -> O2 is :\n";
		std::cout << "expected\n" << model_matrix << std::endl;
		std::cout << "obtained\n" << (M1 * M2 * M3) << std::endl;

		/// Clean and now try with a metric
		/// right -> 1
		/// right -> insula -> 1, 2
		/// right -> insula -> ACIN -> 1, 2, 1
		model_matrix = glm::mat4(1.0f);
		M1 = generateRandomMatrix();
		M2 = generateRandomMatrix();
		M3 = generateRandomMatrix();
		root->children[1]->M = M1;
		root->children[1]->children[2]->M = M2;
		root->children[1]->children[2]->children[1]->M = M3;
		root->search_model_matrix_last(model_matrix, "right", "insula", "ACIN");
		std::cout << "metric for the model matrix for the brain -> right -> insula -> ACIN :\n";
		if (areMatricesEqual(model_matrix, (M1 * M2 * M3)))
		{
			std::cout << "pass\n";
		}
		else
		{
			std::cout << "fail\n";
		}
	}

	tree_node * build_brain_hardcoded()
	{
		/// Build root
		tree_node * root = new tree_node("brain");

		/// Add hemispheres
		root->add_child(new tree_node("left"));
		root->add_child(new tree_node("right"));

		/// Add lobes
		for (int i = 0; i < NUMBER_LOBES; i++)
		{
			root->children[0]->add_child(new tree_node(lobes[i]));
			root->children[1]->add_child(new tree_node(lobes[i]));
		}

		/// Add AAL regions
		for (int i = 0; i < NUMBER_AALREGIONS_OCCIPITAL; i++)
		{
			root->children[0]->children[0]->add_child(new tree_node(AALregions_occipital[i]));
			root->children[1]->children[0]->add_child(new tree_node(AALregions_occipital[i]));
		}
		for (int i = 0; i < NUMBER_AALREGIONS_TEMPORAL; i++)
		{
			root->children[0]->children[1]->add_child(new tree_node(AALregions_temporal[i]));
			root->children[1]->children[1]->add_child(new tree_node(AALregions_temporal[i]));
		}
		for (int i = 0; i < NUMBER_AALREGIONS_INSULA; i++)
		{
			root->children[0]->children[2]->add_child(new tree_node(AALregions_insula[i]));
			root->children[1]->children[2]->add_child(new tree_node(AALregions_insula[i]));
		}

		return root;
	}
#endif
}