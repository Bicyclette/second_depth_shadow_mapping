#version 460 core

struct Material
{
	vec4 color;
	float shininess;
	sampler2D diffuse;
	int hasDiffuse;
	sampler2D specular;
	int hasSpecular;
	sampler2D normal;
	int hasNormal;
	int nbTextures;
};

in vec2 texCoords;

uniform Material material;
uniform sampler2D depthMap;

out vec4 color;

vec4 blurKernel(vec2 offsets[9])
{
	float kernel[9] = {
		1.0f/16.0f, 2.0f/16.0f, 1.0f/16.0f,
		2.0f/16.0f, 4.0f/16.0f, 2.0f/16.0f,
		1.0f/16.0f, 2.0f/16.0f, 1.0f/16.0f
	};

	vec3 res = vec3(0.0f);
	vec3 sampleTex[9];
	for(int i = 0; i < 9; ++i)
	{
		sampleTex[i] = vec3(texture(material.diffuse, texCoords + offsets[i]));
		res += sampleTex[i] * kernel[i];
	}

	return vec4(res, 1.0f);
}

vec4 gammaCorrection(vec4 c)
{
	float gamma = 1.0/2.2;
	return vec4(pow(c.rgb, vec3(gamma)), c.a);
}

void main()
{
	float offset = 1.0f / 300.0f;

	vec2 offsets[9] = {
		vec2(-offset, offset),	// top left
		vec2(0.0f, offset), 	// top center
		vec2(offset, offset),	// top right
		vec2(-offset, 0.0f),	// left
		vec2(0.0f, 0.0f), 		// center
		vec2(offset, 0.0f),		// right
		vec2(-offset, -offset),	// bottom left
		vec2(0.0f, -offset), 	// bottom center
		vec2(offset, -offset),	// bottom right
	};

	// get quad color
	color = texture(material.diffuse, texCoords);

	// reinhard tone mapping
	vec3 mapped = color.rgb / (color.rgb + vec3(1.0));

	// gamma correction
	color = gammaCorrection(vec4(mapped, 1.0));
}
