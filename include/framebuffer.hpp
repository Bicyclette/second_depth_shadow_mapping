#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <GL/glew.h>
#include <vector>
#include <iostream>
#include <memory>
#include <utility>

enum class ATTACHMENT_TYPE
{
	TEXTURE,
	TEXTURE_CUBE_MAP,
	RENDER_BUFFER
};

enum class ATTACHMENT_TARGET
{
	COLOR,
	DEPTH,
	STENCIL,
	DEPTH_STENCIL
};

struct Attachment
{
	GLuint id;
	ATTACHMENT_TYPE type;
	ATTACHMENT_TARGET target;
};

class Framebuffer
{
	public:

		Framebuffer(bool color = true, bool ms = false, bool hdr = false);
		~Framebuffer();

		void addAttachment(ATTACHMENT_TYPE type, ATTACHMENT_TARGET target, int width, int height, int insertPos = -1);
		void updateAttachment(ATTACHMENT_TYPE type, ATTACHMENT_TARGET target, int width, int height);
		
		void createDirectionalDepthFBO(int width, int height);
		void createOmnidirectionalDepthFBO(int width, int height);
		void createMultisampledFBO(int width, int height);
		void createResolveFBO(int width, int height);
		
		void updateDirectionalDepthFBO(int width, int height);
		void updateOmnidirectionalDepthFBO(int width, int height);
		void updateMultisampledFBO(int width, int height);
		void updateResolveFBO(int width, int height);
		
		std::vector<Attachment> & getAttachments();
		void bind();
		void unbind();
		void blitFramebuffer(Framebuffer & writeFBO, int width, int height);
		void blitFramebuffer(std::unique_ptr<Framebuffer> & writeFBO, int width, int height);
		GLuint getId();

	private:

		GLuint fbo;
		bool renderColor;
		bool multiSample;
		bool HDR;
		std::vector<Attachment> attachment;

		void addColorTextureAttachment(int width, int height, int insertPos);
		void addDepthTextureAttachment(int width, int height, int insertPos);
		
		void addColorTextureCubemapAttachment(int width, int height, int insertPos);
		void addDepthTextureCubemapAttachment(int width, int height, int insertPos);
		
		void addColorRenderbufferAttachment(int width, int height, int insertPos);
		void addDepthRenderbufferAttachment(int width, int height, int insertPos);
		void addDepthStencilRenderbufferAttachment(int width, int height, int insertPos);
		
		void updateColorTextureAttachment(int width, int height, int insertPos);
		void updateDepthTextureAttachment(int width, int height, int insertPos);
		
		void updateColorTextureCubemapAttachment(int width, int height, int insertPos);
		void updateDepthTextureCubemapAttachment(int width, int height, int insertPos);
		
		void updateColorRenderbufferAttachment(int width, int height, int insertPos);
		void updateDepthRenderbufferAttachment(int width, int height, int insertPos);
		void updateDepthStencilRenderbufferAttachment(int width, int height, int insertPos);
};

#endif
