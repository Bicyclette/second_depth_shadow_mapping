#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <memory>
#include <utility>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "mesh.hpp"
#include "framebuffer.hpp"
#include "shader_light.hpp"

enum class SHADOW_QUALITY
{
	OFF = 0,
	TINY = 256,
	SMALL = 512,
	MED = 1024,
	HIGH = 2048,
	ULTRA = 4096
};

enum class SHADOW_METHOD
{
	LANCE_WILLIAMS,
	SECOND_DEPTH
};

class Graphics
{
	public:

		Graphics(int width, int height);
		void setShadowQuality(SHADOW_QUALITY quality);
		void setShadowMethod(SHADOW_METHOD method);
		SHADOW_QUALITY getShadowQuality();
		SHADOW_METHOD getShadowMethod();
		void setBias(float b);
		float getBias();
		void setLightView(glm::mat4 view);
		glm::mat4 & getLightView();
		glm::mat4 & getOrthoProjection();
		float getOrthoDimension();
		void setOrthoDimension(float dimension);
		glm::mat4 getSpotPerspProjection(float outerCutOff);
		Shader & getBlinnPhongShader();
		Shader & getPBRShader();
		Shader & getPostProcessingShader();
		Shader & getShadowMappingShader();
		std::unique_ptr<Framebuffer> & getMultisampleFBO();
		std::unique_ptr<Framebuffer> & getNormalFBO();
		std::unique_ptr<Framebuffer> & getStdDepthFBO(int index);
		std::unique_ptr<Mesh> & getQuadMesh();
		void resizeScreen(int width, int height);

	private:

		std::unique_ptr<Framebuffer> multisample; // color + depth + stencil
		std::unique_ptr<Framebuffer> normal; // only color, no multisampling
		std::array<std::unique_ptr<Framebuffer>, 10> stdDepth; // for directional and spotlight shadow mapping

		float orthoDimension;
		SHADOW_QUALITY shadowQuality;
		SHADOW_METHOD shadowMethod;
		float bias;
		glm::mat4 orthoProjection; // for directional lights

		Shader blinnPhong;
		Shader pbr;
		Shader postProcessing;
		Shader shadowMapping;

		std::unique_ptr<Mesh> quad;
		Material quadMaterial;
};

#endif
