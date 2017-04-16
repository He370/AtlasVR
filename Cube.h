#ifndef __CUBE_H__
#define __CUBE_H__

#include <vector>
#include "LoadMesh.h"

MeshData CreateCube();
MeshData CreateCubeAtGridLocation(int rows, int cols, int slices, int i, int j, int k);

void DrawCube(const MeshData& cube);

MeshData CreateBox();
void DrawBox(const MeshData& box);

MeshData CreateCenterline(int n);
void UpdateCenterline(MeshData& line, float* data);
void DrawLine(const MeshData& line);

MeshData CreateLabelLines();
void UpdateLabelLines(MeshData& lines, std::vector<glm::vec3>& data);
void DrawLabelLines(const MeshData& lines);

#endif