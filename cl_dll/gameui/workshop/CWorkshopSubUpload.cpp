// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <string>
#include <vector>
#include <sstream>
#include <FileSystem.h>
#include <tier1/strtools.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/URLLabel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/CheckButtonList.h>
#include <vgui_controls/ProgressBar.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/WizardPanel.h>
#include <KeyValues.h>
#include <current_version.h>
// #include <bhl_urls.h>
#include "../../client_vgui.h"
#include "../gameui_viewport.h"
#include "CWorkshopSubUpload.h"
#include "../../console.h"

// PNG > TGA
#include "../lodepng/lodepng.h"

static CWorkshopSubUpload* pUploader = nullptr;

static const char* szTags_Weapons[] = {
	"Crowbar",
	"Zombie Arms",
	"Sig Sauer",
	"Revolver",
	"M4",
	"MP5",
	"Shotgun",
};

static const char* szTags_Items[] = {
	"Medkit",
	"Armor",
	"Ammo Boxes",
	"Backpack",
	"TNT",
	"Satchel Charge",
};

static const char* szTags_Generic[] = {
	"Sounds",
	"Survivor",
	"Zombie",
	"Background",
	"Music",
	"Sprays",
};

CWorkshopSubUpload::CWorkshopSubUpload(vgui2::Panel* parent)
	: BaseClass(parent, "WorkshopSubUpload")
{
	SetSize(100, 100); // Silence "parent not sized yet" warning

	pDescBox = new vgui2::TextEntry(this, "AddonDescTextBox");
	pTitleBox = new vgui2::TextEntry(this, "AddonTitleTextBox");
	pContentText = new vgui2::TextEntry(this, "BrowseFolderTextBox");
	pAddonImage = new vgui2::ImagePanel(this, "AddonImage");
	pVisibility = new vgui2::ComboBox(this, "Visibility", 5, false);
	pTags[0] = new vgui2::CheckButtonList(this, "Tags1");
	pTags[1] = new vgui2::CheckButtonList(this, "Tags2");
	pTags[2] = new vgui2::CheckButtonList(this, "Tags3");
	pChangeLogLabel = new vgui2::Label(this, "ChangeLogText", "#ZP_UI_Workshop_Upload_Changelog");
	pChangeLogText = new vgui2::TextEntry(this, "ChangeLogTextBox");
	pAddonUpload = new vgui2::Button(this, "AddonUpload", "#ZP_UI_Workshop_Upload_Addon", this, "AddonUpload");
	pAddonReset = new vgui2::Button(this, "AddonReset", "#ZP_UI_Workshop_Upload_Reset", this, "Reset");
	pProgress = new vgui2::ProgressBar(this, "ProgressBar");
	pProgressLabel = new vgui2::Label(this, "ProgressBarText", "#ZP_UI_Workshop_UpdateStatus_PreparingConfig");

	// Load this after we created our objects
	LoadControlSettings(VGUI2_ROOT_DIR "resource/workshop/upload.res");

	// KeyValues
	KeyValues* kv = new KeyValues("Categories", "Key", "Value");

	pVisibility->AddItem("#ZP_UI_Workshop_Upload_Tags_Private", kv);
	pVisibility->AddItem("#ZP_UI_Workshop_Upload_Tags_FriendsOnly", kv);
	pVisibility->AddItem("#ZP_UI_Workshop_Upload_Tags_Public", kv);
	pVisibility->AddItem("#ZP_UI_Workshop_Upload_Tags_FriendsOnly", kv);
	pVisibility->AddItem("#ZP_UI_Workshop_Upload_Tags_Private", kv);
	// Auto select "Show All Achievements"
	pVisibility->ActivateItem(2);

	// Never allow edit
	pVisibility->SetEditable(false);

	// Allow edit on the desc box
	// And make sure it got a scrollbar
	pDescBox->SetEditable(true);
	pDescBox->SetVerticalScrollbar(true);
	pDescBox->SetMultiline(true);
	pDescBox->SetWrap(true);

	pChangeLogText->SetEditable(true);
	pChangeLogText->SetVerticalScrollbar(true);
	pChangeLogText->SetMultiline(true);
	pChangeLogText->SetWrap(true);

	// Make sure we scale it
	pAddonImage->SetShouldScaleImage(true);

	last_folder[0].clear();
	last_folder[1].clear();
	preview_image.clear();

	// Populate the tags
	for (int i = 0; i < ARRAYSIZE(szTags_Weapons); i++)
		pTags[0]->AddItem(szTags_Weapons[i], false, new KeyValues("Item", szTags_Weapons[i], "1"));

	for (int i = 0; i < ARRAYSIZE(szTags_Items); i++)
		pTags[1]->AddItem(szTags_Items[i], false, new KeyValues("Item", szTags_Items[i], "1"));

	for (int i = 0; i < ARRAYSIZE(szTags_Generic); i++)
		pTags[2]->AddItem(szTags_Generic[i], false, new KeyValues("Item", szTags_Generic[i], "1"));

	pUploader = this;

	bUploading = false;

	// only show the changelog if addon is being updated
	SetUpdating(k_PublishedFileIdInvalid);

	vgui2::ivgui()->AddTickSignal(GetVPanel(), 25);
}

CWorkshopSubUpload::~CWorkshopSubUpload()
{
	pUploader = nullptr;
}

void CWorkshopSubUpload::ApplySchemeSettings(vgui2::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void OnFileSelected(DialogData* pData)
{
	if (pData->IsFolder)
		pUploader->UpdateContentPath(pData);
	else
		pUploader->UpdatePreviewImage(pData);
}

void CWorkshopSubUpload::OnCommand(const char* pcCommand)
{
	if (!Q_stricmp(pcCommand, "BrowseImage"))
	{
		CGameUIViewport::Get()->OpenFileExplorer(
			OpenFileDialog_e::FILE_PNG | OpenFileDialog_e::FILE_JPG,
			last_folder[0].c_str(),
			"WORKSHOP",
			OnFileSelected
		);
	}
	else if (!Q_stricmp(pcCommand, "BrowseFolder"))
	{
		CGameUIViewport::Get()->OpenFileExplorer(
			last_folder[1].c_str(),
			"WORKSHOP",
			OnFileSelected
		);
	}
	else if (!Q_stricmp(pcCommand, "AddonUpload"))
	{
		BeginUpload();
	}
	else if (!Q_stricmp(pcCommand, "OnResetOK"))
	{
		ResetPage();
	}
	else if (!Q_stricmp(pcCommand, "Reset"))
	{
		vgui2::MessageBox* pMessageBox = new vgui2::MessageBox("#ZP_Workshop_Reset", "#ZP_Workshop_Reset_Warning", this);
		pMessageBox->SetOKButtonVisible(true);
		pMessageBox->SetCancelButtonVisible(true);
		pMessageBox->SetCommand("OnResetOK");
		pMessageBox->DoModal();
	}
	else
	{
		BaseClass::OnCommand(pcCommand);
	}
}

void CWorkshopSubUpload::OnTick()
{
	BaseClass::OnTick();

	if (!SteamAPI_IsSteamRunning()) return;
	if (!SteamUGC()) return;

	if (bUploading)
	{
		if (handle == k_UGCUpdateHandleInvalid)
		{
			SetUpdating(k_PublishedFileIdInvalid);
			bUploading = false;
			ThrowError("#ZP_Workshop_UploadComplete");
			return;
		}

		uint64 iUpdateProgress[2];
		EItemUpdateStatus update_progress = SteamUGC()->GetItemUpdateProgress(handle, &iUpdateProgress[0], &iUpdateProgress[1]);
		if (update_progress > k_EItemUpdateStatusInvalid)
		{
			pProgress->SetVisible(true);
			pProgressLabel->SetVisible(true);
			switch (update_progress)
			{
			case k_EItemUpdateStatusPreparingConfig:
				pProgressLabel->SetText("#ZP_UI_Workshop_UpdateStatus_PreparingConfig");
				break;
			case k_EItemUpdateStatusPreparingContent:
				pProgressLabel->SetText("#ZP_UI_Workshop_UpdateStatus_PreparingContent");
				break;
			case k_EItemUpdateStatusUploadingContent:
				pProgressLabel->SetText("#ZP_UI_Workshop_UpdateStatus_UploadingContent");
				break;
			case k_EItemUpdateStatusUploadingPreviewFile:
				pProgressLabel->SetText("#ZP_UI_Workshop_UpdateStatus_UploadingPreviewFile");
				break;
			case k_EItemUpdateStatusCommittingChanges:
				pProgressLabel->SetText("#ZP_UI_Workshop_UpdateStatus_CommittingChanges");
				break;
			}

			// We hit the max value
			if (iUpdateProgress[0] == iUpdateProgress[1])
				handle = k_UGCUpdateHandleInvalid;
		}
	}
}

void CWorkshopSubUpload::OnMenuItemSelected()
{
	OnItemsUpdated();
}

void CWorkshopSubUpload::OnTextChanged()
{
	OnItemsUpdated();
}

void CWorkshopSubUpload::OnCheckButtonChecked(KeyValues* pParams)
{
	OnItemsUpdated();
}

void CWorkshopSubUpload::UpdateContentPath(DialogData* pData)
{
	last_folder[1] = pData->LocalPath;


	// Before we apply, we need to check if we find the addoninfo.txt,
	if (!HasAddonInfo(pData))
	{
		// Create a wizard and create the the file that way if its missing????
#if 0
		vgui2::WizardPanel* pWizard = new vgui2::WizardPanel(CGameUIViewport::Get(), "CreateAddonInfo");
		pWizard->DoModal();
#endif
		ThrowError("#ZP_Workshop_AddonInfoMissing");
		return;
	}

	pContentText->SetText(pData->FullPath.c_str());
}

void CWorkshopSubUpload::UpdatePreviewImage(DialogData* pData)
{
	last_folder[0] = pData->LocalPath;
	vgui2::STDReplaceString(last_folder[0], pData->File, "");
	std::string szFile = pData->LocalGamePath;
	const size_t found = pData->FileExtension.size();
	szFile = szFile.substr(0, szFile.size() - found);

	// -- BEGIN PNG READ
	if (pData->FileExtension == ".png")
	{
		std::vector<unsigned char> image; //the raw pixels
		unsigned width, height;
		unsigned error = lodepng::decode(image, width, height, pData->FullPath, LCT_RGB, 8);
		if (error)
		{
			CGameUIViewport::Get()->ShowMessageDialog(
				"#ZP_Workshop",
				vgui2::VarArgs(
					"Failed to decode the image!\nError %i: %s\n",
					error,
					lodepng_error_text(error)
				)
			);
			return;
		}

		if (width > 640 || height > 360)
		{
			std::string errstr;
			if (width > 640) errstr = "The image width is larger than 640 pixels!";
			else if (height > 360) errstr = "The image height is larger than 360 pixels!";
			CGameUIViewport::Get()->ShowMessageDialog(
				"#ZP_Workshop",
				vgui2::VarArgs(
					"Can't import image. Make sure its 640x360 maximum!\nError: %s\n",
					errstr.c_str()
				)
			);
			return;
		}
		// TODO: Create PNG/JPG > TGA so we can preview it ingame.
		// vgui2::write_tga is not setup properly, or I'm doin something wrong.
		// I only get fucky wucky images.
	}
	// -- EDN PNG READ

	preview_image = pData->FullPath;

	pAddonImage->SetImage(szFile.c_str());
}

void CWorkshopSubUpload::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CWorkshopSubUpload::SetUpdating(PublishedFileId_t nItem)
{
	pChangeLogText->SetVisible((nItem > 0) ? true : false);
	pChangeLogLabel->SetVisible((nItem > 0) ? true : false);
	nWorkshopID = nItem;

	pAddonUpload->SetText((nItem > 0) ? "#ZP_UI_Workshop_Upload_Addon_Modify" : "#ZP_UI_Workshop_Upload_Addon");
	pTitleBox->SetText("");
	pDescBox->SetText("");
	pContentText->SetText("");

	last_folder[0].clear();
	last_folder[1].clear();
	preview_image.clear();

	for (int i = 0; i < ARRAYSIZE(szTags_Weapons); i++)
		pTags[0]->SetItemSelected(i, false);

	for (int i = 0; i < ARRAYSIZE(szTags_Items); i++)
		pTags[1]->SetItemSelected(i, false);

	for (int i = 0; i < ARRAYSIZE(szTags_Generic); i++)
		pTags[2]->SetItemSelected(i, false);

	pAddonReset->SetVisible((nItem > 0) ? true : false);
	pAddonReset->SetEnabled((nItem > 0) ? true : false);
	pAddonUpload->SetEnabled(false);
	pProgress->SetVisible(false);
	pProgressLabel->SetVisible(false);
}

static bool DoWeHaveTheTag(const std::vector<std::string>& seglist, const std::string& strInput)
{
	for (size_t i = 0; i < seglist.size(); i++)
	{
		if (strInput == seglist[i])
			return true;
	}
	return false;
}

void CWorkshopSubUpload::SetUploadData(const char* Title, const char* Desc, const char* Tags, ERemoteStoragePublishedFileVisibility Visibility)
{
	pTitleBox->SetText(Title);
	pDescBox->SetText(Desc);
	pVisibility->ActivateItem(Visibility);
	//Split the Tags with ,
	std::stringstream tagstream(Tags);
	std::string segment;
	std::vector<std::string> seglist;
	while (std::getline(tagstream, segment, ','))
		seglist.push_back(segment);

	for (int i = 0; i < ARRAYSIZE(szTags_Weapons); i++)
	{
		if (DoWeHaveTheTag(seglist, szTags_Weapons[i]))
			pTags[0]->SetItemSelected(i, true);
	}

	for (int i = 0; i < ARRAYSIZE(szTags_Items); i++)
	{
		if (DoWeHaveTheTag(seglist, szTags_Items[i]))
			pTags[1]->SetItemSelected(i, true);
	}

	for (int i = 0; i < ARRAYSIZE(szTags_Generic); i++)
	{
		if (DoWeHaveTheTag(seglist, szTags_Generic[i]))
			pTags[2]->SetItemSelected(i, true);
	}
}

void CWorkshopSubUpload::ResetPage()
{
	SetUpdating(false);
}

void CWorkshopSubUpload::BeginUpload()
{
	if (!SteamAPI_IsSteamRunning()) return;
	if (!SteamUGC()) return;

	if (!ValidateTheEntries()) return;

	// Are we uploading new content, or updating?
	if (nWorkshopID > 0)
	{
		handle = SteamUGC()->StartItemUpdate(SteamUtils()->GetAppID(), nWorkshopID);
		PrepareUGCHandle();
		return;
	}

	SteamAPICall_t hSteamAPICall = SteamUGC()->CreateItem(SteamUtils()->GetAppID(), k_EWorkshopFileTypeCommunity);
	m_SteamCallResultOnItemCreated.Set(hSteamAPICall, this, &CWorkshopSubUpload::OnItemCreated);
}

void CWorkshopSubUpload::PrepareUGCHandle()
{
	SteamParamStringArray_t tagarray;
	std::vector<const char*> vArray;
	for (int i = 0; i < ARRAYSIZE(szTags_Weapons); i++)
	{
		bool bIsChecked = pTags[0]->IsItemChecked(i);
		if (!bIsChecked) continue;
		vArray.push_back(szTags_Weapons[i]);
	}

	for (int i = 0; i < ARRAYSIZE(szTags_Items); i++)
	{
		bool bIsChecked = pTags[1]->IsItemChecked(i);
		if (!bIsChecked) continue;
		vArray.push_back(szTags_Items[i]);
	}

	for (int i = 0; i < ARRAYSIZE(szTags_Generic); i++)
	{
		bool bIsChecked = pTags[2]->IsItemChecked(i);
		if (!bIsChecked) continue;
		vArray.push_back(szTags_Generic[i]);
	}

	// Our main text buffer
	char buffer[4028];

	// Title
	pTitleBox->GetText(buffer, sizeof(buffer));
	SteamUGC()->SetItemTitle(handle, buffer);

	// Description
	pDescBox->GetText(buffer, sizeof(buffer));
	SteamUGC()->SetItemDescription(handle, buffer);

	// The content
	pContentText->GetText(buffer, sizeof(buffer));
	SteamUGC()->SetItemContent(handle, buffer);

	// Preview image
	if (!preview_image.empty())
		SteamUGC()->SetItemPreview(handle, preview_image.c_str());

	// Our Tags
	SteamUGC()->SetItemTags(handle, &tagarray);

	// Our Visibility
	SteamUGC()->SetItemVisibility(handle, (ERemoteStoragePublishedFileVisibility)pVisibility->GetActiveItem());

	// Submit the changes
	pChangeLogLabel->GetText(buffer, sizeof(buffer));
	SteamAPICall_t hSteamAPICall = SteamUGC()->SubmitItemUpdate(handle, pChangeLogLabel->IsVisible() ? buffer : "Initial Release");
	m_SteamCallResultOnSubmitItemUpdateResult.Set(hSteamAPICall, this, &CWorkshopSubUpload::OnCallResultOnSubmitItemUpdateResult);

	bUploading = true;
	pAddonUpload->SetEnabled(false);
}

void CWorkshopSubUpload::OnItemsUpdated()
{
	pAddonUpload->SetEnabled(true);
}

bool CWorkshopSubUpload::HasAddonInfo(DialogData* pData)
{
	std::string addoninfo(pData->LocalPath + "addoninfo.txt");
	KeyValues* manifest = new KeyValues("AddonInfo");
	bool bAddonInfoExist = false;
	if (manifest->LoadFromFile(g_pFullFileSystem, addoninfo.c_str(), pData->PathID.c_str()))
		bAddonInfoExist = true;
	manifest->deleteThis();
	return bAddonInfoExist;
}

void CWorkshopSubUpload::ThrowError(const char* szMsg)
{
	CGameUIViewport::Get()->ShowMessageDialog(
		"#ZP_Workshop",
		szMsg
	);
}

bool CWorkshopSubUpload::ValidateTheEntries()
{
	if (nWorkshopID == 0)
	{
		if (preview_image.empty())
		{
			ThrowError("You need to assign a preview image.");
			return false;
		}
	}

	// Our main text buffer
	char buffer[4028];
	pTitleBox->GetText(buffer, sizeof(buffer));
	if (!Q_stricmp(buffer, ""))
	{
		ThrowError("You can't assign an empty title.");
		return false;
	}
	pDescBox->GetText(buffer, sizeof(buffer));
	if (!Q_stricmp(buffer, ""))
	{
		ThrowError("You can't assign an description.");
		return false;
	}
	pContentText->GetText(buffer, sizeof(buffer));
	if (!Q_stricmp(buffer, ""))
	{
		ThrowError("You can't assign empty content.");
		return false;
	}

	bool bHasAtleastOneAssignedTag = false;
	for (int i = 0; i < ARRAYSIZE(szTags_Weapons); i++)
	{
		bool bIsChecked = pTags[0]->IsItemChecked(i);
		if (!bIsChecked) continue;
		bHasAtleastOneAssignedTag = true;
	}

	for (int i = 0; i < ARRAYSIZE(szTags_Items); i++)
	{
		bool bIsChecked = pTags[1]->IsItemChecked(i);
		if (!bIsChecked) continue;
		bHasAtleastOneAssignedTag = true;
	}

	for (int i = 0; i < ARRAYSIZE(szTags_Generic); i++)
	{
		bool bIsChecked = pTags[2]->IsItemChecked(i);
		if (!bIsChecked) continue;
		bHasAtleastOneAssignedTag = true;
	}

	if (!bHasAtleastOneAssignedTag)
	{
		ThrowError("You need to atleast assign one workshop tag.");
		return false;
	}
	return true;
}

void CWorkshopSubUpload::OnCallResultOnSubmitItemUpdateResult(SubmitItemUpdateResult_t* pCallback, bool bIOFailure)
{
	// It failed!
	if (bIOFailure) return;

}

void CWorkshopSubUpload::OnItemCreated(CreateItemResult_t* pCallback, bool bIOFailure)
{
	// It failed!
	if (bIOFailure) return;

	// Now that we have a workshop ID, use it to begin our item update.
	nWorkshopID = pCallback->m_nPublishedFileId;

	// Start the update, and prepare our handle
	handle = SteamUGC()->StartItemUpdate(SteamUtils()->GetAppID(), nWorkshopID);
	PrepareUGCHandle();
}