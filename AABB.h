#ifndef __AABB_H__
#define __AABB_H__

#include <glm/glm.hpp>

struct AABB
{

   AABB();
   AABB(glm::vec3& min, glm::vec3& max);

   glm::vec3 mMin;
   glm::vec3 mMax;

   void extend(glm::vec3& pt);
   void extend(AABB& aabb);
   glm::vec3 getCenter();
   glm::vec3 getDimensions();

   int selectedEdge;
   glm::vec3 getSelectedEdgePt();
   glm::vec3 getSelectedEdgeDir();
   
   bool isValid();

};

#endif