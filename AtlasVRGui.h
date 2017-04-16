#ifndef __ATLASTESTMESHGUI_H__
#define __ATLASTESTMESHGUI_H__

#include "brain_hierarchy.h"

extern int WindowWidth;
extern int WindowHeight;
extern unsigned int program;
extern bool hide_ui;
extern bool hide_boxes;
extern bool hide_lines;
extern bool hide_labels;
extern bool hide_pointer;
extern bool hide_scene_meshes;
extern bool hide_controllers;
extern bool enable_slab_popout;

extern float select_model_uniform_scale;
extern float atlas_mesh_scale;

extern MeshData label_lines;

enum InteractMode { Meshes, Slabs, NumInteractionModes };
extern int interact_mode;

void init_gui();
void gui();
void Draw2dVRGui();

#endif