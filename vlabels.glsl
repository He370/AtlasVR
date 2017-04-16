#version 420             

layout(std140, binding = 3) uniform PassUniforms
{
   mat4 PV;	//camera projection * view matrix

   vec4 La;	//ambient light color
   vec4 Ld;	//diffuse light color
   vec4 Ls;	//specular light color
   vec4 eye_w;	//world-space eye position
   vec4 light_w; //world-space light position

   int pass;
   float time;
};

uniform mat4 M;
uniform mat4 Q;

in vec3 pos_attrib;     
in vec3 tex_coord_attrib;
in vec3 normal_attrib;


out vec3 tex_coord;
out vec2 box_tex_coord;

void main(void)
{      
	gl_Position = PV*M*vec4(pos_attrib, 1.0);
	tex_coord.stp = vec3(Q*vec4(tex_coord_attrib, 1.0));
	box_tex_coord = tex_coord_attrib.st;
}