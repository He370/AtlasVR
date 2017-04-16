#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct Transform
{
   glm::mat4 translation;
   glm::mat4 rotation;

   Transform();
};



struct AnimTransform
{
   Transform currentTransform;
   Transform initTransform;
   Transform targetTransform;

   float t; //interpolation parameter
   float speed;

   AnimTransform();
   void Update(float d_time);
   void SetTarget(const Transform& target);
   void SetTargetTranslation(const glm::mat4& t);
   void SetTargetRotation(const glm::mat4& r);
   void Reverse();

   glm::mat4 GetCurrentMatrix();
   glm::mat4 GetCurrentSlabMatrix();
};


#endif