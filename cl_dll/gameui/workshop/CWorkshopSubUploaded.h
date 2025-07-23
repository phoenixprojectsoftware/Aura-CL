// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef CWORKSHOP_SUB_UPLOADED_H
#define CWORKSHOP_SUB_UPLOADED_H
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/ComboBox.h>
#include "WorkshopItemList.h"

class CWorkshopSubUpload;

class CWorkshopSubUploaded : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CWorkshopSubUploaded, vgui2::PropertyPage);

public:
	CWorkshopSubUploaded(vgui2::Panel* parent);
	~CWorkshopSubUploaded();

	void SetPropertyDialog(vgui2::PropertyDialog* pDialog) { pProperty = pDialog; }
	void SetUploadPage(CWorkshopSubUpload* pDialog) { pUploadPage = pDialog; }

	virtual void AddItem(vgui2::WorkshopItem item);

	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);
	virtual void PerformLayout();

protected:
	void OnSendQueryUGCRequest(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure);
	CCallResult<CWorkshopSubUploaded, SteamUGCQueryCompleted_t> m_SteamCallResultOnSendQueryUGCRequest;
	UGCQueryHandle_t handle;

	struct WorkshopItem
	{
		char Title[k_cchPublishedDocumentTitleMax];
		char Desc[k_cchPublishedDocumentDescriptionMax];
		char Tags[k_cchTagListMax];
		ERemoteStoragePublishedFileVisibility Visibility;
		PublishedFileId_t PublishedFileID;
	};
	std::vector<WorkshopItem> m_Items;
	WorkshopItem GetWorkshopItem(PublishedFileId_t nWorkshopID);

	MESSAGE_FUNC_UINT64(OnWorkshopEdit, "WorkshopEdit", workshopID);

private:
	vgui2::WorkshopItemList* pList;
	vgui2::PropertyDialog* pProperty;
	CWorkshopSubUpload* pUploadPage;
};

#endif
