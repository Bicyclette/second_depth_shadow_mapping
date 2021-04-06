#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTex;
layout (location = 5) in mat4 instanceModel;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform bool instancing;

out VS_OUT
{
	vec2 texCoords;
}vs_out;

void main()
{
	vs_out.texCoords = aTex;
	if(instancing)
	{
		gl_Position = proj * view * instanceModel * vec4(aPos, 1.0f);
	}
	else
	{
		gl_Position = proj * view * model * vec4(aPos, 1.0f);
	}

}
