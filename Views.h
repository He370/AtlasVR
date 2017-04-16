#pragma once

#include "glm/glm.hpp"

enum ImagingPlane
{
   AXIAL_TOP,
   AXIAL_BOTTOM,
   SAGITTAL_RIGHT,
   SAGITTAL_LEFT,
   CORONAL_FRONT,
   CORONAL_BACK,
   ISOMETRIC,
   NUM_IMAGING_PLANES
};

extern glm::mat4 ImagingMatrices[];

