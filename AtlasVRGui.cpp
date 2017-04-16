#include <string>
#include <set>
#include "AtlasVRGui.h"
#include "brain_hierarchy.h"
#include <glm/gtc/type_ptr.hpp>

#include "file_io.h"
#include "VirtualTrackball.h"
#include "Cube.h"
#include "imgui_impl_vr.h"
#include "imgui_draw_3d.h"
#include "TrackedCamera.h"
#include "Views.h"
#include "ControllerSpline.h"
#include <unordered_map>
#include "Labels.h"
#include "Color.h"
#include "Tutorial.h"

extern VirtualTrackball* pTrackballCam;
extern VirtualTrackball* pSceneMeshRotation;
extern std::string selection_name;
extern glm::mat4 M0;

extern GLuint PickID;
extern float frame_render_time;
extern ControllerSpline* pMesh_spl;
extern ControllerSpline slab_spl;

extern void enable_slabs_region(int id, AABB& box, const glm::vec3&);

extern bool lock_3p_camera;

void LookAtSelection();

bool hide_debug_menu = true;
bool hide_labels = false;
bool hide_pointer = false;
bool hide_controllers = false;
bool hide_scene_meshes = false;
bool enable_slab_popout = false;

MeshData label_lines;

void view_menu();
void mode_menu();
void interaction_menu();

int interact_mode = InteractMode::Meshes;

TrackedCamera* ptr_tracked_camera;
bool activate_camera_service = false;
bool camera_service_activated = false;

Tutorial* pTutorial = nullptr;

void init_gui()
{
   if (pTutorial == nullptr)
   {
      pTutorial = new Tutorial; //needs to be created at run time because it creates OpenGL texture objects
   }
   ImGui_Impl_VR_SetPinned(true);
   ImGui_Impl_VR_SetGuiPose(glm::translate(glm::vec3(+2.0f, 1.0f, -1.0f)));
   ImGui_Impl_VR_SetGuiScale(1.0f); 
}

void Draw2dVRGui()
{

   static bool first = true;
   if (first)
   {
      InitLabels();
   }
   first = false;

	//ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
	
	if (ImGui_Impl_VR_IsHidden() == true)
	{
		return;
	}

	if (pTutorial!= nullptr && pTutorial->tutorialMode == true) {
      pTutorial->switchToTutorialMode();
      pTutorial->draw2DtutorialGui();
		return;
	}

	ImVec2 texsize = ImGui_Impl_VR_GetTextureSize();
	static int tab = 0;
	ImGui_Impl_VR_NewFrame(0);

	
	ImGui::SetNextWindowSize(texsize);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::Begin("View", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	if (ImGui::Button("Tutorial Mode"))
	{
		if (pTutorial != nullptr)
		{
			pTutorial->tutorialMode = true;
		}
	}

      view_menu();
	
	ImGui::End();
	ImGui_Impl_VR_Render(0);

	ImGui_Impl_VR_NewFrame(1);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(texsize);
   ImGui::Begin("Select Mode", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
      mode_menu();
	ImGui::End();
	ImGui_Impl_VR_Render(1);

	ImGui_Impl_VR_NewFrame(2);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(texsize);
	ImGui::Begin("Interaction Mode", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
      interaction_menu();
	ImGui::End();
	ImGui_Impl_VR_Render(2);


	ImGui_Impl_VR_NewFrame(3);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(texsize);
	ImGui::Begin("Debugging", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
      static glm::vec3 dolly;
      dolly = pTrackballCam->getDolly();
      if (ImGui::SliderFloat("dolly", &dolly.x, -10.0f, 10.0f))
      {
         pTrackballCam->setDolly(dolly);
      }
      if (ImGui::SliderFloat("height", &dolly.y, -10.0f, 10.0f))
      {
         pTrackballCam->setDolly(dolly);
      }
      
      static bool green_screen = false;
      if (ImGui::Checkbox("Green screen", &green_screen))
      {
         int loc = glGetUniformLocation(program, "green_screen");
         glUniform1i(loc, green_screen);
      }

      ImGui::Checkbox("Hide pointer", &hide_pointer);
      ImGui::Checkbox("Hide controllers", &hide_controllers);
      ImGui::Checkbox("Hide scene meshes", &hide_scene_meshes);
      ImGui::Checkbox("Hide labels", &hide_labels);
      ImGui::Checkbox("Slab Popout", &enable_slab_popout);
      if (ImGui::Button("Untwist"))
      {
         if (pMesh_spl != nullptr)
         {
            pMesh_spl->SetTwist(0.0f);
         }
         slab_spl.SetTwist(0.0f);
      }


      float cd = pMesh_spl->GetTwistCdGain();
      if (ImGui::SliderFloat("cd gain", &cd, -10.0f, +10.0f))
      {
         pMesh_spl->SetTwistCdGain(cd);
      }

      ImGui::Checkbox("Lock 3rd person camera", &lock_3p_camera);

      //ImGui::ImageButton((void*)GetLabelTexture(), ImVec2(512, 4096));

      if (ImGui::SliderFloat("ch_start", &CubehelixParams::ch_start, -10.0f, 10.0f))
      {
         init_hierarchy_colors(HemisphereColors.size(), LobeColors.size(), RegionColors.size());
         set_select_mode_colors();
      }

      if (ImGui::SliderFloat("ch_cycles", &CubehelixParams::ch_cycles, -10.0f, 10.0f))
      {
         init_hierarchy_colors(HemisphereColors.size(), LobeColors.size(), RegionColors.size());
         set_select_mode_colors();
      }

      if (ImGui::SliderFloat("ch_saturation", &CubehelixParams::ch_saturation, -10.0f, 10.0f))
      {
         init_hierarchy_colors(HemisphereColors.size(), LobeColors.size(), RegionColors.size());
         set_select_mode_colors();
      }

      if (ImGui::SliderFloat("ch_gamma", &CubehelixParams::ch_gamma, -10.0f, 10.0f))
      {
         init_hierarchy_colors(HemisphereColors.size(), LobeColors.size(), RegionColors.size());
         set_select_mode_colors();
      }
      
	ImGui::End();
	ImGui_Impl_VR_Render(3);

}

void view_menu()
{
   ImVec2 size(200, 100);
      if (ImGui::Button("Axial Top", size))
      {
         pSceneMeshRotation->setRotation(ImagingMatrices[AXIAL_TOP]);
      }
      ImGui::SameLine();
      if (ImGui::Button("Axial Bottom", size))
      {
         pSceneMeshRotation->setRotation(ImagingMatrices[AXIAL_BOTTOM]);
      }

      if (ImGui::Button("Sagittal Right", size))
      {
         pSceneMeshRotation->setRotation(ImagingMatrices[SAGITTAL_RIGHT]);
      }
      ImGui::SameLine();
      if (ImGui::Button("Sagittal Left", size))
      {
         pSceneMeshRotation->setRotation(ImagingMatrices[SAGITTAL_LEFT]);
      }

      if (ImGui::Button("Coronal Front", size))
      {
         pSceneMeshRotation->setRotation(ImagingMatrices[CORONAL_FRONT]);
      }
      ImGui::SameLine();
      if (ImGui::Button("Coronal Back", size))
      {
         pSceneMeshRotation->setRotation(ImagingMatrices[CORONAL_BACK]);
      }

      if (ImGui::Button("Iso", size))
      {
         pSceneMeshRotation->setRotation(ImagingMatrices[ISOMETRIC]);
      }
      ImGui::SameLine();
      if (ImGui::Button("LookAt selection", size))
      {
         LookAtSelection();
      }
}

void mode_menu()
{
   ImVec2 size(400, 100);
   if (ImGui::Button("Brain", size))
   {
      select_mode = SelectMode::Brain;
      set_select_mode_colors();
   }
   
   if (ImGui::Button("Hemisphere", size))
   {
      select_mode = SelectMode::Hemisphere;
      set_select_mode_colors();
   }

   if (ImGui::Button("Lobe", size))
   {
      select_mode = SelectMode::Lobe;
      set_select_mode_colors();
   }

   if (ImGui::Button("Region", size))
   {
      select_mode = SelectMode::Region;
      set_select_mode_colors();
   }

   if (ImGui::Button("Select All"))
   {
      select_all(true);
   }
   ImGui::SameLine();
   if (ImGui::Button("Unselect All"))
   {
      unselect_all();
   }

   if (ImGui::Button("Hide Selection"))
   {
      hide_selection();
   }
   ImGui::SameLine();
   if (ImGui::Button("Unhide Selection"))
   {
      unhide_selection();
   }
   ImGui::SameLine();
   if (ImGui::Button("Unhide All"))
   {
      unhide_all();
   }
}

void interaction_menu()
{
   ImVec2 size(400, 100);
   if (ImGui::Button("Surface meshes", size))
   {
      interact_mode = InteractMode::Meshes;
      for (auto &model : model_data)
      {
         int id = model.region;
         model_data[id].model_uniforms.instances = 1;
      }
   }

   if (ImGui::Button("Image slabs X", size))
   {
      interact_mode = InteractMode::Slabs;
      for (auto &model : model_data)
      {
         int id = model.region;
         enable_slabs_region(id, BrainAABB, glm::vec3(1.0f, 0.0f, 0.0f));
      }
   }
   if (ImGui::Button("Image slabs Y", size))
   {
      interact_mode = InteractMode::Slabs;
      for (auto &model : model_data)
      {
         int id = model.region;
         enable_slabs_region(id, BrainAABB, glm::vec3(0.0f, 1.0f, 0.0f));
      }
   }
   if (ImGui::Button("Image slabs Z", size))
   {
      interact_mode = InteractMode::Slabs;
      for (auto &model : model_data)
      {
         int id = model.region;
         enable_slabs_region(id, BrainAABB, glm::vec3(0.0f, 0.0f, 1.0f));
      }
   }
}