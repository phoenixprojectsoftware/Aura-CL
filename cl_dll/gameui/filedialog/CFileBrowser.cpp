// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui/IVGui.h>
#include "CFileBrowser.h"
#include "vgui_controls/ImageList.h"
#include "../gameui_viewport.h"
#include "../../client_vgui.h"
#include "tier1/KeyValues.h"

#if _WIN32
#include <winsani_in.h>
#include <windows.h>
#include <winsani_out.h>
#endif

// For date modified
#include <chrono>
#include "../../client_vgui.h"


CFileListPanel::CFileListPanel(CFileBrowser* pOuter, const char* pName) :
	BaseClass(pOuter, pName)
{
	m_pOuter = pOuter;
}

//-----------------------------------------------------------------------------
// Purpose: Forward KEY_ENTER to the CBaseGamesPage.
//-----------------------------------------------------------------------------
void CFileListPanel::OnKeyCodeTyped(vgui2::KeyCode code)
{
	// Let the outer class handle it.
	if (code == vgui2::KEY_ENTER && m_pOuter->OnEnterPressed())
		return;

	BaseClass::OnKeyCodeTyped(code);
}

int __cdecl FileSizes(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2)
{
	KeyValues* s1 = p1.kv;
	KeyValues* s2 = p2.kv;

	if (!s1 && s2)
		return -1;
	if (!s2 && s1)
		return 1;
	if (!s1 && !s2)
		return 0;

	return s1->GetInt("Size") > s2->GetInt("Size");
}

int __cdecl FileModified(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2)
{
	KeyValues* s1 = p1.kv;
	KeyValues* s2 = p2.kv;

	if (!s1 && s2)
		return -1;
	if (!s2 && s1)
		return 1;
	if (!s1 && !s2)
		return 0;

	return Q_stricmp(s1->GetString("Date"), s2->GetString("Date"));
}

int __cdecl FileNames(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2)
{
	KeyValues* s1 = p1.kv;
	KeyValues* s2 = p2.kv;

	if (!s1 && s2)
		return -1;
	if (!s2 && s1)
		return 1;
	if (!s1 && !s2)
		return 0;

	return Q_stricmp(s1->GetString("Name"), s2->GetString("Name"));
}

int __cdecl FileTypes(vgui2::ListPanel* pPanel, const vgui2::ListPanelItem& p1, const vgui2::ListPanelItem& p2)
{
	KeyValues* s1 = p1.kv;
	KeyValues* s2 = p2.kv;

	if (!s1 && s2)
		return -1;
	if (!s2 && s1)
		return 1;
	if (!s1 && !s2)
		return 0;

	return Q_stricmp(s1->GetString("Type"), s2->GetString("Type"));
}

CFileBrowser::CFileBrowser(vgui2::Panel* pParent)
	: BaseClass(pParent, "FileBrowser")
{
	SetTitle("#ZP_UI_FileBrowser", false);
	SetSizeable(false);
	SetSize(100, 80);
	SetPos(0, 0);
	SetDeleteSelfOnClose(true);

	pList = new CFileListPanel(this, "ItemList");
	pFilePath = new vgui2::TextEntry(this, "CurrentLocation");
	pFile = new vgui2::TextEntry(this, "CurrentFile");
	pBackButton = new vgui2::ButtonImage(this, "BackOneFolder", "resource/icon_folderup", this, "BackOneFolder");

	SetScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceSchemeBase.res", "ClientSourceSchemeBase"));

	LoadControlSettings(VGUI2_ROOT_DIR "resource/filebrowser.res");

	// No edit, thanks.
	pFilePath->SetEditable(false);

	pList->AddActionSignalTarget(this);

	pList->SetAllowUserModificationOfColumns(true);

	pList->AddColumnHeader(0, "Icon", "", 24, vgui2::ListPanel::COLUMN_FIXEDSIZE | vgui2::ListPanel::COLUMN_IMAGE);
	pList->AddColumnHeader(1, "Name", "#ZP_UI_FileBrowser_Name", 100, vgui2::ListPanel::COLUMN_RESIZEWITHWINDOW | vgui2::ListPanel::COLUMN_UNHIDABLE);
	pList->AddColumnHeader(2, "Size", "#ZP_UI_FileBrowser_Size", 100, vgui2::ListPanel::COLUMN_RESIZEWITHWINDOW);
	pList->AddColumnHeader(3, "Type", "#ZP_UI_FileBrowser_Type", 100, vgui2::ListPanel::COLUMN_RESIZEWITHWINDOW);
	pList->AddColumnHeader(4, "Date", "#ZP_UI_FileBrowser_DateModified", 100, vgui2::ListPanel::COLUMN_RESIZEWITHWINDOW);

	pList->SetSortFunc(1, FileNames);
	pList->SetSortFunc(2, FileSizes);
	pList->SetSortFunc(3, FileTypes);
	pList->SetSortFunc(4, FileModified);

	pList->SetSortColumn(1);

	SetScheme(CGameUIViewport::Get()->GetScheme());
	InvalidateLayout();

	MoveToFront();
}

void CFileBrowser::ApplySchemeSettings(vgui2::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// load the password icon
	vgui2::ImageList* imageList = new vgui2::ImageList(false);
	imageList->AddImage(vgui2::scheme()->GetImage("resource/icon_file", false));
	imageList->AddImage(vgui2::scheme()->GetImage("resource/icon_folder", false));

	// Fonts
	vgui2::HFont hTextFont = pScheme->GetFont("ListSmall");
	if (hTextFont == vgui2::INVALID_FONT)
		hTextFont = pScheme->GetFont("DefaultSmall");

	if (hTextFont != vgui2::INVALID_FONT)
	{
		pList->SetFont(hTextFont);
		pFilePath->SetFont(hTextFont);
	}

	pList->SetImageList(imageList, true);
}

void CFileBrowser::Open(int eFilter, const char* szFolder, const char* szPathID, DialogSelected_t pFunction)
{
	Q_snprintf(this->szFolder, sizeof(this->szFolder), "");
	Q_snprintf(this->szPathID, sizeof(this->szPathID), "%s", szPathID);
	bIsFolderOnly = (eFilter == -1) ? true : false;
	nFilter = eFilter;
	pFunctor = pFunction;
	bool bIsRoot = !Q_stricmp(szFolder, "") ? true : false;
	OpenFolder(szFolder, this->szPathID, bIsRoot);
}

void CFileBrowser::OnCommand(const char* pcCommand)
{
	if (!Q_stricmp(pcCommand, "Select"))
	{
		if (bIsFolderOnly)
		{
			// If folder, then we check szCurrentFolderSelected instead.
			if (pFunctor)
			{
				std::string fullpath;
				std::string szFolder;
				GetFolder(szCurrentFolderSelected.c_str(), szFolder);
				GetFullPath(szCurrentFolderSelected.c_str(), fullpath);
				DialogData* pData = new DialogData;
				pData->File = szFolder.c_str();
				pData->FileExtension = "";
				pData->LocalPath = szCurrentFolderSelected.c_str();
				pData->LocalGamePath = GetGamePath() + szCurrentFolderSelected;
				pData->FullPath = fullpath.c_str();
				pData->IsFolder = true;
				pData->PathID = szPathID;
				pFunctor(pData);
			}
		}
		else
		{
			KeyValues* kv = pList->GetItem(pList->GetSelectedItem(0));
			if (pFunctor && kv)
			{
				std::string fullpath;
				std::string filext;
				GetFileExtension(kv->GetString("LocalPath"), filext);
				GetFullPath(kv->GetString("LocalPath"), fullpath);
				DialogData* pData = new DialogData;
				pData->File = kv->GetString("Name");
				pData->LocalPath = kv->GetString("LocalPath");
				pData->FileExtension = "." + filext;
				pData->LocalGamePath = GetGamePath() + kv->GetString("LocalPath");
				pData->FullPath = fullpath.c_str();
				pData->IsFolder = kv->GetBool("IsFolder");
				pData->PathID = szPathID;
				pFunctor(pData);
			}
		}
		Close();
	}
	else if (!Q_stricmp(pcCommand, "BackOneFolder"))
	{
		if (Q_stricmp(szCurrentFolderSelected.c_str(), ""))
		{
			// Go back a folder
			std::string path;
			GetLastFolder(szCurrentFolderSelected.c_str(), path);

			bool bIsRoot = false;
			// equals? then it's root.
			if (!Q_stricmp(path.c_str(), szFolder))
				bIsRoot = true;

			OpenFolder(path.c_str(), szPathID, bIsRoot);
		}
	}
	else
	{
		BaseClass::OnCommand(pcCommand);
	}
}

bool CFileBrowser::OnEnterPressed()
{
	int nIndex = pList->GetSelectedItem(0);
	if (nIndex == -1) return false;
	KeyValues* kv = pList->GetItem(pList->GetSelectedItem(0));
	if (kv->GetBool("IsFolder"))
	{
		if (!Q_stricmp(kv->GetString("Name"), ".."))
		{
			// Go back a folder
			std::string path;
			GetLastFolder(kv->GetString("LocalPath"), path);

			bool bIsRoot = false;
			// equals? then it's root.
			if (!Q_stricmp(path.c_str(), szFolder))
				bIsRoot = true;

			OpenFolder(path.c_str(), szPathID, bIsRoot);
		}
		else
		{
			char path[4028];
			Q_snprintf(path, sizeof(path), "%s", kv->GetString("LocalPath"));
			OpenFolder(path, szPathID, false);
		}
		return true;
	}
	OnCommand("Select");
	return true;
}

void CFileBrowser::OnItemSelected()
{
	// Item got selected, update our file textentry. (if it's not '..')
	KeyValues* kv = pList->GetItem(pList->GetSelectedItem(0));
	if (kv)
		pFile->SetText(kv->GetString("Name"));
}

void CFileBrowser::GetFileName(const char* szLocaPath, std::string& output)
{
	std::string path = szLocaPath;
	if (path.size() > 0)
	{
		// Only do this, if the last letter is
		// forward slash, or backslash
		const size_t laststr = path.size() - 1;
		if (path[laststr] == '/' || path[laststr] == '\\')
			path = path.substr(0, path.size() - 1);
	}
	const size_t found = path.find_last_of("/\\");
	output = (found == std::string::npos) ? path : path.substr(found + 1);
}

void CFileBrowser::GetFileExtension(const char* szLocaPath, std::string& output)
{
	GetFileName(szLocaPath, output);
	const size_t found = output.find_last_of(".");
	output = (found == std::string::npos) ? output : output.substr(found + 1);
}

void CFileBrowser::GetFolder(const char* szInput, std::string& output)
{
	std::string path = szInput;
	const size_t found = path.find_last_of("/\\");
	output = (found == std::string::npos) ? path : path.substr(0, found);
}

void CFileBrowser::GetLastFolder(const char* szInput, std::string& output)
{
	std::string path = szInput;
	vgui2::STDReplaceString(path, "../", "");
	path = path.substr(0, path.size() - 1);

	const size_t found = path.find_last_of("/\\");
	output = (found == std::string::npos) ? "" : path.substr(0, found);
	if ((found != std::string::npos))
		output += "/";
}

int CFileBrowser::GetFullPath(const char* szLocalPath, std::string& output)
{
	char buffer[256];
	size_t len = sizeof(buffer);
#ifdef _WIN32
	int bytes = GetModuleFileName(NULL, buffer, len);
	GetFolder(buffer, output);
	output += "/" + GetGamePath() + std::string(szLocalPath);
	return bytes ? bytes : -1;
#else
	ssize_t count = readlink("/proc/self/exe", buffer, len);

	if (count >= 0)
		buffer[count] = '\0';
	GetFolder(buffer, output);
	output += "/" + GetGamePath() + std::string(szLocalPath);
	return count;
#endif
}

std::string CFileBrowser::GetGamePath()
{
	// Path ID for the folders
	std::string pathid;
	pathid.clear();
	if (!Q_stricmp(szPathID, "WORKSHOP")) pathid = "zamnhlmp_workshop/";
	else if (!Q_stricmp(szPathID, "ADDON")) pathid = "zamnhlmp_addon/";
	else if (!Q_stricmp(szPathID, "PLATFORM")) pathid = "platform/";
	else if (!Q_stricmp(szPathID, "DOWNLOAD")) pathid = "zamnhlmp_download/";
	else if (!Q_stricmp(szPathID, "ROOT")) pathid = "/";
	else if (!Q_stricmp(szPathID, "GAME") || !Q_stricmp(szPathID, "MOD")) pathid = "zamnhlmp/";
	return pathid;
}

void CFileBrowser::OpenFolder(const char* szFolder, const char* szPathID, bool bIsRoot)
{
	pFilePath->SetText(szFolder);
	szCurrentFolderSelected = szFolder;

	// Clear it first
	pList->RemoveAll();

	char path[4028];
	Q_snprintf(path, sizeof(path), "%s", szFolder);

	FileFindHandle_t fh;
	char const* fn = g_pFullFileSystem->FindFirst(vgui2::VarArgs("%s*.*", path), &fh, szPathID);
	if (!fn) return;
	do
	{
		// Setup the path string, and lowercase it, so we don't need to search for both uppercase, and lowercase files.
		char strFile[4028];
		strFile[0] = 0;
		V_strcpy_safe(strFile, fn);
		Q_strlower(strFile);

		char strNewPath[4028];
		Q_snprintf(strNewPath, sizeof(strNewPath), "%s%s/", path, strFile);

		FileResult data;
		IsFileValid(fh, strFile, strNewPath, bIsRoot, data);

		if (data.IsValid)
		{
			// Remove the last / if it's not a folder.
			if (!data.IsFolder)
				Q_snprintf(strNewPath, sizeof(strNewPath), "%s%s", path, strFile);

			KeyValues* kv = new KeyValues("Item");
			kv->SetInt("Icon", data.IsFolder ? 2 : 1);
			kv->SetString("Name", strFile);
			if (data.IsFolder)
				kv->SetString("Size", "");
			else
				kv->SetInt("Size", data.Size);
			switch (data.Type)
			{
			default: kv->SetString("Type", "File Unknown Type"); break;
			case FileType_Folder: kv->SetString("Type", "File Folder"); break;
			case FileType_TGA: kv->SetString("Type", "File TGA"); break;
			case FileType_JPG: kv->SetString("Type", "File JPEG"); break;
			case FileType_BSP: kv->SetString("Type", "File BSP"); break;
			case FileType_TXT: kv->SetString("Type", "File Text"); break;
			}
			if (data.Date > 0)
			{
				char time_buf[80];
				time_t time_date = data.Date;
				struct tm ts;
				ts = *localtime(&time_date);
				strftime(time_buf, sizeof(time_buf), "%A %B %e %G", &ts);
				kv->SetString("Date", time_buf);
			}
			else
				kv->SetString("Date", "");
			kv->SetString("LocalPath", strNewPath);
			kv->SetBool("IsFolder", data.IsFolder);
			pList->AddItem(kv, 0, false, false);
		}

		fn = g_pFullFileSystem->FindNext(fh);
	} while (fn);

	g_pFullFileSystem->FindClose(fh);

	// Sort after we are done
	pList->SortList();
}

void CFileBrowser::IsFileValid(FileFindHandle_t fh, const char* szFile, const char* szLocalPath, bool bIsRoot, FileResult& out)
{
	out.IsValid = false;
	out.Size = -1;
	out.Date = 0;
	out.Type = FileType_Unknown;

	bool isSameDir = false;
	if (vgui2::FStrEq(szFile, "."))
		isSameDir = true;

	if (bIsRoot)
	{
		// Only allow to go back if not in root dir.
		if (vgui2::FStrEq(szFile, ".."))
			isSameDir = true;
	}
	out.IsFolder = isSameDir;
	out.Date = g_pFullFileSystem->GetFileModificationTime(szLocalPath);

	if (g_pFullFileSystem->FindIsDirectory(fh) && !isSameDir)
	{
		out.IsValid = true;
		out.IsFolder = true;
		out.Type = FileType_Folder;
	}
	else if (!g_pFullFileSystem->FindIsDirectory(fh))
	{
		FileType eType;
		out.IsValid = IsAllowedInFilter(szLocalPath, eType);
		out.Size = g_pFullFileSystem->Size(szLocalPath);
		out.Type = eType;
	}
}

bool CFileBrowser::IsAllowedInFilter(const char* szLocalPath, FileType& eType)
{
	eType = FileType_Unknown;
	if (bIsFolderOnly) return false;

	std::string FileExt;
	GetFileExtension(szLocalPath, FileExt);
	if (FileExt == "tga") eType = FileType::FileType_TGA;
	else if (FileExt == "jpg") eType = FileType::FileType_JPG;
	else if (FileExt == "png") eType = FileType::FileType_PNG;
	else if (FileExt == "bsp") eType = FileType::FileType_BSP;
	else if (FileExt == "txt") eType = FileType::FileType_TXT;

	bool bIsFile = true;
	int filter = OpenFileDialog_e::FILE_ANY;
	switch (eType)
	{
	case FileType_TGA: filter |= OpenFileDialog_e::FILE_TGA; break;
	case FileType_JPG: filter |= OpenFileDialog_e::FILE_JPG; break;
	case FileType_PNG: filter |= OpenFileDialog_e::FILE_PNG; break;
	case FileType_BSP: filter |= OpenFileDialog_e::FILE_BSP; break;
	case FileType_TXT: filter |= OpenFileDialog_e::FILE_TXT; break;
	}

	return HasFilterFlag(filter);
}

bool CFileBrowser::HasFilterFlag(int flag)
{
	if (nFilter == OpenFileDialog_e::FILE_ANY) return true;
	return ((nFilter & flag) != 0);
}