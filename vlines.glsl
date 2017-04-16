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

layout(std140, binding = 4) uniform ModelUniforms
{
   mat4 M;	//modeling matrix

   mat4 P; //plane orientation

   mat4 M_slab[10]; //slab transforms

   vec4 ka;	//ambient material color
   vec4 kd;	//diffuse material color
   vec4 ks;	//specular material color

   float shininess; 
   int instances;
   int id;
   int selected;

   float slab_width;
   float slab_offset;
   float slab_instance_offset;
};

in vec3 pos_attrib;     
in vec4 tex_coord_attrib;
in vec3 normal_attrib;
in vec3 color_attrib;

flat out float edgeID;
out vec3 color;

void main(void)
{      
	gl_Position = PV*M*vec4(pos_attrib, 1.0);
	edgeID = tex_coord_attrib.x;
	color = color_attrib;
}