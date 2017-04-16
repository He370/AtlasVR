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

uniform vec4 line_color;
uniform float selectedEdge;

flat in float edgeID;
in vec3 color;
out vec4 fragcolor;   

void main(void)
{   

   //if(selectedEdge < 0.0)
   {
      //fragcolor = vec4(0.2, 0.2, 0.2, 1.0);
      //fragcolor = line_color;
	  //fragcolor.a = 0.9f;
	  fragcolor = vec4(color, 0.9);
      
      return;
   }

   fragcolor = vec4(1.0, 0.0, 0.0, 1.0);

   if(edgeID == selectedEdge)
   {
      fragcolor = vec4(0.0, 1.0, 0.0, 1.0);
   }
      
   return;
  
}  

