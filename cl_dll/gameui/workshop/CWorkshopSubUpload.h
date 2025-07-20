// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef CWORKSHOP_SUB_UPLOAD_H
#define CWORKSHOP_SUB_UPLOAD_H
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButtonList.h>
#include <vgui_controls/ImagePanel.h>
#include "WorkshopItemList.h"

class CWorkshopSubUpload : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CWorkshopSubUpload, vgui2::PropertyPage);

public:
	CWorkshopSubUpload(vgui2::Panel* parent);
	~CWorkshopSubUpload();
	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme);
	virtual void PerformLayout();
	virtual void OnCommand(const char* pcCommand);
	virtual void OnTick();

	void UpdateContentPath(DialogData* pData);
	void UpdatePreviewImage(DialogData* pData);
	void SetUpdating(PublishedFileId_t nItem);
	void SetUploadData(const char* Title, const char* Desc, const char* Tags, ERemoteStoragePublishedFileVisibility Visbility);

protected:
	void ResetPage();
	void BeginUpload();
	void PrepareUGCHandle();
	void OnItemsUpdated();

	bool HasAddonInfo(DialogData* pData);
	void ThrowError(const char* szMsg);
	bool ValidateTheEntries();

	bool bUploading;
	PublishedFileId_t nWorkshopID;
	std::string last_folder[2];
	std::string preview_image;

	// Functors
	MESSAGE_FUNC_PARAMS(OnCheckButtonChecked, "CheckButtonChecked", pParams);
	MESSAGE_FUNC(OnTextChanged, "TextChanged");
	MESSAGE_FUNC(OnMenuItemSelected, "MenuItemSelected");

	void OnItemCreated(CreateItemResult_t* pCallback, bool bIOFailure);
	CCallResult<CWorkshopSubUpload, CreateItemResult_t> m_SteamCallResultOnItemCreated;

	void OnCallResultOnSubmitItemUpdateResult(SubmitItemUpdateResult_t* pCallback, bool bIOFailure);
	CCallResult <CWorkshopSubUpload, SubmitItemUpdateResult_t> m_SteamCallResultOnSubmitItemUpdateResult;

	UGCUpdateHandle_t handle;

private:
	vgui2::TextEntry* pDescBox;
	vgui2::TextEntry* pTitleBox;
	vgui2::TextEntry* pContentText;
	vgui2::Button* pAddonUpload;
	vgui2::Button* pAddonReset;
	vgui2::Label* pChangeLogLabel;
	vgui2::TextEntry* pChangeLogText;
	vgui2::ImagePanel* pAddonImage;
	vgui2::ComboBox* pVisibility;
	vgui2::CheckButtonList* pTags[3];
	vgui2::ProgressBar* pProgress;
	vgui2::Label* pProgressLabel;
};

#endif
