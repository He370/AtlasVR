#version 420             

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec4 g_tex_coord[];
in vec4 g_pos[];
in vec3 g_normal[];


flat in int g_instanceID[];
flat in float g_edgeID[];

out vec4 f_tex_coord;
out vec4 f_pos;
out vec3 f_normal;


flat out int instanceID;
flat out float edgeID;

void main() 
{
   for(int i = 0; i < 3; i++) 
   {
      gl_Position = gl_in[i].gl_Position;
      gl_Layer = g_instanceID[i];

      f_tex_coord = g_tex_coord[i];
      f_pos = g_pos[i];
      f_normal = g_normal[i];


      instanceID = g_instanceID[i];
      edgeID = g_edgeID[i];

      EmitVertex();
   }
   EndPrimitive();
}