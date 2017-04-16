#include "ControllerSpline.h"

#include "glm/glm.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/spline.hpp>

ControllerSpline::ControllerSpline(int n) :tan_h(n), nor_h(n), pos_h(n), twist(0.0f)
{
   c[0] = 0.5f;
   c[1] = 0.5f;

   M_left = glm::scale(glm::vec3(1.0f));
   M_right = glm::scale(glm::vec3(1.0f));

   last_a_diff = 0.0f;
   twist_cd_gain = 1.0f;
}


glm::vec3 ControllerSpline::GetPoint(float t)
{
   const int n_points = pos_h.size();
   if (t <= 0.0f)
   {
      return pos_h[0];
   }
   if (t >= 1.0f)
   {
      return pos_h[n_points-1];
   }

   const glm::vec4 tanl(1.0f, 0.0f, 0.0f, 0.0f);
   const glm::vec4 norl(0.0f, 1.0f, 0.0f, 0.0f);
   const glm::vec4 tanr(1.0f, 0.0f, 0.0f, 0.0f);
   const glm::vec4 norr(0.0f, 1.0f, 0.0f, 0.0f);
   const glm::vec4 zero(0.0f, 0.0f, 0.0f, 1.0f);

   const glm::vec4 pl = M_left*zero;
   const glm::vec4 pr = M_right*zero;

   const glm::vec4 tl = M_left*tanl;
   const glm::vec4 tr = M_right*tanr;

   const glm::vec3 herm_tl = c[0] * glm::vec3(tl);
   const glm::vec3 herm_tr = c[1] * glm::vec3(tr);

   glm::vec3 h = glm::hermite(glm::vec3(pl), herm_tl, glm::vec3(pr), herm_tr, t);
   return h;
}

glm::vec3 ControllerSpline::GetNormal(float t)
{
   const int n_points = nor_h.size();
   if (t <= 0.0f)
   {
      return nor_h[0];
   }
   if (t >= 1.0f)
   {
      return nor_h[n_points - 1];
   }

   float fi = t*(n_points - 1);
   int i0 = glm::floor(fi);
   return glm::normalize(glm::mix(nor_h[i0], nor_h[i0 + 1], fi-i0));
}

glm::vec3 ControllerSpline::GetTangentInterp(float t)
{
   const int n_points = tan_h.size();
   if (t <= 0.0f)
   {
      return tan_h[0];
   }
   if (t >= 1.0f)
   {
      return tan_h[n_points - 1];
   }

   float fi = t*(n_points - 1);
   int i0 = glm::floor(fi);
   return glm::normalize(glm::mix(tan_h[i0], tan_h[i0 + 1], fi - i0));
}

glm::vec3 ControllerSpline::GetTangent(float t)
{
   t = glm::clamp(t, 0.0f, 1.0f);
   const float t2 = t*t;
   const float dh1 = 6.0f*t2 - 6.0f*t;
   const float dh2 = -6.0f*t2 + 6.0f*t;
   const float dh3 = 3.0f*t2 - 4.0f*t + 1.0f;
   const float dh4 = 3.0f*t2 - 2.0f*t;

   const glm::vec4 tanl(1.0f, 0.0f, 0.0f, 0.0f);
   const glm::vec4 tanr(1.0f, 0.0f, 0.0f, 0.0f);
   const glm::vec4 zero(0.0f, 0.0f, 0.0f, 1.0f);

   const glm::vec4 pl = M_left*zero;
   const glm::vec4 pr = M_right*zero;

   const glm::vec4 tl = M_left*tanl;
   const glm::vec4 tr = M_right*tanr;

   const glm::vec3 herm_tl = c[0] * glm::vec3(tl);
   const glm::vec3 herm_tr = c[1] * glm::vec3(tr);

   glm::vec3 tan = dh1 * glm::vec3(pl) + dh2 * glm::vec3(pr) + dh3 * herm_tl + dh4 * herm_tr;
   return glm::normalize(tan);
}

glm::vec3 ControllerSpline::GetFrenetNormal(float t)
{
   t = glm::clamp(t, 0.0f, 1.0f);
   const float t2 = t*t;
   const float dh1 = 6.0f*t2 - 6.0f*t;
   const float dh2 = -6.0f*t2 + 6.0f*t;
   const float dh3 = 3.0f*t2 - 4.0f*t + 1.0f;
   const float dh4 = 3.0f*t2 - 2.0f*t;

   const float ddh1 = 12.0f*t - 6.0f;
   const float ddh2 = -12.0f*t + 6.0f;
   const float ddh3 = 6.0f*t - 4.0f;
   const float ddh4 = 6.0f*t - 2.0f;

   const glm::vec4 tanl(1.0f, 0.0f, 0.0f, 0.0f);
   const glm::vec4 tanr(1.0f, 0.0f, 0.0f, 0.0f);
   const glm::vec4 zero(0.0f, 0.0f, 0.0f, 1.0f);

   const glm::vec4 pl = M_left*zero;
   const glm::vec4 pr = M_right*zero;

   const glm::vec4 tl = M_left*tanl;
   const glm::vec4 tr = M_right*tanr;

   const glm::vec3 herm_tl = c[0] * glm::vec3(tl);
   const glm::vec3 herm_tr = c[1] * glm::vec3(tr);

   glm::vec3 x1 = dh1 * glm::vec3(pl) + dh2 * glm::vec3(pr) + dh3 * herm_tl + dh4 * herm_tr;
   glm::vec3 x2 = ddh1 * glm::vec3(pl) + ddh2 * glm::vec3(pr) + ddh3 * herm_tl + ddh4 * herm_tr;

   glm::vec3 tan = glm::normalize(x1);
   glm::vec3 bin = glm::normalize(glm::cross(x1,x2));

   return glm::cross(bin, tan);
}



glm::mat4 ControllerSpline::GetRotation(float t)
{
   glm::vec3 tan = GetTangent(t);
   glm::vec3 nor = GetNormal(t);
   glm::vec3 bin = glm::cross(tan, nor);

   glm::mat4 M;
   M[0] = glm::vec4(tan, 0.0f);
   M[1] = glm::vec4(nor, 0.0f);
   M[2] = glm::vec4(bin, 0.0f);
   return M;
}

glm::mat4 ControllerSpline::GetRotation(int i)
{
   glm::vec3 tan = GetTangent(i);
   glm::vec3 nor = GetNormal(i);
   glm::vec3 bin = glm::cross(tan, nor);

   glm::mat4 M;
   M[0] = glm::vec4(tan, 0.0f);
   M[1] = glm::vec4(nor, 0.0f);
   M[2] = glm::vec4(bin, 0.0f);
   return M;
}

void ControllerSpline::Update(const glm::mat4& Ml, const glm::mat4& Mr)
{
   M_left = Ml;
   M_right = Mr;

   glm::mat4 Rl = Ml;
   Rl[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

   glm::mat4 Rr = Mr;
   Rr[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

   glm::quat Ql = glm::quat(Rl);
   glm::quat Qr = glm::quat(Rr);

   const glm::vec4 tanl(1.0f, 0.0f, 0.0f, 0.0f);
   const glm::vec4 norl(0.0f, 1.0f, 0.0f, 0.0f);
   const glm::vec4 tanr(1.0f, 0.0f, 0.0f, 0.0f);
   const glm::vec4 norr(0.0f, 1.0f, 0.0f, 0.0f);
   const glm::vec4 zero(0.0f, 0.0f, 0.0f, 1.0f);

   const glm::vec4 pl = Ml*zero;
   const glm::vec4 pr = Mr*zero;
   const glm::vec4 tl = Ml*tanl;
   const glm::vec4 tr = Mr*tanr;
   const glm::vec4 nl = Ml*norl;
   const glm::vec4 nr = Mr*norr;

   if (pl == pr)
   {
      return;
   }

   float d = glm::distance(pl, pr);

   const int n_points = tan_h.size();
   const glm::vec3 herm_tl = c[0] * glm::vec3(tl);
   const glm::vec3 herm_tr = c[1] * glm::vec3(tr);

   for (int i = 0; i < n_points; i++)
   {
      float t = i / (n_points - 1.0f);
      glm::vec3 h = glm::hermite(glm::vec3(pl), herm_tl, glm::vec3(pr), herm_tr, t);
      glm::vec3 tan = GetTangent(t);
      glm::vec3 nor = GetFrenetNormal(t);

      tan_h[i] = tan;
      nor_h[i] = nor;
      pos_h[i] = h;

      glm::quat Qinterp = glm::slerp(Ql, Qr, t);
      glm::mat4 Minterp = glm::mat4(Qinterp);
      glm::vec3 norq = glm::vec3(Minterp*glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
      nor_h[i] = norq;
   }

   float l0 = glm::distance(pos_h[1], pos_h[0]);
   float lend = glm::distance(pos_h[n_points - 2], pos_h[n_points - 1]);
   float lmid = glm::distance(pos_h[n_points / 2], pos_h[n_points / 2 + 1]);

   c[0] *= lmid / l0;
   c[1] *= lmid / lend;

   //compute parallel transport frame(wenping wang 2007) 
   for (int i = 0; i < n_points - 1; i++)
   {
      glm::vec3 ri = nor_h[i];
      glm::vec3 ti = tan_h[i];
      glm::vec3 v1 = pos_h[i + 1] - pos_h[i];
      float c1 = glm::dot(v1, v1);

      glm::vec3 rL = ri - (2.0f / c1)*glm::dot(v1, ri)*v1;
      glm::vec3 tL = ti - (2.0f / c1)*glm::dot(v1, ti)*v1;
      glm::vec3 v2 = tan_h[i + 1] - tL;
      float c2 = glm::dot(v2, v2);
      nor_h[i + 1] = rL - (2.0f / c2)*glm::dot(v2, rL)*v2;
   }

   //modify parallel transport to satisfy right boundary condition
   float cosa = dot(nor_h[n_points - 1], glm::vec3(nr));
   glm::vec3 cr = cross(nor_h[n_points - 1], glm::vec3(nr));
   float a_diff = twist_cd_gain*atan2(dot(tan_h[n_points - 1], cr), cosa);

   const float HALF_PI = 3.1415926f / 2.0f;
   const float PI = 3.1415926f;

   if (last_a_diff > 2.0f && a_diff < -2.0f)
   {
      twist += twist_cd_gain*2.0f*PI;
   }
   else if (last_a_diff < -2.0f && a_diff > 2.0f)
   {
      twist -= twist_cd_gain*2.0f*PI;
   }

   last_a_diff = a_diff;

   //adjust for boundary conditions
   for (int i = 0; i < n_points; i++)
   {
      float t = i / (n_points - 1.0f);
      float ang = (a_diff + twist)*t;
      glm::vec3 ti = tan_h[i];
      glm::mat4 R = glm::rotate(ang, ti);
      glm::mat3 R3 = glm::mat3(R);
      nor_h[i] = R3*nor_h[i];
   }
}