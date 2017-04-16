#include <GL/glew.h>

#include "TrackedCamera.h"
#include <iostream>


//Vive Front Camera resolution 612*460
#define m_nTexutureHeight 460
#define m_nTexutureWidth 612

TrackedCamera::TrackedCamera()
{
	m_pVRSystem = nullptr;
	m_pVRTrackedCamera = nullptr;
	m_hTrackedCamera = INVALID_TRACKED_CAMERA_HANDLE;

	m_nCameraFrameWidth = 0;
	m_nCameraFrameHeight = 0;
	m_nCameraFrameBufferSize = 0;
	m_pCameraFrameBuffer = nullptr;

	trackedCameraTextureID = 0;
}


TrackedCamera::~TrackedCamera()
{
}

bool TrackedCamera::InitTrackedCamera(vr::IVRSystem *vr_system) {

	m_pVRSystem = vr_system;

	char systemName[1024];
	char serialNumber[1024];

	m_pVRSystem->GetStringTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String, systemName, sizeof(systemName));
	m_pVRSystem->GetStringTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String, serialNumber, sizeof(serialNumber));

	m_HMDSerialNumberString = serialNumber;

	printf("VR HMD: %s %s\n", systemName, serialNumber);


	m_pVRTrackedCamera = vr::VRTrackedCamera();
	if (!m_pVRTrackedCamera)
	{
		printf("Unable to get Tracked Camera interface.\n");
		return false;
	}

	bool bHasCamera = false;
	vr::EVRTrackedCameraError nCameraError = m_pVRTrackedCamera->HasCamera(vr::k_unTrackedDeviceIndex_Hmd, &bHasCamera);
	if (nCameraError != vr::VRTrackedCameraError_None || !bHasCamera)
	{
		printf("No Tracked Camera Available! (%s)\n", m_pVRTrackedCamera->GetCameraErrorNameFromEnum(nCameraError));
		return false;
	}

	// Accessing the FW description is just a further check to ensure camera communication is valid as expected.
	vr::ETrackedPropertyError propertyError;
	char buffer[128];
	m_pVRSystem->GetStringTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_CameraFirmwareDescription_String, buffer, sizeof(buffer), &propertyError);
	if (propertyError != vr::TrackedProp_Success)
	{
		printf("Failed to get tracked camera firmware description!\n");
		return false;
	}

	printf("Camera Firmware: %s\n\n", buffer);

	return true;

}


bool TrackedCamera::StartVideoPreview() 
{
	printf("StartVideoPreview()\n");
	vr::EVRTrackedCameraError nCameraError;

	// Allocate for camera frame buffer requirements
	uint32_t nCameraFrameBufferSize = 0;
	if (m_pVRTrackedCamera->GetCameraFrameSize(vr::k_unTrackedDeviceIndex_Hmd, vr::VRTrackedCameraFrameType_Undistorted, &m_nCameraFrameWidth, &m_nCameraFrameHeight, &nCameraFrameBufferSize) != vr::VRTrackedCameraError_None)
	{
		printf("GetCameraFrameBounds() Failed!\n");
		return false;
	}

	if (nCameraFrameBufferSize && nCameraFrameBufferSize != m_nCameraFrameBufferSize)
	{
		delete[] m_pCameraFrameBuffer;
		m_nCameraFrameBufferSize = nCameraFrameBufferSize;
		m_pCameraFrameBuffer = new uint8_t[m_nCameraFrameBufferSize];
		memset(m_pCameraFrameBuffer, 0, m_nCameraFrameBufferSize);
	}

	m_nLastFrameSequence = 0;

	//need Timer?

	m_pVRTrackedCamera->AcquireVideoStreamingService(vr::k_unTrackedDeviceIndex_Hmd, &m_hTrackedCamera);
	if (m_hTrackedCamera == INVALID_TRACKED_CAMERA_HANDLE)
	{
		printf("AcquireVideoStreamingService() Failed!\n");
		return false;
	}

	return true;
}

void TrackedCamera::StopVideoPreview() 
{
	printf("StopVideoPreview()\n");

	m_pVRTrackedCamera->ReleaseVideoStreamingService(m_hTrackedCamera);
	m_hTrackedCamera = INVALID_TRACKED_CAMERA_HANDLE;
}


void TrackedCamera::GetTrackedCameraTextureID()
{
	if (!m_pVRTrackedCamera || !m_hTrackedCamera) 
		return;

	// get the frame header only
	vr::CameraVideoStreamFrameHeader_t frameHeader;
	vr::EVRTrackedCameraError nCameraError = m_pVRTrackedCamera->GetVideoStreamFrameBuffer(m_hTrackedCamera, vr::VRTrackedCameraFrameType_Undistorted, nullptr, 0, &frameHeader, sizeof(frameHeader));
	if (nCameraError != vr::VRTrackedCameraError_None)
		return;

	if (frameHeader.nFrameSequence == m_nLastFrameSequence)
	{
		// frame hasn't changed yet, nothing to do
		return;
	}

	// Frame has changed, do the more expensive frame buffer copy
	nCameraError = m_pVRTrackedCamera->GetVideoStreamFrameBuffer(m_hTrackedCamera, vr::VRTrackedCameraFrameType_Undistorted, m_pCameraFrameBuffer, m_nCameraFrameBufferSize, &frameHeader, sizeof(frameHeader));
	if (nCameraError != vr::VRTrackedCameraError_None)
		return;

	m_nLastFrameSequence = frameHeader.nFrameSequence;

	if (trackedCameraTextureID == 0) {
		InitCameraTexture(frameHeader.nWidth, frameHeader.nHeight);
	}

	//m_pCameraPreviewImage->SetFrameImage(m_pCameraFrameBuffer, m_nCameraFrameWidth, m_nCameraFrameHeight, &frameHeader);
	uint8_t TextureBuffer[m_nTexutureHeight][m_nTexutureWidth][3];
	uint8_t* tempCameraFrameBuffer = m_pCameraFrameBuffer;
	for (uint32_t y = 0; y < m_nTexutureHeight; y++)
	{
		for (uint32_t x = 0; x < m_nTexutureWidth; x++)
		{
			TextureBuffer[y][x][0] = tempCameraFrameBuffer[0];
			TextureBuffer[y][x][1] = tempCameraFrameBuffer[1];
			TextureBuffer[y][x][2] = tempCameraFrameBuffer[2];
			tempCameraFrameBuffer += 4;
		}
	}
	glBindTexture(GL_TEXTURE_2D, trackedCameraTextureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameHeader.nWidth, frameHeader.nHeight, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)TextureBuffer);
}

//initiate Camera Texture after activate camera service
void TrackedCamera::InitCameraTexture(GLuint Cwidth,GLuint Cheight) {

	glGenTextures(1, &trackedCameraTextureID);
	glBindTexture(GL_TEXTURE_2D, trackedCameraTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Cwidth, Cheight, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

}
