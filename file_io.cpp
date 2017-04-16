#include <fstream>
#include "file_io.h"
#include "VirtualTrackball.h"

extern VirtualTrackball* pTrackballCam;
extern AnimTransform CameraAnim;


std::ostream &operator<< (std::ostream &out, const glm::mat4 &M)
{
   out << "mat4 ";
   for (int i = 0; i<4; i++)
   {
      out << M[i].x << " " << M[i].y << " " << M[i].z << " " << M[i].w << " ";
   }
   return out;
}

std::istream& operator>> (std::istream &in, glm::mat4 &M)
{
   std::string header; //should be "mat4"
   in >> header;
   for (int i = 0; i<4; i++)
   {
      in >> M[i].x >> M[i].y >> M[i].z >> M[i].w;
   }

   return in;
}

std::ostream &operator<< (std::ostream &out, const glm::vec3 &v)
{
   out << "vec3 ";
   out << v.x << " " << v.y << " " << v.z << " ";
   return out;
}

std::istream& operator>> (std::istream &in, glm::vec3 &v)
{
   std::string header; //should be "vec3"
   in >> header;
   in >> v.x >> v.y >> v.z;

   return in;
}

std::ostream &operator<< (std::ostream &out, const glm::quat &v)
{
   out << "quat ";
   out << v.x << " " << v.y << " " << v.z << " " << v.w << " ";
   return out;
}

std::istream& operator>> (std::istream &in, glm::quat &v)
{
   std::string header; //should be "quat"
   in >> header;
   in >> v.x >> v.y >> v.z >> v.w;

   return in;
}

std::ostream &operator<< (std::ostream &out, const Transform &t)
{
   out << "Transform ";
   out << t.translation << " " << t.rotation << " ";
   return out;
}

std::istream& operator>> (std::istream &in, Transform &t)
{
   std::string header; //should be "Transform"
   in >> header;
   in >> t.translation >>  t.rotation;

   return in;
}

std::ostream &operator<< (std::ostream &out, const AnimTransform &t)
{
   out << "AnimTransform ";
   out << t.currentTransform << " " << t.initTransform << " " << t.targetTransform << " ";
   out << std::endl;
   return out;
}

std::istream& operator>> (std::istream &in, AnimTransform &t)
{
   std::string header; //should be "AnimTransform"
   in >> header;
   in >> t.currentTransform >> t.initTransform >> t.targetTransform;

   return in;
}

std::ostream &operator<< (std::ostream &out, const ModelUniforms &u)
{
   const std::string spc(" ");
   out << "ModelUniforms ";
   out << u.P << spc;
   out << u.instances << spc;
   out << u.slab_width << spc;
   out << u.slab_offset << spc;
   out << u.slab_instance_offset << spc;
   out << std::endl;

   return out;
}

std::istream& operator>> (std::istream &in, ModelUniforms &u)
{
   std::string header; //should be "ModelUniforms"
   in >> header;
   in >> u.P;
   in >> u.instances;
   in >> u.slab_width;
   in >> u.slab_offset;
   in >> u.slab_instance_offset;

   return in;
}

void file_load_interp(const std::string& path)
{
   std::fstream fs;
   std::string spc(" ");
   fs.open(path, std::fstream::in);

   std::string header;


   fs >> header; //Camera

   float dolly;
   fs >> dolly;

   glm::mat4 V;
   fs >> V;


   CameraAnim.SetTargetTranslation(glm::translate(glm::vec3(0.0f, 0.0f, dolly)));
   CameraAnim.SetTargetRotation(V);

   glm::mat4 Vinit = pTrackballCam->getAccumRotationMatrix();
   Vinit[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
   CameraAnim.initTransform.translation = glm::translate(pTrackballCam->getDolly());
   CameraAnim.initTransform.rotation = Vinit;


   fs >> header;

   fs >> BrainAABB.selectedEdge;

   for (auto &aabb : HemisphereAABB)
   {
      fs >> aabb.selectedEdge;
   }

   for (auto &aabb : LobeAABB)
   {
      fs >> aabb.selectedEdge;
   }

   for (auto &aabb : RegionAABB)
   {
      fs >> aabb.selectedEdge;
   }

   //Model Data
   fs >> header;

   for (auto &model : model_data)
   {
      fs >> model.hidden;
   }

   //Model Uniforms
   fs >> header;

   for (auto &model : model_data)
   {
      ModelUniforms pre_uniforms = model.model_uniforms;

      fs >> model.model_uniforms;

      if (pre_uniforms.instances > model.model_uniforms.instances)
      {
         model.model_uniforms.instances = pre_uniforms.instances;
         model.model_uniforms.P = pre_uniforms.P;
         model.model_uniforms.slab_width = pre_uniforms.slab_width;
         model.model_uniforms.slab_offset = pre_uniforms.slab_offset;
         model.model_uniforms.slab_instance_offset = pre_uniforms.slab_instance_offset;
         

      }
      
   }

   fs >> header;
   for (auto &x : HemisphereMatrices)
   {
      Transform init = x.currentTransform;
      fs >> x;

      x.initTransform = init;
      x.t = 1.0f;
   }


   fs >> header;
   for (auto &x : LobeMatrices)
   {
      Transform init = x.currentTransform;
      fs >> x;

      x.initTransform = init;
      x.t = 1.0f;
   }

   fs >> header;
   for (auto &x : RegionMatrices)
   {
      Transform init = x.currentTransform;
      fs >> x;

      x.initTransform = init;
      x.t = 1.0f;
   }

   fs >> header;
   for (auto &v : InstanceMatrices)
   {
      for (auto &x : v)
      {
         Transform init = x.currentTransform;
         fs >> x;

         x.initTransform = init;
         x.t = 1.0f;
      }
   }

   fs.close();
}


void file_load(const std::string& path)
{
   std::fstream fs;
   std::string spc(" ");
   fs.open(path, std::fstream::in);

   std::string header;


   fs >> header; //Camera

   float dolly;
   fs >> dolly;

   glm::mat4 V;
   fs >> V;

   pTrackballCam->setDolly(glm::vec3(dolly, 0.0f, 0.0f));
   pTrackballCam->setRotation(V);


   fs >> header;

   fs >> BrainAABB.selectedEdge;

   for (auto &aabb : HemisphereAABB)
   {
      fs >> aabb.selectedEdge;
   }

   for (auto &aabb : LobeAABB)
   {
      fs >> aabb.selectedEdge;
   }

   for (auto &aabb : RegionAABB)
   {
      fs >> aabb.selectedEdge;
   }

   //Model Data
   fs >> header;

   for (auto &model : model_data)
   {
      fs >> model.hidden;
   }

   //Model Uniforms
   fs >> header;

   for (auto &model : model_data)
   {
      fs >> model.model_uniforms;
   }

   fs >> header;
   for (auto &x : HemisphereMatrices)
   {
      fs >> x;
   }


   fs >> header;
   for (auto &x : LobeMatrices)
   {
      fs >> x;
   }

   fs >> header;
   for (auto &x : RegionMatrices)
   {
      fs >> x;
   }

   fs >> header;
   for (auto &v : InstanceMatrices)
   {
      for (auto &x : v)
      {
         fs >> x;
      }
   }

   fs.close();
}




void file_save(const std::string& path)
{
   std::fstream fs;
   std::string spc(" ");
   fs.open(path, std::fstream::out);

   fs << "Camera" << std::endl;

   fs << pTrackballCam->getDolly() << std::endl;

   glm::mat4 V = pTrackballCam->getAccumRotationMatrix();
   V[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
   fs << V << std::endl;

   fs << "AABB_Selected_Edges" << std::endl;

   fs << BrainAABB.selectedEdge << std::endl;

   for (auto &aabb : HemisphereAABB)
   {
      fs << aabb.selectedEdge << spc;
   }
   fs << std::endl;

   for (auto &aabb : LobeAABB)
   {
      fs << aabb.selectedEdge << spc;
   }
   fs << std::endl;

   for (auto &aabb : RegionAABB)
   {
      fs << aabb.selectedEdge << spc;
   }
   fs << std::endl;

   fs << "Model_Data" << std::endl;
   for (auto &model : model_data)
   {
      fs << model.hidden << spc;
   }
   fs << std::endl;

   fs << "Model_Uniforms" << std::endl;
   for (auto &model : model_data)
   {
      fs << model.model_uniforms;
   }

   fs << "HemisphereMatrices" << std::endl;
   for (auto &x : HemisphereMatrices)
   {
      fs << x;
   }
   fs << std::endl;

   fs << "LobeMatrices" << std::endl;
   for (auto &x : LobeMatrices)
   {
      fs << x;
   }
   fs << std::endl;

   fs << "RegionMatrices" << std::endl;
   for (auto &x : RegionMatrices)
   {
      fs << x;
   }
   fs << std::endl;

   fs << "InstanceMatrices" << std::endl;
   for (auto &v : InstanceMatrices)
   {
      for (auto &x : v)
      {
         fs << x;
      }
   }
   fs << std::endl;

   fs.close();
}