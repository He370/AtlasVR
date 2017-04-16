#ifndef __VRCALLBACKS_H__
#define __VRCALLBACKS_H__

#include <openvr.h>
#include "SDL_keycode.h"


extern glm::mat4 M_controller[2];

void idle();

void vr_init(vr::IVRSystem* hmd);
void vr_event_callback(vr::VREvent_t* event);
void vr_motion_callback(vr::TrackedDeviceIndex_t device, vr::VRControllerState_t* state, glm::mat4& M);
void vr_keydown(SDL_Keycode key);

#endif