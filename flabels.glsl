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

uniform sampler2D labelTex;

out vec4 fragcolor;
in vec3 tex_coord;  
in vec2 box_tex_coord; 

void main(void)
{   
	vec2 edge1 = vec2(1.0) - smoothstep(vec2(0.0), 2.0f*fwidth(box_tex_coord)+vec2(0.01), box_tex_coord);
	vec2 edge2 = smoothstep(vec2(1.0)-2.0f*fwidth(box_tex_coord)-vec2(0.01), vec2(1.0), box_tex_coord);
	float border = max(max(edge1.x, edge1.y), max(edge2.x, edge2.y));

	fragcolor = vec4(border) + texture(labelTex, tex_coord.st);
	
	fragcolor.rgb = vec3(1.0)-fragcolor.rgb;
	//fragcolor.rgb = vec3(tex_coord, 0.0);
}  

