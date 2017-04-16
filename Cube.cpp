#include <glm/glm.hpp>
#include <GL/glew.h>

#include "Cube.h"



MeshData CreateCube()
{
   MeshData meshdata;

   meshdata.mScaleFactor = 0.5f;

   meshdata.mVboNormals = -1;

   using namespace glm;
   vec3 pos[8] = {vec3(-1.0f, -1.0f, -1.0f), vec3(+1.0f, -1.0f, -1.0f), vec3(+1.0f, +1.0f, -1.0f), vec3(-1.0f, +1.0f, -1.0f),
   vec3(-1.0f, -1.0f, +1.0f), vec3(+1.0f, -1.0f, +1.0f), vec3(+1.0f, +1.0f, +1.0f), vec3(-1.0f, +1.0f, +1.0f)};

   vec3 tex[8] = {vec3(-1.0f, -1.0f, -1.0f), vec3(+1.0f, -1.0f, -1.0f), vec3(+1.0f, +1.0f, -1.0f), vec3(-1.0f, +1.0f, -1.0f),
   vec3(-1.0f, -1.0f, +1.0f), vec3(+1.0f, -1.0f, +1.0f), vec3(+1.0f, +1.0f, +1.0f), vec3(-1.0f, +1.0f, +1.0f)};

   unsigned short idx[36] = { 0,2,1, 2,0,3, //bottom
                              0,5,4, 5,0,1, //front
                              1,6,5, 6,1,2, //right 
                              2,7,6, 7,2,3, //back
                              3,4,7, 4,3,0, //left
                              4,5,6, 6,7,4};//top


   //shader attrib locations
   int pos_loc = -1;
   int tex_coord_loc = -1;
   int normal_loc = -1;

   GLint program = -1;
   glGetIntegerv(GL_CURRENT_PROGRAM, &program);

   pos_loc = glGetAttribLocation(program, "pos_attrib");
   tex_coord_loc = glGetAttribLocation(program, "tex_coord_attrib");
   normal_loc = glGetAttribLocation(program, "normal_attrib");

   meshdata.mNumIndices = 36;

   glGenVertexArrays(1, &meshdata.mVao);
	glBindVertexArray(meshdata.mVao);

   //Buffer indices
   glGenBuffers(1, &meshdata.mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshdata.mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

	//Buffer vertices
	glGenBuffers(1, &meshdata.mVboVerts);
	glBindBuffer(GL_ARRAY_BUFFER, meshdata.mVboVerts);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, 0, 0, 0);
		

   // buffer for vertex texture coordinates
	glGenBuffers(1, &meshdata.mVboTexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, meshdata.mVboTexCoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tex), tex, GL_STATIC_DRAW);
	glEnableVertexAttribArray(tex_coord_loc);
	glVertexAttribPointer(tex_coord_loc, 3, GL_FLOAT, 0, 0, 0);

   // no normals
   meshdata.mVboNormals = -1;
   glDisableVertexAttribArray(normal_loc);
      
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

   return meshdata;
}

void DrawCube(const MeshData& cube)
{
   glBindVertexArray(cube.mVao);
   glDrawElements(GL_TRIANGLES, cube.mNumIndices, GL_UNSIGNED_SHORT, 0);
}

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

MeshData CreateCubeAtGridLocation(int rows, int cols, int slices, int i, int j, int k)
{
   MeshData meshdata;

   meshdata.mScaleFactor = 0.5f;

   //meshdata.mVboNormals = -1;

   using namespace glm;
   vec3 pos[8] = {vec3(-1.0f, -1.0f, -1.0f), vec3(+1.0f, -1.0f, -1.0f), vec3(+1.0f, +1.0f, -1.0f), vec3(-1.0f, +1.0f, -1.0f),
                  vec3(-1.0f, -1.0f, +1.0f), vec3(+1.0f, -1.0f, +1.0f), vec3(+1.0f, +1.0f, +1.0f), vec3(-1.0f, +1.0f, +1.0f)};

   vec3 tex[8] = {vec3(-1.0f, -1.0f, -1.0f), vec3(+1.0f, -1.0f, -1.0f), vec3(+1.0f, +1.0f, -1.0f), vec3(-1.0f, +1.0f, -1.0f),
                  vec3(-1.0f, -1.0f, +1.0f), vec3(+1.0f, -1.0f, +1.0f), vec3(+1.0f, +1.0f, +1.0f), vec3(-1.0f, +1.0f, +1.0f)};

   unsigned int idx[36] = { 0,2,1, 2,0,3, //bottom
                              0,5,4, 5,0,1, //front
                              1,6,5, 6,1,2, //right 
                              2,7,6, 7,2,3, //back
                              3,4,7, 4,3,0, //left
                              4,5,6, 6,7,4};//top

   
   glm::mat4 T = glm::translate(glm::vec3(1.0f, 1.0f, 1.0f));
   glm::mat4 S1 = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));

   glm::mat4 Sgrid = glm::scale(glm::vec3(1.0f/rows, 1.0f/cols, 1.0f/slices));
   glm::mat4 Tgrid = glm::translate(glm::vec3(float(i)/rows, float(j)/cols, float(k)/slices));

   glm::mat4 S2 = glm::scale(glm::vec3(256.0f, 256.0f, 256.0f));

   glm::mat4 M = S2*Tgrid*Sgrid*S1*T;
   for(int ix=0; ix<8; ix++)
   {
      pos[ix] = vec3(M*glm::vec4(pos[ix], 1.0f));
      tex[ix] = vec3(M*glm::vec4(tex[ix], 1.0f));
   }

   meshdata.mBbMin = aiVector3D(pos[0].x, pos[0].y, pos[0].z);
   meshdata.mBbMax = aiVector3D(pos[6].x, pos[6].y, pos[6].z);


   //shader attrib locations
   int pos_loc = -1;
   int tex_coord_loc = -1;
   int normal_loc = -1;

   GLint program = -1;
   glGetIntegerv(GL_CURRENT_PROGRAM, &program);

   pos_loc = glGetAttribLocation(program, "pos_attrib");
   tex_coord_loc = glGetAttribLocation(program, "tex_coord_attrib");
   normal_loc = glGetAttribLocation(program, "normal_attrib");

   meshdata.mNumIndices = 36;

   glGenVertexArrays(1, &meshdata.mVao);
	glBindVertexArray(meshdata.mVao);

   //Buffer indices
   glGenBuffers(1, &meshdata.mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshdata.mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

	//Buffer vertices
	glGenBuffers(1, &meshdata.mVboVerts);
	glBindBuffer(GL_ARRAY_BUFFER, meshdata.mVboVerts);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, 0, 0, 0);


   //Buffer vertices
	glGenBuffers(1, &meshdata.mVboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, meshdata.mVboVerts);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
	glEnableVertexAttribArray(normal_loc);
	glVertexAttribPointer(normal_loc, 3, GL_FLOAT, 0, 0, 0);
		

   // buffer for vertex texture coordinates
	glGenBuffers(1, &meshdata.mVboTexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, meshdata.mVboTexCoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tex), tex, GL_STATIC_DRAW);
	glEnableVertexAttribArray(tex_coord_loc);
	glVertexAttribPointer(tex_coord_loc, 3, GL_FLOAT, 0, 0, 0);

   // no normals
   //meshdata.mVboNormals = -1;
   //glDisableVertexAttribArray(normal_loc);
      
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

   return meshdata;
}

MeshData CreateBox()
{

   MeshData meshdata;

   meshdata.mScaleFactor = 1.0f;

   meshdata.mVboNormals = -1;

   using namespace glm;
   vec3 pos[24] = { //12 edges
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

   vec3 tex[24] = { 
      vec3(0.0f / 256.0f), vec3(0.0f / 256.0f), //bottom face
      vec3(1.0f / 256.0f), vec3(1.0f / 256.0f), //bottom face
      vec3(2.0f / 256.0f), vec3(2.0f / 256.0f), //bottom face
      vec3(3.0f / 256.0f), vec3(3.0f / 256.0f), //bottom face
      vec3(4.0f / 256.0f), vec3(4.0f / 256.0f), //top face
      vec3(5.0f / 256.0f), vec3(5.0f / 256.0f), //top face
      vec3(6.0f / 256.0f), vec3(6.0f / 256.0f), //top face
      vec3(7.0f / 256.0f), vec3(7.0f / 256.0f), //top face
      vec3(8.0f / 256.0f), vec3(8.0f / 256.0f), //vertical edges
      vec3(9.0f / 256.0f), vec3(9.0f / 256.0f), //vertical edges
      vec3(10.0f / 256.0f), vec3(10.0f / 256.0f), //vertical edges
      vec3(11.0f / 256.0f), vec3(11.0f / 256.0f)  //vertical edges
   };

   unsigned short idx[24] = { 
      0,1, 2,3, 4,5, 6,7,
      8,9, 10,11, 12,13, 14,15,
      16,17, 18,19, 20,21, 22,23
      };
      
   //shader attrib locations
   int pos_loc = -1;
   int tex_coord_loc = -1;
   int normal_loc = -1;

   GLint program = -1;
   glGetIntegerv(GL_CURRENT_PROGRAM, &program);

   pos_loc = glGetAttribLocation(program, "pos_attrib");
   tex_coord_loc = glGetAttribLocation(program, "tex_coord_attrib");
   normal_loc = glGetAttribLocation(program, "normal_attrib");

   meshdata.mNumIndices = 24;

   glGenVertexArrays(1, &meshdata.mVao);
   glBindVertexArray(meshdata.mVao);

   //Buffer indices
   glGenBuffers(1, &meshdata.mIndexBuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshdata.mIndexBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

   //Buffer vertices
   glGenBuffers(1, &meshdata.mVboVerts);
   glBindBuffer(GL_ARRAY_BUFFER, meshdata.mVboVerts);
   glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
   glEnableVertexAttribArray(pos_loc);
   glVertexAttribPointer(pos_loc, 3, GL_FLOAT, 0, 0, 0);


   // buffer for vertex texture coordinates
   glGenBuffers(1, &meshdata.mVboTexCoords);
   glBindBuffer(GL_ARRAY_BUFFER, meshdata.mVboTexCoords);
   glBufferData(GL_ARRAY_BUFFER, sizeof(tex), tex, GL_STATIC_DRAW);
   glEnableVertexAttribArray(tex_coord_loc);
   glVertexAttribPointer(tex_coord_loc, 3, GL_FLOAT, 0, 0, 0);

   // no normals
   meshdata.mVboNormals = -1;
   //glDisableVertexAttribArray(normal_loc);

   glBindVertexArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   return meshdata;
}


void DrawBox(const MeshData& box)
{
   glBindVertexArray(box.mVao);
   glDrawElements(GL_LINES, box.mNumIndices, GL_UNSIGNED_SHORT, 0);
}

#include <vector>
MeshData CreateCenterline(int n)
{
   MeshData meshdata;

   meshdata.mScaleFactor = 1.0f;

   meshdata.mVboNormals = -1;

   using namespace glm;
   std::vector<glm::vec3> pos(n);
   std::vector<glm::vec3> tex(n);
   std::vector<unsigned short> idx(n);

   for (int i = 0; i < n; i++)
   {
      pos[i] = glm::vec3(0.0f) + float(i)/float(n-1)*glm::vec3(200.0f);
      tex[i] = glm::vec3(float(i)/float(n-1));
      idx[i] = i;
   }

   //shader attrib locations
   int pos_loc = -1;
   int tex_coord_loc = -1;
   int normal_loc = -1;

   GLint program = -1;
   glGetIntegerv(GL_CURRENT_PROGRAM, &program);

   pos_loc = glGetAttribLocation(program, "pos_attrib");
   tex_coord_loc = glGetAttribLocation(program, "tex_coord_attrib");
   normal_loc = glGetAttribLocation(program, "normal_attrib");

   meshdata.mNumIndices = n;

   glGenVertexArrays(1, &meshdata.mVao);
   glBindVertexArray(meshdata.mVao);

   //Buffer indices
   glGenBuffers(1, &meshdata.mIndexBuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshdata.mIndexBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, n*sizeof(unsigned short), idx.data(), GL_STATIC_DRAW);

   //Buffer vertices
   glGenBuffers(1, &meshdata.mVboVerts);
   glBindBuffer(GL_ARRAY_BUFFER, meshdata.mVboVerts);
   glBufferData(GL_ARRAY_BUFFER, n*sizeof(glm::vec3), pos.data(), GL_DYNAMIC_DRAW);
   glEnableVertexAttribArray(pos_loc);
   glVertexAttribPointer(pos_loc, 3, GL_FLOAT, 0, 0, 0);


   // buffer for vertex texture coordinates
   glGenBuffers(1, &meshdata.mVboTexCoords);
   glBindBuffer(GL_ARRAY_BUFFER, meshdata.mVboTexCoords);
   glBufferData(GL_ARRAY_BUFFER, n*sizeof(glm::vec3), tex.data(), GL_STATIC_DRAW);
   glEnableVertexAttribArray(tex_coord_loc);
   glVertexAttribPointer(tex_coord_loc, 3, GL_FLOAT, 0, 0, 0);

   // no normals
   meshdata.mVboNormals = -1;
   glDisableVertexAttribArray(normal_loc);

   glBindVertexArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   return meshdata;
}

void UpdateCenterline(MeshData& line, float* data)
{
   glBindBuffer(GL_ARRAY_BUFFER, line.mVboVerts);
   glBufferData(GL_ARRAY_BUFFER, line.mNumIndices*sizeof(glm::vec3), data, GL_DYNAMIC_DRAW);
}

void DrawLine(const MeshData& line)
{
   glBindVertexArray(line.mVao);
   glDrawElements(GL_LINES, line.mNumIndices, GL_UNSIGNED_SHORT, 0);
}


MeshData CreateLabelLines()
{
   MeshData meshdata;

   meshdata.mScaleFactor = 1.0f;

   meshdata.mVboNormals = -1;

   using namespace glm;

   //shader attrib locations
   int pos_loc = -1;
   int tex_coord_loc = -1;
   int normal_loc = -1;

   GLint program = -1;
   glGetIntegerv(GL_CURRENT_PROGRAM, &program);

   pos_loc = glGetAttribLocation(program, "pos_attrib");
   tex_coord_loc = glGetAttribLocation(program, "tex_coord_attrib");
   normal_loc = glGetAttribLocation(program, "normal_attrib");

   meshdata.mNumIndices = 0;

   glGenVertexArrays(1, &meshdata.mVao);
   glBindVertexArray(meshdata.mVao);

   //Buffer vertices
   glGenBuffers(1, &meshdata.mVboVerts);
   glBindBuffer(GL_ARRAY_BUFFER, meshdata.mVboVerts);
   glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
   glEnableVertexAttribArray(pos_loc);
   glVertexAttribPointer(pos_loc, 3, GL_FLOAT, 0, 0, 0);

   // no normals
   meshdata.mVboNormals = -1;
   glDisableVertexAttribArray(normal_loc);

   meshdata.mVboTexCoords = -1;
   glDisableVertexAttribArray(tex_coord_loc);

   glBindVertexArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   return meshdata;
}

void UpdateLabelLines(MeshData& lines, std::vector<glm::vec3>& data)
{
   lines.mNumIndices = data.size();
   if (lines.mNumIndices > 0)
   {
      glBindBuffer(GL_ARRAY_BUFFER, lines.mVboVerts);
      glBufferData(GL_ARRAY_BUFFER, lines.mNumIndices*sizeof(glm::vec3), &data[0], GL_DYNAMIC_DRAW);
   }
}

void DrawLabelLines(const MeshData& lines)
{
   if(lines.mNumIndices > 0)
   {
      glBindVertexArray(lines.mVao);
      glDrawArrays(GL_LINES, 0, lines.mNumIndices);
   }
}