#version 330 core

in vec2 texcoord;

uniform vec2 resolution;
uniform vec2 mouse;
uniform float time;

layout (location = 0) out vec4 fragcolor;
void main()
{
	fragcolor = vec4(0.0, 1.0, 0.0, 0.0);
}
