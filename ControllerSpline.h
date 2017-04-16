#pragma once

#include <vector>
#include "glm/glm.hpp"

class ControllerSpline
{
   public:
      ControllerSpline(int n);
      void Update(const glm::mat4& Ml, const glm::mat4& Mr);

      glm::vec3 GetPoint(int i) { return pos_h[i]; }
      glm::vec3 GetNormal(int i) { return nor_h[i]; }
      glm::vec3 GetTangent(int i) { return tan_h[i]; }
      glm::mat4 GetRotation(int i);

      glm::vec3 GetPoint(float t);
      glm::vec3 GetNormal(float t);
      glm::vec3 GetTangent(float t);
      glm::mat4 GetRotation(float t);
      glm::vec3 GetTangentInterp(float t);
      glm::vec3 GetFrenetNormal(float t);

      float GetC(int i) { return c[i]; }
      float GetTwistCdGain(void) { return twist_cd_gain; }

      void SetC(int i, float ci) { c[i] = ci; }
      void SetTwist(float tw) { twist = tw; }
      void SetTwistCdGain(float cd) { twist_cd_gain = cd; }

   private:
      std::vector<glm::vec3> tan_h;
      std::vector<glm::vec3> nor_h;
      std::vector<glm::vec3> pos_h;

      glm::mat4 M_left;
      glm::mat4 M_right;

      float c[2];
      float twist = 0.0f;
      float last_a_diff;
      float twist_cd_gain;

};
