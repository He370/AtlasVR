#version 400

out vec4 fragcolor;      
in vec2 textCoord;

uniform sampler2D diffuseColor;
//uniform float time;
             
void main(void)
{   

	fragcolor = texture(diffuseColor, textCoord);
	//fragcolor = vec4(color.rgb, 0.5);
	//fragcolor = texture(0, textCoord);
	//fragcolor = vec4(1.0, 0.0, 0.0, 0.5);

}


