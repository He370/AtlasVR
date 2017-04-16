#version 400  	 	 
	     
uniform mat4 PVM;
//uniform float time;

in vec3 pos_attrib;			
in vec2 tex_coord_attrib;
//in vec3 normal_attrib;  

out vec2 textCoord;

void main(void)
{
	float k = 100.0f;
	float aspect = 1.0/2.0;
	gl_Position = PVM*vec4(vec3(pos_attrib.x*aspect,pos_attrib.y,pos_attrib.z)*k, 1.0);
	textCoord = tex_coord_attrib;

}