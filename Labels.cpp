#include <vector>
#include "Labels.h"
#include "imgui/imgui.h"
#include "imgui_impl_vr.h"
#include "brain_hierarchy.h"
#include <glm/gtx/transform.hpp>


static GLuint		   g_LabelTexture = 0;
static GLuint        g_FontTexture = 0;
static int			   g_TexWidth = 1024;
static int			   g_TexHeight = 5120;
static GLuint		   g_LabelFBO = 0;
static int			   g_LastViewport[4] = { 0,0,0,0 };
static ImGuiContext* g_Context;

static std::vector<glm::vec3> brain_label_pos;
static std::vector<glm::vec3> brain_label_size;

static std::vector<glm::vec3> hemisphere_label_pos;
static std::vector<glm::vec3> hemisphere_label_size;

static std::vector<glm::vec3> lobe_label_pos;
static std::vector<glm::vec3> lobe_label_size;

static std::vector<glm::vec3> region_label_pos;
static std::vector<glm::vec3> region_label_size;

static MeshData LeaderLines;
static std::vector<glm::vec3> LeaderVertices;
static std::vector<glm::vec3> LeaderColors;
static glm::vec3 CurrentColor(0.0f, 1.0f, 0.0f);

extern GLuint label_program;
extern GLuint line_program;

struct CubeData
{
   GLuint cube_vao;
   GLuint cube_vbo;
   GLuint cube_ibo;
};
CubeData cube;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void BeginLeaderLines()
{
   LeaderVertices.resize(0);
   LeaderColors.resize(0);
}

void SetLeaderLineColor(const glm::vec3& color)
{
   CurrentColor = color;
}

void PushLeaderLine(const glm::vec3& v0, const glm::vec3& v1)
{
   LeaderVertices.push_back(v0);
   LeaderVertices.push_back(v1);
   LeaderColors.push_back(CurrentColor);
   LeaderColors.push_back(CurrentColor);
}

void BufferLeaderLines()
{
   glBindVertexArray(LeaderLines.mVao);
   glBindBuffer(GL_ARRAY_BUFFER, LeaderLines.mVboVerts);
   glBufferData(GL_ARRAY_BUFFER, (LeaderColors.size() + LeaderVertices.size()) * sizeof(glm::vec3), 0, GL_DYNAMIC_DRAW);
   glBufferSubData(GL_ARRAY_BUFFER, 0, LeaderVertices.size() * sizeof(glm::vec3), LeaderVertices.data());
   glBufferSubData(GL_ARRAY_BUFFER, LeaderVertices.size() * sizeof(glm::vec3), LeaderColors.size() * sizeof(glm::vec3), LeaderColors.data());

   int pos_loc = glGetAttribLocation(line_program, "pos_attrib");
   int color_loc = glGetAttribLocation(line_program, "color_attrib");
   glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, 0);
   glVertexAttribPointer(color_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(LeaderVertices.size() * sizeof(glm::vec3)));
   glBindVertexArray(0);

}

void EndLeaderLines()
{
   BufferLeaderLines();
}

void InitLeaderLines()
{
   LeaderLines.mScaleFactor = 1.0f;

   //shader attrib locations
   int pos_loc = -1;
   int tex_coord_loc = -1;
   int normal_loc = -1;
   int color_loc = -1;

   pos_loc = glGetAttribLocation(line_program, "pos_attrib");
   tex_coord_loc = glGetAttribLocation(line_program, "tex_coord_attrib");
   normal_loc = glGetAttribLocation(line_program, "normal_attrib");
   color_loc = glGetAttribLocation(line_program, "color_attrib");

   LeaderLines.mNumIndices = 0;

   glGenVertexArrays(1, &LeaderLines.mVao);
   glBindVertexArray(LeaderLines.mVao);

   //Buffer vertices
   glGenBuffers(1, &LeaderLines.mVboVerts);
   glBindBuffer(GL_ARRAY_BUFFER, LeaderLines.mVboVerts);
   glBufferData(GL_ARRAY_BUFFER, 2 * 2 * 116 * sizeof(glm::vec3), 0, GL_DYNAMIC_DRAW);
   glEnableVertexAttribArray(pos_loc);
   glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, 0);

   glEnableVertexAttribArray(color_loc);
   glVertexAttribPointer(color_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(2 * 116 * sizeof(glm::vec3)));

   // no normals
   LeaderLines.mVboNormals = -1;
   glDisableVertexAttribArray(normal_loc);

   glBindVertexArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void DrawLeaderLines()
{
   glBindVertexArray(LeaderLines.mVao);
   glDrawArrays(GL_LINES, 0, LeaderVertices.size());
   glBindVertexArray(0);
}

glm::mat4 GetLabelTexMatrix(int mode, int i)
{
   glm::vec3 label_pos, label_size;

   switch (mode)
   {
   case 0: //brain
      label_pos = brain_label_pos[i];
      label_size = brain_label_size[i];
      break;
   case 1: //hemisphere
      label_pos = hemisphere_label_pos[i];
      label_size = hemisphere_label_size[i];
      break;
   case 2: //lobe
      label_pos = lobe_label_pos[i];
      label_size = lobe_label_size[i];
      break;
   case 3: //region
      label_pos = region_label_pos[i];
      label_size = region_label_size[i];
      break;

   }

   glm::mat4 S = glm::scale(glm::vec3(label_size.x/g_TexWidth, label_size.y / g_TexHeight, 1.0f));
   glm::mat4 T = glm::translate(glm::vec3(label_pos.x/g_TexWidth, 1.0f-label_pos.y/g_TexHeight, 0.0f));
   return T*S;
}

glm::mat4 GetLabelModelMatrix(int mode, int i)
{
   glm::vec3 label_pos, label_size;

   switch (mode)
   {
   case 0: //brain
      label_pos = brain_label_pos[i];
      label_size = brain_label_size[i];
      break;
   case 1: //hemisphere
      label_pos = hemisphere_label_pos[i];
      label_size = hemisphere_label_size[i];
      break;
   case 2: //lobe
      label_pos = lobe_label_pos[i];
      label_size = lobe_label_size[i];
      break;
   case 3: //region
      label_pos = region_label_pos[i];
      label_size = region_label_size[i];
      break;

   }

   const glm::vec3 size(0.5f, 0.5f, 0.01f);
   glm::mat4 S = glm::scale(glm::vec3(label_size.x / g_TexWidth, label_size.y / g_TexWidth, 1.0f)*size);
   return S;
}


void CreateCube()
{
   glGenVertexArrays(1, &cube.cube_vao);
   glBindVertexArray(cube.cube_vao);

   using namespace glm;
   vec3 pos[8] = { vec3(-1.0f, -1.0f, -1.0f), vec3(+1.0f, -1.0f, -1.0f), vec3(+1.0f, +1.0f, -1.0f), vec3(-1.0f, +1.0f, -1.0f),
      vec3(-1.0f, -1.0f, +1.0f), vec3(+1.0f, -1.0f, +1.0f), vec3(+1.0f, +1.0f, +1.0f), vec3(-1.0f, +1.0f, +1.0f) };

   vec3 tex[8] = { vec3(0.0f, 0.0f, 0.0f), vec3(+1.0f, 0.0f, 0.0f), vec3(+1.0f, +1.0f, 0.0f), vec3(0.0f, +1.0f, 0.0f),
      vec3(0.0f, 0.0f, +1.0f), vec3(+1.0f, 0.0f, +1.0f), vec3(+1.0f, +1.0f, +1.0f), vec3(0.0f, +1.0f, +1.0f) };

   unsigned short idx[36] = { 0,2,1, 2,0,3, //bottom
      0,5,4, 5,0,1, //front
      1,6,5, 6,1,2, //right 
      2,7,6, 7,2,3, //back
      3,4,7, 4,3,0, //left
      4,5,6, 6,7,4 };//top
                     //Buffer vertices
   int datasize = sizeof(pos) + sizeof(tex);
   glGenBuffers(1, &cube.cube_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, cube.cube_vbo);
   glBufferData(GL_ARRAY_BUFFER, datasize, 0, GL_STATIC_DRAW);
   glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);
   glBufferSubData(GL_ARRAY_BUFFER, sizeof(pos), sizeof(tex), tex);


   //Buffer indices
   glGenBuffers(1, &cube.cube_ibo);
   int nIndices = 12;
   int indexsize = sizeof(idx);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.cube_ibo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexsize, idx, GL_STATIC_DRAW);


   //shader attrib locations
   int pos_loc = -1;
   int tex_coord_loc = -1;
   int normal_loc = -1;

   pos_loc = glGetAttribLocation(label_program, "pos_attrib");
   tex_coord_loc = glGetAttribLocation(label_program, "tex_coord_attrib");
   normal_loc = glGetAttribLocation(label_program, "normal_attrib");

   glEnableVertexAttribArray(pos_loc);
   glEnableVertexAttribArray(tex_coord_loc);

   glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));
   glVertexAttribPointer(tex_coord_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(8 * 3 * sizeof(float)));

   glBindVertexArray(0);
}

void DrawCube()
{
   glBindVertexArray(cube.cube_vao);
   glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
   glBindVertexArray(0);
}


GLuint GetLabelTexture() 
{
   return g_LabelTexture;
}

void CreateLabelFontTexture()
{
   // Build texture atlas
   ImGuiIO& io = ImGui::GetIO();
   unsigned char* pixels;
   int width, height;
   io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

   // Upload texture to graphics system
   GLint last_texture;
   glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
   glGenTextures(1, &g_FontTexture);
   glBindTexture(GL_TEXTURE_2D, g_FontTexture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

   // Store our identifier
   io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

   // Restore state
   glBindTexture(GL_TEXTURE_2D, last_texture);
}

void DrawImGuiLabels()
{
   float column_pos = 0.0f;
   ImGui::SetNextWindowPos(ImVec2(column_pos, 0.0f));
   float max_w = 0.0f;
   //brain
   {
      ImGui::Begin("Brain", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
      ImGui::Text("Brain");

      ImVec2 size = ImGui::GetWindowSize();
      ImVec2 pos = ImGui::GetWindowPos();
      max_w = glm::max(max_w, size.x);

      brain_label_size[0] = glm::vec3(size.x, size.y, 0.0f);
      brain_label_pos[0] = glm::vec3(pos.x, pos.y + size.y, 0.0f); //adjust position to lower-left

      ImGui::End();
      ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + size.y));
   }

   for (auto &node : HemisphereNodes)
   {
      const int id = node->id;
      ImGui::Begin(node->name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
      ImGui::Text(node->name.c_str());

      ImVec2 size = ImGui::GetWindowSize();
      ImVec2 pos = ImGui::GetWindowPos();
      max_w = glm::max(max_w, size.x);

      hemisphere_label_size[id] = glm::vec3(size.x, size.y, 0.0f);
      hemisphere_label_pos[id] = glm::vec3(pos.x, pos.y + size.y, 0.0f); //adjust position to lower-left

      ImGui::End();
      ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + size.y));
   }
   column_pos += max_w;
   ImGui::SetNextWindowPos(ImVec2(column_pos, 0.0f));
   max_w = 0.0f;
   
   for (auto &node : LobeNodes)
   {
      const int id = node->id;
      std::string s = node->parent->name;
      std::string delimiter = " ";
      std::string side = s.substr(0, s.find(delimiter)); // token is "left" or "right"
      std::string name = side + " " + node->name;
      ImGui::Begin(name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
      ImGui::Text(name.c_str());

      ImVec2 size = ImGui::GetWindowSize();
      ImVec2 pos = ImGui::GetWindowPos();
      max_w = glm::max(max_w, size.x);

      lobe_label_size[id] = glm::vec3(size.x, size.y, 0.0f);
      lobe_label_pos[id] = glm::vec3(pos.x, pos.y + size.y, 0.0f); //adjust position to lower-left

      ImGui::End();
      ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + size.y));
   }
 
   column_pos += max_w;
   ImGui::SetNextWindowPos(ImVec2(column_pos, 0.0f));
   max_w = 0.0f;
   for (auto &node : RegionNodes)
   {
      const int id = node->id;
      std::string s = node->parent->parent->name;
      std::string delimiter = " ";
      std::string side = s.substr(0, s.find(delimiter)); // token is "left" or "right"
      std::string name = side + " " + node->name;

      ImGui::Begin(name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
      ImGui::Text(name.c_str());

      ImVec2 size = ImGui::GetWindowSize();
      ImVec2 pos = ImGui::GetWindowPos();
      max_w = glm::max(max_w, size.x);

      region_label_size[id] = glm::vec3(size.x, size.y, 0.0f);
      region_label_pos[id] = glm::vec3(pos.x, pos.y + size.y, 0.0f); //adjust position to lower-left

      ImGui::End();
      ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + size.y));
   }
}

void InitLabels()
{ 
   brain_label_pos.resize(1);
   brain_label_size.resize(1);

   hemisphere_label_pos.resize(HemisphereNodes.size());
   hemisphere_label_size.resize(HemisphereNodes.size());

   lobe_label_pos.resize(LobeNodes.size());
   lobe_label_size.resize(LobeNodes.size());

   region_label_pos.resize(RegionNodes.size());
   region_label_size.resize(RegionNodes.size());
  
   ImGui_Impl_VR_NewFrame(0);
   ImGui_Impl_VR_Render(0);

   ImGuiContext* last_context = ImGui::GetCurrentContext(); // this will be reset at end of function
   g_Context = ImGui::CreateContext();
   ImGui::SetCurrentContext(g_Context);
   CreateLabelFontTexture();
   
   glGenTextures(1, &g_LabelTexture);
   glBindTexture(GL_TEXTURE_2D, g_LabelTexture);
   glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, g_TexWidth, g_TexHeight);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


   glDisable(GL_DEPTH_TEST);

   glGetIntegerv(GL_VIEWPORT, g_LastViewport);
   glViewport(0, 0, g_TexWidth, g_TexHeight);

   glGenFramebuffers(1, &g_LabelFBO);
   glBindFramebuffer(GL_FRAMEBUFFER, g_LabelFBO);
   glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, g_LabelTexture, 0);

   glDrawBuffer(GL_COLOR_ATTACHMENT0);
   float clearcolor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
   glClearBufferfv(GL_COLOR, 0, clearcolor);

   ImGuiIO& io = ImGui::GetIO();
   
   // Setup display size (every frame to accommodate for window resizing)
   io.DisplaySize = ImVec2((float)g_TexWidth, (float)g_TexHeight);
   io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
   io.DeltaTime = 1.0f / 60.0f;
   io.RenderDrawListsFn = ImGui_Impl_VR_RenderDrawLists;

   ImGui::NewFrame();
      DrawImGuiLabels();
   ImGui::Render();
   glClearBufferfv(GL_COLOR, 0, clearcolor);
   ImGui::NewFrame();
      DrawImGuiLabels();
   ImGui::Render();
   glFinish();
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   glBindTexture(GL_TEXTURE_2D, g_LabelTexture);
   glGenerateMipmap(GL_TEXTURE_2D);
   glDrawBuffer(GL_BACK);
   glEnable(GL_DEPTH_TEST);
   glViewport(g_LastViewport[0], g_LastViewport[1], g_LastViewport[2], g_LastViewport[3]);

   ImGui::SetCurrentContext(last_context);

   CreateCube();

   InitLeaderLines();
}