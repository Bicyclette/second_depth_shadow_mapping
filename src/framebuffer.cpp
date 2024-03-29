#include "framebuffer.hpp"

Framebuffer::Framebuffer(bool color, bool ms, bool hdr) :
	renderColor(color),
	multiSample(ms),
	HDR(hdr)
{
	glGenFramebuffers(1, &fbo);
}

Framebuffer::~Framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	for(int i{0}; i < attachment.size(); ++i)
	{
		if(attachment.at(i).type == ATTACHMENT_TYPE::TEXTURE)
		{
			switch(attachment.at(i).target)
			{
				case ATTACHMENT_TARGET::COLOR:
					if(multiSample)
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
					else
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
					break;
				case ATTACHMENT_TARGET::DEPTH:
					if(multiSample)
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
					else
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
					break;
				case ATTACHMENT_TARGET::STENCIL:
					if(multiSample)
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
					else
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
					break;
				default:
					break;
			}
			glDeleteTextures(1, &attachment.at(i).id);
		}
		else if(attachment.at(i).type == ATTACHMENT_TYPE::TEXTURE_CUBE_MAP)
		{
			switch(attachment.at(i).target)
			{
				case ATTACHMENT_TARGET::COLOR:
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
					break;
				case ATTACHMENT_TARGET::DEPTH:
					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
					break;
				default:
					break;
			}
			glDeleteTextures(1, &attachment.at(i).id);
		}
		else if(attachment.at(i).type == ATTACHMENT_TYPE::RENDER_BUFFER)
		{
			switch(attachment.at(i).target)
			{
				case ATTACHMENT_TARGET::COLOR:
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
					break;
				case ATTACHMENT_TARGET::DEPTH:
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
					break;
				case ATTACHMENT_TARGET::STENCIL:
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
					break;
				case ATTACHMENT_TARGET::DEPTH_STENCIL:
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
					break;
				default:
					break;
			}
			glDeleteRenderbuffers(1, &attachment.at(i).id);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &fbo);
}

void Framebuffer::addAttachment(ATTACHMENT_TYPE type, ATTACHMENT_TARGET target, int width, int height, int insertPos)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	if(type == ATTACHMENT_TYPE::TEXTURE)
	{
		switch(target)
		{
			case ATTACHMENT_TARGET::COLOR:
				addColorTextureAttachment(width, height, insertPos);
				break;
			case ATTACHMENT_TARGET::DEPTH:
				addDepthTextureAttachment(width, height, insertPos);
				break;
			default:
				break;
		}
	}
	else if(type == ATTACHMENT_TYPE::TEXTURE_CUBE_MAP)
	{
		switch(target)
		{
			case ATTACHMENT_TARGET::COLOR:
				addColorTextureCubemapAttachment(width, height, insertPos);
				break;
			case ATTACHMENT_TARGET::DEPTH:
				addDepthTextureCubemapAttachment(width, height, insertPos);
				break;
			default:
				break;
		}
	}
	else if(type == ATTACHMENT_TYPE::RENDER_BUFFER)
	{
		switch(target)
		{
			case ATTACHMENT_TARGET::COLOR:
				addColorRenderbufferAttachment(width, height, insertPos);
				break;
			case ATTACHMENT_TARGET::DEPTH:
				addDepthRenderbufferAttachment(width, height, insertPos);
				break;
			case ATTACHMENT_TARGET::DEPTH_STENCIL:
				addDepthStencilRenderbufferAttachment(width, height, insertPos);
				break;
			default:
				break;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::addColorTextureAttachment(int width, int height, int insertPos)
{
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	
	struct Attachment buffer;
	buffer.type = ATTACHMENT_TYPE::TEXTURE;
	buffer.target = ATTACHMENT_TARGET::COLOR;
	GLint internalFormat = (HDR) ? GL_RGBA16F : GL_RGBA;
	GLenum type = (HDR) ? GL_FLOAT : GL_UNSIGNED_BYTE;

	if(multiSample)
	{
		glGenTextures(1, &buffer.id);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, buffer.id);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, internalFormat, width, height, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, buffer.id, 0);
	}
	else
	{
		glGenTextures(1, &buffer.id);
		glBindTexture(GL_TEXTURE_2D, buffer.id);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, type, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.id, 0);
	}

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
	else
	{
		if(insertPos == -1)
			attachment.push_back(buffer);
		else
			attachment.insert(attachment.begin() + insertPos, buffer);
	}
}

void Framebuffer::addDepthTextureAttachment(int width, int height, int insertPos)
{
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	
	struct Attachment buffer;
	buffer.type = ATTACHMENT_TYPE::TEXTURE;
	buffer.target = ATTACHMENT_TARGET::DEPTH;

	glGenTextures(1, &buffer.id);
	glBindTexture(GL_TEXTURE_2D, buffer.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.id, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
	else
	{
		if(insertPos == -1)
			attachment.push_back(buffer);
		else
			attachment.insert(attachment.begin() + insertPos, buffer);
	}
}

void Framebuffer::addColorTextureCubemapAttachment(int width, int height, int insertPos)
{
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};

	struct Attachment buffer;
	buffer.type = ATTACHMENT_TYPE::TEXTURE_CUBE_MAP;
	buffer.target = ATTACHMENT_TARGET::COLOR;
	GLint internalFormat = (HDR) ? GL_RGBA16F : GL_RGBA;
	GLenum type = (HDR) ? GL_FLOAT : GL_UNSIGNED_BYTE;

	glGenTextures(1, &buffer.id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, buffer.id);
	for(int i{0}; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, GL_RGBA, type, nullptr);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffer.id, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
	else
	{
		if(insertPos == -1)
			attachment.push_back(buffer);
		else
			attachment.insert(attachment.begin() + insertPos, buffer);
	}
}

void Framebuffer::addDepthTextureCubemapAttachment(int width, int height, int insertPos)
{
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};

	struct Attachment buffer;
	buffer.type = ATTACHMENT_TYPE::TEXTURE_CUBE_MAP;
	buffer.target = ATTACHMENT_TARGET::DEPTH;

	glGenTextures(1, &buffer.id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, buffer.id);
	for(int i{0}; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, buffer.id, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
	else
	{
		if(insertPos == -1)
			attachment.push_back(buffer);
		else
			attachment.insert(attachment.begin() + insertPos, buffer);
	}
}

void Framebuffer::addColorRenderbufferAttachment(int width, int height, int insertPos)
{
	struct Attachment buffer;
	buffer.type = ATTACHMENT_TYPE::RENDER_BUFFER;
	buffer.target = ATTACHMENT_TARGET::COLOR;
	GLint internalFormat = (HDR) ? GL_RGBA16F : GL_RGBA8;
	
	glGenRenderbuffers(1, &buffer.id);
	glBindRenderbuffer(GL_RENDERBUFFER, buffer.id);
	if(multiSample)
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, internalFormat, width, height);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer.id);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
	else
	{
		if(insertPos == -1)
			attachment.push_back(buffer);
		else
			attachment.insert(attachment.begin() + insertPos, buffer);
	}
}

void Framebuffer::addDepthRenderbufferAttachment(int width, int height, int insertPos)
{
	struct Attachment buffer;
	buffer.type = ATTACHMENT_TYPE::RENDER_BUFFER;
	buffer.target = ATTACHMENT_TARGET::DEPTH;

	glGenRenderbuffers(1, &buffer.id);
	glBindRenderbuffer(GL_RENDERBUFFER, buffer.id);
	if(multiSample)
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer.id);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
	else
	{
		if(insertPos == -1)
			attachment.push_back(buffer);
		else
			attachment.insert(attachment.begin() + insertPos, buffer);
	}
}

void Framebuffer::addDepthStencilRenderbufferAttachment(int width, int height, int insertPos)
{
	struct Attachment buffer;
	buffer.type = ATTACHMENT_TYPE::RENDER_BUFFER;
	buffer.target = ATTACHMENT_TARGET::DEPTH_STENCIL;

	glGenRenderbuffers(1, &buffer.id);
	glBindRenderbuffer(GL_RENDERBUFFER, buffer.id);
	if(multiSample)
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer.id);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
	else
	{
		if(insertPos == -1)
			attachment.push_back(buffer);
		else
			attachment.insert(attachment.begin() + insertPos, buffer);
	}
}

void Framebuffer::updateColorTextureAttachment(int width, int height, int insertPos)
{
	if(multiSample)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
	else
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glDeleteTextures(1, &attachment.at(insertPos).id);

	attachment.erase(attachment.begin() + insertPos);

	if(insertPos == attachment.size())
		addColorTextureAttachment(width, height, -1);
	else
		addColorTextureAttachment(width, height, insertPos);
}

void Framebuffer::updateDepthTextureAttachment(int width, int height, int insertPos)
{
	if(multiSample)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
	else
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	glDeleteTextures(1, &attachment.at(insertPos).id);

	attachment.erase(attachment.begin() + insertPos);

	if(insertPos == attachment.size())
		addDepthTextureAttachment(width, height, -1);
	else
		addDepthTextureAttachment(width, height, insertPos);
}

void Framebuffer::updateColorTextureCubemapAttachment(int width, int height, int insertPos)
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
	glDeleteTextures(1, &attachment.at(insertPos).id);

	attachment.erase(attachment.begin() + insertPos);

	if(insertPos == attachment.size())
		addColorTextureCubemapAttachment(width, height, -1);
	else
		addColorTextureCubemapAttachment(width, height, insertPos);
}

void Framebuffer::updateDepthTextureCubemapAttachment(int width, int height, int insertPos)
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
	glDeleteTextures(1, &attachment.at(insertPos).id);

	attachment.erase(attachment.begin() + insertPos);

	if(insertPos == attachment.size())
		addDepthTextureCubemapAttachment(width, height, -1);
	else
		addDepthTextureCubemapAttachment(width, height, insertPos);
}

void Framebuffer::updateColorRenderbufferAttachment(int width, int height, int insertPos)
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
	glDeleteRenderbuffers(1, &attachment.at(insertPos).id);

	attachment.erase(attachment.begin() + insertPos);

	if(insertPos == attachment.size())
		addColorRenderbufferAttachment(width, height, -1);
	else
		addColorRenderbufferAttachment(width, height, insertPos);
}

void Framebuffer::updateDepthRenderbufferAttachment(int width, int height, int insertPos)
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
	glDeleteRenderbuffers(1, &attachment.at(insertPos).id);

	attachment.erase(attachment.begin() + insertPos);

	if(insertPos == attachment.size())
		addDepthRenderbufferAttachment(width, height, -1);
	else
		addDepthRenderbufferAttachment(width, height, insertPos);
}

void Framebuffer::updateDepthStencilRenderbufferAttachment(int width, int height, int insertPos)
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
	glDeleteRenderbuffers(1, &attachment.at(insertPos).id);

	attachment.erase(attachment.begin() + insertPos);

	if(insertPos == attachment.size())
		addDepthStencilRenderbufferAttachment(width, height, -1);
	else
		addDepthStencilRenderbufferAttachment(width, height, insertPos);
}

void Framebuffer::updateAttachment(ATTACHMENT_TYPE type, ATTACHMENT_TARGET target, int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	for(int i{0}; i < attachment.size(); ++i)
	{
		if(attachment.at(i).type == type && attachment.at(i).target == target)
		{
			if(type == ATTACHMENT_TYPE::TEXTURE)
			{
				switch(target)
				{
					case ATTACHMENT_TARGET::COLOR:
						updateColorTextureAttachment(width, height, i);
						break;
					case ATTACHMENT_TARGET::DEPTH:
						updateDepthTextureAttachment(width, height, i);
						break;
					default:
						break;
				}
			}
			else if(type == ATTACHMENT_TYPE::TEXTURE_CUBE_MAP)
			{
				switch(target)
				{
					case ATTACHMENT_TARGET::COLOR:
						updateColorTextureCubemapAttachment(width, height, i);
						break;
					case ATTACHMENT_TARGET::DEPTH:
						updateDepthTextureCubemapAttachment(width, height, i);
						break;
					default:
						break;
				}
			}
			else if(type == ATTACHMENT_TYPE::RENDER_BUFFER)
			{
				switch(target)
				{
					case ATTACHMENT_TARGET::COLOR:
						updateColorRenderbufferAttachment(width, height, i);
						break;
					case ATTACHMENT_TARGET::DEPTH:
						updateDepthRenderbufferAttachment(width, height, i);
						break;
					case ATTACHMENT_TARGET::DEPTH_STENCIL:
						updateDepthStencilRenderbufferAttachment(width, height, i);
						break;
					default:
						break;
				}
			}
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::createDirectionalDepthFBO(int width, int height)
{
	multiSample = false;
	renderColor = false;
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	
	struct Attachment buffer;
	buffer.type = ATTACHMENT_TYPE::TEXTURE;
	buffer.target = ATTACHMENT_TARGET::DEPTH;
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	glGenTextures(1, &buffer.id);
	glBindTexture(GL_TEXTURE_2D, buffer.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.id, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
	else
		attachment.push_back(buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Framebuffer::createOmnidirectionalDepthFBO(int width, int height)
{
	multiSample = false;
	renderColor = false;
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};

	struct Attachment buffer;
	buffer.type = ATTACHMENT_TYPE::TEXTURE_CUBE_MAP;
	buffer.target = ATTACHMENT_TARGET::DEPTH;

	glGenTextures(1, &buffer.id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, buffer.id);
	for(int i{0}; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, buffer.id, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
	else
		attachment.push_back(buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::createMultisampledFBO(int width, int height)
{
	multiSample = true;
	renderColor = true;
	struct Attachment bufferColor;
	bufferColor.type = ATTACHMENT_TYPE::TEXTURE;
	bufferColor.target = ATTACHMENT_TARGET::COLOR;
	
	struct Attachment bufferDS;
	bufferDS.type = ATTACHMENT_TYPE::RENDER_BUFFER;
	bufferDS.target = ATTACHMENT_TARGET::DEPTH_STENCIL;
	
	glGenTextures(1, &bufferColor.id);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, bufferColor.id);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, width, height, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glGenRenderbuffers(1, &bufferDS.id);
	glBindRenderbuffer(GL_RENDERBUFFER, bufferDS.id);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, bufferColor.id, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, bufferDS.id);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
	else
	{
		attachment.push_back(bufferColor);
		attachment.push_back(bufferDS);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::createResolveFBO(int width, int height)
{
	multiSample = false;
	renderColor = true;
	struct Attachment buffer;
	buffer.type = ATTACHMENT_TYPE::TEXTURE;
	buffer.target = ATTACHMENT_TARGET::COLOR;

	glGenTextures(1, &buffer.id);
	glBindTexture(GL_TEXTURE_2D, buffer.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.id, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
	else
		attachment.push_back(buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::updateDirectionalDepthFBO(int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	glDeleteTextures(1, &attachment.at(0).id);
	attachment.clear();
	createDirectionalDepthFBO(width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::updateOmnidirectionalDepthFBO(int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
	glDeleteTextures(1, &attachment.at(0).id);
	attachment.clear();
	createOmnidirectionalDepthFBO(width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::updateMultisampledFBO(int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
	glDeleteTextures(1, &attachment.at(0).id);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
	glDeleteRenderbuffers(1, &attachment.at(1).id);
	attachment.clear();
	createMultisampledFBO(width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::updateResolveFBO(int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glDeleteTextures(1, &attachment.at(0).id);
	attachment.clear();
	createResolveFBO(width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::vector<Attachment> & Framebuffer::getAttachments()
{
	return attachment;
}

void Framebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::blitFramebuffer(Framebuffer & writeFBO, int width, int height)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, writeFBO.getId());
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void Framebuffer::blitFramebuffer(std::unique_ptr<Framebuffer> & writeFBO, int width, int height)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, writeFBO->getId());
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

GLuint Framebuffer::getId()
{
	return fbo;
}
