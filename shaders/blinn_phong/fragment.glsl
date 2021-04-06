#version 460 core

struct Light
{
	int type; // 0 => directional, 1 => spot
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;
	vec3 ambientStrength;
	vec3 diffuseStrength;
	vec3 specularStrength;
	mat4 lightSpaceMatrix;
};

struct Material
{
	vec3 color_diffuse;
	vec3 color_specular;
	vec3 color_ambient;
	float opacity;
	float shininess;
	sampler2D diffuse;
	int hasDiffuse;
	sampler2D specular;
	int hasSpecular;
	sampler2D normal;
	int hasNormal;
	int nbTextures;
};

struct Camera
{
	vec3 viewPos;
};

in VS_OUT
{
	vec2 texCoords;
	vec3 normal;
	vec3 fragPos;
	mat3 TBN;
} fs_in;

uniform Camera cam;

uniform int solid;
uniform int shadowOn;
uniform int shadowMethod;
uniform float bias;
uniform int lightCount;
uniform Light light[10];
uniform sampler2D depthMap[10];

uniform Material material;

out vec4 fragColor;

float linearizeDepth(float depth)
{
	float near = 0.1f;
	float far = 100.0f;
	float ndc = depth * 2.0f - 1.0f;
	float z = (2.0f * near * far) / (far + near - ndc * (far - near));
	return z / far;
}

vec3 calculateDiffuse(vec3 lightDir, vec3 diffuseStrength, vec3 objColor)
{
	vec3 norm;
	if(material.hasNormal == 1)
	{
		norm = texture(material.normal, fs_in.texCoords).rgb;
		norm = normalize(norm * 2.0f - 1.0f);
		lightDir = fs_in.TBN * lightDir;
	}
	else
		norm = normalize(fs_in.normal);
	float diff = max(dot(norm, -lightDir), 0.0f);
	return diffuseStrength * diff * objColor;
}

vec3 calculateSpecular(vec3 viewPos, vec3 lightDir, vec3 specularStrength, vec3 objColor)
{
	vec3 fragPos;
	vec3 norm;
	if(material.hasNormal == 1)
	{
		fragPos = fs_in.TBN * fs_in.fragPos;
		norm = texture(material.normal, fs_in.texCoords).rgb;
		norm = normalize(norm * 2.0f - 1.0f);
		lightDir = fs_in.TBN * lightDir;
	}
	else
	{
		fragPos = fs_in.fragPos;
		norm = normalize(fs_in.normal);
	}
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 halfwayDir = normalize(-lightDir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0f), material.shininess);
	return specularStrength * spec * objColor;
}

float calculateShadow(vec4 fragPosLightSpace, vec3 lightDir, int l)
{
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(depthMap[l], 0);

	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
	// transform to [0,1] range
	projCoords = (projCoords * 0.5) + 0.5;
	if(projCoords.z > 1.0)
	{
		shadow = 0.0;
		return shadow;
	}

	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float depth = texture(depthMap[l], projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += (currentDepth - bias) > depth ? 1.0 : 0.0;
		}
	}
	
	return shadow / 9.0;
}

float percentIllumination(int l, vec3 lightDir)
{
	float illumCount = 0.0f;
	float virtualIllumCount = 0.0f;
	vec2 texelSize = 1.0 / textureSize(depthMap[l], 0);

	// get pixel world coordinates
	vec3 pW = fs_in.fragPos;

	// do normal depth comparisons
	vec4 pL = light[l].lightSpaceMatrix * vec4(pW, 1.0f);
	vec3 point = pL.xyz / pL.w;
	point = (point * 0.5) + 0.5;
	
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float depth = texture(depthMap[l], point.xy + vec2(x, y) * texelSize).r;
			if(solid == 1)
				illumCount += point.z <= depth ? 1.0f : 0.0f;
			else if(solid == 0)
				illumCount += point.z <= (depth + bias) ? 1.0f : 0.0f;
		}
	}

	if(illumCount < 9.0f && illumCount > 0.0f)
	{
		// get normal at pixel in world coordinates
		vec3 nW = fs_in.normal;
		float dW = -(nW.x * pW.x + nW.y * pW.y + nW.z * pW.z);
		// get tangent plane in lightsource coordinates
		vec4 tL = transpose(inverse(light[l].lightSpaceMatrix)) * vec4(nW, dW);

		for(int x = -1; x <= 1; ++x)
		{
			for(int y = -1; y <= 1; ++y)
			{
				float xL = x * texelSize.x + point.x;
				float yL = y * texelSize.y + point.z;
				float depth = (-tL.x * xL) / tL.z + (-tL.y * yL) / tL.z + (-tL.w / tL.z);
				if(solid == 1)
					illumCount += point.z <= depth ? 1.0f : 0.0f;
				else if(solid == 0)
					illumCount += point.z <= (depth + bias) ? 1.0f : 0.0f;
			}
		}

		return illumCount / 9.0f;
	}
	else
		return illumCount;
}

void main()
{
	// early discard
	if(material.nbTextures > 0)
	{
		if(texture(material.diffuse, fs_in.texCoords).a == 0.0f)
			discard;
	}

	// start
	vec3 color;

	for(int l = 0; l < lightCount; ++l)
	{
		vec3 lightPos = light[l].position;
		vec3 fragPos = fs_in.fragPos;
		vec3 viewPos = cam.viewPos;

		if(material.hasNormal == 1)
		{
			lightPos = fs_in.TBN * light[l].position;
			fragPos = fs_in.TBN * fs_in.fragPos;
			viewPos = fs_in.TBN * cam.viewPos;
		}

		vec3 lightDir;
		float dist = length(lightPos - fragPos);
		float theta;
		float epsilon;
		float intensity;

		if(light[l].type == 0)
		{
			lightDir = light[l].direction;
		}
		else if(light[l].type == 1)
		{
			lightDir = normalize(fs_in.fragPos - light[l].position);
			theta = dot(lightDir, normalize(light[l].direction));
			epsilon = light[l].cutOff - light[l].outerCutOff;
			intensity = clamp((theta - light[l].outerCutOff) / epsilon, 0.0f, 1.0f);
		}

		// calculate shadow
		float illumination;
		if(shadowOn == 1)
			illumination = (shadowMethod == 0) ? 1.0f - calculateShadow(light[l].lightSpaceMatrix * vec4(fs_in.fragPos, 1.0f), lightDir, l) : percentIllumination(l, lightDir);
		else if(shadowOn == 0)
			illumination = 1.0f;

		if(material.nbTextures > 0)
		{
			// ambient
			vec3 ambient = light[l].ambientStrength * texture(material.diffuse, fs_in.texCoords).rgb;

			if(light[l].type == 1 && theta > light[l].outerCutOff)
			{
				// diffuse
				vec3 diffuse = calculateDiffuse(lightDir, light[l].diffuseStrength, texture(material.diffuse, fs_in.texCoords).rgb);

				// specular
				vec3 specular;
				if(material.hasSpecular == 1)
					specular = calculateSpecular(viewPos, lightDir, light[l].specularStrength, texture(material.specular, fs_in.texCoords).rgb);
				else
					specular = calculateSpecular(viewPos, lightDir, light[l].specularStrength, material.color_specular);
				
				diffuse = diffuse * intensity * illumination;
				specular = specular * intensity * illumination;
			
				// putting it all together
				color += ambient + diffuse + specular;
			}
			else if(light[l].type == 1)
			{
				color += ambient;
			}
			else
			{
				// diffuse
				vec3 diffuse = calculateDiffuse(lightDir, light[l].diffuseStrength, texture(material.diffuse, fs_in.texCoords).rgb) * illumination;

				// specular
				vec3 specular;
				if(material.hasSpecular == 1)
					specular = calculateSpecular(viewPos, lightDir, light[l].specularStrength, texture(material.specular, fs_in.texCoords).rgb) * illumination;
				else
					specular = calculateSpecular(viewPos, lightDir, light[l].specularStrength, material.color_specular) * illumination;
				
				// putting it all together
				color += (ambient + diffuse + specular);
			}
		}

		else
		{
			// ambient
			vec3 ambient = light[l].ambientStrength * material.color_ambient;

			if(light[l].type == 1 && theta > light[l].outerCutOff)
			{
				// diffuse
				vec3 diffuse = calculateDiffuse(lightDir, light[l].diffuseStrength, material.color_diffuse) * illumination;

				// specular
				vec3 specular = calculateSpecular(viewPos, lightDir, light[l].specularStrength, material.color_specular) * illumination;
				
				diffuse = diffuse * intensity;
				specular = specular * intensity;
			
				// putting it all together
				color += ambient + diffuse + specular;
			}
			else if(light[l].type == 1)
			{
				color += ambient;
			}
			else
			{
				// diffuse
				vec3 diffuse = calculateDiffuse(lightDir, light[l].diffuseStrength, material.color_diffuse) * illumination;

				// specular
				vec3 specular = calculateSpecular(viewPos, lightDir, light[l].specularStrength, material.color_specular) * illumination;

				// putting it all together
				color += (ambient + diffuse + specular);
			}
		}
	}

	float alpha = (material.hasDiffuse == 1) ? texture(material.diffuse, fs_in.texCoords).a * material.opacity : material.opacity;
	fragColor = vec4(color, alpha);
}
