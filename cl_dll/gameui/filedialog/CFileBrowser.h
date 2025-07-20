// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef CREATE_FILE_BROWSER_DIALOG
#define CREATE_FILE_BROWSER_DIALOG
#include <string>
#include <vgui_controls/Frame.h>
#include <vgui_controls/ButtonImage.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/ListPanel.h>
#include <FileSystem.h>
#include "IFileDialogManager.h"

class CFileBrowser;
class CFileListPanel : public vgui2::ListPanel
{
public:
	DECLARE_CLASS_SIMPLE(CFileListPanel, vgui2::ListPanel);

	CFileListPanel(CFileBrowser* pOuter, const char* pName);

	virtual void OnKeyCodeTyped(vgui2::KeyCode code);

private:
	CFileBrowser* m_pOuter;
};

class CFileBrowser : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(CFileBrowser, vgui2::Frame);

public:
	CFileBrowser(vgui2::Panel* pParent);
	void ApplySchemeSettings(vgui2::IScheme* pScheme);
	void Open(int eFilter, const char* szFolder, const char* szPathID, DialogSelected_t pFunction);
	void OnCommand(const char* pcCommand);

	bool OnEnterPressed();

protected:
	MESSAGE_FUNC(OnItemSelected, "ItemSelected");

private:
	void GetFileName(const char* szLocaPath, std::string& output);
	void GetFileExtension(const char* szLocaPath, std::string& output);
	void GetFolder(const char* szInput, std::string& output);
	void GetLastFolder(const char* szInput, std::string& output);
	int GetFullPath(const char* szLocalPath, std::string& output);
	std::string GetGamePath();

	void OpenFolder(const char* szFolder, const char* szPathID, bool bIsRoot);


	enum FileType
	{
		FileType_Unknown,
		FileType_Folder,
		FileType_TGA,
		FileType_JPG,
		FileType_PNG,
		FileType_BSP,
		FileType_TXT,
	};

	struct FileResult
	{
		bool IsValid;
		bool IsFolder;
		int Size;
		FileType Type;
		uint64 Date;
	};
	void IsFileValid(FileFindHandle_t fh, const char* szFile, const char* szLocalPath, bool bIsRoot, FileResult& out);
	bool IsAllowedInFilter(const char* szLocalPath, FileType& eType);
	bool HasFilterFlag(int flag);

	char szFolder[52];
	char szPathID[32];
	int nFilter;
	bool bIsFolderOnly;
	DialogSelected_t pFunctor;

	std::string szCurrentFolderSelected;

	CFileListPanel* pList;
	vgui2::TextEntry* pFilePath;
	vgui2::TextEntry* pFile;
	vgui2::ButtonImage* pBackButton;
};

#endif
