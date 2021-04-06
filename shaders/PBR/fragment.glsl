#version 460 core

struct Light
{
	int type; // 0 => directional, 1 => spot
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;
	vec3 color;
	mat4 lightSpaceMatrix;
};

struct Material
{
	vec3 albedo;
	float metallic;
	float roughness;
	float ao;
	float opacity;
	sampler2D albedoMap;
	int hasAlbedo;
	sampler2D metallicRoughMap;
	int hasMetallicRough;
	sampler2D normalMap;
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
	mat4 viewMatrix;
	mat4 projMatrix;
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

const float PI = 3.14159265359;
mat3 TBN;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(material.normalMap, fs_in.texCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.fragPos);
    vec3 Q2  = dFdy(fs_in.fragPos);
    vec2 st1 = dFdx(fs_in.texCoords);
    vec2 st2 = dFdy(fs_in.texCoords);

    vec3 N   = normalize(fs_in.normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
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
		if(texture(material.albedoMap, fs_in.texCoords).a == 0.0f)
			discard;
	}

	vec3 albedo;
	float metallic;
	float roughness;
	float ao = material.ao;
	float opacity = material.opacity;
	if(material.hasAlbedo == 1)
    	albedo = pow(texture(material.albedoMap, fs_in.texCoords).rgb, vec3(2.2f));
	else
		albedo = material.albedo;
	if(material.hasMetallicRough == 1)
	{
    	metallic = texture(material.metallicRoughMap, fs_in.texCoords).b;
    	roughness = texture(material.metallicRoughMap, fs_in.texCoords).g;
	}
	else
	{
		metallic = material.metallic;
		roughness = material.roughness;
	}

	vec3 N;
	if(material.hasNormal == 1)
    	N = getNormalFromMap();
	else
		N = fs_in.normal;
    vec3 V = normalize(cam.viewPos - fs_in.fragPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < lightCount; ++i) 
    {
		// calculate per-light radiance
        vec3 L = normalize(light[i].position - fs_in.fragPos);
        vec3 H = normalize(V + L);
        float distance = length(light[i].position - fs_in.fragPos);
        float attenuation;
		if(light[i].type == 1)
			attenuation = 1.0f / (distance * distance);
		else
			attenuation = 1.0f;
        vec3 radiance = light[i].color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator    = NDF * G * F; 
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);        

		// calculate shadow
		float illumination;
		float theta;
		float intensity;
		vec3 lightDir;

		if(light[i].type == 0)
			lightDir = light[i].direction;
		else if(light[i].type == 1)
		{
			lightDir = normalize(fs_in.fragPos - light[i].position);
			theta = dot(lightDir, normalize(light[i].direction));
			float epsilon = light[i].cutOff - light[i].outerCutOff;
			intensity = clamp((theta - light[i].outerCutOff) / epsilon, 0.0f, 1.0f);
		}

		if(shadowOn == 1)
			illumination = (shadowMethod == 0) ? 1.0f - calculateShadow(light[i].lightSpaceMatrix * vec4(fs_in.fragPos, 1.0f), lightDir, i) : percentIllumination(i, lightDir);
		else if(shadowOn == 0)
			illumination = 1.0f;

		// add to outgoing radiance Lo
		if(light[i].type == 1 && theta > light[i].outerCutOff)
        	Lo += (kD * albedo / PI + specular) * radiance * NdotL * illumination * intensity;
		else if(light[i].type != 1)
        	Lo += (kD * albedo / PI + specular) * radiance * NdotL * illumination;
    }

    vec3 ambient = albedo * ao * 0.03;
    vec3 color = ambient + Lo;
    fragColor = vec4(color, 1.0);
}
