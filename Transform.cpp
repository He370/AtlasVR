#include "Transform.h"
#include <glm/gtx/transform.hpp>

Transform::Transform() : translation(1.0f), rotation(1.0f)
{

}


AnimTransform::AnimTransform() : t(0.0f), speed(1.0f)
{

}

void AnimTransform::Update(float d_time)
{
   
   t -= speed*d_time;

   if(t >= 0.0f)
   {
      currentTransform.translation = glm::mix(targetTransform.translation, initTransform.translation, t);
      currentTransform.rotation = glm::mix(targetTransform.rotation, initTransform.rotation, t);
   }
}

void AnimTransform::Reverse()
{
   Transform temp = initTransform;
   initTransform = targetTransform;
   targetTransform = temp;
   t = 1.0f;
}

void AnimTransform::SetTarget(const Transform& target)
{
   t = 1.0f;
   targetTransform = target;
   initTransform = currentTransform;
}

void AnimTransform::SetTargetTranslation(const glm::mat4& target)
{
   t = 1.0f;
   targetTransform.translation = target;
   initTransform.translation = currentTransform.translation;
}

void AnimTransform::SetTargetRotation(const glm::mat4& q)
{
   t = 1.0f;
   targetTransform.rotation = q;
   initTransform.rotation = currentTransform.rotation;
}

glm::mat4 AnimTransform::GetCurrentMatrix()
{
   glm::mat4 T = targetTransform.translation;
   glm::mat4 R = targetTransform.rotation;

   return T*R;
}

glm::mat4 AnimTransform::GetCurrentSlabMatrix()
{
   glm::mat4 T = currentTransform.translation;
   glm::mat4 R = currentTransform.rotation;

   return T*R;
}