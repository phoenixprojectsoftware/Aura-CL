#ifndef VGUI_STEAM_AVATAR_IMAGE_H
#define VGUI_STEAM_AVATAR_IMAGE_H

#include <Color.h>
#include <vgui/IImage.h>

#include <steamworks/steam_api.h>

// Small VGUI image which retrieves and draws a player's Steam avatar.

class CSteamAvatarImage : public vgui2::IImage
{
public:
	CSteamAvatarImage();
	~CSteamAvatarImage() override;

	void SetSteamID(uint64 steamID);
	void Clear();

	bool IsValid() const
	{
		return m_bValid;
	}

	uint64 GetSteamID() const
	{
		return m_ulSteamID;
	}

	// vgui2::IImage implementation.
	void Paint() override;
	void SetPos(int x, int y) override;
	void GetContentSize(int& wide, int& tall) override;
	void GetSize(int& wide, int& tall) override;
	void SetSize(int wide, int tall) override;
	void SetColor(Color color) override;

private:
	void TryLoadAvatar();

	uint64 m_ulSteamID;

	int m_iTextureID;
	int m_iX;
	int m_iY;
	int m_iWide;
	int m_iTall;

	Color m_Color;

	bool m_bValid;
	bool m_bLoadPending;

	float m_flNextLoadAttempt;
};

#endif // VGUI_STEAM_AVATAR_IMAGE_H
