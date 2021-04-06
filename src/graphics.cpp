#include "graphics.hpp"

Graphics::Graphics(int width, int height) :
	multisample{std::make_unique<Framebuffer>(true, true, true)},
	normal{std::make_unique<Framebuffer>(true, false, true)},
	stdDepth{
		std::make_unique<Framebuffer>(false),
		std::make_unique<Framebuffer>(false),
		std::make_unique<Framebuffer>(false),
		std::make_unique<Framebuffer>(false),
		std::make_unique<Framebuffer>(false),
		std::make_unique<Framebuffer>(false),
		std::make_unique<Framebuffer>(false),
		std::make_unique<Framebuffer>(false),
		std::make_unique<Framebuffer>(false),
		std::make_unique<Framebuffer>(false)
		},
	shadowQuality(SHADOW_QUALITY::ULTRA),
	shadowMethod(SHADOW_METHOD::SECOND_DEPTH),
	bias(0.0005f),
	orthoDimension(10.0f),
	orthoProjection(glm::ortho(-orthoDimension, orthoDimension, -orthoDimension, orthoDimension, 0.1f, 100.0f)),
	blinnPhong("../shaders/blinn_phong/vertex.glsl", "../shaders/blinn_phong/fragment.glsl", SHADER_TYPE::BLINN_PHONG),
	pbr("../shaders/PBR/vertex.glsl", "../shaders/PBR/fragment.glsl", SHADER_TYPE::PBR),
	postProcessing("../shaders/post_processing/vertex.glsl", "../shaders/post_processing/fragment.glsl", SHADER_TYPE::POST_PROCESSING),
	shadowMapping("../shaders/shadowMapping/vertex.glsl", "../shaders/shadowMapping/fragment.glsl", SHADER_TYPE::SHADOWS)
{
	multisample->addAttachment(ATTACHMENT_TYPE::TEXTURE, ATTACHMENT_TARGET::COLOR, width, height);
	multisample->addAttachment(ATTACHMENT_TYPE::RENDER_BUFFER, ATTACHMENT_TARGET::DEPTH_STENCIL, width, height);

	normal->addAttachment(ATTACHMENT_TYPE::TEXTURE, ATTACHMENT_TARGET::COLOR, width, height);

	if(shadowQuality != SHADOW_QUALITY::OFF)
	{
		for(int i{0}; i < 10; ++i)
		{
			stdDepth.at(i)->addAttachment(ATTACHMENT_TYPE::TEXTURE, ATTACHMENT_TARGET::DEPTH, static_cast<int>(shadowQuality), static_cast<int>(shadowQuality));
		}
	}

	quadMaterial.textures.push_back(Texture(normal->getAttachments().at(0).id, TEXTURE_TYPE::DIFFUSE, "."));

	glm::vec3 normal(0.0f, 0.0f, 1.0f);
	std::vector<Vertex> vertices{{
		Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), normal, glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(1.0f, -1.0f, 0.0f), normal, glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(1.0f, 1.0f, 0.0f), normal, glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-1.0f, 1.0f, 0.0f), normal, glm::vec2(0.0f, 1.0f))
	}};

	std::vector<int> indices{{
		0, 1, 2,
		0, 2, 3
	}};

	quad = std::make_unique<Mesh>(vertices, indices, quadMaterial, "post processing quad");
}

void Graphics::setShadowQuality(SHADOW_QUALITY quality)
{
	shadowQuality = quality;
	if(shadowQuality != SHADOW_QUALITY::OFF)
	{
		for(int i{0}; i < 10; ++i)
			stdDepth.at(i)->updateAttachment(ATTACHMENT_TYPE::TEXTURE, ATTACHMENT_TARGET::DEPTH, static_cast<int>(shadowQuality), static_cast<int>(shadowQuality));
	}
}

void Graphics::setShadowMethod(SHADOW_METHOD method)
{
	shadowMethod = method;
}

SHADOW_QUALITY Graphics::getShadowQuality()
{
	return shadowQuality;
}

SHADOW_METHOD Graphics::getShadowMethod()
{
	return shadowMethod;
}

void Graphics::setBias(float b)
{
	bias = b;
}

float Graphics::getBias()
{
	return bias;
}

glm::mat4 & Graphics::getOrthoProjection()
{
	return orthoProjection;
}

float Graphics::getOrthoDimension()
{
	return orthoDimension;
}

void Graphics::setOrthoDimension(float dimension)
{
	orthoDimension = dimension;
	orthoProjection = glm::ortho(-orthoDimension, orthoDimension, -orthoDimension, orthoDimension, 0.1f, 100.0f);
}

glm::mat4 Graphics::getSpotPerspProjection(float outerCutOff)
{
	float aspectRatio = static_cast<float>(shadowQuality) / static_cast<float>(shadowQuality);
	return glm::perspective(glm::radians(outerCutOff * 2.0f), aspectRatio, 0.1f, 100.0f);
}

Shader & Graphics::getBlinnPhongShader()
{
	return blinnPhong;
}

Shader & Graphics::getPBRShader()
{
	return pbr;
}

Shader & Graphics::getPostProcessingShader()
{
	return postProcessing;
}

Shader & Graphics::getShadowMappingShader()
{
	return shadowMapping;
}

std::unique_ptr<Framebuffer> & Graphics::getMultisampleFBO()
{
	return multisample;
}

std::unique_ptr<Framebuffer> & Graphics::getNormalFBO()
{
	return normal;
}

std::unique_ptr<Framebuffer> & Graphics::getStdDepthFBO(int index)
{
	return stdDepth.at(index);
}

std::unique_ptr<Mesh> & Graphics::getQuadMesh()
{
	return quad;
}

void Graphics::resizeScreen(int width, int height)
{
	multisample->updateAttachment(ATTACHMENT_TYPE::TEXTURE, ATTACHMENT_TARGET::COLOR, width, height);
	multisample->updateAttachment(ATTACHMENT_TYPE::RENDER_BUFFER, ATTACHMENT_TARGET::DEPTH_STENCIL, width, height);

	normal->updateAttachment(ATTACHMENT_TYPE::TEXTURE, ATTACHMENT_TARGET::COLOR, width, height);
}
