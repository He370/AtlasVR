#include "brain_hierarchy.h"
#include "Color.h"
#include "Cube.h"
#include "VirtualTrackball.h"
#include <set>

extern VirtualTrackball* pTrackballCam;

char* SelectNames[] = { "Brain", "Hemisphere", "Lobe", "Region" };
int select_mode = SelectMode::Brain;
int selected_slab = 0;
std::set<int> SelectedHemisphereIDs;
std::set<int> SelectedLobeIDs;
std::set<int> SelectedRegionIDs;

std::vector<AnimTransform> HemisphereMatrices;
std::vector<AnimTransform> LobeMatrices;
std::vector<AnimTransform> RegionMatrices;
std::vector<std::vector<AnimTransform>> InstanceMatrices;
std::vector<std::vector<AnimTransform>> SelectionMatrices;

std::vector<AnimTransform> SelectedHemisphereMatrices;
std::vector<AnimTransform> SelectedLobeMatrices;
std::vector<AnimTransform> SelectedRegionMatrices;
std::vector<float> SelectedMeshU;

std::vector<ModelData> model_data;

glm::vec4 BrainColor;
std::vector<glm::vec4> HemisphereColors;
std::vector<glm::vec4> LobeColors;
std::vector<glm::vec4> RegionColors;

AABB BrainAABB;
std::vector<AABB> HemisphereAABB;
std::vector<AABB> LobeAABB;
std::vector<AABB> RegionAABB;

std::vector<MeshData> HemisphereLines;
std::vector<MeshData> LobeLines;
std::vector<MeshData> RegionLines;

gg::tree_node* BrainRoot = nullptr;
std::vector<gg::tree_node*> RegionNodes;
std::vector<gg::tree_node*> LobeNodes;
std::vector<gg::tree_node*> HemisphereNodes;

void next_select_mode()
{
	select_mode = (select_mode + 1) % (SelectMode::NumModes);
	set_select_mode_colors();
}

void initInstanceMatrices()
{
   InstanceMatrices.resize(RegionMatrices.size());
   SelectionMatrices.resize(RegionMatrices.size());
   SelectedMeshU.resize(RegionMatrices.size());

   for (int i = 0; i < RegionMatrices.size(); i++)
   {
      InstanceMatrices[i].resize(10);
      SelectionMatrices[i].resize(10);

      for (int s = 0; s < 10; s++)
      {
         SelectionMatrices[i][s].speed = 3.0f;
      }
   }
}

int get_selected_id(int id)
{
   if (id == -1)
   {
      return -1;
   }

   int ret = -1;
   switch (select_mode)
   {
      case SelectMode::Brain:
         ret = 0;
         break;
      case SelectMode::Hemisphere:
         ret = model_data[id].hemisphere;
         break;

      case SelectMode::Lobe:
         ret = model_data[id].lobe;
         break;

      case SelectMode::Region:
         ret = id;
         break;
   }

   return ret;
}

void applySelectedMatrices()
{
   for (int i = 0; i < HemisphereMatrices.size(); i++)
   {
      if (is_hemisphere_selected(i))
      {
         HemisphereMatrices[i] = SelectedHemisphereMatrices[i];
      }
   }
   for (int i = 0; i < LobeMatrices.size(); i++)
   {
      if(is_lobe_selected(i))
      {
         LobeMatrices[i] = SelectedLobeMatrices[i];
      }
   }
   for (int i = 0; i < RegionMatrices.size(); i++)
   {
      if (model_data[i].model_uniforms.selected ==true)
      {
         RegionMatrices[i] = SelectedRegionMatrices[i];
      }
   }
}

void set_selected_edge(int edge)
{
   switch(select_mode)
   {
      case Brain:
      case Hemisphere:
      {  
         int num_hemispheres = HemisphereAABB.size();
         for (int i = 0; i < num_hemispheres; i++)
         {
            if (is_hemisphere_selected(i) == true)
            {
               HemisphereAABB[i].selectedEdge = edge;
            }
         }
      }
      break;

      case Lobe:
      {
         int num_lobes = LobeAABB.size();
         for (int i = 0; i < num_lobes; i++)
         {
            if (is_lobe_selected(i) == true)
            {
               LobeAABB[i].selectedEdge = edge;
            }
         }
      }
      break;

      case Region:
      {
         int num_regions = RegionAABB.size();
         for (int i = 0; i < num_regions; i++)
         {
            if (model_data[i].model_uniforms.selected != 0)
            {
               RegionAABB[i].selectedEdge = edge;
            }
         }
      }
      break;
   }
}

void set_select_mode_colors()
{
   switch(select_mode)
   {
      case Brain:
      {
         for (auto &model : model_data)
         {
            model.model_uniforms.ka = BrainColor;
            model.model_uniforms.kd = model.model_uniforms.ka;
         }
      }
         break;
      case Hemisphere:
      {  
         for (auto &model : model_data) 
         {
            int h = model.hemisphere;
            model.model_uniforms.ka = HemisphereColors[h];
            model.model_uniforms.kd = model.model_uniforms.ka;
         }
      }
         break;

      case Lobe:
      {
         for (auto &model : model_data) 
         {
            int lobe = model.lobe;
            model.model_uniforms.ka = LobeColors[lobe];
            model.model_uniforms.kd = model.model_uniforms.ka;
         }
      }
         break;

      case Region:
      {
         for (auto &model : model_data) 
         {
            int region = model.region;
            model.model_uniforms.ka = RegionColors[region];
            model.model_uniforms.kd = model.model_uniforms.ka;
         }
      }
         break;
   }
}

void initLines(int nhemispheres, int nlobes, int nregions)
{
   const int n = 20;
   HemisphereLines.resize(nhemispheres);
   
   for (int i = 0; i < HemisphereLines.size(); i++)
   {
      HemisphereLines[i] = CreateCenterline(n);
   }

   LobeLines.resize(nlobes);

   for (int i = 0; i < LobeLines.size(); i++)
   {
      LobeLines[i] = CreateCenterline(n);
   }

   RegionLines.resize(nregions);

   for (int i = 0; i < RegionLines.size(); i++)
   {
      RegionLines[i] = CreateCenterline(n);
   }
}

void updateLines()
{
   const int n = HemisphereLines[0].mNumIndices;
   const float dt = 1.0f/n;
   std::vector<glm::vec3> data(n);

   Transform identity;
   identity.translation = glm::translate(glm::vec3(0.0f));
   identity.rotation = glm::mat4(1.0f);

   int i=0;
   for (auto &xform : HemisphereMatrices)
   {
      AnimTransform transform = xform;
      transform.t = 1.0;

      transform.initTransform = identity;

      transform.targetTransform = transform.currentTransform;
      transform.Update(0.0f);


      //data[0] = BrainAABB.getCenter();

      for (int j = 0; j < n; j++)
      {
         glm::mat4 M = transform.GetCurrentMatrix();
         glm::vec4 pt = glm::vec4(HemisphereAABB[i].getCenter(), 1.0f);

         data[j] = glm::vec3(M*pt);
         transform.Update(dt);
      }
      UpdateCenterline(HemisphereLines[i], (float*)&data[0]);

      i++;
   }

   i = 0;
   for (auto &xform : LobeMatrices)
   {
      AnimTransform transform = xform;
      transform.t = 1.0;

      transform.initTransform = identity;
      transform.targetTransform = transform.currentTransform;
      transform.Update(0.0f);

      int h = getHemisphereFromLobe(i);
      glm::mat4 H(1.0f);
      if(h>=0)
      {
         H = HemisphereMatrices[h].GetCurrentMatrix();
         glm::vec4 pt = glm::vec4(HemisphereAABB[h].getCenter(), 1.0f);
         //data[0] = HemisphereAABB[h].getCenter();
         data[0] = glm::vec3(H*pt);
      }

      if(LobeAABB[i].isValid())
      {
         for (int j = 1; j < n; j++)
         {
            glm::mat4 M = transform.GetCurrentMatrix();
            glm::vec4 pt = glm::vec4(LobeAABB[i].getCenter(), 1.0f);
            data[j] = glm::vec3(H*M*pt);
            transform.Update(dt);
         }
      }
      else
      {
         for (int j = 1; j < n; j++)
         {
            data[j] = data[0];
         }
      }
      UpdateCenterline(LobeLines[i], (float*)&data[0]);

      i++;
   }

   i=0;
   for (auto &xform : RegionMatrices)
   {
      AnimTransform transform = xform;
      transform.t = 1.0;

      transform.initTransform = identity;

      transform.targetTransform = transform.currentTransform;
      transform.Update(0.0f);


      //data[0] = BrainAABB.getCenter();

      for (int j = 0; j < n; j++)
      {
         glm::mat4 M = transform.GetCurrentMatrix();
         glm::vec4 pt = glm::vec4(RegionAABB[i].getCenter(), 1.0f);

         data[j] = glm::vec3(M*pt);
         transform.Update(dt);
      }
      UpdateCenterline(RegionLines[i], (float*)&data[0]);

      i++;
   }
}

void initAABB(int nhemispheres, int nlobes, int nregions)
{
   HemisphereAABB.resize(nhemispheres);
   LobeAABB.resize(nlobes);
   RegionAABB.resize(nregions);

   for (auto &model : model_data)
   {
      aiVector3D& bmax = model.mesh_data.mBbMax;
      aiVector3D& bmin = model.mesh_data.mBbMin;

      glm::vec3 pmin(bmin.x, bmin.y, bmin.z);
      glm::vec3 pmax(bmax.x, bmax.y, bmax.z);

      RegionAABB[model.region] = AABB(pmin, pmax);
      LobeAABB[model.lobe].extend(RegionAABB[model.region]);
      HemisphereAABB[model.hemisphere].extend(RegionAABB[model.region]);
      BrainAABB.extend(HemisphereAABB[model.hemisphere]);
   }
}

void init_hierarchy_colors(int nhemispheres, int nlobes, int nregions)
{
   HemisphereColors.resize(nhemispheres);
   LobeColors.resize(nlobes);
   RegionColors.resize(nregions);

   BrainColor = glm::vec4(Get_Colour_Cubehelix(0.5f), 1.0f);

   for(int i=0; i<nhemispheres; i++)
   {
      HemisphereColors[i] = glm::vec4(Get_Colour_Cubehelix(0.125 + 0.75f*float(i)/nhemispheres), 1.0f);
   }

   for(int i=0; i<nlobes; i++)
   {
      LobeColors[i] = glm::vec4(Get_Colour_Cubehelix(0.05f + 2.5f*float(i%7)/nlobes), 1.0f);
   }

   for(int i=0; i<nregions; i++)
   {
      RegionColors[i] = glm::vec4(Get_Colour_Cubehelix(0.1f + 0.8f*float(i)/nregions), 1.0f);
   }

}

void ReverseAllTransforms()
{
   for (auto &xform : HemisphereMatrices)
   {
      xform.Reverse();
   }

   for (auto &xform : LobeMatrices)
   {
      xform.Reverse();
   }

   for (auto &xform : RegionMatrices)
   {
      xform.Reverse();
   }

   for (auto &region : InstanceMatrices)
   {
      for (auto &instance : region)
      {
         instance.Reverse();
      }
   }

}

void UpdateAllTransforms(float d_time)
{
   for (auto &xform : HemisphereMatrices)
   {
      xform.Update(d_time);
   }

   for (auto &xform : LobeMatrices)
   {
      xform.Update(d_time);
   }

   for (auto &xform : RegionMatrices)
   {
      xform.Update(d_time);
   }

   int i=0;
   for (auto &region : InstanceMatrices)
   {
      int s = 0;
      for (auto &instance : region)
      {
         instance.Update(d_time);
         SelectionMatrices[i][s].Update(d_time);
         //glm::mat4 selectM = SelectionMatrices[i][s].GetCurrentMatrix();
         glm::mat4 selectM = SelectionMatrices[i][s].GetCurrentSlabMatrix();

         model_data[i].model_uniforms.M_slab[s] = instance.GetCurrentMatrix()*selectM;
         s++;
      }
      i++;
   }
}

glm::mat4 getModelGlobalMatrix(const ModelData& model)
{
   glm::mat4& H = HemisphereMatrices[model.hemisphere].GetCurrentMatrix();
   glm::mat4& L = LobeMatrices[model.lobe].GetCurrentMatrix();
   glm::mat4& R = RegionMatrices[model.region].GetCurrentMatrix();

   return H*L*R;
}

glm::mat4 getModelGlobalSelectedMatrix(const ModelData& model)
{
   glm::mat4& H = SelectedHemisphereMatrices[model.hemisphere].GetCurrentMatrix();
   glm::mat4& L = SelectedLobeMatrices[model.lobe].GetCurrentMatrix();
   glm::mat4& R = SelectedRegionMatrices[model.region].GetCurrentMatrix();

   return H*L*R;
}


int getHemisphereFromLobe(int lobe)
{
   return LobeNodes[lobe]->parent->id;
}

glm::mat4 getLobeGlobalMatrix(int lobe)
{

   int hemisphere = getHemisphereFromLobe(lobe);
   glm::mat4& H = HemisphereMatrices[hemisphere].GetCurrentMatrix();
   glm::mat4& L = LobeMatrices[lobe].GetCurrentMatrix();

   return H*L;
}

glm::mat4 getHemisphereGlobalMatrix(int hemisphere)
{
   return HemisphereMatrices[hemisphere].GetCurrentMatrix();
}

void unselect_all()
{
   for (auto &model : model_data)
   {
      model.model_uniforms.selected = false;
   }
}

void hide_selection()
{
   for (auto &model : model_data)
   {
      if (model.model_uniforms.selected == true)
      {
         model.hidden = true;
      }
   }
}

void unhide_selection()
{
   for (auto &model : model_data)
   {
      if (model.model_uniforms.selected == true)
      {
         model.hidden = false;
      }
   }
}

void unhide_all()
{
   for (auto &model : model_data)
   {
      model.hidden = false;
   }
}

bool is_hemisphere_selected(int hemisphere)
{
   bool selected = true;
   for (auto &model : model_data)
   {
      if (model.hemisphere == hemisphere && model.model_uniforms.selected==0)
      {
         selected = false;
      }
   }

   return selected;
}

bool is_lobe_selected(int lobe)
{
   bool selected = true;
   for (auto &model : model_data)
   {
      if (model.lobe == lobe && model.model_uniforms.selected == 0)
      {
         selected = false;
      }
   }

   return selected;
}

void select_all(int val)
{
   for (auto &model : model_data)
   {
      model.model_uniforms.selected = val;
   }
}

void select_hemisphere(int hemisphere, int val)
{
   for (auto &model : model_data)
   {
      if (model.hemisphere == hemisphere)
      {
         model.model_uniforms.selected = val;
      }
   }
}

void select_lobe(int lobe, int val)
{
   for (auto &model : model_data)
   {
      if (model.lobe == lobe)
      {
         model.model_uniforms.selected = val;
      }
   }
}

void select_region(int region, int val)
{
   model_data[region].model_uniforms.selected = val;
}

void explode_regions(float x, float y, float z)
{
   glm::vec3 scale(x,y,z);
   for (auto &model : model_data)
   {
      if (model.model_uniforms.selected == 1)
      {
         glm::vec3 t = RegionAABB[model.region].getCenter() - LobeAABB[model.lobe].getCenter();
         RegionMatrices[model.region].SetTargetTranslation(glm::translate(scale*t));
      }
   }
}

void explode_regions(float r)
{
   for (auto &model : model_data)
   {
      if (model.model_uniforms.selected == 1)
      {
         glm::vec3 t = RegionAABB[model.region].getCenter() - LobeAABB[model.lobe].getCenter();
         RegionMatrices[model.region].SetTargetTranslation(glm::translate(r*t));
      }
   }
}

bool model_comp(ModelData* a, ModelData* b)
{
   const int ix_a = a->region;
   const int ix_b = b->region;

   glm::vec3 ca = RegionAABB[ix_a].getCenter();
   glm::vec3 cb = RegionAABB[ix_b].getCenter();

   return (ca.x < cb.x);
}

void linear_explode_regions(float x, float y, float z)
{
   std::vector<ModelData*> selected_models;
   AABB group;
   float total_width = 0.0f;
   glm::vec3 cen(0.0f);
   float n = 0.0f;
   for (auto &model : model_data)
   {
      if (model.model_uniforms.selected == 1)
      {
         selected_models.push_back(&model);
         group.extend(RegionAABB[model.region]);
         total_width += RegionAABB[model.region].getDimensions().x;
         cen += RegionAABB[model.region].getCenter();
         n+=1.0f;
      }
   }

   if(total_width == 0.0f)
   {
      return;
   }

   cen /= n;

   float new_x = cen.x - 0.5f*total_width;
   std::sort(selected_models.begin(), selected_models.end(), model_comp);
   for (auto &model : selected_models)
   {
      int id = model->region;
      glm::vec3 c0 = RegionAABB[id].getCenter();
      new_x += 0.5*RegionAABB[id].getDimensions().x;
      RegionMatrices[id].SetTargetTranslation(glm::translate(glm::vec3(x*new_x-c0.x, 0.0f, 0.0f)));    
      new_x += 0.5*RegionAABB[id].getDimensions().x;
   }

}

void explode_slabs(float x, float y, float z)
{
   for (auto &model : model_data)
   {
      if (model.model_uniforms.selected == 1)
      {
         for (int i = 0; i < MAX_SLABS; i++)
         {
            glm::vec3 t(i*x, i*y, i*z);
            glm::mat4 T = glm::translate(256.0f*t);
            model.model_uniforms.M_slab[i] = T;

            InstanceMatrices[model.region][i].SetTargetTranslation(glm::translate(256.0f*t));
         }
      }
   }
}


void fan_slabs(float angle_scale)
{
   for (auto &model : model_data)
   {
      if (model.model_uniforms.selected == 1)
      {
         for (int i = 0; i<InstanceMatrices[model.region].size(); i++)
         {
            glm::vec3 cen = RegionAABB[model.region].getSelectedEdgePt();
            glm::vec3 axis = glm::normalize(RegionAABB[model.region].getSelectedEdgeDir());
            glm::quat q = angleAxis(angle_scale*i, axis);

            if (select_mode == SelectMode::Hemisphere)
            {
               int h = model.hemisphere;
               if (is_hemisphere_selected(h) == true)
               {
                  cen = HemisphereAABB[h].getSelectedEdgePt();
                  axis = HemisphereAABB[h].getSelectedEdgeDir();
                  q = angleAxis(angle_scale*i, axis);
               }
            }
            else if (select_mode == SelectMode::Lobe)
            {
               int l = model.lobe;
               if (is_lobe_selected(l) == true)
               {
                  cen = LobeAABB[l].getSelectedEdgePt();
                  axis = LobeAABB[l].getSelectedEdgeDir();
                  q = angleAxis(angle_scale*i, axis);
               }
            }

            InstanceMatrices[model.region][i].SetTargetRotation(glm::toMat4(q));
         }
      }
   }
}

void unselect_slabs(int slab)
{
   for (auto &model : model_data)
   {
      if (model.model_uniforms.selected == 1 && model.model_uniforms.instances > 1)
      {
         SelectionMatrices[model.region][slab].SetTargetTranslation(glm::mat4(1.0f));
         SelectionMatrices[model.region][slab].SetTargetRotation(glm::mat4(1.0f));
      }
   }
}

void select_slabs(int slab)
{
   for (auto &model : model_data)
   {
      if (model.model_uniforms.selected == 1 && model.model_uniforms.instances > 1)
      {
         //compute displacement
         glm::vec3 d(0.0f);
         glm::vec3 cen(0.0f);
         glm::vec3 axis(0.0f);
         if(select_mode == SelectMode::Hemisphere)
         {
            glm::vec3 brain = BrainAABB.getCenter();
            glm::vec3 hem = HemisphereAABB[model.hemisphere].getCenter();
            glm::vec3 dim = HemisphereAABB[model.hemisphere].getDimensions();
            d = glm::normalize(hem-brain);

            axis = glm::normalize(HemisphereAABB[model.hemisphere].getSelectedEdgeDir());
            float mag = glm::dot(dim, glm::abs(d));
            d = mag*d;
            cen = hem;
         }
         if(select_mode == SelectMode::Lobe)
         {
            glm::vec3 hem = HemisphereAABB[model.hemisphere].getCenter();
            glm::vec3 lobe = LobeAABB[model.lobe].getCenter();
            glm::vec3 dim = LobeAABB[model.lobe].getDimensions();
            d = glm::normalize(lobe-hem);

            axis = glm::normalize(LobeAABB[model.lobe].getSelectedEdgeDir());
            d = d-glm::dot(d,axis)*axis;
            d = normalize(d);

            float mag = glm::dot(dim, glm::abs(d));
            d = mag*d;
            cen = lobe;
         }

         if(select_mode == SelectMode::Region)
         {
            glm::vec3 lobe = LobeAABB[model.lobe].getCenter();
            glm::vec3 reg = RegionAABB[model.region].getCenter();
            glm::vec3 dim = RegionAABB[model.region].getDimensions();
            d = glm::normalize(reg-lobe);

            axis = glm::normalize(RegionAABB[model.region].getSelectedEdgeDir());
            d = d-glm::dot(d,axis)*axis;
            d = normalize(d);

            float mag = glm::dot(dim, glm::abs(d));
            d = mag*d;
            cen = reg;
         }

         SelectionMatrices[model.region][slab].SetTargetTranslation(glm::translate(d));

         //rotate to face viewer (buggy)
         /*
         glm::mat4 V = pTrackballCam->getAccumRotationMatrix();
         V[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

         glm::vec3 look = glm::vec3(-V[0][2], -V[1][2], -V[2][2]);
         
         glm::quat q = glm::toQuat(glm::orientation(look, glm::normalize(axis)));
         
         SelectionMatrices[model.region][slab].SetTargetRotation(q, cen);
         //*/
      }
   }
}

void rotate_region_planes(float y)
{
   for (auto &model : model_data)
   {
      if (model.model_uniforms.selected == 1)
      {
         glm::vec3 center = RegionAABB[model.region].getCenter()/256.0f;

         glm::mat4 T1 = glm::translate(-center);
         glm::mat4 R = glm::rotate(y, glm::vec3(0.0f, 1.0f, 0.0f));
         glm::mat4 T2 = glm::translate(center);
         
         model.model_uniforms.P = T2*R*T1;
      }
   }
}

void rotate_hemisphere_planes(float y)
{
   for (auto &model : model_data)
   {
      if (model.model_uniforms.selected == 1)
      {
         glm::vec3 center = HemisphereAABB[model.hemisphere].getCenter()/256.0f;

         glm::mat4 T1 = glm::translate(-center);
         glm::mat4 R = glm::rotate(y, glm::vec3(0.0f, 1.0f, 0.0f));
         glm::mat4 T2 = glm::translate(center);
         
         model.model_uniforms.P = T2*R*T1;
      }
   }
}


void fan_regions(float angle_scale)
{
   glm::vec3 cen = BrainAABB.getCenter();
   for (auto &model : model_data)
   {
      glm::vec3 Bbcen = RegionAABB[model.region].getCenter() - BrainAABB.getCenter();
      float angle = atan2(Bbcen.y, Bbcen.x);

      glm::quat q = glm::angleAxis(angle*angle_scale, glm::vec3(0.0f, 0.0f, 1.0f));

      if (model.model_uniforms.selected == 1)
      {
         RegionMatrices[model.region].SetTargetRotation(glm::toMat4(q));
      }
   }
}

void explode_lobes(float x, float y, float z)
{

   glm::vec3 t(x,y,z);

   for (auto &model : model_data)
   {
      const int lobe = model.lobe;
      const int hemisphere = model.hemisphere;

      if (is_lobe_selected(lobe))
      {
         LobeMatrices[lobe].SetTargetTranslation(glm::translate(t));
      }
   }

}

void explode_lobes(float r)
{

   for (auto &model : model_data)
   {
      const int lobe = model.lobe;
      const int hemisphere = model.hemisphere;

      if (is_lobe_selected(lobe))
      {
         glm::vec3 t = LobeAABB[lobe].getCenter() - HemisphereAABB[hemisphere].getCenter();
         LobeMatrices[lobe].SetTargetTranslation(glm::translate(r*t));
      }
   }

}


void fan_lobes(float angle_scale)
{
   for (auto &model : model_data)
   {
      const int lobe = model.lobe;

      if (is_lobe_selected(lobe))
      {
         glm::vec3 t = LobeAABB[lobe].getCenter() - BrainAABB.getCenter();
         const float angle = atan2(t.y, t.x);
         const glm::vec3 axis(0.0f, 0.0f, 1.0f);
         
         glm::quat q = angleAxis(angle*angle_scale, axis);
         glm::vec3 cen = BrainAABB.getCenter();
         LobeMatrices[lobe].SetTargetRotation(glm::toMat4(q));
      }
   }
}

void explode_hemispheres(float x, float y, float z)
{

   glm::vec3 t0 = glm::vec3(-x, y, z);
   glm::vec3 t1 = glm::vec3(0, y, z);
   glm::vec3 t2 = glm::vec3(+x, y, z);

   if (is_hemisphere_selected(0) == true)
   {
      HemisphereMatrices[0].SetTargetTranslation(glm::translate(t0));
   }
   if (is_hemisphere_selected(1) == true)
   {
      HemisphereMatrices[1].SetTargetTranslation(glm::translate(t1));
   }
   if (is_hemisphere_selected(2) == true)
   {
      HemisphereMatrices[2].SetTargetTranslation(glm::translate(t2));
   }
}

void fan_hemispheres(float angle, glm::vec3& axis)
{

   if (is_hemisphere_selected(0) == true)
   {
      glm::vec3 cen = HemisphereAABB[0].getSelectedEdgePt();
      glm::vec3 axis = HemisphereAABB[0].getSelectedEdgeDir();
      glm::quat q = angleAxis(angle, axis);

      HemisphereMatrices[0].SetTargetRotation(glm::toMat4(q));
   }
   if (is_hemisphere_selected(2) == true)
   {
      glm::vec3 cen = HemisphereAABB[2].getSelectedEdgePt();
      glm::vec3 axis = HemisphereAABB[2].getSelectedEdgeDir();
      glm::quat q = angleAxis(-angle, axis);

      HemisphereMatrices[2].SetTargetRotation(glm::toMat4(q));
   }
}