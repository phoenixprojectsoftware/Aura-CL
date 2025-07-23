// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "IFileDialogManager.h"
#include "../gameui_viewport.h"
#include "CFileBrowser.h"

class CFileDialogManager : public IFileDialogManager
{
public:
	CFileDialogManager();
	~CFileDialogManager();
	void OpenFileBrowser(int eFilter, const char* szFolder, const char* szPathID, DialogSelected_t pFunction);
	void OpenFolderBrowser(const char* szFolder, const char* szPathID, DialogSelected_t pFunction);
};

IFileDialogManager* g_pFileDialogManager = nullptr;
static CFileDialogManager sFileManager;

CFileDialogManager::CFileDialogManager()
{
	g_pFileDialogManager = this;
}

CFileDialogManager::~CFileDialogManager()
{
	g_pFileDialogManager = nullptr;
}

void CFileDialogManager::OpenFileBrowser(int eFilter, const char* szFolder, const char* szPathID, DialogSelected_t pFunction)
{
	CFileBrowser* pFileBrowser = new CFileBrowser(CGameUIViewport::Get());
	pFileBrowser->Open(eFilter, szFolder, szPathID, pFunction);
	pFileBrowser->Activate();
}

void CFileDialogManager::OpenFolderBrowser(const char* szFolder, const char* szPathID, DialogSelected_t pFunction)
{
	CFileBrowser* pFileBrowser = new CFileBrowser(CGameUIViewport::Get());
	pFileBrowser->Open(-1, szFolder, szPathID, pFunction);
	pFileBrowser->Activate();
}