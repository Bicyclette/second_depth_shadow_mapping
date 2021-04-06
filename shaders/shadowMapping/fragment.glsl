#version 460 core

uniform sampler2D diffuse;
uniform int hasDiffuse;

float linearizeDepth(float depth)
{
	float near = 0.1f;
	float far = 100.0f;
	float ndc = depth * 2.0f - 1.0f;
	float z = (2.0f * near * far) / (far + near - ndc * (far - near));
	return z / far;
}

in VS_OUT
{
	vec2 texCoords;
}fs_in;

uniform vec3 lightPosition;

void main()
{
	if(hasDiffuse == 1 && texture(diffuse, fs_in.texCoords).a == 0.0f)
		gl_FragDepth = 1.0f;
	else
	{
		gl_FragDepth = gl_FragCoord.z;
	}
}
