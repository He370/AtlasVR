#ifndef __BRAINHIERARCHY_H__
#define __BRAINHIERARCHY_H__

#include <vector>
#include <set>
#include "uniforms.h"
#include "Transform.h"
#include "LoadMesh.h"
#include "AABB.h"
#include "BuildTree.h"

enum SelectMode{ Brain, Hemisphere, Lobe, Region, NumModes };
extern char* SelectNames[];
extern int select_mode;
extern int selected_slab;
extern std::set<int> SelectedHemisphereIDs;
extern std::set<int> SelectedLobeIDs;
extern std::set<int> SelectedRegionIDs;

extern glm::vec4 BrainColor;
extern std::vector<glm::vec4> HemisphereColors;
extern std::vector<glm::vec4> LobeColors;
extern std::vector<glm::vec4> RegionColors;

extern std::vector<AnimTransform> HemisphereMatrices;
extern std::vector<AnimTransform> LobeMatrices;
extern std::vector<AnimTransform> RegionMatrices;

extern std::vector<AnimTransform> SelectedHemisphereMatrices;
extern std::vector<AnimTransform> SelectedLobeMatrices;
extern std::vector<AnimTransform> SelectedRegionMatrices;

extern std::vector<float> SelectedMeshU;

extern std::vector<std::vector<AnimTransform>> InstanceMatrices; //slab matrices, really
extern std::vector<std::vector<AnimTransform>> SelectionMatrices; //selection matrices

extern AABB BrainAABB;
extern std::vector<AABB> HemisphereAABB;
extern std::vector<AABB> LobeAABB;
extern std::vector<AABB> RegionAABB;

extern std::vector<MeshData> HemisphereLines;
extern std::vector<MeshData> LobeLines;
extern std::vector<MeshData> RegionLines;

extern gg::tree_node* BrainRoot;
extern std::vector<gg::tree_node*> RegionNodes;
extern std::vector<gg::tree_node*> LobeNodes;
extern std::vector<gg::tree_node*> HemisphereNodes;

struct ModelData
{
   MeshData mesh_data;
   ModelUniforms model_uniforms;

   int hemisphere;
   int lobe;
   int region;
   bool hidden;
};

int get_selected_id(int);

void initInstanceMatrices();
void initModelMatrices();
void initAABB(int nhemispheres, int nlobes, int nregions);

void initLines(int nhemispheres, int nlobes, int nregions);
void updateLines();

void init_hierarchy_colors(int hemispheres, int lobes, int regions);
void set_select_mode_colors();
void set_selected_edge(int edge);
void next_select_mode();

bool is_hemisphere_selected(int hemisphere);
bool is_lobe_selected(int lobe);
bool model_comp(ModelData* a, ModelData* b);

extern std::vector<ModelData> model_data;
void select_all(int val);
void unselect_all();
void hide_selection();
void unhide_selection();
void unhide_all();

void applySelectedMatrices();

void select_hemisphere(int hemisphere, int val);
void select_lobe(int lobe, int val);

bool is_hemisphere_selected(int hemisphere);
bool is_lobe_selected(int lobe);
void select_region(int region, int val);

void explode_slabs(float x, float y, float z);
void fan_slabs(float angle_scale);
void select_slabs(int slab);
void unselect_slabs(int slab);

void linear_explode_regions(float x, float y, float z);
void explode_regions(float x, float y, float z);
void explode_regions(float r);
void fan_regions(float angle_scale);

void explode_lobes(float r);
void explode_lobes(float x, float y, float z);
void fan_lobes(float angle_scale);

void explode_hemispheres(float x, float y, float z);
void fan_hemispheres(float angle, glm::vec3& axis);

void rotate_region_planes(float y);
void rotate_hemisphere_planes(float y);

void UpdateAllTransforms(float d_time);
void ReverseAllTransforms();

int getHemisphereFromLobe(int lobe);
glm::mat4 getModelGlobalMatrix(const ModelData& model);
glm::mat4 getModelGlobalSelectedMatrix(const ModelData& model);

glm::mat4 getLobeGlobalMatrix(int lobe);
glm::mat4 getHemisphereGlobalMatrix(int hemisphere);


#endif