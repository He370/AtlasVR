#include "brain_hierarchy.h"
#include "vr_callbacks.h"
#include <GL/freeglut.h>
#include "VirtualTrackball.h"
#include "uniforms.h"
#include "Views.h"
#include "imgui_impl_vr.h"
#include "AtlasVR.h"
#include "Tutorial.h"

extern VirtualTrackball* pTrackballCam;
extern VirtualTrackball* pSceneMeshRotation;
extern AnimTransform CameraAnim;
extern const int PickW, PickH;
extern Tutorial* pTutorial;

extern bool grab_movie;
extern int render_mode;
extern int win;
extern int select_mode;
extern bool showHelpImage;
extern float reparam[2];

bool hide_ui = false;
bool hide_boxes = false;
bool hide_lines = true;

void draw_pick_pass(glm::mat4& V);

void LoadShader();
void handle_selection(int id, bool single);
void toggle_slabs(int id);
void next_select_mode();

void UpdateAllTransforms(float d_time);
int pick(int x, int y);

static vr::IVRSystem*		g_HMD = 0;
static glm::vec2 g_AxisPos[2] = { glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f) };
static glm::vec2 g_AxisClickedPos[2] = { glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f) };
//static float last_axis = 0.0f;
static bool axis_dragging[2] = { false, false };
static ImagingPlane current_view = ImagingPlane(0);

glm::mat4 M_controller[2] = { glm::mat4(1.0f), glm::mat4(1.0f) };

void vr_init(vr::IVRSystem* hmd)
{
	g_HMD = hmd;
}

void idle()
{
	static int last_time = 0;
	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time-last_time;
	last_time = time;
	float delta_seconds = 0.001f*elapsed;

   const float fixed_time_step = 1.0f/60.0f;

   PassUniformData.time += fixed_time_step;


   UpdateAllTransforms(fixed_time_step);

   if(hide_lines == false)
   {
      updateLines(); //TEMP DISABLE LINES
   }

   /*
   CameraAnim.Update(fixed_time_step);
   if(CameraAnim.t > 0.0f)
   {
      pTrackballCam->setRotation(CameraAnim.currentTransform.rotation);
   }
   */
}

void vr_event_callback(vr::VREvent_t* event)
{
	if (g_HMD == 0)
	{
		return;
	}
	const int left_controller = g_HMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
	const int right_controller = g_HMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

	if (event->trackedDeviceIndex == left_controller) //left controller is gui
	{
		const vr::VREvent_Controller_t controller = event->data.controller;
		if (controller.button == vr::k_EButton_SteamVR_Trigger)
		{
			if (event->eventType == vr::VREvent_ButtonTouch)
			{
            if (pTutorial->tutorialMode == false)
            {
               next_select_mode();
            }
			}
		}

      if (controller.button == vr::k_EButton_ApplicationMenu)
      {
         ImGui_Impl_VR_GrabScreenshot();
      }

		if (controller.button == vr::k_EButton_Grip)
		{
			if (event->eventType == vr::VREvent_ButtonPress)
			{
            if (pTutorial->tutorialMode == false)
            {
               showHelpImage = !showHelpImage;
            }
			}
		}

      if (controller.button == vr::k_EButton_Axis0)
      {

         vr::VRControllerState_t state;
         if (g_HMD->GetControllerState(left_controller, &state))
         {
            if (event->eventType == vr::VREvent_ButtonTouch)
            {
               g_AxisPos[0] = glm::vec2(state.rAxis[0].x, state.rAxis[0].y);
               g_AxisClickedPos[0] = g_AxisPos[0];
               axis_dragging[0] = true;
            }
            else if (event->eventType == vr::VREvent_ButtonUntouch)
            {
               axis_dragging[0] = false;
               g_AxisPos[0] = glm::vec2(0.0f, 0.0f);
               g_AxisClickedPos[0] = g_AxisPos[0];
            }
         }
      }
	}


	//convert right touchpad events to glut mouse events for trackball camera
	if (event->trackedDeviceIndex == right_controller) //right controller is mouse
	{
		const vr::VREvent_Controller_t controller = event->data.controller;
		if (controller.button == vr::k_EButton_SteamVR_Trigger)
		{
			if (event->eventType == vr::VREvent_ButtonTouch)
			{
				int id = pick(PickW / 2, PickH / 2);
				if (id >= 0)
				{
					bool pick_single = false;
					if (pTutorial != nullptr && pTutorial->tutorialMode==true) pick_single = true;
					handle_selection(id, pick_single);
					if(pTutorial != nullptr && pTutorial->tutorialMode==true) pTutorial->pickLobe(id);
				}

			}
		}
		if (controller.button == vr::k_EButton_Grip)
		{
			if (event->eventType == vr::VREvent_ButtonPress)
			{
            /*
				int id = pick(PickW / 2, PickH / 2);
				if (id >= 0)
				{
					toggle_slabs(id);
				}
            */
            if (pTutorial->tutorialMode == false)
            {
               applySelectedMatrices();
            }
			}
		}

		if (controller.button == vr::k_EButton_Axis0)
		{

			vr::VRControllerState_t state;
			if (g_HMD->GetControllerState(right_controller, &state))
			{
				if (event->eventType == vr::VREvent_ButtonTouch)
				{
					g_AxisPos[1] = glm::vec2(state.rAxis[0].x, state.rAxis[0].y);
					g_AxisClickedPos[1] = g_AxisPos[1];
					axis_dragging[1] = true;

               rotation_touch_callback(GLUT_DOWN);
				}
				else if (event->eventType == vr::VREvent_ButtonUntouch)
				{
					axis_dragging[1] = false;
					g_AxisPos[1] = glm::vec2(0.0f, 0.0f);
					g_AxisClickedPos[1] = g_AxisPos[1];

               rotation_touch_callback(GLUT_UP);
				}
				else if (event->eventType == vr::VREvent_ButtonPress)
				{
               if (pTutorial->tutorialMode == false)
               {
                  current_view = ImagingPlane((current_view + 1) % NUM_IMAGING_PLANES);
                  pTrackballCam->setRotation(ImagingMatrices[current_view]);
               }
				}

			}
		}
	}
}

void vr_motion_callback(vr::TrackedDeviceIndex_t device, vr::VRControllerState_t* state, glm::mat4& M)
{
	if (g_HMD == 0)
	{
		return;
	}
	const int left_controller = g_HMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
	const int right_controller = g_HMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
	if (device == right_controller)
	{
		M_controller[1] = M;
		if (state->ulButtonTouched & ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad))
		{
			
			if (axis_dragging[1] == true)
			{
            rotation_drag_callback(state->rAxis[0].x, state->rAxis[0].y);
            //float dx = (state->rAxis[0].x - g_AxisPos[1].x);
			}
			
			g_AxisPos[1] = glm::vec2(state->rAxis[0].x, state->rAxis[0].y);
		}
		draw_pick_pass(M);
      pick(PickW / 2, PickH / 2);
	}
	if (device == left_controller)
	{
		M_controller[0] = M;
      if (state->ulButtonTouched & ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad))
      {
         if (axis_dragging[0] == true)
         {
            float dx = (state->rAxis[0].x - g_AxisPos[0].x);
            reparam[0] += 10.0f*dx;
         }
         g_AxisPos[0] = glm::vec2(state->rAxis[0].x, state->rAxis[0].y);
      }
	}

}

void vr_keydown(SDL_Keycode key) //should really only use this for debugging
{
	if (key == SDLK_r)
	{
		LoadShader();
	}
}