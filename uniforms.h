#ifndef __UNIFORMS_H__
#define __UNIFORMS_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

struct PassUniforms
{
   glm::mat4 PV;	//camera projection * view matrix

   glm::vec4 La;	//ambient light color
   glm::vec4 Ld;	//diffuse light color
   glm::vec4 Ls;	//specular light color

   glm::vec4 eye_w;	//world-space eye position
   glm::vec4 light_w; //world-space light position

   int pass;
   float time;

};

extern PassUniforms defaultPassUniforms;
extern PassUniforms PassUniformData;

extern const int MAX_SLABS;

struct ModelUniforms
{
   glm::mat4 M;	//modeling matrix

   glm::mat4 P; //plane orientation

   glm::mat4 M_slab[10];	//slab transforms

   glm::vec4 ka;	//ambient material color
   glm::vec4 kd;	//diffuse material color
   glm::vec4 ks;	//specular material color

   float shininess;
   int instances;
   int id;
   int selected;

   float slab_width;
   float slab_offset;
   float slab_instance_offset;
   
};

extern ModelUniforms defaultModelUniforms;
extern ModelUniforms ModelUniformData;


#endif
