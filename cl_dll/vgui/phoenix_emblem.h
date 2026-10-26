#pragma once

#include <Color.h>
#include <vgui/IImage.h>
#include <vgui/VGUI2.h>

class CPhoenixEmblemImage final : public vgui2::IImage
{
public:
	CPhoenixEmblemImage();

	void Paint() override;

	void SetPos(int x, int y) override;
	void GetContentSize(int& wide, int& tall) override;
	void GetSize(int& wide, int& tall) override;
	void SetSize(int wide, int tall) override;
	void SetColor(Color color) override;

private:
	void EnsureTextureLoaded();

	int m_iX;
	int m_iY;
	int m_iWide;
	int m_iTall;
	int m_iTextureID;

	Color m_Color;
};
