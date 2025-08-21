// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef GAMEUI_FILEDIALOG_MANAGER_H
#define GAMEUI_FILEDIALOG_MANAGER_H

#include <string>

enum OpenFileDialog_e
{
	FILE_ANY = 0,		// Any file
	FILE_TGA = (1 << 0),	// .tga files
	FILE_JPG = (1 << 1),	// .jpg files
	FILE_PNG = (1 << 2),	// .png files
	FILE_BSP = (1 << 3),	// .bsp files
	FILE_TXT = (1 << 4),	// .txt files
};

// File selected dialog data
struct DialogData
{
	std::string File;
	std::string FileExtension;
	std::string LocalPath;
	std::string LocalGamePath;
	std::string FullPath;
	std::string PathID;
	bool IsFolder;
};

// The function we will fire (if valid), once we have selected our
// file or folder.
typedef void (*DialogSelected_t)(DialogData* pData);

class IFileDialogManager
{
public:
	virtual void OpenFileBrowser(int eFilter, const char* szFolder, const char* szPathID, DialogSelected_t pFunction) = 0;
	virtual void OpenFolderBrowser(const char* szFolder, const char* szPathID, DialogSelected_t pFunction) = 0;
};

extern IFileDialogManager* g_pFileDialogManager;

#endif