#ifndef __VIRTUALTRACKBALL_H__
#define __VIRTUALTRACKBALL_H__

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_inverse.hpp>

class VirtualTrackball 
{
private:
    int windowWidth;
    int windowHeight;
    int mouseEvent;
    float rollSpeed;
    float angle;
    glm::vec3 prevPos;
    glm::vec3 currPos;
    glm::vec3 camAxis;

    glm::mat4 accum_rot;

    glm::vec3 dolly;
	float dollyscale = 1.0f;
    
    bool xAxis;
    bool yAxis;
    
public:
    VirtualTrackball(float roll_speed = 1.0f, bool x_axis = true, bool y_axis = true );
    glm::vec3 toScreenCoord( double x, double y );

    void setAngleAxis(float angle, glm::vec3& axis);
    void setRotation(glm::mat4& V);
    glm::vec3 getDolly() {return dolly;}
	float getDollyScale() { return dollyscale; }
    
    void mouseButtonCallback(int button, int state);
    void cursorCallback(double x, double y );
    void setDolly(glm::vec3 d);
    
    glm::mat4 createViewRotationMatrix();
    glm::mat4 createModelRotationMatrix( glm::mat4& view_matrix );
    glm::mat4 getAccumRotationMatrix();
    
};

#endif