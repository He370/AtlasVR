#include <iostream>

#include "glUtils.h"

#include <GL/glew.h>
//#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glext.h>

using namespace std;

//check for errors and print error codes.
void glError()
{
#ifdef _DEBUG
   GLenum errCode;
   const GLubyte *errString;
   if ((errCode = glGetError()) != GL_NO_ERROR)
   {
      errString = gluErrorString(errCode);
      cout << "OpenGL Error: " << errString << endl;
   }
#else
	return;
#endif
}

bool CheckFramebufferStatus()
{
   GLenum status;
   status = (GLenum)glCheckFramebufferStatus(GL_FRAMEBUFFER);
   switch (status) {
   case GL_FRAMEBUFFER_COMPLETE:
      return true;
   case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      printf("Framebuffer incomplete, incomplete attachment\n");
      return false;
   case GL_FRAMEBUFFER_UNSUPPORTED:
      printf("Unsupported framebuffer format\n");
      return false;
   case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      printf("Framebuffer incomplete, missing attachment\n");
      return false;
   case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      printf("Framebuffer incomplete, missing draw buffer\n");
      return false;
   case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      printf("Framebuffer incomplete, missing read buffer\n");
      return false;
   }
   return false;
}