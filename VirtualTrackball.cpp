#include <algorithm>
#include <glm/gtx/orthonormalize.hpp>
#include "VirtualTrackball.h"
#include <GL/glut.h>

VirtualTrackball::VirtualTrackball(float roll_speed, bool x_axis, bool y_axis  ) 
{
   int vp[4];
   glGetIntegerv( GL_VIEWPORT, vp);
   windowWidth  = vp[2];
   windowHeight = vp[3];
    
   mouseEvent = 0;
   rollSpeed  = roll_speed;
   angle      = 0.0f;
   camAxis    = glm::vec3(0.0f, 1.0f, 0.0f);

   accum_rot = glm::mat4(1.0f);

   dolly = glm::vec3(0.0f);
   dollyscale = 1.0f;
    
   xAxis = x_axis;
   yAxis = y_axis;
}

glm::vec3 VirtualTrackball::toScreenCoord( double x, double y ) 
{
   glm::vec3 coord(0.0f);
    /*
   if( xAxis )
   {
      coord.x =  float(2.0 * x - windowWidth ) / float(windowWidth);
   }
   if( yAxis )
   {
      coord.y = -float(2.0 * y - windowHeight) / float(windowHeight);
   }
   */

   if (xAxis)
   {
	   coord.x = x;
   }
   if (yAxis)
   {
	   coord.y = y;
   }

   coord = glm::clamp( coord, glm::vec3(-1.0f), glm::vec3(1.0f) );

   float length_squared = coord.x * coord.x + coord.y * coord.y;
   if( length_squared <= 1.0f )
   {
      coord.z = sqrt( 1.0f - length_squared );
   }    
   else
   {
      coord = glm::normalize( coord );
   }
   return coord;
}

void VirtualTrackball::setDolly(glm::vec3 d)
{
   dolly = d;
}

void VirtualTrackball::setAngleAxis(float angle, glm::vec3& axis)
{
   accum_rot = glm::rotate(angle, axis); 
}

void VirtualTrackball::setRotation(glm::mat4& V)
{
   accum_rot = V;
   accum_rot[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}


void VirtualTrackball::mouseButtonCallback( int button, int state )
{
    mouseEvent = ( state == GLUT_DOWN && button == GLUT_LEFT_BUTTON );

    if(state == GLUT_UP && button == GLUT_LEFT_BUTTON)
    {
      if(angle > 0.01f)
      {
         glm::mat3 accum = glm::mat3(glm::rotate( glm::degrees(angle) * rollSpeed, camAxis )*accum_rot);
         accum_rot = glm::mat4(glm::orthonormalize(accum));
         angle = 0.0f;
      }
    }

    if(state == GLUT_DOWN && button == 3)
    {
       dollyscale += 0.1f;
    }
    if(state == GLUT_DOWN && button == 4)
    {
       dollyscale -= 0.1f;
    }
}

void VirtualTrackball::cursorCallback( double x, double y )
{
   if( mouseEvent == 0 )
   {
      mouseEvent = 1;
      return;
   }
   else if( mouseEvent == 1 ) 
   {
      /* Start of trackball, remember the first position */
      prevPos     = toScreenCoord( x, y );
      mouseEvent  = 2;
      return;
   }
    
   /* Tracking the subsequent */
   currPos  = toScreenCoord( x, y );
    
   /* Calculate the angle in radians, and clamp it between 0 and 90 degrees */
   angle    = acos( std::min(1.0f, glm::dot(prevPos, currPos) ));
    
	/* Cross product to get the rotation axis, but it's still in camera coordinate */
   /*
	camAxis  = glm::cross( prevPos, currPos );
	if (abs(angle) < 1e-6f)
	{	
		camAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	}
   */
   glm::vec3 axis = glm::cross(prevPos, currPos);
   if (glm::length(axis) > 0.001f)
   {
      camAxis = axis;
   }
}

glm::mat4 VirtualTrackball::getAccumRotationMatrix() 
{
    return glm::translate(dollyscale*dolly)*glm::rotate( glm::degrees(angle) * rollSpeed, camAxis )*accum_rot;
}

glm::mat4 VirtualTrackball::createViewRotationMatrix() 
{
    return glm::translate(dollyscale*dolly)*glm::rotate( glm::degrees(angle) * rollSpeed, camAxis );
}


glm::mat4 VirtualTrackball::createModelRotationMatrix( glm::mat4& view_matrix )
{
    glm::vec3 axis = glm::inverse(glm::mat3(view_matrix)) * camAxis;
    return glm::rotate( glm::degrees(angle) * rollSpeed, axis )*glm::translate(dollyscale*dolly);
}