#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <stdlib.h>
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include <omp.h>
#include <memory>
#include <utility>
#include <fstream>
#include "rapidxml.hpp"
#include "mesh.hpp"
#include "shader_light.hpp"

glm::mat4 assimpMat4_to_glmMat4(aiMatrix4x4 & m);
glm::mat3 assimpMat3_to_glmMat3(aiMatrix3x3 & m);

class Object
{
	public:

		Object(const std::string & path, glm::mat4 model = glm::mat4(1.0f));
		~Object();
		void draw(Shader& shader, bool shadowPass = false, DRAWING_MODE mode = DRAWING_MODE::SOLID, bool lance_williams = false);
        std::vector<std::shared_ptr<Mesh>>& getMeshes();
		void setInstancing(const std::vector<glm::mat4> & models);
		void resetInstancing();
		std::string getName();
		bool isSolid();
		
	private:

		void load(const std::string & path);
		void exploreNode(aiNode* node, const aiScene* scene);
		std::shared_ptr<Mesh> getMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<struct Texture> loadMaterialTextures(
						const aiScene* scene,
						aiMaterial* mat,
						aiTextureType type,
						TEXTURE_TYPE t);
		
		std::string name;

		std::string directory;
		std::string fullPath;
		std::vector<Texture> texturesLoaded;
		std::vector<std::shared_ptr<Mesh>> meshes;
		bool solid;

		GLuint instanceVBO;
		std::vector<glm::mat4> instanceModel;
		glm::mat4 model;
		bool instancing;
};

#endif
