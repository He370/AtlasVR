#include "Color.h"

namespace CubehelixParams
{
   float ch_start = 0.5, ch_cycles = -1.5, ch_saturation = 1.5, ch_gamma = 2.0;
};

glm::vec3 Get_Colour_Cubehelix (float f)
{
   using namespace CubehelixParams;
   double phi, a;
   
   phi = 2. * 3.14 * (ch_start/3.0 + f * ch_cycles);
   if (ch_gamma != 1.0)
   {
      f = pow(f, 1. / ch_gamma);
   }
   a = ch_saturation * f * (1. - f) / 2.;
   double r = f + a * (-0.14861 * cos(phi) + 1.78277 * sin(phi));
   double g = f + a * (-0.29227 * cos(phi) - 0.90649 * sin(phi));
   double b = f + a * ( 1.97294 * cos(phi));
   if ( r > 1.0 ) r = 1.0;
   if ( r < 0.0 ) r = 0.0;
   if ( g > 1.0 ) g = 1.0;
   if ( g < 0.0 ) g = 0.0;
   if ( b > 1.0 ) b = 1.0;
   if ( b < 0.0 ) b = 0.0;
   return glm::vec3(r, g, b);
}