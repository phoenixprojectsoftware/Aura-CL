#pragma once

#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Types.h>

class CGameMenuRenderInterface final : public Rml::RenderInterface
{
public:
	CGameMenuRenderInterface();
	~CGameMenuRenderInterface() override;

	void SetViewportSize(int width, int height);

	Rml::CompiledGeometryHandle CompileGeometry(
		Rml::Span<const Rml::Vertex> vertices,
		Rml::Span<const int> indices) override;

	void RenderGeometry(
		Rml::CompiledGeometryHandle geometry,
		Rml::Vector2f translation,
		Rml::TextureHandle texture) override;

	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

	Rml::TextureHandle LoadTexture(
		Rml::Vector2i& texture_dimensions,
		const Rml::String& source) override;

	Rml::TextureHandle GenerateTexture(
		Rml::Span<const Rml::byte> source,
		Rml::Vector2i source_dimensions) override;

	void ReleaseTexture(Rml::TextureHandle texture) override;

	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(Rml::Rectanglei region) override;

	void SetTransform(const Rml::Matrix4f* transform) override;

private:
	int m_iViewportWidth;
	int m_iViewportHeight;

	bool m_bScissorEnabled;
	Rml::Rectanglei m_ScissorRegion;
};
