#pragma once

void InitOpenGL(int w, int h);
void display(glm::mat4& P, glm::mat4& V, glm::mat4& M, GLuint fbo);
int pick(int x, int y);
void rotation_drag_callback(float x, float y);
void rotation_touch_callback(int state);

