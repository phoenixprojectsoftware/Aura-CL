#include "GameMenuRenderInterface.h"

#if defined(_WIN32) && !defined(FOUNDATION)
#include <winsani_in.h>
#include <Windows.h>
#include <winsani_out.h>
#endif

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <vector>

namespace
{
	struct GameMenuGeometry
	{
		std::vector<Rml::Vertex> vertices;
		std::vector<int> indices;
	};

	static GLuint TextureHandleToGLuint(Rml::TextureHandle handle)
	{
		return static_cast<GLuint>(handle);
	}

	static Rml::TextureHandle GLuintToTextureHandle(GLuint texture)
	{
		return static_cast<Rml::TextureHandle>(texture);
	}

	static GameMenuGeometry* GeometryHandleToPtr(Rml::CompiledGeometryHandle handle)
	{
		return reinterpret_cast<GameMenuGeometry*>(handle);
	}

	static Rml::CompiledGeometryHandle PtrToGeometryHandle(GameMenuGeometry* geometry)
	{
		return reinterpret_cast<Rml::CompiledGeometryHandle>(geometry);
	}
}

CGameMenuRenderInterface::CGameMenuRenderInterface()
	: m_iViewportWidth(640)
	, m_iViewportHeight(480)
	, m_bScissorEnabled(false)
	, m_ScissorRegion(Rml::Rectanglei::MakeInvalid())
{
}

CGameMenuRenderInterface::~CGameMenuRenderInterface()
{
}

void CGameMenuRenderInterface::SetViewportSize(int width, int height)
{
	if (width > 0)
		m_iViewportWidth = width;
	if (height > 0)
		m_iViewportHeight = height;
}

Rml::CompiledGeometryHandle CGameMenuRenderInterface::CompileGeometry(
	Rml::Span<const Rml::Vertex> vertices,
	Rml::Span<const int> indices)
{
	if (vertices.empty() || indices.empty())
		return 0;

	GameMenuGeometry* geometry = new GameMenuGeometry();

	geometry->vertices.assign(vertices.begin(), vertices.end());
	geometry->indices.assign(indices.begin(), indices.end());

	return PtrToGeometryHandle(geometry);
}

void CGameMenuRenderInterface::RenderGeometry(
	Rml::CompiledGeometryHandle geometry_handle,
	Rml::Vector2f translation,
	Rml::TextureHandle texture_handle)
{
	GameMenuGeometry* geometry = GeometryHandleToPtr(geometry_handle);

	if (!geometry)
		return;

	if (geometry->vertices.empty() || geometry->indices.empty())
		return;

	glPushAttrib(
		GL_ENABLE_BIT |
		GL_COLOR_BUFFER_BIT |
		GL_TEXTURE_BIT |
		GL_TRANSFORM_BIT |
		GL_VIEWPORT_BIT |
		GL_SCISSOR_BIT
	);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// RmlUi coordinates are top-left origin.
	// This creates a 2D projection matching screen pixels.
	glOrtho(0.0, m_iViewportWidth, m_iViewportHeight, 0.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);

	// RmlUi 6.x generated texture data and vertex colours are premultiplied alpha.
	// Recommended blend mode for premultiplied alpha:
	// final = src + dst * (1 - srcAlpha)
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	if (m_bScissorEnabled)
	{
		glEnable(GL_SCISSOR_TEST);

		const int x = m_ScissorRegion.Left();
		const int y = m_iViewportHeight - m_ScissorRegion.Bottom();
		const int w = m_ScissorRegion.Width();
		const int h = m_ScissorRegion.Height();

		glScissor(x, y, w, h);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}

	if (texture_handle != 0)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, TextureHandleToGLuint(texture_handle));
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBegin(GL_TRIANGLES);

	for (size_t i = 0; i < geometry->indices.size(); ++i)
	{
		const int vertex_index = geometry->indices[i];

		if (vertex_index < 0 || vertex_index >= static_cast<int>(geometry->vertices.size()))
			continue;

		const Rml::Vertex& vertex = geometry->vertices[vertex_index];

		glColor4ub(
			vertex.colour.red,
			vertex.colour.green,
			vertex.colour.blue,
			vertex.colour.alpha
		);

		glTexCoord2f(
			vertex.tex_coord.x,
			vertex.tex_coord.y
		);

		glVertex2f(
			vertex.position.x + translation.x,
			vertex.position.y + translation.y
		);
	}

	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glPopAttrib();
}

void CGameMenuRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry_handle)
{
	GameMenuGeometry* geometry = GeometryHandleToPtr(geometry_handle);
	delete geometry;
}

Rml::TextureHandle CGameMenuRenderInterface::LoadTexture(
	Rml::Vector2i& texture_dimensions,
	const Rml::String& source)
{
	// For the first pass, do not load external image files.
	// Text should still work through GenerateTexture(), as long as a font is loaded.
	texture_dimensions = Rml::Vector2i(0, 0);

	// gEngfuncs.Con_DPrintf("CGameMenuRenderInterface: LoadTexture not implemented for '%s'\n", source.c_str());

	return 0;
}

Rml::TextureHandle CGameMenuRenderInterface::GenerateTexture(
	Rml::Span<const Rml::byte> source,
	Rml::Vector2i source_dimensions)
{
	if (source.empty())
		return 0;

	if (source_dimensions.x <= 0 || source_dimensions.y <= 0)
		return 0;

	GLuint texture = 0;
	glGenTextures(1, &texture);

	if (texture == 0)
		return 0;

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// RmlUi gives us tightly packed RGBA8 pixel data.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		source_dimensions.x,
		source_dimensions.y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		source.data()
	);

	glBindTexture(GL_TEXTURE_2D, 0);

	return GLuintToTextureHandle(texture);
}

void CGameMenuRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle)
{
	if (texture_handle == 0)
		return;

	GLuint texture = TextureHandleToGLuint(texture_handle);
	glDeleteTextures(1, &texture);
}

void CGameMenuRenderInterface::EnableScissorRegion(bool enable)
{
	m_bScissorEnabled = enable;
}

void CGameMenuRenderInterface::SetScissorRegion(Rml::Rectanglei region)
{
	m_ScissorRegion = region;
}

void CGameMenuRenderInterface::SetTransform(const Rml::Matrix4f* transform)
{
	// Ignored for first pass.
	// Avoid transforms/rotations/scales in RCSS until this is implemented.
}
