#include "Tutorial.h"
#include "imgui_impl_vr.h"
#include "AtlasVRGui.h"
#include "VirtualTrackball.h"
#include "Views.h"
#include "LoadTexture.h"
#include "ControllerSpline.h"

extern VirtualTrackball* pSceneMeshRotation;
extern VirtualTrackball* pTrackballCam;
extern ControllerSpline* pMesh_spl;
extern ControllerSpline slab_spl;

void Tutorial::tutorialPickScript()
{
	switch (pickStage) {
	case 0:
		if (pickMission == 1) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Task 1");
			ImGui::TextWrapped("Select either the Left or Right Frontal Lobe.");
		}
		else if (pickMission == 0) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Frontal Lobe:");
			ImGui::TextWrapped(infoFrontal.c_str());
			if (ImGui::Button("Previous", ImVec2(130.0f, 50.0f))) {
				stage = 1;
				unselect_all();
            ImGui::SetScrollY(0.0f);
			}
			ImGui::SameLine();  if (ImGui::Button("Next", ImVec2(100.0f, 50.0f))) {
				pickStage = 1;
				pickMission = 2;
				unselect_all();
            ImGui::SetScrollY(0.0f);
			}
		}
		break;
	case 1:
		if (pickMission == 2) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Task 2");
			ImGui::TextWrapped("Select either the Left or Right Parietal Lobe.");
		}
		else if (pickMission == 0) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Parietal Lobe:");
			ImGui::TextWrapped(infoParietal.c_str());
			if (ImGui::Button("Previous", ImVec2(130.0f, 50.0f))) {
				pickStage = 0;
				pickMission = 1;
				unselect_all();
            ImGui::SetScrollY(0.0f);
			}
			ImGui::SameLine();  if (ImGui::Button("Next", ImVec2(100.0f, 50.0f))) {
				pickStage = 2;
				pickMission = 3;
				unselect_all();
            ImGui::SetScrollY(0.0f);
			}
		}
		break;
	case 2:
		if (pickMission == 3) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Task 3");
			ImGui::TextWrapped("Select either the Left or Right Occipital Lobe.");
		}
		else if (pickMission == 0) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Occipital Lobe:");
			ImGui::TextWrapped(infoOccipital.c_str());
			if (ImGui::Button("Previous", ImVec2(130.0f, 50.0f))) {
				pickStage = 1;
				pickMission = 2;
				unselect_all();
            ImGui::SetScrollY(0.0f);
			}
			ImGui::SameLine();  if (ImGui::Button("Next", ImVec2(100.0f, 50.0f))) {
				pickStage = 3;
				pickMission = 4;
				unselect_all();
            ImGui::SetScrollY(0.0f);
			}
		}
		break;
	case 3:
		if (pickMission == 4) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Task 4");
			ImGui::TextWrapped("Select either the Left or Right Temporal Lobe.");
		}
		else if (pickMission == 0) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Temporal Lobe:");
			ImGui::TextWrapped(infoTemporal.c_str());
			if (ImGui::Button("Previous", ImVec2(130.0f, 50.0f))) {
				pickStage = 2;
				pickMission = 3;
				unselect_all();
            ImGui::SetScrollY(0.0f);
			}
			ImGui::SameLine();  if (ImGui::Button("Next", ImVec2(100.0f, 50.0f))) {
				pickStage = 0;
				stage = 3;
				pickMission = 0;
				select_all(1);
            if (pMesh_spl != nullptr)
            {
               pMesh_spl->SetTwist(0.0f);
            }
            slab_spl.SetTwist(0.0f);
            ImGui::SetScrollY(0.0f);
			}
		}
		break;
	}

}

void Tutorial::tutorialScript()
{
	switch (stage) {
	case 0:
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Step 1:");
		ImGui::TextWrapped("Point the right controller at the brain,"
         " then swipe with your thumb on the trackpad to rotate."
         " The name of the lobe you are pointing at will be displayed on the right controller."
         " Point the right controller at this window and swipe up and down to scroll."
         );
      ImGui::ImageButton(images[0], ImVec2(350.0f, 350.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
      
	  if (ImGui::Button("Next", ImVec2(100.0f, 50.0f))) {
			stage = 1;
         ImGui::SetScrollY(0.0f);
		}
		break;
	case 1:
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Step 2:");
		ImGui::TextWrapped("Use trigger on the right controller to select lobes of the brain");
      ImGui::ImageButton(images[1], ImVec2(350.0f, 350.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		
	  if (ImGui::Button("Previous", ImVec2(130.0f, 50.0f))) {
		  stage = 0;
		  unselect_all();
        ImGui::SetScrollY(0.0f);
	  }
	  ImGui::SameLine();  if (ImGui::Button("Next", ImVec2(100.0f, 50.0f))) {
			stage = 2;
			unselect_all();
         ImGui::SetScrollY(0.0f);
		}
		break;
	case 2:
		if (pickMission == -1) pickMission = 1;
		tutorialPickScript();
		break;
	case 3:
		//select_all();
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Free Mode:");
		ImGui::TextWrapped("Now you can explore the brain by swiping left and right on the left controller trackpad. The right controller trackpad will rotate the selection.");
		
		if (ImGui::Button("Select ALL", ImVec2(200.0f, 50.0f))) {
			select_all(1);
		}
      ImGui::SameLine(); if (ImGui::Button("Unselect ALL", ImVec2(200.0f, 50.0f))) {
			unselect_all();
		}

      if (ImGui::Button("Previous", ImVec2(130.0f, 50.0f))) {
         stage = 2;
         pickStage = 0;
         pickMission = 1;
         unselect_all();
         ImGui::SetScrollY(0.0f);
      }
      ImGui::SameLine(); if (ImGui::Button("Next", ImVec2(100.0f, 50.0f))) {
         stage = 4;
      }

		break;
	case 4:
		ImGui::TextWrapped("Congratulations! You've finished the whole tutorial. Take off the headset and take a post-test, or go back and review.");
		if (ImGui::Button("Go back and review", ImVec2(250.0f, 50.0f))) {
			stage = 3;
			select_all(1);
		}
		break;
	}

}

void Tutorial::pickLobe(int id)
{
	std::string selection_name;

	if (id < 0 || id >= model_data.size())
	{
		return;
	}

	const int hemisphere = model_data[id].hemisphere;
	const int lobe = model_data[id].lobe;

	const int val = model_data[id].model_uniforms.selected;
	//const int newval = 1 - val;

	switch (select_mode)
	{
	case SelectMode::Brain:
		selection_name = "Whole Brain";
		break;
	case SelectMode::Hemisphere:
		selection_name = HemisphereNodes[hemisphere]->name;
		break;

	case SelectMode::Lobe:
		selection_name = LobeNodes[lobe]->name;
		break;
	}

   const short int duration = 2000;
	switch (pickMission)
	{
	case 1:
		if (selection_name == "Frontal") 
         pickMission = 0;
         ImGui_Impl_VR_TriggerHapticPulse(1, duration);
		break;
	case 2:
		if (selection_name == "Parietal") 
         pickMission = 0;
         ImGui_Impl_VR_TriggerHapticPulse(1, duration);
		break;
	case 3:
		if (selection_name == "Occipital") 
         pickMission = 0;
         ImGui_Impl_VR_TriggerHapticPulse(1, duration);
		break;
	case 4:
		if (selection_name == "Temporal") 
         pickMission = 0;
         ImGui_Impl_VR_TriggerHapticPulse(1, duration);
		break;
	}

}

void Tutorial::switchToTutorialMode()
{
	if (initiated == true) return;
	pSceneMeshRotation->setRotation(ImagingMatrices[SAGITTAL_RIGHT]);
   pTrackballCam->setRotation(ImagingMatrices[SAGITTAL_RIGHT]);
	select_mode = SelectMode::Lobe;
	set_select_mode_colors();
	initiated = true;
	unselect_all();
}

void Tutorial::draw2DtutorialGui()
{
	ImVec2 texsize = ImGui_Impl_VR_GetTextureSize();
	static int tab = 0;
	ImGui_Impl_VR_NewFrame(0);

	ImGui::SetNextWindowSize(texsize);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
   ImGui::Begin("Tutorial", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

   tutorialScript();

   ImGui::End();
	ImGui_Impl_VR_Render(0);

	ImGui_Impl_VR_NewFrame(1);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(texsize);
	ImGui::Begin(" ", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::End();
	ImGui_Impl_VR_Render(1);

	ImGui_Impl_VR_NewFrame(2);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(texsize);
	ImGui::Begin(" ", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::End();
	ImGui_Impl_VR_Render(2);

	ImGui_Impl_VR_NewFrame(3);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(texsize);
   ImGui::Begin(" ", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

   ImGui::End();
	ImGui_Impl_VR_Render(3);
}

Tutorial::Tutorial()
{
   //Load the tutorial images
   void* image = nullptr;
   image = (void*)LoadTexture("images/trackpad.png");
   images.push_back(image);
   image = (void*)LoadTexture("images/trigger.png");
   images.push_back(image);

	infoFrontal= std::string("  The frontal lobe is located at the front of each cerebral hemisphere and positioned in front of")
		+" the parietal lobe and above and in front of the temporal lobe. It is separated from the parietal lobe by"
		+" a space between tissues called the central sulcus, and from the temporal lobe by a deep fold called"
		+" the lateral sulcus also called the Sylvian fissure. The precentral gyrus, forming the posterior border"
		+" of the frontal lobe, contains the primary motor cortex, which controls voluntary movements of specific"
		+" body parts.\n"
		+"\n"
		+"   The frontal lobe contains most of the dopamine - delicate neurons in the cerebral cortex. The"
		+" dopamine system is associated with reward, attention, short - term memory tasks, planning,"
		+" and motivation. Dopamine tends to limit and select sensory information arriving from the thalamus to the forebrain. \n"
		+ " \n";
	infoTemporal = std::string("   The temporal lobe is located beneath the lateral fissure on both cerebral hemispheres of the")
		+ " mammalian brain. The temporal lobe is involved in processing sensory input into derived meanings"
		+ " for the appropriate retention of visual memories, language comprehension, and emotion association.\n"
		+ " \n";
	infoOccipital = std::string("   The occipital lobe is the visual processing center of the mammalian brain containing most of the")
		+" anatomical region of the visual cortex. The primary visual cortex is Brodmann area 17, commonly"
		+" called V1(visual one). Human V1 is located on the medial side of the occipital lobe within"
		+" the calcarine sulcus; the full extent of V1 often continues onto the posterior pole of the occipital lobe.\n \n"
		+" V1 is often also called striate cortex because it can be identified by a large stripe of myelin, the Stria"
		+" of Gennari. Visually driven regions outside V1 are called extrastriate cortex. There are many"
		+" extrastriate regions, and these are specialized for different visual tasks, such as visuospatial"
		+" processing, color differentiation, and motion perception.\n"
		+ " \n";
	infoParietal = std::string("   The parietal lobe is positioned above the occipital lobe and behind the frontal lobe and central sulcus.\n")
		+ "\n"
		+ "   The parietal lobe integrates sensory information among various modalities, including spatial sense"
		+ " and navigation(proprioception), the main sensory receptive area for the sense of touch"
		+ " (mechanoreception) in the somatosensory cortex which is just posterior to the central sulcus in"
		+ " the postcentral gyrus, and the dorsal stream of the visual system. The major sensory inputs from"
		+ " the skin(touch, temperature, and pain receptors), relay through the thalamus to the parietal lobe.\n"
		+ " \n";
}


Tutorial::~Tutorial()
{
}
