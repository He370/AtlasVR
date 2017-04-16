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

uniform sampler3D tex3d;

uniform float slider[5];
uniform vec4 world_eye;
uniform mat4 M_manip_slabs[10];
uniform mat4 Pv_vr;
uniform int flag_manip=0;

in vec3 pos_attrib;     
in vec4 tex_coord_attrib;
in vec3 normal_attrib;

out vec4 g_tex_coord;
out vec4 g_pos;
out vec3 g_normal;

mat4 g_M_inst;
flat out int g_instanceID;
flat out float g_edgeID;


void main(void)
{      
   g_M_inst = mat4(1.0);
   g_pos = M*vec4(pos_attrib, 1.0);
   g_normal = (M*vec4(normal_attrib, 0.0)).xyz;
   g_instanceID = gl_InstanceID;

   if(pass==2)
   {
      gl_Position = PV*M*vec4(pos_attrib, 1.0);
      g_pos = M*vec4(pos_attrib, 1.0);
      return;
   }

   if(pass==3)
   {
      gl_Position = PV*M*vec4(pos_attrib, 1.0);
      g_pos = M*vec4(pos_attrib, 1.0);
      return;
   }
   else
   {

      
 

		if(flag_manip==1)
		{	
			g_M_inst = M_manip_slabs[gl_InstanceID];
			g_pos = g_M_inst*vec4(pos_attrib, 1.0);
			g_normal = (g_M_inst*vec4(normal_attrib, 0.0)).xyz;
			gl_Position = Pv_vr*g_M_inst*vec4(pos_attrib, 1.0);
		}
		else
		{
			g_M_inst = M_slab[gl_InstanceID];
			g_pos = M*g_M_inst*vec4(pos_attrib, 1.0);
			g_normal = (M*g_M_inst*vec4(normal_attrib, 0.0)).xyz;
			gl_Position = PV*M*g_M_inst*vec4(pos_attrib, 1.0);
		}
   }

   g_tex_coord = tex_coord_attrib;
   g_tex_coord.xyz = 0.5*(g_tex_coord.xyz+vec3(1.0));

   if(pass==4 || pass==5 || pass==6 || pass == 7 || pass == 8)
   {
      vec3 tex3d_size = vec3(textureSize(tex3d, 0));
      g_tex_coord = vec4(pos_attrib/tex3d_size, 1.0);
   }

   if(pass == 9)
   {
      g_edgeID = tex_coord_attrib.x;
   }

}