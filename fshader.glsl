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

uniform sampler2DArray backfaces;
uniform samplerCube cubemap;
uniform sampler1D tex1d;
uniform sampler3D tex3d;

uniform mat4 M_manip_slabs[10];
uniform mat4 Pv_vr;
uniform int flag_manip=0;

uniform vec4 world_eye;

uniform float slider[5];
uniform float selectedEdge;
uniform int selected_slab;
uniform bool green_screen = false;

in vec4 f_tex_coord;
in vec4 f_pos;
in vec3 f_normal;
mat4 M_inst;

flat in int instanceID;
flat in float edgeID;

out vec4 fragcolor;   

vec4 surf_instance_volume_render();
void draw_instance_backfaces();

vec4 surf_instanced_raytracedcolor(vec3 rayStart, vec3 rayStop);
float distToSlab(vec3 pos);
vec4 slab_lighting(vec3 pos, vec4 color);
vec4 lighting();

vec4 surf_instanced_analytic(vec3 rayStart, vec3 rayStop);
float intersectSlab(vec3 rayStart, vec3 rayStop);

vec3 tex3d_size;

vec4 sky(vec3 dir)
{
	dir = normalize(dir);
	return mix(vec4(0.86, 0.86, 0.86, 1.0), vec4(1.0, 1.0, 1.0, 1.0), smoothstep(-0.5, -0.25, dir.y)); 
}

void main(void)
{   
	if(flag_manip==1)
	{
		M_inst = M_manip_slabs[instanceID];
		vec4 clipPos = Pv_vr*f_pos;
	   //vec4 clipPos = PV*M*vec4(256.0*f_tex_coord.xyz, 1.0);
	   gl_FragDepth = 0.5*(gl_DepthRange.diff*clipPos.z/clipPos.w + gl_DepthRange.near+gl_DepthRange.far); //all passes must write depth, per spec
	}
	else
	{
		M_inst = M_slab[instanceID];
		vec4 clipPos = PV*f_pos;
   //vec4 clipPos = PV*M*vec4(256.0*f_tex_coord.xyz, 1.0);
   gl_FragDepth = 0.5*(gl_DepthRange.diff*clipPos.z/clipPos.w + gl_DepthRange.near+gl_DepthRange.far); //all passes must write depth, per spec
	}

   tex3d_size = vec3(textureSize(tex3d, 0));

   

   if(pass == 1)
	{
		fragcolor = f_tex_coord;
		return;	
	}

   else if(pass == 2)
   {
	  if(green_screen == true)
	  {
		fragcolor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	  }
	  else
	  {
		fragcolor = sky(f_pos.xyz);
	  }
      return;
   }

   else if(pass == 3)
   {
      fragcolor = lighting();
   }

   else if(pass == 4) //backfaces
	{
		fragcolor = f_tex_coord;
		return;	
	}

   if(pass == 5)
   {
      fragcolor = vec4(1.0, 1.0, 0.0, 1.0);
      return;
   }

   if(pass == 6) //picking
   {
      fragcolor = vec4(float(id)/255.0, 1.0, 1.0, 1.0);
      return;
   }

   if(pass == 7) 
   {
      draw_instance_backfaces();
      return;
   }

   if(pass == 8) 
   {
      fragcolor = surf_instance_volume_render();
      /*
      if(selected==1)
      {
         fragcolor.r += 0.2*sin(10.0*time);    
      }
      */
      return;
   }

   if(pass == 9) //draw boxes
   {
      fragcolor = vec4(1.0, 0.0, 0.0, 1.0);
      if(edgeID == selectedEdge)
      {
         fragcolor = vec4(0.0, 1.0, 0.0, 1.0);
      }
      return;
   }
}  

vec4 surf_instance_volume_render()
{
   vec3 rayStart = vec3(f_tex_coord);
   vec3 rayStop = texelFetch(backfaces, ivec3(gl_FragCoord.xy, instanceID), 0).xyz;

   //return f_tex_coord; //see front face tex coords

   //return vec4(rayStop, 1.0); //see back face tex coords

   //return texture(tex3d, f_tex_coord.yxz);

   //vec4 color = surf_instanced_raytracedcolor(rayStart, rayStop);
   vec4 color = surf_instanced_analytic(rayStart, rayStop);

   return color;
}

float distToSlab(vec3 pos)
{
   if(instances <= 1)
   {
      return -1000.0;
   }

   vec4 Rpos = P*vec4(pos.xyz, 1.0);

   float d = abs(Rpos.x-slab_offset - slab_instance_offset*(instanceID-4.5)) - 1.04*slab_width;

   return d;
}

float ray_plane_intersect(vec3 r0, vec3 r1, vec3 p, vec3 n)
{
	return dot(p-r0, n)/dot(r1-r0, n);
}

float plane_dist(vec3 pt, vec3 p, vec3 n)
{
   return dot(pt-p, n);
}

float intersectSlab(vec3 rayStart, vec3 rayStop)
{
	vec4 r0 = vec4(rayStart.xyz, 1.0);
	vec4 r1 = vec4(rayStop.xyz, 1.0);
	vec3 dir = r1.xyz-r0.xyz;

	vec3 p0 = vec3(slab_offset + slab_instance_offset*(instanceID-4.5) - 0.95*slab_width, 0.0, 0.0);
	vec3 n0 = vec3(-1.0, 0.0, 0.0);

	p0 = vec3(P*vec4(p0, 1.0));
	n0 = vec3(P*vec4(n0, 0.0));

	float d0 = plane_dist(rayStart, p0, n0);
	float d1 = plane_dist(rayStop, p0, n0);

	if(d0 > 0.0 && d1 > 0.0)
	{
		return 100.0f; //discard
	}

	float t0 = ray_plane_intersect(r0.xyz, r1.xyz, p0, n0);

	vec3 p1 = vec3(slab_offset + slab_instance_offset*(instanceID-4.5) + 0.95*slab_width, 0.0, 0.0);
	vec3 n1 = vec3(+1.0, 0.0, 0.0);

	p1 = vec3(P*vec4(p1, 1.0));
	n1 = vec3(P*vec4(n1, 0.0));

	d0 = plane_dist(rayStart, p1, n1);
	d1 = plane_dist(rayStop, p1, n1);

	if(d0 > 0.0 && d1 > 0.0)
	{
		return 100.0f; //discard
	}
	float t1 = ray_plane_intersect(r0.xyz, r1.xyz, p1, n1);
	return min(t0,t1);
	
}

vec3 normalToSlab(vec3 pos)
{
	const float h = 0.000001;
	const vec3 Xh = vec3(h, 0.0, 0.0);	
	const vec3 Yh = vec3(0.0, h, 0.0);	
	const vec3 Zh = vec3(0.0, 0.0, h);	

   float d0 = distToSlab(pos);
	return normalize(vec3(distToSlab(pos+Xh)-d0, distToSlab(pos+Yh)-d0, distToSlab(pos+Zh)-d0));
}

void draw_instance_backfaces()
{
   fragcolor = f_tex_coord;
}

vec4 surf_instanced_analytic(vec3 rayStart, vec3 rayStop)
{
	float t = intersectSlab(rayStart, rayStop);

	if(t <= 0.0)
	{
		return lighting();
	}

	if(t >= 1.0)
	{
		discard;
	}

	vec3 pos = mix(rayStart, rayStop, t);
	vec4 color = mix(kd, vec4(1.0), 0.5)*texture(tex3d, vec3(pos.y, 1.0-pos.x, pos.z));
	vec4 clipPos = PV*M*M_inst*vec4(tex3d_size*pos, 1.0);
	if(flag_manip==1)
	{
		clipPos = Pv_vr*M_inst*vec4(tex3d_size*pos, 1.0);
	}
	gl_FragDepth = 0.5*(gl_DepthRange.diff*clipPos.z/clipPos.w + gl_DepthRange.near+gl_DepthRange.far);

	return color; 
}

vec4 surf_instanced_raytracedcolor(vec3 rayStart, vec3 rayStop)
{

	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

	int MaxSamples = 180;

	vec3 rayDir = normalize(rayStop-rayStart);
	float travel = distance(rayStop, rayStart);

   float stepSize0 = travel/MaxSamples;
	float stepSize = stepSize0;
	vec3 pos = rayStart;
	vec3 step = rayDir*stepSize;

   float dist = distToSlab(pos);

   if(dist <= 0.0)
   {
      float w = fwidth(dist);
      float s = 0.3 + 0.7*smoothstep(-0.001, -0.0015, dist);
      //float s = 0.3 + 0.7*smoothstep(-2.0*w, -4.0*w, dist); //constant width lines
      return vec4(s,s,s,1.0)*lighting();
   }


   const float eps = 0.0001;
	for (int i=0; i < MaxSamples && travel > 0.0; ++i, pos += step, travel -= stepSize)
	{
		dist = distToSlab(pos);

      if(dist >= -eps && dist < eps)
      {

         //opaque
            color = texture(tex3d, vec3(pos.y, 1.0-pos.x, pos.z));
            //color = texture(tex1d, color.r); //use transfer function

            //color = slab_lighting(pos, color);

            color = pow(color, vec4(1.5, 1.5, 1.5, 1.0));

         //mip
         /*
            stepSize = 0.25/256.0;
            for(int j=0; j<MaxSamples; j++)
            {
               color = texture(tex3d, pos.yxz);
               dist = distToSlab(pos);
               if(dist <= -0.005)// hit offset slab
               {
                  break;
               }

               if(color.r > 0.5)
               {
                  color = vec4(1.0, 0.0, 0.0, 1.0);
                  break;
               }

               if(travel < stepSize) //hit back face
               {
                  color = mix(color, ka*La, 0.15);
                  break;
               }

               pos += rayDir*stepSize;
               travel -= stepSize;
               

            }
            //color = pow(color, vec4(5.5, 5.5, 5.5, 1.0));
            //*/


         if(selected_slab != instanceID)
         {
            //color *= 0.6;
         }

         //color = texture(tex3d, pos.yxz+vec3(slider[1]-0.5, slider[2]-0.5, slider[3]-0.5));
         //color = texture(tex3d, slider[0]*(vec3(pos.y, 1.0-pos.x, pos.z)-vec3(0.5)) + vec3(0.5));

         //vec4 clipPos = PV*M*M_inst*vec4(256.0*pos, 1.0);
         vec4 clipPos = PV*M*M_inst*vec4(tex3d_size*pos, 1.0);
		 if(flag_manip==1)
			{
				clipPos = Pv_vr*M_inst*vec4(tex3d_size*pos, 1.0);
			}

         gl_FragDepth = 0.5*(gl_DepthRange.diff*clipPos.z/clipPos.w + gl_DepthRange.near+gl_DepthRange.far);
         return color; 
      }

      stepSize = 0.9*dist;
		step = rayDir*stepSize;
	}

   discard;
}

vec4 lighting()
{
   vec3 l = normalize(light_w.xyz-f_pos.xyz);
   vec3 n = normalize(f_normal);

   vec3 r = normalize(reflect(-l, n));
   vec3 v = normalize(world_eye.xyz-f_pos.xyz);

   vec4 phong = 0.2*Ls*ks*pow(max(0.0, dot(r,v)), shininess) + 0.4*Ld*kd*max(0.0, dot(n, l)) + La*ka;

   l = v;
   r = normalize(reflect(-l, n));
   phong += 0.2*Ls*ks*pow(max(0.0, dot(r,v)), shininess*0.5f) + 0.4*Ld*kd*max(0.0, dot(n, l)+0.3);

   l = vec3(0.0, 0.0, 1.0);
   r = normalize(reflect(-l, n));
   phong += 0.2*Ls*ks*pow(max(0.0, dot(r,v)), shininess) + 0.3*Ld*kd*max(0.0, dot(n, l)+0.0);

   float darken = 0.60 + 0.40*smoothstep(0.25, 0.55, abs(dot(n,v)));
   phong.rgb *= darken;

   float gamma = 0.8;
   phong = pow(phong, vec4(gamma, gamma, gamma, 1.0));
   
   float s = 0.95 + 0.15*step(0.1, float(selected));
   phong.rgb *= s;
   return phong; 
}

vec4 slab_lighting(vec3 pos, vec4 color)
{
   vec3 l = normalize(light_w.xyz-pos.xyz);
   vec3 n = normalToSlab(pos);

   vec3 r = normalize(reflect(-l, n));
   vec3 v = normalize(world_eye.xyz-f_pos.xyz);

   vec4 phong = 0.5*Ls*ks*pow(max(0.0, dot(r,v)), shininess) + 0.4*Ld*color*max(0.0, dot(n, l)) + La*color;

   l = v;
   r = normalize(reflect(-l, n));
   phong += 0.2*Ls*ks*pow(max(0.0, dot(r,v)), shininess*0.5f) + 0.4*Ld*color*max(0.0, dot(n, l)+0.3);

   l = vec3(0.0, 0.0, 1.0);
   r = normalize(reflect(-l, n));
   phong += 0.2*Ls*ks*pow(max(0.0, dot(r,v)), shininess) + 0.3*Ld*color*max(0.0, dot(n, l)+0.0);

   //float s = 0.95 + 0.15*step(0.1, float(selected));
   //phong.rgb *= s;
   //phong  =  1.4*Ld*color*max(0.0, dot(n, l));
   //phong.rgb = n;
   return phong; 
}