#include <windows.h>

#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/spline.hpp>

#include "InitShader.h"
#include "LoadMesh.h"
#include "LoadTexture.h"
#include "Cube.h"
#include "VirtualTrackball.h"
#include "uniforms.h"
#include "vr_callbacks.h"
#include "Color.h"
#include "file_io.h"
#include "glUtils.h"
#include "brain_hierarchy.h"
#include "AtlasVR.h"
#include "AtlasVRGui.h"
#include "ControllerSpline.h"

#include "BuildTree.h"
#include "HelpImage.h"
#include "Labels.h"
#include "Tutorial.h"

extern Tutorial* pTutorial;

GLuint vr_fbo;

using namespace std;
int win = 0;

int Tex3d_loc = -1;
int Tex1d_loc = -1;
int selected_slab_loc = -1;

//shader attrib locations
int pos_loc = -1;
int tex_coord_loc = -1;

//bool grab_movie = false;

VirtualTrackball* pTrackballCam;
VirtualTrackball* pSceneMeshRotation;
AnimTransform CameraAnim;


glm::mat4 M0 = glm::scale(glm::vec3(1.0f/128.0f))*glm::translate(glm::vec3(-128.0f, -128.0f, -128.0f));
glm::mat4 PVvr_last;
glm::mat4 HMDpose;

//opengl shader object
GLuint line_program = -1;
GLuint program = -1;
GLuint image_program = -1;
GLuint label_program = -1;
GLuint uniform_buffer = -1;

GLuint model_uniform_buffer = -1;
GLuint pass_uniform_buffer = -1;

GLuint TextureID = 0;
GLuint FBOID = 0;       // framebuffer object,
GLuint RBOID = 0;       // and renderbuffer (for depth buffering)
GLuint TimerQuery = 0;

extern const int PickW = 256, PickH = 256;
GLuint PickID = 0;
GLuint PickFBO = 0;       // framebuffer object,
GLuint PickRBO = 0;       // and renderbuffer (for depth buffering)
int last_picked = -1;

GLuint tex3d_id = -1;
GLuint tex1d_id = -1;

GLuint instance_back_faces_tex;

//static const std::string tex3dname = "IIT/IITmean_t1_256.nii.gz";
//static const std::string tex3dname = "ch2bet/ch2bet.nii.gz";
static const std::string tex3dname = "pad_ch2bet/ch2bet.nii.gz";
static const std::string tex1dname = "palette.png";

std::string selection_name(" ");


int WindowWidth = 1512;
int WindowHeight = 1680;
float frame_render_time = 0.0f;

//scales
//scale of mesh above right controller
float atlas_mesh_scale = 1.0f;
float select_model_uniform_scale = 10.0f;

MeshData cube_mesh;
MeshData box_mesh;

HelpImage helpImage;
bool showHelpImage = false;

bool lock_3p_camera = false;

//Manipulated slabs
const int n_manip_slabs = 10;
glm::mat4 M_manip_slabs[n_manip_slabs];
glm::mat4 Mr = glm::scale(glm::vec3(1.0f));
glm::mat4 Ml = glm::scale(glm::vec3(1.0f));

ControllerSpline slab_spl(n_manip_slabs);
ControllerSpline* pMesh_spl = nullptr;


void LoadAALScene();
int update_manip_slabs();

float reparam[2] = { 1.0f, 1.0f };

#include <GL/freeglut.h>

void rotation_drag_callback(float x, float y)
{
   if(last_picked != -1)
   {
      
      pSceneMeshRotation->cursorCallback(x, y);
   }
   pTrackballCam->cursorCallback(x, y);
   
}

void rotation_touch_callback(int state)
{
   if(last_picked != -1)
   {
      pSceneMeshRotation->mouseButtonCallback(GLUT_LEFT_BUTTON, state);
   }
   pTrackballCam->mouseButtonCallback(GLUT_LEFT_BUTTON, state);
}

float hermite(float p0, float m0, float p1, float m1, float t)
{
   const float t2 = t*t;
   const float t3 = t2*t;
   const float h00 = 2.0f*t3 - 3.0f*t2 + 1.0f;
   const float h10 = t3 - 2.0f*t2 + t;
   const float h01 = -2.0f*t3 + 3.0f*t2;
   const float h11 = t3 - t2;

   return h00*p0 + h10*m0 + h01*p1 + h11*m1;
}

void LookAtSelection()
{
   glm::vec3 t(-128.0f, -128.0f, -128.0f);
   switch (select_mode)
   {
      case SelectMode::Brain:
         t = -BrainAABB.getCenter(); 
      break;

      case SelectMode::Hemisphere:
         for(int i=0; i< HemisphereAABB.size(); i++)
         {
            if(is_hemisphere_selected(i) == true)
            {
               glm::mat4 M = getHemisphereGlobalMatrix(i);
               glm::vec4 t0 = glm::vec4(HemisphereAABB[i].getCenter(), 1.0f);
               t = glm::vec3(-(M*t0));
               break;
            }
         }
      break;

      case SelectMode::Lobe:
         for(int i=0; i< LobeAABB.size(); i++)
         {
            if(is_lobe_selected(i) == true)
            {
               if(LobeAABB[i].isValid())
               {
                  glm::mat4 M = getLobeGlobalMatrix(i);
                  glm::vec4 t0 = glm::vec4(LobeAABB[i].getCenter(), 1.0f);
                  t = glm::vec3(-(M*t0));
                  break;
               }
            }
         }
      break;

      case SelectMode::Region:
         for(int i=0; i< model_data.size(); i++)
         {
            if(model_data[i].model_uniforms.selected == true)
            {

               glm::mat4 M = getModelGlobalMatrix(model_data[i]);
               glm::vec4 t0 = glm::vec4(RegionAABB[i].getCenter(), 1.0f);
               t = glm::vec3(-(M*t0));
               break;
            }
         }
      break;
   }
   glm::mat4 T = glm::translate(t);
   M0 = glm::scale(glm::vec3(1.0f/128.0f))*T;
}

void DrawPass2(glm::mat4& P, glm::mat4& V) //skybox
{
	//draw pass : draw skybox
	using namespace glm;
	glBindFramebuffer(GL_FRAMEBUFFER, vr_fbo);
	glDisable(GL_CULL_FACE);

	PassUniformData.pass = 2;
	glm::mat4 Vsky = V;
	Vsky[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 oldPV = PassUniformData.PV;
	PassUniformData.PV = P*Vsky;
	glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);

	glm::mat4 S = glm::scale(glm::vec3(10.0f));
	
	glm::mat4 oldM = ModelUniformData.M;
	ModelUniformData.M = S;
	glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer); //Bind the OpenGL UBO before we update the data.
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &ModelUniformData);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);

	DrawCube(cube_mesh);

	// restore state;
	ModelUniformData.M = oldM;
	PassUniformData.PV = oldPV;
}


bool model_comp_region(ModelData* a, ModelData* b)
{
   const int ix_a = a->region;
   const int ix_b = b->region;

   glm::vec3 ca = RegionAABB[ix_a].getCenter();
   glm::vec3 cb = RegionAABB[ix_b].getCenter();

   return (ca.x < cb.x);
}

bool model_comp_cen(const glm::vec4& a, const glm::vec4& b)
{
   return (a.x < b.x);
}

void DrawPass3()
{
   //draw pass 3: draw region meshes
   using namespace glm;
   //glDrawBuffer(GL_BACK); //no - vr renders to texture
   glBindFramebuffer(GL_FRAMEBUFFER, vr_fbo);
   glDisable(GL_CULL_FACE);

   PassUniformData.pass=3;

   PassUniformData.PV = PVvr_last;
   glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);

   ModelUniformData.M = M0;
   glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &ModelUniformData);


   glEnable(GL_DEPTH_TEST);
   glDisable(GL_BLEND);

   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   
   //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);

   glm::mat4 S = glm::scale(glm::vec3(1.3f));
   glm::mat4 V = pTrackballCam->getAccumRotationMatrix();

   glm::mat4 T0 = glm::translate(glm::vec3(-128.0f, -128.0f, -128.0f));
   glm::mat4 T1 = glm::translate(glm::vec3(+128.0f, +128.0f, +128.0f));
   glm::mat4 Rscene = T1*pSceneMeshRotation->getAccumRotationMatrix()*T0;
   if(hide_scene_meshes == false)
   {
      //draw scene meshes
      for (auto &model : model_data) 
      {
         if (model.hidden == false && model.model_uniforms.instances==1)
         {
            glm::mat4 X = getModelGlobalMatrix(model);

            int id = -1;
            glm::vec3 cen;
            switch (select_mode)
            {
            case SelectMode::Brain:
               cen = BrainAABB.getCenter();
               break;
            case SelectMode::Hemisphere:
               id = model.hemisphere;
               cen = HemisphereAABB[id].getCenter();
               break;

            case SelectMode::Lobe:
               id = model.lobe;
               cen = LobeAABB[id].getCenter();
               break;

            case SelectMode::Region:
               id = model.region;
               cen = RegionAABB[id].getCenter();
               break;
            }

            int mouse_over = get_selected_id(last_picked);
            if ((id == mouse_over && id != -1) || (select_mode == SelectMode::Brain && last_picked != -1))
            {
               X = X*glm::scale(glm::vec3(1.0 + 0.01f*sin(5.0f*PassUniformData.time)));
            }

            model.model_uniforms.M = S*X*Rscene;
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &model.model_uniforms);

            glBindVertexArray(model.mesh_data.mVao);
            glDrawElements(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
         }
      }
   }

   //draw selected meshes
   if (interact_mode == InteractMode::Meshes)
   {

      //presorting
      static std::vector<glm::vec4> AABBcens;
      AABBcens.resize(0);

      float total_width = 0.0f;
      float fudge = 1.0f;

      //glm::mat4 R = pTrackballCam->getAccumRotationMatrix();
      //R[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

      glm::mat4 Tc = glm::translate(glm::vec3(-128.0f));
      glm::mat4 Rc = glm::inverse(Tc)*V*Tc;

      std::vector<AABB>* pAABB;
      if (select_mode == SelectMode::Region)
      {
         pAABB = &RegionAABB;
      }
      else if (select_mode == SelectMode::Lobe)
      {
         pAABB = &LobeAABB;
      }
      else
      {
         pAABB = &HemisphereAABB;
      }

      float n = 0.0f;
      for (auto &aabb : *pAABB)
      {
         if(!aabb.isValid())
         { 
            aabb.mMin = glm::vec3(0.0f);
            aabb.mMax = glm::vec3(0.0f);
         }

            total_width += fudge*aabb.getDimensions().x;

            glm::vec4 c = glm::vec4(aabb.getCenter(), 1.0f);
            glm::vec4 c2 = Rc*c;
            c2.w = n;

            AABBcens.push_back(c2);
            n += 1.0f;
      }

      float new_x = 0.0f;
      std::sort(AABBcens.begin(), AABBcens.end(), model_comp_cen);

      std::vector<float> nu(AABBcens.size(), 0.0f);
      for (auto &cen : AABBcens)
      {
         int ix = cen.w;
         AABB aabb = (*pAABB)[ix];
         if (!aabb.isValid())
         {
            aabb.mMin = glm::vec3(0.0f);
            aabb.mMax = glm::vec3(0.0f);
         }
         const float w = aabb.getDimensions().x;
         new_x += fudge*0.5f*w;
         nu[ix] = (new_x/total_width)*256.0f;
         new_x += fudge*0.5f*w;
      }

      //presorting

      PassUniformData.PV = PVvr_last;
      glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);
      glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer);
      S = glm::scale(glm::vec3(1.0f / select_model_uniform_scale));
      glm::mat4 T = glm::translate(glm::vec3(0.0f, 0.1f, 0.0f));

      const glm::vec4 zero(0.0f, 0.0f, 0.0f, 1.0f);
      glm::vec4 pl = Ml*zero;
      glm::vec4 pr = Mr*zero;
      float d = glm::distance(pl, pr);
      glm::mat4 S2 = glm::scale(glm::vec3(pow(d, 0.5f)*3.0f)); //temp
      //glm::mat4 S2 = glm::scale(glm::vec3(1.0f));

      glm::mat4 V = pTrackballCam->getAccumRotationMatrix();
      V[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
      glm::mat4 Vc = glm::inverse(M0)*V*M0;

      for (auto &model : model_data)
      {
         if (model.model_uniforms.selected == true)
         {

            float u0 = 0.5f;
            if (select_mode == SelectMode::Region)
            {
               int region = model.region;
               u0 = nu[region];
            }
            else if (select_mode == SelectMode::Lobe)
            {
               int lobe = model.lobe;
               u0 = nu[lobe];
            }
            else if (select_mode == SelectMode::Hemisphere)
            {
               int hemi = model.hemisphere;
               u0 = u0 = nu[hemi];
            }
            else if (select_mode == SelectMode::Brain)
            {
               u0 = 128.0f;
            }
           
            u0 = (u0 - 128.0f)*0.9f + 128.0f;
            u0 = u0 / 255.0f;
            u0 = clamp(u0, 0.0f, 1.0f);

            //reparam todo move outside loop)
            reparam[0] = clamp(reparam[0], -15.0f, 18.0f);
            reparam[1] = 2.0f - reparam[0];

            float u = hermite(0.0f, reparam[0], 1.0f, reparam[1], u0);
            u = clamp(u, 0.0f, 1.0f);

            bool drawTutorialLabel = (pTutorial->tutorialMode == true && pTutorial->getStage() < 3);
            if (drawTutorialLabel)
            {
               u = 0.0f;
            }

            glm::vec3 pos = pMesh_spl->GetPoint(u);
            glm::mat4 offset = glm::translate(pos);
            glm::mat4 Minterp = pMesh_spl->GetRotation(u);

            //test pop out code
            //glm::vec3 n = pMesh_spl->GetNormal(u);
            //glm::mat4 Pop = glm::translate(0.1f*n)*glm::scale(glm::vec3(2.0f));
            //float w = glm::smoothstep(0.05f, 0.15f, abs(u - 0.5f));
            //Pop = glm::mix(Pop, glm::scale(vec3(1.0f)), w);
            //model.model_uniforms.M = offset*Minterp*Pop*S2*T*V*S*M0;

            model.model_uniforms.M = offset*Minterp*S2*T*V*S*M0;

            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &model.model_uniforms);

            glBindVertexArray(model.mesh_data.mVao);
            glDrawElements(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);


            SelectedRegionMatrices[model.region].SetTargetTranslation(glm::translate(pTrackballCam->getDolly())*glm::scale(glm::vec3(10.0f))*glm::translate(-pMesh_spl->GetPoint(0.5f)));
            SelectedRegionMatrices[model.region].SetTargetRotation(offset*Minterp*S2*T*V*S*M0);
            SelectedMeshU[model.region] = u;
         }
      }
   }
}

void DrawPass4() //cube back faces for volume rendering
{
   glEnable(GL_CULL_FACE);
   glCullFace(GL_FRONT);

   using namespace glm;

   PassUniformData.pass=4;
   glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);

   ModelUniformData.M = glm::scale(glm::vec3(1.0f));
   glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &ModelUniformData);


   //draw pass 1: back faces of mesh to texture
   //set uniforms

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D_ARRAY, 0); //unbind texture so we can write to them (remember, no read-write access)
   glBindFramebuffer(GL_FRAMEBUFFER, FBOID);

   //Subsequent drawing should be captured by the framebuffer attached texture
   glDrawBuffer(GL_COLOR_ATTACHMENT0);
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      
   //clear the color texture and depth texture
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);

   for (auto &model : model_data) 
   {
      if (model.hidden == false && model.model_uniforms.instances==1)
      {
         glm::mat4 X = getModelGlobalMatrix(model);

         model.model_uniforms.M = M0*X;
         //model.model_uniforms.instances = 1;

         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &model.model_uniforms);

         glBindVertexArray(model.mesh_data.mVao);
	      //glDrawElements(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
         glDrawElementsInstanced(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0, model.model_uniforms.instances);
      }
   }
 
   //unbind framebuffer object
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   //end pass 1
}

void DrawPass5() //volume rendering
{
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   using namespace glm;

   PassUniformData.pass=5;
   glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);

   ModelUniformData.M = glm::mat4(1.0f);
   glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &ModelUniformData);

   glActiveTexture(GL_TEXTURE3);
   glBindTexture(GL_TEXTURE_3D, tex3d_id);
   if (Tex3d_loc != -1)
   {
      glUniform1i(Tex3d_loc, 3);
   }

   glActiveTexture(GL_TEXTURE4);
   glBindTexture(GL_TEXTURE_1D, tex1d_id);
   if (Tex1d_loc != -1)
   {
      glUniform1i(Tex1d_loc, 4);
   }
   

   glDrawBuffer(GL_BACK);
   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D_ARRAY, TextureID); 

   for (auto &model : model_data) 
   {
      if (model.hidden == false && model.model_uniforms.instances==1)
      {
         glm::mat4 X = getModelGlobalMatrix(model);

         model.model_uniforms.M = M0*X;
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &model.model_uniforms);

         glBindVertexArray(model.mesh_data.mVao);
	      //glDrawElements(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
         glDrawElementsInstanced(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0, model.model_uniforms.instances);
      }
   }
}

void DrawPass6() //picking
{
   using namespace glm;
   glUseProgram(program);

   PassUniformData.pass=6;
   glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);

   ModelUniformData.M = glm::mat4(1.0f);
   glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &ModelUniformData);


   //draw pass 6: mesh ids to PickID texture
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, 0); //unbind texture so we can write to them (remember, no read-write access)
   glBindFramebuffer(GL_FRAMEBUFFER, PickFBO);

   //Subsequent drawing should be captured by the framebuffer attached texture
   glDrawBuffer(GL_COLOR_ATTACHMENT0);
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      
   //clear the color texture and depth texture
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);

   glm::mat4 S = glm::scale(glm::vec3(1.3f));
   //glm::mat4 V = pTrackballCam->getAccumRotationMatrix();
   glm::mat4 T0 = glm::translate(glm::vec3(-128.0f, -128.0f, -128.0f));
   glm::mat4 T1 = glm::translate(glm::vec3(+128.0f, +128.0f, +128.0f));
   glm::mat4 Rscene = T1*pSceneMeshRotation->getAccumRotationMatrix()*T0;

   for (auto &model : model_data) 
   {
      if (model.hidden == false)
      {
         glm::mat4 X = getModelGlobalMatrix(model);

         model.model_uniforms.M = S*X*Rscene;
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &model.model_uniforms);

         glBindVertexArray(model.mesh_data.mVao);
	      //glDrawElements(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
         glDrawElementsInstanced(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0, model.model_uniforms.instances);
      }
   }
   
 
   //unbind framebuffer object
   glBindFramebuffer(GL_FRAMEBUFFER, vr_fbo);

   //end pass 6
}

void DrawPass7() //instanced cube back faces to 3d image for volume rendering
{
   glEnable(GL_CULL_FACE);
   glCullFace(GL_FRONT);

   using namespace glm;

   PassUniformData.pass = 7;
   glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);

   glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer); //Bind the OpenGL UBO before we update the data.

   //draw pass 7: back faces of cube to texture
   //set uniforms

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D_ARRAY, 0); //unbind texture so we can write to them (remember, no read-write access)
   glBindFramebuffer(GL_FRAMEBUFFER, FBOID);

   //Subsequent drawing should be captured by the framebuffer attached texture
   glDrawBuffer(GL_COLOR_ATTACHMENT0);
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

   glBindImageTexture(0, instance_back_faces_tex, 0, /*layered=*/GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
   int image_loc = glGetUniformLocation(program, "instance_backfaces");
   glUniform1i(image_loc, 0);

   //clear the color texture and depth texture
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);

   for (auto &model : model_data)
   {
      if (model.hidden == false && model.model_uniforms.instances > 1)
      {
         glm::mat4 X = getModelGlobalMatrix(model);
         model.model_uniforms.M = M0*X;
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &model.model_uniforms);

         glBindVertexArray(model.mesh_data.mVao);
         glDrawElementsInstanced(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0, model.model_uniforms.instances);
      }
   }

   //unbind framebuffer object
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   //end pass 7
}

void DrawPass8() //volume rendering
{
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   using namespace glm;

   PassUniformData.pass = 8;
   glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);

   glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer); //Bind the OpenGL UBO before we update the data.

   glActiveTexture(GL_TEXTURE3);
   glBindTexture(GL_TEXTURE_3D, tex3d_id);
   if (Tex3d_loc != -1)
   {
      glUniform1i(Tex3d_loc, 3);
   }

   glActiveTexture(GL_TEXTURE4);
   glBindTexture(GL_TEXTURE_1D, tex1d_id);
   if (Tex1d_loc != -1)
   {
      glUniform1i(Tex1d_loc, 4);
   }

   glBindImageTexture(0, instance_back_faces_tex, 0, /*layered=*/GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
   int image_loc = glGetUniformLocation(program, "instance_backfaces");
   glUniform1i(image_loc, 0);

   glDrawBuffer(GL_BACK);
   //don't clear, drawing on top of pass 5

   glUseProgram(program);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D_ARRAY, TextureID);

   for (auto &model : model_data)
   {
      if (model.hidden == false && model.model_uniforms.instances > 1)
      {
         glm::mat4 X = getModelGlobalMatrix(model);

         model.model_uniforms.M = M0*X;
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &model.model_uniforms);

         glBindVertexArray(model.mesh_data.mVao);
         glDrawElementsInstanced(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0, model.model_uniforms.instances);
      }
   }
}

void DrawPass7_8() //instanced cube back faces to 3d image for volume rendering
{

   using namespace glm;

   //draw pass 7: back faces of cube to texture
   //set uniforms

   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

   //uniforms for manip slabs
   int error = update_manip_slabs();
   int manip_slabs_loc = glGetUniformLocation(program, "M_manip_slabs");
   int pv_vr_loc = glGetUniformLocation(program, "Pv_vr");
   glUniformMatrix4fv(manip_slabs_loc,10,false, glm::value_ptr(M_manip_slabs[0]));
   int flag_manip_loc = glGetUniformLocation(program, "flag_manip");
   glUniformMatrix4fv(pv_vr_loc,1,false, glm::value_ptr(PVvr_last));

   //TEMP
   PassUniformData.PV = PVvr_last;
   glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);
   //TEMP

   int image_loc = glGetUniformLocation(program, "instance_backfaces");
   glUniform1i(image_loc, 0);

   glActiveTexture(GL_TEXTURE3);
   glBindTexture(GL_TEXTURE_3D, tex3d_id);
   if (Tex3d_loc != -1)
   {
      glUniform1i(Tex3d_loc, 3);
   }

   glActiveTexture(GL_TEXTURE4);
   glBindTexture(GL_TEXTURE_1D, tex1d_id);
   if (Tex1d_loc != -1)
   {
      glUniform1i(Tex1d_loc, 4);
   }

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D_ARRAY, 0); //unbind texture so we can write to them (remember, no read-write access)
   glBindFramebuffer(GL_FRAMEBUFFER, FBOID);
   glDrawBuffer(GL_COLOR_ATTACHMENT0);

   //clear the color texture and depth texture
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   glClearDepth(0.0f);


   for (auto &model : model_data)
   {
      if (model.hidden == false && model.model_uniforms.instances > 1)
      {
         //glm::mat4 V = pTrackballCam->getAccumRotationMatrix();
         //V[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
         //model.model_uniforms.P = V;

         glCullFace(GL_FRONT);

         PassUniformData.pass = 7;
         glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);

         glm::mat4 X = getModelGlobalMatrix(model);
         model.model_uniforms.M = X;

         glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer); //Bind the OpenGL UBO before we update the data.
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &model.model_uniforms);

         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D_ARRAY, 0); //unbind texture so we can write to them (remember, no read-write access)
         glBindFramebuffer(GL_FRAMEBUFFER, FBOID);
         glDrawBuffer(GL_COLOR_ATTACHMENT0);
         glClear(GL_DEPTH_BUFFER_BIT);
         glDepthFunc(GL_GREATER);
		 
         glBindVertexArray(model.mesh_data.mVao);
         if (hide_scene_meshes == false)
         {
            glDrawElementsInstanced(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0, model.model_uniforms.instances);
         }

         if (interact_mode == InteractMode::Slabs)
         {
            glUniform1i(flag_manip_loc, 1);
            glDrawElementsInstanced(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0, model.model_uniforms.instances);
            glUniform1i(flag_manip_loc, 0);
         }
 
         //pass 8
         glCullFace(GL_BACK);
         PassUniformData.pass = 8;
         glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);

         glBindImageTexture(0, instance_back_faces_tex, 0, /*layered=*/GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
		 
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D_ARRAY, TextureID);

         //glBindFramebuffer(GL_FRAMEBUFFER, 0);
         //glDrawBuffer(GL_BACK);
		 glBindFramebuffer(GL_FRAMEBUFFER, vr_fbo);
         glDepthFunc(GL_LEQUAL);

         if (hide_scene_meshes == false)
         {
            glDrawElementsInstanced(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0, model.model_uniforms.instances);
         }

         if (interact_mode == InteractMode::Slabs)
         {
            glUniform1i(flag_manip_loc, 1);
            glDrawElementsInstanced(GL_TRIANGLES, model.mesh_data.mNumIndices, GL_UNSIGNED_INT, 0, model.model_uniforms.instances);
            glUniform1i(flag_manip_loc, 0);
         }
      }
   }
   

   glClearDepth(1.0f);
   glDisable(GL_CULL_FACE);

   //unbind framebuffer object
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   //end pass 1
}


void DrawLabels(glm::mat4& Pvr, glm::mat4& Vvr) //draw labels
{
   if (interact_mode == InteractMode::Slabs)
   {
      return;
   }

   using namespace glm;

   glUseProgram(label_program);
   glBindFramebuffer(GL_FRAMEBUFFER, vr_fbo);
   glCullFace(GL_FRONT);
   glEnable(GL_DEPTH_TEST);

   int M_loc = glGetUniformLocation(label_program, "M");
   int Q_loc = glGetUniformLocation(label_program, "Q");
   int labelTex_loc = glGetUniformLocation(label_program, "labelTex");
   if (labelTex_loc != -1)
   {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, GetLabelTexture());
      glUniform1i(labelTex_loc, 0); // we bound our texture to texture unit 0
   }

   PassUniformData.pass = 10;
   glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);

   glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer); //Bind the OpenGL UBO before we update the data.

   //draw mouseover label
   if (last_picked > -1 && hide_scene_meshes == false)
   {
      int id = get_selected_id(last_picked);

      glm::mat4 S = GetLabelModelMatrix(select_mode, id);
      glm::mat4 T = glm::translate(glm::vec3(0.0f, 0.01f, -0.1f));
      glm::mat4 R = glm::rotate(-0.75f, glm::vec3(1.0f, 0.0f, 0.0f));
      glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M_controller[1]*T*R*S));

      glm::mat4 Q = GetLabelTexMatrix(select_mode, id);
      glUniformMatrix4fv(Q_loc, 1, false, glm::value_ptr(Q));
      DrawCube();
   }

   glm::vec4 cam_pos = glm::inverse(HMDpose)*glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
   glm::mat4 Tshift = glm::translate(glm::vec3(0.0f, 1.0f, 0.0f)); //shift labels so leader touches bottom
   BeginLeaderLines();
   //draw selection labels
   for (auto &model : model_data)
   {
      int id = get_selected_id(model.model_uniforms.id);
      if (model.hidden == false && model.model_uniforms.selected == true && id > -1)
      {
         glm::mat4 X = SelectedRegionMatrices[model.region].targetTransform.rotation;

         glm::vec3 c(128.0f);
         if (select_mode == SelectMode::Region)
         {
            c = RegionAABB[id].getCenter();
         }
         else if (select_mode == SelectMode::Lobe)
         {
            c = LobeAABB[id].getCenter();
         }
         else if (select_mode == SelectMode::Hemisphere)
         {
            c = HemisphereAABB[id].getCenter();
         }

         float u = SelectedMeshU[model.region];

         bool drawTutorialLabel = (pTutorial->tutorialMode == true && pTutorial->getStage() < 3);
         if (drawTutorialLabel)
         {
            u = 0.0f;
         }

         if (pTutorial->tutorialMode == true && pTutorial->getStage() == 3)
         {
            if ((id >= 1 && id <= 4) || (id >= 15 && id <= 18))
            {
               //draw these only
            }
            else
            {
               continue;
            }
         }

         const float min_u = 0.3f;
         const float max_u = 0.7f;
         const float step = 0.2f;
         if ((u > min_u && u < max_u) || drawTutorialLabel)
         {
            float shrink = glm::smoothstep(min_u, min_u+step, u)*glm::smoothstep(max_u, max_u-step, u);
            if (drawTutorialLabel)
            {
               shrink = 1.0f;
            }

            //glm::vec3 n = pMesh_spl->GetNormal(u);
            glm::vec3 cw = glm::vec3(X*glm::vec4(c, 1.0f));
            glm::vec3 p = pMesh_spl->GetPoint(u);
            glm::vec3 d = glm::normalize(cw - p);
            float len = glm::length(cw-p);
            len = shrink*glm::max(0.2f, len);
            glm::mat4 Tn = glm::translate(d*len);

            glm::mat4 T = glm::translate(c);
            glm::mat4 S = GetLabelModelMatrix(select_mode, id)*glm::scale(glm::vec3(shrink));

            glm::vec4 w_pos = Tn*X*T*glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glm::mat4 Tlabel = glm::translate(glm::vec3(w_pos));

            vec4 to_label = w_pos - cam_pos;
            float ang = atan2(to_label.x, to_label.z) + 3.1415926f;
            glm::mat4 R = glm::rotate(ang, glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(Tlabel*R*S*Tshift));

            glm::mat4 Q = GetLabelTexMatrix(select_mode, id);
            glUniformMatrix4fv(Q_loc, 1, false, glm::value_ptr(Q));
            DrawCube();

            const glm::vec4 zero(0.0f, 0.0f, 0.0f, 1.0f);
            glm::vec3 v0 = glm::vec3(X*T*zero);
            glm::vec3 v1 = glm::vec3(Tn*X*T*zero);

            SetLeaderLineColor(glm::vec3(model.model_uniforms.kd));
            PushLeaderLine(v0, v1);
         }
      }
   }
   EndLeaderLines();
}

void DrawPass9() //draw bounding boxes
{
   using namespace glm;

   glUseProgram(line_program);
  
   //glDrawBuffer(GL_BACK);
   glBindFramebuffer(GL_FRAMEBUFFER, vr_fbo);
   glEnable(GL_DEPTH_TEST);

   glEnable(GL_LINE_SMOOTH);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   glEnable(GL_BLEND); //for line smoothing
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   PassUniformData.pass = 9;
   glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PassUniformData), &PassUniformData);

   glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer); //Bind the OpenGL UBO before we update the data.

   int edge_loc = glGetUniformLocation(line_program, "selectedEdge");

   int color_loc = glGetUniformLocation(line_program, "line_color");

   glLineWidth(3.5f);
   glUniform1f(edge_loc, -1.0f);

   glEnable(GL_LINE_SMOOTH);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

   ModelUniformData.M = glm::scale(glm::vec3(1.0f));
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelUniformData), &ModelUniformData);

   if (interact_mode == InteractMode::Meshes)
   {
      DrawLeaderLines();
   }
   

   if(select_mode == SelectMode::Lobe || select_mode == SelectMode::Hemisphere)
   {
      int num_hemispheres = HemisphereAABB.size();
      for (int i = 0; i < num_hemispheres; i++)
      {
         glUniform4fv(color_loc, 1, (float*)(&HemisphereColors[i].data));
         DrawLine(HemisphereLines[i]);
      }
   }

   if(select_mode == SelectMode::Lobe)
   {
      int num_lobes = LobeAABB.size();
      for (int i = 0; i < num_lobes; i++)
      {
         glUniform4fv(color_loc, 1, (float*)(&LobeColors[i].data));
         DrawLine(LobeLines[i]);
      }
   }

   if(select_mode == SelectMode::Region)
   {
      int num_regions = RegionAABB.size();
      for (int i = 0; i < num_regions; i++)
      {
         glUniform4fv(color_loc, 1, (float*)(&RegionColors[i].data));
         DrawLine(RegionLines[i]);
      }
   }
   
   glDisable(GL_BLEND);
}


void DrawPass10(glm::mat4& P, glm::mat4& V) //draw HelpImage
{
	using namespace glm;

	glUseProgram(image_program);

	glBindFramebuffer(GL_FRAMEBUFFER, vr_fbo);
	glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND); //for line smoothing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	helpImage.P = P;
	helpImage.V = V;
	helpImage.M_left_controller = M_controller[0];
	helpImage.M_right_controller = M_controller[1];

	helpImage.drawHelpImages();

	glDisable(GL_BLEND);

	glUseProgram(program);
}


void draw_pick_pass(glm::mat4& V)
{
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	glViewport(0, 0, (GLsizei)PickW, (GLsizei)PickH);

	float aspect = float(PickW) / float(PickH);
	const glm::mat4 P = glm::perspective(glm::radians(10.0f), aspect, 0.01f, 500.0f);
	glm::mat4 V2 = pTrackballCam->getAccumRotationMatrix()*glm::scale(glm::vec3(1.0f, -1.0f, 1.0f))*glm::rotate(-3.141592f / 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));//HACK why scale -1???
	//PassUniformData.PV = P*glm::inverse(V)*V2;
   PassUniformData.PV = P*glm::inverse(V);
	DrawPass6();

	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
}

void display(glm::mat4& Pvr, glm::mat4& Vvr0, glm::mat4& Mvr0, GLuint fbo)
{
	vr_fbo = fbo;
	float aspect = float(WindowWidth)/float(WindowHeight);
   
   glm::mat4 Vvr = Vvr0*Mvr0;
	PVvr_last = Pvr*Vvr;
   HMDpose = Mvr0;
   
   glm::mat4 V = pTrackballCam->getAccumRotationMatrix()*glm::scale(glm::vec3(1.0f, -1.0f, 1.0f))*glm::rotate(-3.141592f / 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));//HACK why scale -1???
   glm::mat4 Veye = Vvr*V;

   //todo use eye_w in unform buffer
   glm::vec4 eye = glm::inverse(Vvr)*glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

   glUseProgram(program);

   int eye_loc = glGetUniformLocation(program, "world_eye");
   if(eye_loc != -1)
   {
      glUniform4fv(eye_loc, 1, glm::value_ptr(eye));
   }
   if(selected_slab_loc != -1)
   {
      glUniform1i(selected_slab_loc, selected_slab);
   }
   PassUniformData.PV = Pvr*Veye;
   PassUniformData.eye_w = eye;
   
   glBeginQuery(GL_TIME_ELAPSED_EXT, TimerQuery);

	  DrawPass2(Pvr, Vvr);
	  if(showHelpImage){ DrawPass10(Pvr, Vvr); }
	  //draw Help pictures

	  DrawPass3();
	  //HACK restore matrices
	  PassUniformData.PV = Pvr*Vvr*V;

      DrawPass7_8(); //instanced cutaway objects
		
      
      PassUniformData.PV = Pvr*Vvr;
      if (hide_labels == false)
      {
         DrawLabels(Pvr, Vvr);
         DrawPass9(); //draw label leader lines
      }
     else
     {
        glBindFramebuffer(GL_FRAMEBUFFER, vr_fbo);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_TEST);
     }
      PassUniformData.PV = Pvr*Vvr*V;

      

   glEndQuery(GL_TIME_ELAPSED); 
   
   GLint available=0;
   while(!available)
   {
		glGetQueryObjectiv(TimerQuery, GL_QUERY_RESULT_AVAILABLE, &available);
   }

   GLuint64 elapsed;
   glGetQueryObjectui64v(TimerQuery, GL_QUERY_RESULT, &elapsed);
   frame_render_time = 1e-9*elapsed;
   printf("delta_seconds = %f\n", frame_render_time);

}


void LoadShader()
{
   if(program != -1)
	{
		glDeleteProgram(program);
	}

   //create and load shaders
	program = InitShader("vshader.glsl", "gshader.glsl", "fshader.glsl");
   glUseProgram(program);

   pos_loc = glGetAttribLocation(program, "pos_attrib");
   tex_coord_loc = glGetAttribLocation(program, "tex_coord_attrib");

   selected_slab_loc = glGetUniformLocation(program, "selected_slab");
   Tex3d_loc = glGetUniformLocation(program, "tex3d");
   Tex1d_loc = glGetUniformLocation(program, "tex1d");

   int tex_loc = glGetUniformLocation(program, "backfaces");
   if(tex_loc != -1)
   {
      glUniform1i(tex_loc, 0);
   }

}


void InitPicking()
{
   //create empty texture
   glGenTextures(1, &PickID);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, PickID);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, PickW, PickH, 0, GL_RGBA, GL_FLOAT, 0);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   

   //create the depth buffer for the framebuffer object
   glGenRenderbuffers(1, &PickRBO);
   glBindRenderbuffer(GL_RENDERBUFFER, PickRBO);
   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, PickW, PickH);

   //create the framebuffer object and attach the texture and the depth buffer
   glGenFramebuffers(1, &PickFBO);
   glBindFramebuffer(GL_FRAMEBUFFER, PickFBO);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PickID, 0);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, PickRBO);
   CheckFramebufferStatus();
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void InitOpenGL(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   
   InitPicking();

   //create layered framebuffer
   glGenTextures(1, &TextureID);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D_ARRAY, TextureID);

   glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32F, WindowWidth, WindowHeight, 10);
   glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   

   //create the depth buffer for the framebuffer object
   GLuint depthTex;
   glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depthTex);
 
   glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32, WindowWidth, WindowHeight, 10);

   //create the framebuffer object and attach the texture and the depth buffer
   glGenFramebuffers(1, &FBOID);
   glBindFramebuffer(GL_FRAMEBUFFER, FBOID);
   glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureID, 0);
   glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);
   CheckFramebufferStatus();
   glBindFramebuffer(GL_FRAMEBUFFER, 0);


   glGenBuffers(1, &pass_uniform_buffer);
   glBindBuffer(GL_UNIFORM_BUFFER, pass_uniform_buffer);
   glBufferData(GL_UNIFORM_BUFFER, sizeof(PassUniforms), NULL, GL_STREAM_DRAW); //Allocate memory for the buffer, but don't copy (since pointer is null).
   const int passBindingPoint = 3; //This values comes from the binding value specified in the block layout.
   glBindBufferBase(GL_UNIFORM_BUFFER, passBindingPoint, pass_uniform_buffer); //Associate this uniform buffer with the uniform block in the shader that has binding = 3.
   glBindBuffer(GL_UNIFORM_BUFFER, 0);

   glGenBuffers(1, &model_uniform_buffer);
   glBindBuffer(GL_UNIFORM_BUFFER, model_uniform_buffer);
   glBufferData(GL_UNIFORM_BUFFER, sizeof(ModelUniforms), NULL, GL_STREAM_DRAW); //Allocate memory for the buffer, but don't copy (since pointer is null).
   const int modelBindingPoint = 4; //This values comes from the binding value specified in the block layout.
   glBindBufferBase(GL_UNIFORM_BUFFER, modelBindingPoint, model_uniform_buffer); //Associate this uniform buffer with the uniform block in the shader that has binding = 4.
   glBindBuffer(GL_UNIFORM_BUFFER, 0);


   //Set initial values for UBO variables
   PassUniformData = defaultPassUniforms;
   ModelUniformData = defaultModelUniforms;

   LoadShader();

   pTrackballCam = new VirtualTrackball(0.1f, true, false);
   pTrackballCam->setDolly(glm::vec3(-2.0f, 1.0f, -4.0));

   pSceneMeshRotation = new VirtualTrackball(0.1f, true, false);
   pSceneMeshRotation->setDolly(glm::vec3(0.0f));

   LoadAALScene();

   cube_mesh = CreateCube();

   line_program = InitShader("vlines.glsl", "flines.glsl");
   label_program = InitShader("vlabels.glsl", "flabels.glsl");
   image_program = InitShader("vImages.glsl", "fImages.glsl");
   box_mesh = CreateBox();

   helpImage.InitHelpImage();

   glGenQueries(1, &TimerQuery);
   //glBeginQuery(GL_TIME_ELAPSED, TimerQuery);

   init_gui();
}

void LoadAALScene()
{
   tex3d_id = LoadTexture3D(tex3dname);
   glBindTexture(GL_TEXTURE_3D, tex3d_id);
   glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   tex1d_id = LoadTexture1D(tex1dname);
   glBindTexture(GL_TEXTURE_1D, tex1d_id);
   glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   BrainRoot = gg::build_brain_from_file("aal.txt");

   //Load all of the meshes
   MeshData mesh;
   ModelUniforms model_uniforms = defaultModelUniforms;

   const int nhemispheres = count_nodes_at_depth(1, BrainRoot);
   const int nlobes = count_nodes_at_depth(2, BrainRoot);
   const int nregions = count_nodes_at_depth(3, BrainRoot);

   push_nodes_at_depth(RegionNodes, 3, BrainRoot);
   std::sort(RegionNodes.begin(), RegionNodes.end(), gg::node_comp);

   push_nodes_at_depth(LobeNodes, 2, BrainRoot);
   std::sort(LobeNodes.begin(), LobeNodes.end(), gg::node_comp);

   push_nodes_at_depth(HemisphereNodes, 1, BrainRoot);
   std::sort(HemisphereNodes.begin(), HemisphereNodes.end(), gg::node_comp);

   HemisphereMatrices.resize(nhemispheres);
   LobeMatrices.resize(nlobes);
   RegionMatrices.resize(nregions);

   SelectedHemisphereMatrices.resize(nhemispheres);
   SelectedLobeMatrices.resize(nlobes);
   SelectedRegionMatrices.resize(nregions);

   for (int i = 0; i < RegionMatrices.size(); i++)
   {
      RegionMatrices[i].SetTargetTranslation(glm::translate(pTrackballCam->getDolly())*glm::scale(glm::vec3(3.0f)));
      RegionMatrices[i].SetTargetRotation(M0);
   }

   initInstanceMatrices();

   init_hierarchy_colors(nhemispheres, nlobes, nregions);

   const int max_num_meshes = 2000; //set to ~20 for fast loading

   ModelData model{mesh, model_uniforms, 0, 0, 0, false};

   int i = 0;

   int total_indices = 0;
   for (auto & n : RegionNodes)
   {
         int id = n->id;
         char filename[1024]; 
         sprintf(filename, "%s%05d%s", "aal", id+1, ".dae");
         std::string meshFileName(filename);
         std::string dir("./Atlas_pad/");
         //std::string dir("./Atlas_pad_low_res/");

         mesh = LoadMesh(dir+meshFileName);

         model.mesh_data = mesh;
         model.model_uniforms = model_uniforms;

         model.model_uniforms.kd = RegionColors[i];
         model.model_uniforms.ka = model.model_uniforms.kd;
         model.model_uniforms.id = id;

         //set hierarchy
         model.region = id;
         model.hemisphere = n->parent->parent->id;
         model.lobe = n->parent->id;
       
         //setup slab width and location
         model.model_uniforms.slab_width = ((model.mesh_data.mBbMax.x - model.mesh_data.mBbMin.x) / 20.0f)/ 256.0f;
         model.model_uniforms.slab_offset = ((model.mesh_data.mBbMax.x + model.mesh_data.mBbMin.x) / 2.0f)/ 256.0f;
         model.model_uniforms.slab_instance_offset = model.model_uniforms.slab_width;

         model_data.push_back(model);

         if(model_data.size() > max_num_meshes)
         {
            break;
         }
         total_indices += model.mesh_data.mNumIndices;

         i++;
   }
   std::cout << std::endl<<"Num triangles = " << std::to_string(total_indices/3)<<std::endl;
   initAABB(nhemispheres, nlobes, nregions);
   initLines(nhemispheres, nlobes, nregions);

   pMesh_spl = new ControllerSpline(256);
   select_all(true);
   set_select_mode_colors();
}


void handle_selection(int id, bool single)
{
   if (id < 0 || id >= model_data.size())
   {
      return;
   }

   const int hemisphere = model_data[id].hemisphere;
   const int lobe = model_data[id].lobe;

   const int val = model_data[id].model_uniforms.selected;
   const int newval = 1-val;

   if (single == true)
   {
      unselect_all();
      selection_name = " ";
   }
   
   switch (select_mode)
   {
      case SelectMode::Brain:
         select_all(newval);
         if(newval == 1)
         {
            selection_name = "Whole Brain";
         }
      break;
      case SelectMode::Hemisphere:
         select_hemisphere(hemisphere, newval);
         if(newval == 1)
         {
            selection_name = HemisphereNodes[hemisphere]->name;
         }
      break;

      case SelectMode::Lobe:
         select_lobe(lobe, newval);
         if(newval == 1)
         {
            selection_name = LobeNodes[lobe]->name;
            if(LobeNodes[lobe]->parent->id != 1)
            {
               selection_name = LobeNodes[lobe]->parent->name + " " + selection_name;
            }
         }
      break;

      case SelectMode::Region:
         select_region(id, newval);
         if(newval == 1)
         {
            selection_name = RegionNodes[id]->name;
            if(RegionNodes[id]->parent->parent->id != 1)
            {
               selection_name = RegionNodes[id]->parent->parent->name + " " + selection_name;
            }
         }
      break;
   }

}



void enable_slabs_region(int id, AABB& box, const glm::vec3& dir = glm::vec3(1.0f, 0.0f, 0.0f))
{
   ModelData& model = model_data[id];
   model.model_uniforms.instances = MAX_SLABS;

   glm::vec3 imageSize(256.0f); //TODO fix this assumption that the image is 256x256x256

   //glm::vec3 dir = box.getSelectedEdgeDir();
   //glm::vec3  dir(0.0f, 1.0f, 0.0f);

   glm::vec3 center = box.getCenter() / imageSize; //scale to tex coords
   glm::vec3 size = box.getDimensions() / imageSize;

   if (dir.x != 0.0f)
   {
      model.model_uniforms.P = glm::mat4(1.0);
      model.model_uniforms.slab_width = size.x / 20.0f;
      model.model_uniforms.slab_offset = center.x / 1.0f;
      model.model_uniforms.slab_instance_offset = 2.0f*model.model_uniforms.slab_width;
   }
   if (dir.y != 0.0f)
   {
      glm::mat4 T1 = glm::translate(-glm::vec3(0.5));
      glm::mat4 R = glm::rotate(-3.141592f / 2.0f, glm::vec3(0.0f, 0.0f, 1.0f)); //was -
      glm::mat4 T2 = glm::translate(glm::vec3(0.5));
      model.model_uniforms.P = T2*R*T1;

      model.model_uniforms.slab_width = size.y / 20.0f;
      model.model_uniforms.slab_offset = center.y / 1.0f;
      model.model_uniforms.slab_instance_offset = 2.0f*model.model_uniforms.slab_width;
   }
   else if (dir.z != 0.0f)
   {
      glm::mat4 T1 = glm::translate(-glm::vec3(0.5));
      glm::mat4 R = glm::rotate(3.141592f / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));//was +
      glm::mat4 T2 = glm::translate(glm::vec3(0.5));
      model.model_uniforms.P = T2*R*T1;

      model.model_uniforms.slab_width = size.z / 20.0f;
      model.model_uniforms.slab_offset = center.z / 1.0f;
      model.model_uniforms.slab_instance_offset = 2.0f*model.model_uniforms.slab_width;
   }
}

void toggle_slabs_region(int id)
{
   if(id > model_data.size())
   {
      return;
   }

   if (model_data[id].model_uniforms.instances > 1)
   {
      model_data[id].model_uniforms.instances = 1;
   }
   else
   {
      enable_slabs_region(id, RegionAABB[id]);
   }
}

void toggle_slabs_lobe(int lobe)
{
   for (auto &model : model_data) 
   {
      if(model.lobe == lobe)
      {  
         if (model.model_uniforms.instances > 1)
         {
            model.model_uniforms.instances = 1;
         }
         else
         {
            int id = model.region;
            enable_slabs_region(id, LobeAABB[lobe]);
         }

      }
   }
}

void toggle_slabs_hemisphere(int hemisphere)
{
   for (auto &model : model_data) 
   {
      if(model.hemisphere == hemisphere)
      {
         if (model.model_uniforms.instances > 1)
         {
            model.model_uniforms.instances = 1;
         }
         else
         {
            int id = model.region;
            enable_slabs_region(id, HemisphereAABB[hemisphere]);
         }
      }
   }
}

void toggle_slabs(int id)
{
   if(select_mode == SelectMode::Region)
   {
      toggle_slabs_region(id);
   }
   else if(select_mode == SelectMode::Lobe)
   {
      int lobe =  model_data[id].lobe;
      toggle_slabs_lobe(lobe);
   }
   else if(select_mode == SelectMode::Hemisphere)
   {
      int hemisphere =  model_data[id].hemisphere;
      toggle_slabs_hemisphere(hemisphere);
   }
}

int pick(int x, int y)
{
   int id = -1;
   GLubyte buffer[4];

   glBindFramebuffer(GL_FRAMEBUFFER, PickFBO);
   glReadBuffer(GL_COLOR_ATTACHMENT0);
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   glReadPixels(x, PickH-y, 1, 1, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   cout << "Picked: " << std::to_string(buffer[0]) << std::to_string(buffer[1]) << std::to_string(buffer[2]) << std::to_string(buffer[3]) << endl;

   if(buffer[3] > 0)
   {
      id = buffer[2];
   }
   last_picked = id;
   return id;
}

int update_manip_slabs() 
{
	Ml = M_controller[0];
	Mr = M_controller[1];

   slab_spl.Update(Ml, Mr);

   if (pMesh_spl != nullptr)
   {
      pMesh_spl->Update(Ml, Mr);
   }


   const glm::vec4 zero(0.0f, 0.0f, 0.0f, 1.0f);
   glm::vec4 pl = Ml*zero;
   glm::vec4 pr = Mr*zero;
   float d = glm::distance(pl, pr);
   glm::mat4 S2 = glm::scale(glm::vec3(pow(d, 0.5f)*3.0f)); //temp

   glm::mat4 V = pTrackballCam->getAccumRotationMatrix();
   V[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

   glm::mat4 S = glm::scale(glm::vec3(1.0f / select_model_uniform_scale));
   glm::mat4 T = glm::translate(glm::vec3(0.0f, 0.1f, 0.0f));
   
   //reparam
   reparam[0] = glm::clamp(reparam[0], -15.0f, 18.0f);
   reparam[1] = 2.0f - reparam[0];

	for (int i = 0; i < n_manip_slabs; i++)
	{
      float u0 = 256.0f*((i) / float(n_manip_slabs-1));
      u0 = (u0 - 128.0f)*0.85f + 128.0f;
      u0 = u0 / 255.0f;
      u0 = glm::clamp(u0, 0.0f, 1.0f);

      float u = hermite(0.0f, reparam[0], 1.0f, reparam[1], u0);
      u = glm::clamp(u, 0.0f, 1.0f);
      glm::vec3 pos = pMesh_spl->GetPoint(u);
      glm::mat4 offset = glm::translate(pos);
      glm::mat4 Minterp = pMesh_spl->GetRotation(u);


      //test pop out code enable_slab_popout
      glm::mat4 Pop = glm::mat4(1.0f);
      glm::mat4 B = glm::mat4(1.0f);
      if (enable_slab_popout)
      {
         
         glm::vec3 n = pMesh_spl->GetNormal(u);
         Pop = glm::translate(0.30f*n)*glm::scale(glm::vec3(1.0f));
         float w = glm::smoothstep(0.05f, 0.10f, abs(u - 0.5f));
         Pop = glm::mix(Pop, glm::scale(glm::vec3(1.0f)), w);

         glm::mat4 Pslab = model_data[0].model_uniforms.P;

         glm::mat4 Vinv = V*Pslab*glm::rotate(3.141592f / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f))*HMDpose*Minterp;
         Vinv[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
         Vinv = glm::inverse(Vinv);
         B = glm::interpolate(Vinv, glm::scale(glm::vec3(1.0f)), w);
         
      }

      if (d < 0.1f)
      {
         offset = glm::translate(glm::vec3(0.0f));
      }

      M_manip_slabs[i] = offset*Minterp*Pop*S2*T*B*V*S*M0;

	}

	return 0;
}



