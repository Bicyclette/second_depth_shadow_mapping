#include "object.hpp"
#include "stb_image.h"

glm::mat4 assimpMat4_to_glmMat4(aiMatrix4x4 & m)
{
	glm::mat4 matrix;
	matrix[0][0] = m.a1; matrix[1][0] = m.a2; matrix[2][0] = m.a3; matrix[3][0] = m.a4;
	matrix[0][1] = m.b1; matrix[1][1] = m.b2; matrix[2][1] = m.b3; matrix[3][1] = m.b4;
	matrix[0][2] = m.c1; matrix[1][2] = m.c2; matrix[2][2] = m.c3; matrix[3][2] = m.c4;
	matrix[0][3] = m.d1; matrix[1][3] = m.d2; matrix[2][3] = m.d3; matrix[3][3] = m.d4;
	return matrix;
}

glm::mat3 assimpMat3_to_glmMat3(aiMatrix3x3 & m)
{
	glm::mat3 matrix;
	matrix[0][0] = m.a1; matrix[1][0] = m.a2; matrix[2][0] = m.a3; 
	matrix[0][1] = m.b1; matrix[1][1] = m.b2; matrix[2][1] = m.b3; 
	matrix[0][2] = m.c1; matrix[1][2] = m.c2; matrix[2][2] = m.c3; 
	return matrix;
}

Object::Object(const std::string & path, glm::mat4 aModel) :
	model(aModel),
	instancing(false)
{
	load(path);
}

Object::~Object()
{
	if(instancing)
	{
		for(int i{0}; i < meshes.size(); ++i)
		{
			meshes.at(i)->bindVAO();

			glDisableVertexAttribArray(5);
			glDisableVertexAttribArray(6);
			glDisableVertexAttribArray(7);
			glDisableVertexAttribArray(8);

			glBindVertexArray(0);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &instanceVBO);
		instanceModel.clear();
		instancing = false;
	}
}

void Object::draw(Shader& shader, bool shadowPass, DRAWING_MODE mode, bool lance_williams)
{
	shader.use();
	shader.setMatrix("model", model);
	shader.setMatrix("normalMatrix", glm::transpose(glm::inverse(model)));

	int meshCount = meshes.size();

	for(int i{0}; i < meshCount; ++i)
	{
		meshes.at(i)->draw(shader, instancing, instanceModel.size(), shadowPass, mode, lance_williams);
	}
}

std::vector<std::shared_ptr<Mesh>>& Object::getMeshes()
{
    return meshes;
}

void Object::setInstancing(const std::vector<glm::mat4> & models)
{
	instancing = true;
	instanceModel = models;

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), models.data(), GL_STATIC_DRAW);

	for(int i{0}; i < meshes.size(); ++i)
	{
		meshes.at(i)->bindVAO();

		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);

		glBindVertexArray(0);
	}
}

void Object::resetInstancing()
{
	for(int i{0}; i < meshes.size(); ++i)
	{
		meshes.at(i)->bindVAO();

		glDisableVertexAttribArray(5);
		glDisableVertexAttribArray(6);
		glDisableVertexAttribArray(7);
		glDisableVertexAttribArray(8);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glDeleteBuffers(1, &instanceVBO);
	instanceModel.clear();
	instancing = false;
}

void Object::load(const std::string & path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || ! scene->mRootNode)
	{
		std::cerr << importer.GetErrorString() << std::endl;
		return;
	}

	fullPath = path;
	directory = path.substr(0, path.find_last_of('/'));

	name = scene->mRootNode->mName.C_Str();
	int dotIndex = name.find_first_of(".");
	name = name.substr(0, dotIndex);

	exploreNode(scene->mRootNode, scene);
	
	// check if object is solid or not
	solid = meshes.at(0)->isSolid();
	for(int i{1}; i < meshes.size(); ++i)
	{
		if(meshes.at(i)->isSolid() != solid)
		{
			solid = false;
			break;
		}
	}
}

void Object::exploreNode(aiNode* node, const aiScene* scene)
{
	int nb_meshes = node->mNumMeshes;
	int nb_children = node->mNumChildren;

	for(int i{0}; i < nb_meshes; ++i)
	{
		aiMesh* currMesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(getMesh(currMesh, scene));
	}

	for(int i = 0; i < nb_children; i++)
	{
		exploreNode(node->mChildren[i], scene);
	}
}

std::shared_ptr<Mesh> Object::getMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;
	struct Material material;
	std::string meshName(mesh->mName.C_Str());
	
	// vertices
	int nb_vertices = mesh->mNumVertices;
	
	glm::vec3 v_pos;
	glm::vec3 v_norm;
	glm::vec2 v_tex_coords;
	glm::vec3 v_tangent;
	glm::vec3 v_bTangent;

	aiVector3D vertex_pos;
	aiVector3D vertex_norm;
	aiVector3D vertex_tangent;
	aiVector3D vertex_bTangent;
	
	for(int i{0}; i < nb_vertices; ++i)
	{
		vertex_pos = mesh->mVertices[i];
		vertex_norm = mesh->mNormals[i];
		if(mesh->HasNormals())
		{
			vertex_tangent = mesh->mTangents[i];
			vertex_bTangent = mesh->mBitangents[i];
		}
		else
		{
			vertex_tangent = aiVector3D(0.0f, 0.0f, 0.0f);
			vertex_bTangent = aiVector3D(0.0f, 0.0f, 0.0f);
		}

		v_pos = glm::vec3(vertex_pos.x, vertex_pos.y, vertex_pos.z);
		v_norm = glm::vec3(vertex_norm.x, vertex_norm.y, vertex_norm.z);
		v_tangent = glm::vec3(vertex_tangent.x, vertex_tangent.y, vertex_tangent.z);
		v_bTangent = glm::vec3(vertex_bTangent.x, vertex_bTangent.y, vertex_bTangent.z);
		
		if(mesh->mTextureCoords[0])
		{
			v_tex_coords.x = mesh->mTextureCoords[0][i].x;
			v_tex_coords.y = mesh->mTextureCoords[0][i].y;
		}
		else
			v_tex_coords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(Vertex(v_pos, v_norm, v_tex_coords, v_tangent, v_bTangent));
	}

	// indices
	int nb_faces = mesh->mNumFaces;
	int nb_indices_face = 0;

	for(int i{0}; i < nb_faces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		nb_indices_face = face.mNumIndices;

		for(int j{0}; j < nb_indices_face; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// material
	aiMaterial* mesh_material = scene->mMaterials[mesh->mMaterialIndex];
    
	aiColor3D color_diffuse;
	aiColor3D color_specular;
	aiColor3D color_ambient;
	float opacity;
	float shininess;
	float roughness;
	float metallic;
	std::vector<Texture> textures;

	std::vector<Texture> diffuse = loadMaterialTextures(scene, mesh_material, aiTextureType_DIFFUSE, TEXTURE_TYPE::DIFFUSE);
	textures.insert(textures.end(), diffuse.begin(), diffuse.end());
	
	std::vector<Texture> specular = loadMaterialTextures(scene, mesh_material, aiTextureType_SPECULAR, TEXTURE_TYPE::SPECULAR);
	textures.insert(textures.end(), specular.begin(), specular.end());
	
	std::vector<Texture> normal = loadMaterialTextures(scene, mesh_material, aiTextureType_NORMALS, TEXTURE_TYPE::NORMAL);
	if(normal.size() == 0)
		normal = loadMaterialTextures(scene, mesh_material, aiTextureType_HEIGHT, TEXTURE_TYPE::NORMAL);
	textures.insert(textures.end(), normal.begin(), normal.end());

	std::vector<Texture> metallicRoughMaps = loadMaterialTextures(scene, mesh_material, aiTextureType_UNKNOWN, TEXTURE_TYPE::METALLIC_ROUGHNESS);
	textures.insert(textures.end(), metallicRoughMaps.begin(), metallicRoughMaps.end());

	mesh_material->Get(AI_MATKEY_COLOR_DIFFUSE, color_diffuse);
	mesh_material->Get(AI_MATKEY_COLOR_SPECULAR, color_specular);
	mesh_material->Get(AI_MATKEY_COLOR_AMBIENT, color_ambient);
	mesh_material->Get(AI_MATKEY_OPACITY, opacity);
	mesh_material->Get(AI_MATKEY_SHININESS, shininess);
	mesh_material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughness);
	mesh_material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metallic);

	material.textures = textures;
    material.color_diffuse = glm::vec3(color_diffuse.r, color_diffuse.g, color_diffuse.b);
    material.color_specular = glm::vec3(color_specular.r, color_specular.g, color_specular.b);
    material.color_ambient = glm::vec3(color_ambient.r, color_ambient.g, color_ambient.b);
	material.opacity = opacity;
	material.shininess = shininess;
	material.roughness = roughness;
	material.metallic = metallic;

	// check if mesh is solid
	bool solid{true};
	std::string file{fullPath.substr(0, fullPath.find_last_of('.')) + ".xml"};
	std::ifstream metadata{file};
	if(!metadata.fail())
	{
		std::vector<char> buffer((std::istreambuf_iterator<char>(metadata)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');

		rapidxml::xml_document<> doc;
		doc.parse<0>(&buffer[0]);
		rapidxml::xml_node<> * root_node = doc.first_node("Object");

		for(rapidxml::xml_node<> * mesh_node = root_node->first_node("Mesh"); mesh_node; mesh_node = mesh_node->next_sibling())
		{
			rapidxml::xml_attribute<> * attr_name = mesh_node->first_attribute("name");
			if(strcmp(attr_name->value(), meshName.c_str()) == 0)
			{
				rapidxml::xml_attribute<> * attr_solid = attr_name->next_attribute();
				int s = atoi(attr_solid->value());
				solid = (s == 1) ? true : false;
			}
		}
	}

	// pack everything
    return std::make_shared<Mesh>(vertices, indices, material, meshName, solid);
}

std::vector<struct Texture> Object::loadMaterialTextures(
	const aiScene* scene,
	aiMaterial* mat,
	aiTextureType type,
	TEXTURE_TYPE t)
{
	std::vector<Texture> texs;

	for(int i{0}; i < mat->GetTextureCount(type); ++i)
	{
		aiString path;
		mat->GetTexture(type, i, &path);
		int index = std::string(path.C_Str()).find_last_of("/");
		std::string texName = std::string(path.C_Str()).substr(index + 1);
		std::string texPath = std::string(directory + "/" + texName);

		if(texName.at(0) == '*' && index == -1)
		{
			int embedded{static_cast<int>(texName.at(1) - '0')};

			aiTexture* embTex = scene->mTextures[embedded];
			aiTexel* texData = embTex->pcData;

			Texture texture = createTextureFromData(embTex, t, false);
			texs.push_back(texture);
		}
		else
		{
			bool skip{false};
			for(int j{0}; j < texturesLoaded.size(); ++j)
			{
				if(std::strcmp(texturesLoaded.at(j).path.c_str(), texPath.c_str()) == 0)
				{
					texs.push_back(texturesLoaded.at(j));
					skip = true;
					break;
				}
			}

			if(!skip)
			{
				Texture texture = createTexture(texPath, t, false);
				texs.push_back(texture);
				texturesLoaded.push_back(texture);
			}
		}
	}

	return texs;
}

bool Object::isSolid()
{
	return solid;
}
