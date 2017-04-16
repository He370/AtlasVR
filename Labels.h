#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

GLuint GetLabelTexture();
void InitLabels();

glm::mat4 GetLabelTexMatrix(int mode, int i);
glm::mat4 GetLabelModelMatrix(int mode, int i);

void SetLeaderLineColor(const glm::vec3& color);
void BeginLeaderLines();
void PushLeaderLine(const glm::vec3& v0, const glm::vec3& v1);
void EndLeaderLines();
void DrawLeaderLines();
void DrawCube();
