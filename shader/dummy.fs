#version 330 core

in vec2 texcoord;

uniform vec2 resolution;
uniform vec2 mouse;
uniform float time;

layout (location = 0) out vec4 fragcolor;
void main()
{
	vec3 color = vec3(0);
	vec2 position = vec2(0.0, 0.0);
	vec2 coord = 2.0*gl_FragCoord.xy/resolution - vec2(1.0);
	
	coord.x *= resolution.x/resolution.y;
	float v = dot(coord, coord);
	float d = length(coord);
	if(v < 0.1 + abs(cos(time*5.0))*0.2){
		color.r = 1.0;
	}

	fragcolor = vec4(color, 1.0);
}
