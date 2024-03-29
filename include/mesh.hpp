#ifndef _MESH_HPP_
#define _MESH_HPP_

#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "shader_light.hpp"

enum class DRAWING_MODE
{
	SOLID,
	WIREFRAME
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tangent;
	glm::vec3 biTangent;

	Vertex(
		glm::vec3 pos,
		glm::vec3 norm,
		glm::vec2 tex,
		glm::vec3 tang = glm::vec3(0.0f),
		glm::vec3 bTang = glm::vec3(0.0f))
	{
		position = pos;
		normal = norm;
		texCoords = tex;
		tangent = tang;
		biTangent = bTang;
	}
};

class Mesh
{
	public:

		Mesh(std::vector<Vertex> aVertices, std::vector<int> aIndices, Material m, std::string aName, bool aSolid = true);
        ~Mesh();
		std::string getName();
		std::vector<Vertex> const& getVertices() const;
		std::vector<int> const& getIndices() const;
		Material & getMaterial();
		bool isSolid();
		void bindVAO() const;
		void draw(Shader& s, bool instancing = false, int amount = 1, bool shadowPass = false, DRAWING_MODE mode = DRAWING_MODE::SOLID, bool lance_williams = false);

	private:

		GLuint vao;
		GLuint vbo;
		GLuint ebo;

		std::string name;
		std::vector<Vertex> vertices;
		std::vector<int> indices;
		Material material;
		bool solid;
};

#endif
