#ifndef __COLOR_H__
#define __COLOR_H__

#include <glm/glm.hpp>

namespace CubehelixParams
{
   extern float ch_start, ch_cycles, ch_saturation, ch_gamma;
};

glm::vec3 Get_Colour_Cubehelix (float f);

#endif