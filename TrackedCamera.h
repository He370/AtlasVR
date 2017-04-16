#ifndef __TRACKEDCAMERA_H__
#define __TRACKEDCAMERA_H__

#include<openvr.h>
#include<GL/glew.h>
#include<string>

/*

To use this class:
InitTrackedCamera();
StartVideoPreview();
loop{
   GetTrackedCameraTextureID();
   YourTextureID = GetCameraTextureID();
}
StopVideoPreview();

*/
class TrackedCamera
{
public:
	TrackedCamera();
	~TrackedCamera();

	/*bool InitTrackedCamera(vr::IVRSystem *vr_system);
	    Check to ensure camera communication is valid
		Must enable "camera" in steamVR settings.
	*/
	bool InitTrackedCamera(vr::IVRSystem *vr_system);

	/*bool StartVideoPreview();
		Acquire video stream service and get camera handler m_hTrackedCamera.
	*/
	bool StartVideoPreview();

	/*void StopVideoPreview();
		Stop video stream service
	*/
	void StopVideoPreview();

	/*void GetTrackedCameraTextureID();
	    Get CameraFrameBuffer and copy the RGB information to Texture trackedCameraTextureID;
	*/
	void GetTrackedCameraTextureID();

	GLuint GetCameraTextureID() { return GLuint(trackedCameraTextureID); };

	vr::glUInt_t trackedCameraTextureID;

private:

	vr::IVRSystem					*m_pVRSystem;
	vr::IVRTrackedCamera			*m_pVRTrackedCamera;

	vr::TrackedCameraHandle_t		m_hTrackedCamera;

	std::string m_HMDSerialNumberString;

	uint32_t				m_nCameraFrameWidth;
	uint32_t				m_nCameraFrameHeight;
	uint32_t				m_nCameraFrameBufferSize;
	uint8_t					*m_pCameraFrameBuffer;

	uint32_t				m_nLastFrameSequence;

	void InitCameraTexture(GLuint Cwidth, GLuint Cheight);

};

#endif

