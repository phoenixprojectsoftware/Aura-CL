#ifndef WEAPONS_NO_CLASSES
class CPenguin : public CBasePlayerWeapon
{
public:
	using BaseClass = CBasePlayerWeapon;

#ifndef CLIENT_DLL
	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
#endif

	void Precache() override;

	void Spawn() override;

	BOOL Deploy() override;

	void Holster(int skiplocal = 0) override;

	void WeaponIdle() override;

	void PrimaryAttack() override;

	void SecondaryAttack() override;

	int iItemSlot() override;

	int GetItemInfo(ItemInfo* p) override;

	//TODO: requires oldweapons cvar
	BOOL UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	int m_fJustThrown;
	unsigned short m_usPenguinFire;
};
#endif