#include "AABB.h"

AABB::AABB() :mMin(FLT_MAX), mMax(FLT_MIN), selectedEdge(0)
{

}


AABB::AABB(glm::vec3& min, glm::vec3& max) :mMin(min), mMax(max), selectedEdge(0)
{

}

void AABB::extend(glm::vec3& pt)
{
   mMin = glm::min(mMin, pt);
   mMax = glm::max(mMax, pt);
}

void AABB::extend(AABB& aabb)
{
   mMin = glm::min(mMin, aabb.mMin);
   mMax = glm::max(mMax, aabb.mMax);

}

glm::vec3 AABB::getCenter()
{
   return 0.5f*(mMin + mMax);
}

glm::vec3 AABB::getDimensions()
{
   return mMax-mMin;
}

bool AABB::isValid()
{
   return glm::all(glm::greaterThan(mMax, mMin));
}


glm::vec3 AABB::getSelectedEdgePt()
{
   using namespace glm;
   const vec3 pos[24] = { //12 edges
      vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, +1.0f, -1.0f), //bottom face
      vec3(-1.0f, +1.0f, -1.0f), vec3(+1.0f, +1.0f, -1.0f), //bottom face
      vec3(+1.0f, +1.0f, -1.0f), vec3(+1.0f, -1.0f, -1.0f), //bottom face
      vec3(+1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f, -1.0f), //bottom face
      vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f, +1.0f), //top face
      vec3(-1.0f, +1.0f, -1.0f), vec3(-1.0f, +1.0f, +1.0f), //top face
      vec3(+1.0f, -1.0f, -1.0f), vec3(+1.0f, -1.0f, +1.0f), //top face
      vec3(+1.0f, +1.0f, -1.0f), vec3(+1.0f, +1.0f, +1.0f), //top face
      vec3(-1.0f, -1.0f, +1.0f), vec3(-1.0f, +1.0f, +1.0f), //vertical edges
      vec3(-1.0f, +1.0f, +1.0f), vec3(+1.0f, +1.0f, +1.0f), //vertical edges
      vec3(+1.0f, +1.0f, +1.0f), vec3(+1.0f, -1.0f, +1.0f), //vertical edges
      vec3(+1.0f, -1.0f, +1.0f), vec3(-1.0f, -1.0f, +1.0f)  //vertical edges
   };

   glm::vec3 p = pos[selectedEdge*2];

   if(p.x < 0.0)
   {
      p.x = mMin.x;
   }
   else
   {
      p.x = mMax.x;
   }

   if(p.y < 0.0)
   {
      p.y = mMin.y;
   }
   else
   {
      p.y = mMax.y;
   }

   if(p.z < 0.0)
   {
      p.z = mMin.z;
   }
   else
   {
      p.z = mMax.z;
   }
   return p;


}

glm::vec3 AABB::getSelectedEdgeDir()
{
   using namespace glm;
   const vec3 pos[24] = { //12 edges
      vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, +1.0f, -1.0f), //bottom face
      vec3(-1.0f, +1.0f, -1.0f), vec3(+1.0f, +1.0f, -1.0f), //bottom face
      vec3(+1.0f, +1.0f, -1.0f), vec3(+1.0f, -1.0f, -1.0f), //bottom face
      vec3(+1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f, -1.0f), //bottom face
      vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f, +1.0f), //top face
      vec3(-1.0f, +1.0f, -1.0f), vec3(-1.0f, +1.0f, +1.0f), //top face
      vec3(+1.0f, -1.0f, -1.0f), vec3(+1.0f, -1.0f, +1.0f), //top face
      vec3(+1.0f, +1.0f, -1.0f), vec3(+1.0f, +1.0f, +1.0f), //top face
      vec3(-1.0f, -1.0f, +1.0f), vec3(-1.0f, +1.0f, +1.0f), //vertical edges
      vec3(-1.0f, +1.0f, +1.0f), vec3(+1.0f, +1.0f, +1.0f), //vertical edges
      vec3(+1.0f, +1.0f, +1.0f), vec3(+1.0f, -1.0f, +1.0f), //vertical edges
      vec3(+1.0f, -1.0f, +1.0f), vec3(-1.0f, -1.0f, +1.0f)  //vertical edges
   };

   glm::vec3 p0 = pos[selectedEdge*2];
   glm::vec3 p1 = pos[selectedEdge*2+1];

   return glm::abs(p1-p0);
}
