#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#include <GL/glew.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>

class Light;
class DirectionalLight;
class SpotLight;

enum class SHADER_TYPE
{
	BLINN_PHONG,
	PBR,
	SHADOWS,
	POST_PROCESSING
};

class Shader
{
	public:

		Shader(const std::string & vertex_shader_file, const std::string & fragment_shader_file, SHADER_TYPE t = SHADER_TYPE::BLINN_PHONG);
		Shader(const std::string & vertex_shader_file, const std::string & geometry_shader_file, const std::string & fragment_shader_file, SHADER_TYPE t = SHADER_TYPE::BLINN_PHONG);
		~Shader();
		GLuint getId() const;
		SHADER_TYPE getType();
		void setInt(const std::string & name, int v) const;
		void setFloat(const std::string & name, float v) const;
		void setVec2f(const std::string & name, glm::vec2 v) const;
		void setVec3f(const std::string & name, glm::vec3 v) const;
		void setVec4f(const std::string & name, glm::vec4 v) const;
		void setMatrix(const std::string & name, glm::mat4 m) const;
		void setLighting(std::vector<std::shared_ptr<DirectionalLight>> & dLights, std::vector<std::shared_ptr<SpotLight>> & sLight);
		void use() const;

	private:

		void compile(const char * vertex_shader_code, const char * fragment_shader_code);
		void compile(const char * vertex_shader_code, const char * geometry_shader_code, const char * fragment_shader_code);

		GLuint id;
		SHADER_TYPE type;
};

enum class TEXTURE_TYPE
{
	DIFFUSE,
	SPECULAR,
	NORMAL,
	METALLIC_ROUGHNESS
};

struct Texture
{
	GLuint id;
	TEXTURE_TYPE type;
	std::string path;

	Texture(){}

	Texture(GLuint aId, TEXTURE_TYPE aType, std::string aPath)
	{
		id = aId;
		type = aType;
		path = aPath;
	}
};

struct Material
{
	float opacity;
    glm::vec3 color_diffuse;
	glm::vec3 color_specular;
	glm::vec3 color_ambient;
	float shininess;
	float roughness;
	float metallic;
	std::vector<Texture> textures; // [0] = diffuse, [1] = specular, [2] = normal, [3] = metallicRough
};

struct Texture createTexture(const std::string & texPath, TEXTURE_TYPE t, bool flip);
struct Texture createTextureFromData(aiTexture* embTex, TEXTURE_TYPE t, bool flip);

enum class LIGHT_TYPE
{
	DIRECTIONAL,
	SPOT
};

class Light
{
	public:

		Light(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec);
		virtual ~Light();
		virtual void draw() = 0;
		glm::vec3 getPosition();
		void setPosition(glm::vec3 pos);
		glm::vec3 getAmbientStrength();
		glm::vec3 getDiffuseStrength();
		glm::vec3 getSpecularStrength();
		void setAmbientStrength(glm::vec3 c);
		void setDiffuseStrength(glm::vec3 c);
		void setSpecularStrength(glm::vec3 c);
		void setModelMatrix(glm::mat4 m);
		void setViewMatrix(glm::mat4 m);
		void setProjMatrix(glm::mat4 m);
		virtual LIGHT_TYPE getType() = 0;

	protected:

		glm::vec3 ambientStrength;
		glm::vec3 diffuseStrength;
		glm::vec3 specularStrength;

		glm::vec3 position;
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		
		GLuint vao;
		GLuint vbo;
		
		struct Texture icon;
};

class DirectionalLight : public Light
{
	public:

		DirectionalLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, glm::vec3 dir);
		virtual void draw() override;
		void draw(float orthoDim);
		glm::vec3 getDirection();
		void setDirection(glm::vec3 dir);
		virtual LIGHT_TYPE getType() override;

	private:

		glm::vec3 direction;
		Shader shaderIcon;
		Shader shaderDirection;
};

class SpotLight : public Light
{
	public:

		SpotLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, glm::vec3 dir, float innerAngle, float outerAngle);
		virtual void draw() override;
		glm::vec3 getDirection();
		void setDirection(glm::vec3 dir);
		float getCutOff();
		void setCutOff(float cutoff);
		float getOuterCutOff();
		void setOuterCutOff(float out);
		virtual LIGHT_TYPE getType() override;

	private:

		glm::vec3 direction;
		float cutOff; // radians
		float outerCutOff; // radians
		Shader shaderIcon;
		Shader shaderCutOff;
};

#endif
