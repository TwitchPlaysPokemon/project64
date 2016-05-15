/*
 * Project 64 - A Nintendo 64 emulator.
 *
 * (c) Copyright 2001 zilmar (zilmar@emulation64.com) and 
 * Jabo (jabo@emulation64.com).
 *
 * pj64 homepage: www.pj64.net
 *
 * Permission to use, copy, modify and distribute Project64 in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Project64 is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Project64 or software derived from Project64.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so if they want them.
 *
 */

#include <Windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <stdio.h>
#include "main.h"
#include "cpu.h"
#include "plugin.h"

typedef struct {
	char     szFullFileName[MAX_PATH+1];
	char     Status[60];
	char     FileName[200];
	char     InternalName[22];
	char     GoodName[200];
	char     CartID[3];
	char     PluginNotes[250];
	char     CoreNotes[250];
	char     UserNotes[250];
	char     Developer[30];
	char     ReleaseDate[30];
	char     Genre[15];
	int		 Players;
	COLORREF TextColor;
	int      SelColor;
	COLORREF SelTextColor;
	HBRUSH   SelColorBrush;
	int      RomSize;
	BYTE     Manufacturer;
	BYTE     Country;
	DWORD    CRC1;
	DWORD    CRC2;
	int      CicChip;
} ROM_INFO;

typedef struct {
	BYTE     Country;
	DWORD    CRC1;
	DWORD    CRC2;
	long     Fpos;
} ROM_LIST_INFO;

typedef struct {
	int    ListCount;
	int    ListAlloc;
	ROM_INFO * List;
} ITEM_LIST;

typedef struct {
	int    ListCount;
	int    ListAlloc;
	ROM_LIST_INFO * List;
} ROM_LIST;

#define RB_FileName			0
#define RB_InternalName		1
#define RB_GoodName			2
#define RB_Status			3
#define RB_RomSize			4
#define RB_CoreNotes		5
#define RB_PluginNotes		6
#define RB_UserNotes		7
#define RB_CartridgeID		8
#define RB_Manufacturer		9
#define RB_Country			10
#define RB_Developer		11
#define RB_CRC1				12
#define RB_CRC2				13
#define RB_CICChip			14
#define RB_ReleaseDate		15
#define RB_Genre			16
#define RB_Players			17

char * GetSortField          ( void );
void LoadRomList             ( void );
void FillRomExtensionInfo    ( ROM_INFO * pRomInfo );
BOOL FillRomInfo             ( ROM_INFO * pRomInfo );
void SetSortAscending        ( void );
void SetSortField            ( char * FieldName );
void SaveRomList             ( void );

int CALLBACK RomList_CompareItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

ROMBROWSER_FIELDS RomBrowserFields[] =
{
	"File Name",              -1, RB_FileName,      280,"RomBrowserField1",
	"Internal Name",          -1, RB_InternalName,  200,"RomBrowserField2",
	"Good Name",               1, RB_GoodName,      280,"RomBrowserField3",
	"Status",                  0, RB_Status,        100,"RomBrowserField4",
	"Rom Size",               -1, RB_RomSize,       100,"RomBrowserField5",
	"Notes (Core)",            2, RB_CoreNotes,     100,"RomBrowserField6",
	"Notes (default plugins)", 3, RB_PluginNotes,   100,"RomBrowserField7",
	"Notes (User)",           -1, RB_UserNotes,     100,"RomBrowserField8",
	"Cartridge ID",           -1, RB_CartridgeID,   100,"RomBrowserField9",
	"Manufacturer",           -1, RB_Manufacturer,  100,"RomBrowserField10",
	"Country",                -1, RB_Country,       100,"RomBrowserField11",
	"Developer",              -1, RB_Developer,     100,"RomBrowserField12",
	"CRC1",                   -1, RB_CRC1,          100,"RomBrowserField13",
	"CRC2",                   -1, RB_CRC2,          100,"RomBrowserField14",
	"CIC Chip",               -1, RB_CICChip,       100,"RomBrowserField15",
	"Release Date",           -1, RB_ReleaseDate,   100,"RomBrowserField16",
	"Genre",                  -1, RB_Genre,         100,"RomBrowserField17",
	"players",                -1, RB_Players,       100,"RomBrowserField18"
};

HWND hRomList= NULL;
int NoOfFields, FieldType[(sizeof(RomBrowserFields) / sizeof(RomBrowserFields[0])) + 1];
BOOL SortAscending = TRUE;

ITEM_LIST ItemList = {0,0,NULL};
ROM_LIST RDBList = {0,0,NULL};

void AddRomToList (char * RomLocation) {
	LV_ITEM  lvItem;
	ROM_INFO * pRomInfo;
	int index;

	if (ItemList.ListAlloc == 0) {
		ItemList.List = (ROM_INFO *)malloc(100 * sizeof(ROM_INFO));
		ItemList.ListAlloc = 100;
	} else if (ItemList.ListAlloc == ItemList.ListCount) {
		ItemList.ListAlloc += 100;
		ItemList.List = (ROM_INFO *)realloc(ItemList.List, ItemList.ListAlloc * sizeof(ROM_INFO));
		if (ItemList.List == NULL) {
			DisplayError("Failed");
			ExitThread(0);
		}
	}
	pRomInfo = &ItemList.List[ItemList.ListCount];
	if (pRomInfo == NULL) { return; }

	memset(pRomInfo, 0, sizeof(ROM_INFO));	
	memset(&lvItem, 0, sizeof(lvItem));

	strncpy(pRomInfo->szFullFileName, RomLocation, MAX_PATH);
	if (!FillRomInfo(pRomInfo)) { return;  }
	
	lvItem.mask = LVIF_TEXT | LVIF_PARAM;		
	lvItem.iItem = ListView_GetItemCount(hRomList);
	lvItem.lParam = (LPARAM)ItemList.ListCount;
	lvItem.pszText = LPSTR_TEXTCALLBACK;
	ItemList.ListCount += 1;
	
	index = ListView_InsertItem(hRomList, &lvItem);	
	if (_stricmp(pRomInfo->szFullFileName,LastRoms[0]) == 0) {
		ListView_SetItemState(hRomList,index,LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(hRomList,index,FALSE);
	}
}

void CreateRomListControl (HWND hParent) {
	hRomList = CreateWindowEx( WS_EX_CLIENTEDGE,WC_LISTVIEW,NULL,
					WS_TABSTOP | WS_VISIBLE | WS_CHILD | LVS_OWNERDRAWFIXED |
					WS_BORDER | LVS_SINGLESEL | LVS_REPORT,
					0,0,0,0,hParent,(HMENU)IDC_ROMLIST,hInst,NULL);
	
	ResetRomBrowserColomuns();
	LoadRomList();
}

void FixRomBrowserColoumnLang (void) {
	char * LangFile, Language[100];
	int count;

	LangFile = GetLangFileName();
	_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);
	for (count = 0; count < NoOfFields; count ++) {
		_GetPrivateProfileString(Language,RomBrowserFields[count].LangID,
			RomBrowserFields[count].Name,RomBrowserFields[count].Name,
			sizeof(RomBrowserFields[count].Name),LangFile);		
	}
	ResetRomBrowserColomuns();
}

void HideRomBrowser (void) {
	DWORD X, Y;
	long Style;

	if (hCPU != NULL) { return; }	
	if (hRomList == NULL) { return; }

	IgnoreMove = TRUE;
	if (IsRomBrowserMaximized()) { ShowWindow(hMainWindow,SW_RESTORE); }
	ShowWindow(hMainWindow,SW_HIDE);
	Style = GetWindowLong(hMainWindow,GWL_STYLE);
	Style = Style &	~(WS_SIZEBOX | WS_MAXIMIZEBOX);
	SetWindowLong(hMainWindow,GWL_STYLE,Style);
	if (GetStoredWinPos( "Main", &X, &Y ) ) {
		SetWindowPos(hMainWindow,NULL,X,Y,0,0, SWP_NOZORDER | SWP_NOSIZE);		 
	}			
	EnableWindow(hRomList,FALSE);
	ShowWindow(hRomList,SW_HIDE);
	SetupPlugins(hMainWindow);
	
	SendMessage(hMainWindow,WM_USER + 17,0,0);
	ShowWindow(hMainWindow,SW_SHOW);
	IgnoreMove = FALSE;
}

void IndexRomDataBase (void) {
	char *Input = NULL, *Data = NULL, * Pos, CurrentSection[300];
	int DataLen = 0, DataLeft, result, count;
	ROM_LIST_INFO * item;
	static long Fpos = 0;
	FILE * fInput;

	fInput = fopen(GetIniFileName(),"rb");
	if (fInput == NULL) { return; }
	RDBList.ListCount = 0;
	RDBList.ListAlloc = 0;
	RDBList.List      = NULL;
	
	DataLeft = 0;
	do {
		Fpos = ftell(fInput) - DataLeft;
		result = fGetString2(fInput,&Input,&Data,&DataLen,&DataLeft);
		if (result <= 1) { continue; }
		
		Pos = Input;
		while (Pos != NULL) {
			Pos = strchr(Pos,'/');
			if (Pos != NULL) {
				if (Pos[1] == '/') { Pos[0] = 0; } else { Pos += 1; }
			}
		}
		
		for (count = strlen(&Input[0]) - 1; count >= 0; count --) {
			if (Input[count] != ' ' && Input[count] != '\r') { break; }
			Input[count] = 0;
		}
		//stip leading spaces
		if (strlen(Input) <= 1) { continue; }
		if (Input[0] == '[') {
			if (Input[strlen(Input) - 1] != ']') { continue; }
			strcpy(CurrentSection,&Input[1]);
			CurrentSection[strlen(CurrentSection) - 1] = 0;
			if (CurrentSection[8] != '-') { continue; }
			if (CurrentSection[17] != '-') { continue; }
			if (CurrentSection[18] != 'C') { continue; }
			if (CurrentSection[19] != ':') { continue; }

			if (RDBList.ListAlloc == 0) {
				RDBList.List = (ROM_LIST_INFO *)malloc(100 * sizeof(ROM_LIST_INFO));
				RDBList.ListAlloc = 100;
			} else if (RDBList.ListAlloc == RDBList.ListCount) {
				RDBList.ListAlloc += 100;
				RDBList.List = (ROM_LIST_INFO *)realloc(RDBList.List, RDBList.ListAlloc * sizeof(ROM_LIST_INFO));
				if (RDBList.List == NULL) {
					DisplayError("Failed");
					ExitThread(0);
				}
			}
			item = &RDBList.List[RDBList.ListCount];
			item->CRC1 = AsciiToHex(CurrentSection);
			item->CRC2 = AsciiToHex(&CurrentSection[9]);
			item->Country = (BYTE)AsciiToHex(&CurrentSection[20]);
			item->Fpos  = Fpos;
			RDBList.ListCount += 1;
			continue;
		}
	} while (result >= 0);
}

BOOL IsRomBrowserMaximized (void) {
	long lResult;
	HKEY hKeyResults = 0;
	char String[200];

	sprintf(String,"Software\\N64 Emulation\\%s\\Page Setup",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0, KEY_ALL_ACCESS,&hKeyResults);
	
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Value, Bytes = 4;

		lResult = RegQueryValueEx(hKeyResults,"RomBrowser Maximized",0,&Type,(LPBYTE)(&Value),&Bytes);
		if (Type == REG_DWORD && lResult == ERROR_SUCCESS) { 
			RegCloseKey(hKeyResults);
			return Value;
		}
		RegCloseKey(hKeyResults);
	}
	return FALSE;
}

BOOL IsSortAscending (void) {
	long lResult;
	HKEY hKeyResults = 0;
	char String[200];

	sprintf(String,"Software\\N64 Emulation\\%s\\Page Setup",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0, KEY_ALL_ACCESS,&hKeyResults);
	
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Value, Bytes = 4;

		lResult = RegQueryValueEx(hKeyResults,"Sort Ascending",0,&Type,(LPBYTE)(&Value),&Bytes);
		if (Type == REG_DWORD && lResult == ERROR_SUCCESS) { 
			RegCloseKey(hKeyResults);
			return Value;
		}
		RegCloseKey(hKeyResults);
	}
	return TRUE;
}

void LoadRomList (void) {
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
	char fname[_MAX_FNAME],ext[_MAX_EXT];
	char FileName[_MAX_PATH], * SortField;
	int Size, count, index;
	ROM_INFO * pRomInfo;
	LV_ITEM  lvItem;
	DWORD dwRead;
	HANDLE hFile;

	GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
	_splitpath( path_buffer, drive, dir, fname, ext );
	sprintf(FileName,"%s%s%s",drive,dir,CacheFileName);

	hFile = CreateFile(FileName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		RefreshRomBrowser();
		return;
	}
	Size = 0;
	ReadFile(hFile,&Size,sizeof(Size),&dwRead,NULL);
	if (Size != sizeof(ROM_INFO) || dwRead != sizeof(Size)) {
		CloseHandle(hFile);
		RefreshRomBrowser();
		return;
	}
	if (ItemList.ListAlloc != 0) {
		free(ItemList.List);
		ItemList.ListAlloc = 0;
		ItemList.ListCount = 0;
		ItemList.List = NULL;		
	}
	ReadFile(hFile,&ItemList.ListCount,sizeof(ItemList.ListCount),&dwRead,NULL);
	ItemList.List = (ROM_INFO *)malloc(ItemList.ListCount * sizeof(ROM_INFO));
	ItemList.ListAlloc = ItemList.ListCount;
	ReadFile(hFile,ItemList.List,sizeof(ROM_INFO) * ItemList.ListCount,&dwRead,NULL);
	CloseHandle(hFile);
	ListView_DeleteAllItems(hRomList);
	for (count = 0; count < ItemList.ListCount; count ++) {
		pRomInfo = &ItemList.List[count];
		memset(&lvItem, 0, sizeof(lvItem));

		lvItem.mask = LVIF_TEXT | LVIF_PARAM;		
		lvItem.iItem = ListView_GetItemCount(hRomList);
		lvItem.lParam = (LPARAM)count;
		lvItem.pszText = LPSTR_TEXTCALLBACK;

		index = ListView_InsertItem(hRomList, &lvItem);	
		if (_stricmp(pRomInfo->szFullFileName,LastRoms[0]) == 0) {
			ListView_SetItemState(hRomList,index,LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
			ListView_EnsureVisible(hRomList,index,FALSE);
		}
		if (pRomInfo->SelColor < 0) { 
			pRomInfo->SelColorBrush = (HBRUSH)(COLOR_HIGHLIGHT + 1);
		} else {
			pRomInfo->SelColorBrush = CreateSolidBrush(pRomInfo->SelColor);
		}
	}
	SortField = GetSortField();
	SortAscending = IsSortAscending();

	for (index = 0; index < NoOfFields; index++) {
		if (_stricmp(RomBrowserFields[index].Name,SortField) == 0) { break; }
	}
	if (NoOfFields == index) { SetSortField(""); return;  }
	ListView_SortItems(hRomList, RomList_CompareItems, RomBrowserFields[index].ID);
	
}

void LoadRomBrowserColoumnInfo (void) {
	LPSTR IniFileName;
	char String[200];
	int count;

	IniFileName = GetIniFileName();
	NoOfFields = sizeof(RomBrowserFields) / sizeof(RomBrowserFields[0]);
	for (count = 0; count < NoOfFields; count ++) {
		RomBrowserFields[count].Pos  = 
			_GetPrivateProfileInt("Rom Browser",RomBrowserFields[count].Name,
			RomBrowserFields[count].Pos,IniFileName);	
		sprintf(String,"%s.Width",RomBrowserFields[count].Name);
		RomBrowserFields[count].ColWidth = 
			_GetPrivateProfileInt("Rom Browser",String,RomBrowserFields[count].ColWidth,IniFileName);	
	}

	FixRomBrowserColoumnLang();
}

void FillRomExtensionInfo(ROM_INFO * pRomInfo) {
	LPSTR IniFileName, ExtIniFileName, NotesIniFileName;
	char Identifier[100], String[100];
	ROM_LIST_INFO * item;
	long * Fpos;
	int count;

	IniFileName = GetIniFileName();
	NotesIniFileName = GetNotesIniFileName();
	ExtIniFileName = GetExtIniFileName();

	sprintf(Identifier,"%08X-%08X-C:%X",pRomInfo->CRC1,pRomInfo->CRC2,pRomInfo->Country);

	Fpos = (long *)_GetPrivateProfileString(NULL,NULL,NULL,NULL,0,NULL);
	for (count = 0; count < RDBList.ListCount; count++) {
		item = &RDBList.List[count];
		if (item->CRC1 != pRomInfo->CRC1) { continue; }
		if (item->CRC2 != pRomInfo->CRC2) { continue; }
		if (item->Country != pRomInfo->Country) { continue; }
		*Fpos = item->Fpos;
		break;
	}
	pRomInfo->UserNotes[0]   = 0;
	pRomInfo->Developer[0]   = 0;
	pRomInfo->ReleaseDate[0] = 0;
	pRomInfo->Genre[0]       = 0;
	pRomInfo->Players        = 1;
	pRomInfo->CoreNotes[0]   = 0;
	pRomInfo->PluginNotes[0] = 0;
	strcpy(pRomInfo->GoodName,"Bad ROM? Use GoodN64 & check for updated INI");
	strcpy(pRomInfo->Status,Default_RomStatus);

	if (count != RDBList.ListCount) { 
		//Rom Notes
		if (RomBrowserFields[RB_UserNotes].Pos >= 0) {
			_GetPrivateProfileString(Identifier,"Note","",pRomInfo->UserNotes,sizeof(pRomInfo->UserNotes),NotesIniFileName);
		}

		//Rom Extension info
		if (RomBrowserFields[RB_Developer].Pos >= 0) {
			_GetPrivateProfileString(Identifier,"Developer","",pRomInfo->Developer,sizeof(pRomInfo->Developer),ExtIniFileName);
		}
		if (RomBrowserFields[RB_ReleaseDate].Pos >= 0) {
			_GetPrivateProfileString(Identifier,"ReleaseDate","",pRomInfo->ReleaseDate,sizeof(pRomInfo->ReleaseDate),ExtIniFileName);
		}
		if (RomBrowserFields[RB_Genre].Pos >= 0) {
			_GetPrivateProfileString(Identifier,"Genre","",pRomInfo->Genre,sizeof(pRomInfo->Genre),ExtIniFileName);
		}
		if (RomBrowserFields[RB_Players].Pos >= 0) {
			pRomInfo->Players = _GetPrivateProfileInt(Identifier,"Players",1,ExtIniFileName);
		}

		//Rom Settings
		if (RomBrowserFields[RB_GoodName].Pos >= 0) {
			_GetPrivateProfileString(Identifier,"Good Name",pRomInfo->GoodName,pRomInfo->GoodName,sizeof(pRomInfo->GoodName),IniFileName);
		}
		_GetPrivateProfileString(Identifier,"Status",pRomInfo->Status,pRomInfo->Status,sizeof(pRomInfo->Status),IniFileName);
		if (RomBrowserFields[RB_CoreNotes].Pos >= 0) {
			_GetPrivateProfileString(Identifier,"Core Note","",pRomInfo->CoreNotes,sizeof(pRomInfo->CoreNotes),IniFileName);
		}
		if (RomBrowserFields[RB_PluginNotes].Pos >= 0) {
			_GetPrivateProfileString(Identifier,"Plugin Note","",pRomInfo->PluginNotes,sizeof(pRomInfo->PluginNotes),IniFileName);
		}
	}

	_GetPrivateProfileString("Rom Status",pRomInfo->Status,"000000",String,7,IniFileName);	
	pRomInfo->TextColor = (AsciiToHex(String) & 0xFFFFFF);
	pRomInfo->TextColor = (pRomInfo->TextColor & 0x00FF00) | ((pRomInfo->TextColor >> 0x10) & 0xFF) | ((pRomInfo->TextColor & 0xFF) << 0x10);
	
	sprintf(String,"%s.Sel",pRomInfo->Status);
	_GetPrivateProfileString("Rom Status",String,"FFFFFFFF",String,9,IniFileName);	
	count = AsciiToHex(String);
	if (count < 0) { 
		pRomInfo->SelColor = - 1;
		pRomInfo->SelColorBrush = (HBRUSH)(COLOR_HIGHLIGHT + 1);
	} else {
		count = (AsciiToHex(String) & 0xFFFFFF);
		count = (count & 0x00FF00) | ((count >> 0x10) & 0xFF) | ((count & 0xFF) << 0x10);
		pRomInfo->SelColor = count;
		pRomInfo->SelColorBrush = CreateSolidBrush(count);
	}

	sprintf(String,"%s.Seltext",pRomInfo->Status);
	_GetPrivateProfileString("Rom Status",String,"FFFFFF",String,7,IniFileName);	
	pRomInfo->SelTextColor = (AsciiToHex(String) & 0xFFFFFF);
	pRomInfo->SelTextColor = (pRomInfo->SelTextColor & 0x00FF00) | ((pRomInfo->SelTextColor >> 0x10) & 0xFF) | ((pRomInfo->SelTextColor & 0xFF) << 0x10);
}

BOOL FillRomInfo(ROM_INFO * pRomInfo) {
	char drive[_MAX_DRIVE] ,dir[_MAX_DIR], ext[_MAX_EXT];
	BYTE RomData[0x1000];
	int count;
	
	if (RomBrowserFields[RB_CICChip].Pos >= 0) {
		if (!LoadDataFromRomFile(pRomInfo->szFullFileName,RomData,sizeof(RomData),&pRomInfo->RomSize)) { return FALSE; }
	} else {
		if (!LoadDataFromRomFile(pRomInfo->szFullFileName,RomData,0x40,&pRomInfo->RomSize)) { return FALSE; }
	}

	_splitpath( pRomInfo->szFullFileName, drive, dir, pRomInfo->FileName, ext );

	if (RomBrowserFields[RB_InternalName].Pos >= 0) {
		memcpy(pRomInfo->InternalName,(void *)(RomData + 0x20),20);
		for( count = 0 ; count < 20; count += 4 ) {
			pRomInfo->InternalName[count] ^= pRomInfo->InternalName[count+3];
			pRomInfo->InternalName[count + 3] ^= pRomInfo->InternalName[count];
			pRomInfo->InternalName[count] ^= pRomInfo->InternalName[count+3];			
			pRomInfo->InternalName[count + 1] ^= pRomInfo->InternalName[count + 2];
			pRomInfo->InternalName[count + 2] ^= pRomInfo->InternalName[count + 1];
			pRomInfo->InternalName[count + 1] ^= pRomInfo->InternalName[count + 2];			
		}
		pRomInfo->InternalName[21] = '\0';
	}
	pRomInfo->CartID[0] = *(RomData + 0x3F);
	pRomInfo->CartID[1] = *(RomData + 0x3E);
	pRomInfo->CartID[2] = '\0';
	pRomInfo->Manufacturer = *(RomData + 0x38);
	pRomInfo->Country = *(RomData + 0x3D);
	pRomInfo->CRC1 = *(DWORD *)(RomData + 0x10);
	pRomInfo->CRC2 = *(DWORD *)(RomData + 0x14);
	if (RomBrowserFields[RB_CICChip].Pos >= 0) {
		pRomInfo->CicChip = GetCicChipID(RomData);
	}
	
	FillRomExtensionInfo(pRomInfo);
	return TRUE;
}

int GetRomBrowserSize ( DWORD * nWidth, DWORD * nHeight ) {
	long lResult;
	HKEY hKeyResults = 0;
	char String[200];

	sprintf(String,"Software\\N64 Emulation\\%s\\Page Setup",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0, KEY_ALL_ACCESS,&hKeyResults);
	
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Value, Bytes = 4;

		lResult = RegQueryValueEx(hKeyResults,"Rom Browser Width",0,&Type,(LPBYTE)(&Value),&Bytes);
		if (Type == REG_DWORD && lResult == ERROR_SUCCESS) { 
			*nWidth = Value;
		} else {
			RegCloseKey(hKeyResults);
			return FALSE;
		}
	
		lResult = RegQueryValueEx(hKeyResults,"Rom Browser Height",0,&Type,(LPBYTE)(&Value),&Bytes);
		if (Type == REG_DWORD && lResult == ERROR_SUCCESS) { 
			*nHeight = Value;
		} else {
			RegCloseKey(hKeyResults);
			return FALSE;
		}
		RegCloseKey(hKeyResults);
		return TRUE;
	}
	return FALSE;
}

char * GetSortField ( void ) {
	static char String[200];
	long lResult;
	HKEY hKeyResults = 0;

	sprintf(String,"Software\\N64 Emulation\\%s\\Page Setup",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0, KEY_ALL_ACCESS,&hKeyResults);	
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Bytes = sizeof(String);

		lResult = RegQueryValueEx(hKeyResults,"Rom Browser Sort Field",0,&Type,(LPBYTE)String,&Bytes);
		if (lResult == ERROR_SUCCESS) { 
			RegCloseKey(hKeyResults);
			return String;
		}
		RegCloseKey(hKeyResults);
	}
	strcpy(String,"");
	return String;
}

void RefreshRomBrowser (void) {
	char RomDir[MAX_PATH+1], * SortField;
	int index;

	if (!hRomList) { return; }
	if (ItemList.ListAlloc != 0) {
		free(ItemList.List);
		ItemList.ListAlloc = 0;
		ItemList.ListCount = 0;
		ItemList.List = NULL;		
	}
	ListView_DeleteAllItems(hRomList);
	GetRomDirectory(RomDir);
	RDBList.ListAlloc = 0;
	ItemList.ListAlloc = 0;
	IndexRomDataBase();
	FillRomList (RomDir);	
	if (RDBList.ListAlloc != 0) {
		RDBList.ListAlloc = 0;
		free(RDBList.List);
		RDBList.List = NULL;
	}
	SaveRomList();

	SortField = GetSortField();
	SortAscending = IsSortAscending();

	for (index = 0; index < NoOfFields; index++) {
		if (_stricmp(RomBrowserFields[index].Name,SortField) == 0) { break; }
	}
	if (NoOfFields == index) { SetSortField(""); return;  }
	ListView_SortItems(hRomList, RomList_CompareItems, RomBrowserFields[index].ID);
}

void ResetRomBrowserColomuns (void) {
	int Coloumn, index;
	LV_COLUMN lvColumn;
	char szString[300];

	SaveRomBrowserColoumnInfo();
    memset(&lvColumn,0,sizeof(lvColumn));
	lvColumn.mask = LVCF_FMT;
	while (ListView_GetColumn(hRomList,0,&lvColumn)) {
		ListView_DeleteColumn(hRomList,0);
	}

	//Add Colomuns
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.pszText = szString;

	for (Coloumn = 0; Coloumn < NoOfFields; Coloumn ++) {
		for (index = 0; index < NoOfFields; index ++) {
			if (RomBrowserFields[index].Pos == Coloumn) { break; }
		}
		if (index == NoOfFields || RomBrowserFields[index].Pos != Coloumn) {
			FieldType[Coloumn] = -1;
			break;
		}
		FieldType[Coloumn] = RomBrowserFields[index].ID;
		lvColumn.cx = RomBrowserFields[index].ColWidth;
		strncpy(szString, RomBrowserFields[index].Name, sizeof(szString));
		ListView_InsertColumn(hRomList, Coloumn, &lvColumn);
	}
}

void ResizeRomListControl (WORD nWidth, WORD nHeight) {
	if (IsWindow(hRomList)) {
		if (IsWindow(hStatusWnd)) {
			RECT rc;

			GetWindowRect(hStatusWnd, &rc);
			nHeight -= (WORD)(rc.bottom - rc.top);
		}
		MoveWindow(hRomList, 0, 0, nWidth, nHeight, TRUE);
	}
}

void RomList_ColoumnSortList(LPNMLISTVIEW pnmv) {
	int index;

	for (index = 0; index < NoOfFields; index++) {
		if (RomBrowserFields[index].Pos == pnmv->iSubItem) { break; }
	}
	if (NoOfFields == index) { return; }
	if (_stricmp(GetSortField(),RomBrowserFields[index].Name) == 0) {
		SortAscending = SortAscending?FALSE:TRUE;
	} else {
		SetSortField (RomBrowserFields[index].Name);
		SortAscending = TRUE;
	}
	SetSortAscending();
	ListView_SortItems(hRomList, RomList_CompareItems, RomBrowserFields[index].ID);

}

int CALLBACK RomList_CompareItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	ROM_INFO * pRomInfo1 = &ItemList.List[SortAscending?lParam1:lParam2];
	ROM_INFO * pRomInfo2 = &ItemList.List[SortAscending?lParam2:lParam1];

	switch (lParamSort) {
	case RB_FileName: return (int)lstrcmpi(pRomInfo1->FileName, pRomInfo2->FileName);
	case RB_InternalName: return (int)lstrcmpi(pRomInfo1->InternalName, pRomInfo2->InternalName);
	case RB_GoodName: return (int)lstrcmpi(pRomInfo1->GoodName, pRomInfo2->GoodName);
	case RB_Status: return (int)lstrcmpi(pRomInfo1->Status, pRomInfo2->Status);
	case RB_RomSize: return (int)pRomInfo1->RomSize - (int)pRomInfo2->RomSize;
	case RB_CoreNotes: return (int)lstrcmpi(pRomInfo1->CoreNotes, pRomInfo2->CoreNotes);
	case RB_PluginNotes: return (int)lstrcmpi(pRomInfo1->PluginNotes, pRomInfo2->PluginNotes);
	case RB_UserNotes: return (int)lstrcmpi(pRomInfo1->UserNotes, pRomInfo2->UserNotes);
	case RB_CartridgeID: return (int)lstrcmpi(pRomInfo1->CartID, pRomInfo2->CartID);
	case RB_Manufacturer: return (int)pRomInfo1->Manufacturer - (int)pRomInfo2->Manufacturer;
	case RB_Country: return (int)pRomInfo1->Country - (int)pRomInfo2->Country;
	case RB_Developer: return (int)lstrcmpi(pRomInfo1->Developer, pRomInfo2->Developer);
	case RB_CRC1: return (int)pRomInfo1->CRC1 - (int)pRomInfo2->CRC1;
	case RB_CRC2: return (int)pRomInfo1->CRC2 - (int)pRomInfo2->CRC2;
	case RB_CICChip: return (int)pRomInfo1->CicChip - (int)pRomInfo2->CicChip;
	case RB_ReleaseDate: return (int)lstrcmpi(pRomInfo1->ReleaseDate, pRomInfo2->ReleaseDate);
	case RB_Players: return (int)pRomInfo1->Players - (int)pRomInfo2->Players;
	}
	return 0;
}
 

void RomList_DeleteItem(LPNMHDR pnmh) {
}

void RomList_GetDispInfo(LPNMHDR pnmh) {
	LV_DISPINFO * lpdi = (LV_DISPINFO *)pnmh;
	//ROM_INFO * pRomInfo = (ROM_INFO *)lpdi->item.lParam;
	ROM_INFO * pRomInfo = &ItemList.List[lpdi->item.lParam];

	switch(FieldType[lpdi->item.iSubItem]) {
	case RB_FileName: strncpy(lpdi->item.pszText, pRomInfo->FileName, lpdi->item.cchTextMax); break;
	case RB_InternalName: strncpy(lpdi->item.pszText, pRomInfo->InternalName, lpdi->item.cchTextMax); break;
	case RB_GoodName: strncpy(lpdi->item.pszText, pRomInfo->GoodName, lpdi->item.cchTextMax); break;
	case RB_CoreNotes: strncpy(lpdi->item.pszText, pRomInfo->CoreNotes, lpdi->item.cchTextMax); break;
	case RB_PluginNotes: strncpy(lpdi->item.pszText, pRomInfo->PluginNotes, lpdi->item.cchTextMax); break;
	case RB_Status: strncpy(lpdi->item.pszText, pRomInfo->Status, lpdi->item.cchTextMax); break;
	case RB_RomSize: sprintf(lpdi->item.pszText,"%.1f MBit",(float)pRomInfo->RomSize/0x20000); break;
	case RB_CartridgeID: strncpy(lpdi->item.pszText, pRomInfo->CartID, lpdi->item.cchTextMax); break;
	case RB_Manufacturer:
		switch (pRomInfo->Manufacturer) {
		case 'N':strncpy(lpdi->item.pszText, "Nintendo", lpdi->item.cchTextMax); break;
		case 0:  strncpy(lpdi->item.pszText, "None", lpdi->item.cchTextMax); break;
		default: sprintf(lpdi->item.pszText, "(Unknown %c (%X))", pRomInfo->Manufacturer,pRomInfo->Manufacturer); break;
		}
		break;		
	case RB_Country:
		switch (pRomInfo->Country) {
		case '7': strncpy(lpdi->item.pszText, "Beta", lpdi->item.cchTextMax); break;
		case 'A': strncpy(lpdi->item.pszText, "NTSC", lpdi->item.cchTextMax); break;
		case 'D': strncpy(lpdi->item.pszText, "Germany", lpdi->item.cchTextMax); break;
		case 'E': strncpy(lpdi->item.pszText, "America", lpdi->item.cchTextMax); break;
		case 'F': strncpy(lpdi->item.pszText, "France", lpdi->item.cchTextMax); break;
		case 'J': strncpy(lpdi->item.pszText, "Japan", lpdi->item.cchTextMax); break;
		case 'I': strncpy(lpdi->item.pszText, "Italy", lpdi->item.cchTextMax); break;
		case 'P': strncpy(lpdi->item.pszText, "Europe", lpdi->item.cchTextMax); break;
		case 'S': strncpy(lpdi->item.pszText, "Spain", lpdi->item.cchTextMax); break;
		case 'U': strncpy(lpdi->item.pszText, "Australia", lpdi->item.cchTextMax); break;
		case 'X': strncpy(lpdi->item.pszText, "PAL", lpdi->item.cchTextMax); break;
		case 'Y': strncpy(lpdi->item.pszText, "PAL", lpdi->item.cchTextMax); break;
		case 0: strncpy(lpdi->item.pszText, "None", lpdi->item.cchTextMax); break;
		default: sprintf(lpdi->item.pszText, "Unknown %c (%02X)", pRomInfo->Country,pRomInfo->Country); break;
		}
		break;			
	case RB_CRC1: sprintf(lpdi->item.pszText,"0x%08X",pRomInfo->CRC1); break;
	case RB_CRC2: sprintf(lpdi->item.pszText,"0x%08X",pRomInfo->CRC2); break;
	case RB_CICChip: 
		if (pRomInfo->CicChip < 0) { 
			sprintf(lpdi->item.pszText,"Unknown CIC Chip"); 
		} else {
			sprintf(lpdi->item.pszText,"CIC-NUS-610%d",pRomInfo->CicChip); 
		}
		break;
	case RB_UserNotes: strncpy(lpdi->item.pszText, pRomInfo->UserNotes, lpdi->item.cchTextMax); break;
	case RB_Developer: strncpy(lpdi->item.pszText, pRomInfo->Developer, lpdi->item.cchTextMax); break;
	case RB_ReleaseDate: strncpy(lpdi->item.pszText, pRomInfo->ReleaseDate, lpdi->item.cchTextMax); break;
	case RB_Genre: strncpy(lpdi->item.pszText, pRomInfo->Genre, lpdi->item.cchTextMax); break;
	case RB_Players: sprintf(lpdi->item.pszText,"%d",pRomInfo->Players); break;
	default: strncpy(lpdi->item.pszText, " ", lpdi->item.cchTextMax);
	}
	if (strlen(lpdi->item.pszText) == 0) { strcpy(lpdi->item.pszText," "); }
}

void RomList_OpenRom(LPNMHDR pnmh) {
	ROM_INFO * pRomInfo;
	LV_ITEM lvItem;
	DWORD ThreadID;
	LONG iItem;

	iItem = ListView_GetNextItem(hRomList, -1, LVNI_SELECTED);
	if (iItem == -1) { return; }

	memset(&lvItem, 0, sizeof(LV_ITEM));
	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = iItem;
	if (!ListView_GetItem(hRomList, &lvItem)) { return; }
	pRomInfo = &ItemList.List[lvItem.lParam];

	if (!pRomInfo) { return; }
	strcpy(CurrentFileName,pRomInfo->szFullFileName);
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)OpenChosenFile,NULL,0, &ThreadID);	
}

void RomListDrawItem (LPDRAWITEMSTRUCT ditem) {
	RECT rcItem, rcDraw;
	ROM_INFO * pRomInfo;
	char String[300];
	LV_ITEM lvItem;
	BOOL bSelected;
	HBRUSH hBrush;
    LV_COLUMN lvc; 
	int nColumn;

	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = ditem->itemID;
	if (!ListView_GetItem(hRomList, &lvItem)) { return; }
	lvItem.state = ListView_GetItemState(hRomList, ditem->itemID, -1);
	bSelected = (lvItem.state & LVIS_SELECTED);
	pRomInfo = &ItemList.List[lvItem.lParam];
	if (bSelected) {
		hBrush = pRomInfo->SelColorBrush;
		SetTextColor(ditem->hDC,pRomInfo->SelTextColor);
	} else {
		hBrush = (HBRUSH)(COLOR_WINDOW + 1);
		SetTextColor(ditem->hDC,pRomInfo->TextColor);
	}
	FillRect( ditem->hDC, &ditem->rcItem,hBrush);	
	SetBkMode( ditem->hDC, TRANSPARENT );
	
	//Draw
	ListView_GetItemRect(hRomList,ditem->itemID,&rcItem,LVIR_LABEL);
	ListView_GetItemText(hRomList,ditem->itemID, 0, String, sizeof(String)); 
	memcpy(&rcDraw,&rcItem,sizeof(RECT));
	rcDraw.right -= 3;
	DrawText(ditem->hDC, String, strlen(String), &rcDraw, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);	
	
    memset(&lvc,0,sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_WIDTH; 
	for(nColumn = 1; ListView_GetColumn(hRomList,nColumn,&lvc); nColumn += 1) {		
		rcItem.left = rcItem.right; 
        rcItem.right += lvc.cx; 

		ListView_GetItemText(hRomList,ditem->itemID, nColumn, String, sizeof(String)); 
		memcpy(&rcDraw,&rcItem,sizeof(RECT));
		rcDraw.right -= 3;
		DrawText(ditem->hDC, String, strlen(String), &rcDraw, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);
	}
}

void RomListNotify(LPNMHDR pnmh) {
	switch (pnmh->code) {
	case LVN_DELETEITEM:  RomList_DeleteItem(pnmh); break;
	case LVN_GETDISPINFO: RomList_GetDispInfo(pnmh); break;
	case LVN_COLUMNCLICK: RomList_ColoumnSortList((LPNMLISTVIEW)pnmh); break;
	case NM_RETURN:       RomList_OpenRom(pnmh); break;
	case NM_DBLCLK:       RomList_OpenRom(pnmh); break;
	}
}

BOOL RomListVisible(void) {
	if (hRomList == NULL) { return FALSE; }
	return (IsWindowVisible(hRomList));
}

void SaveRomBrowserColoumnInfo (void) {
	char szString[300], szName[200] ,szWidth[20];
	int Coloumn, index;
	LV_COLUMN lvColumn;
	LPSTR IniFileName;

	if (hRomList == NULL) { return; }
	IniFileName = GetIniFileName();

    memset(&lvColumn,0,sizeof(lvColumn));
	lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	lvColumn.pszText = szString;
	lvColumn.cchTextMax = sizeof(szString);
	for (Coloumn = 0;ListView_GetColumn(hRomList,Coloumn,&lvColumn); Coloumn++) {
		strcpy(szName,szString);
		strcat(szString,".Width");
		sprintf(szWidth,"%d",lvColumn.cx);
		_WritePrivateProfileString("Rom Browser",szString,szWidth,IniFileName);
		for (index = 0; index < NoOfFields; index ++) {
			if (strcmp(RomBrowserFields[index].Name,szName) == 0) {
				RomBrowserFields[index].ColWidth = lvColumn.cx;
				break; 
			}
		}
	}
}

void SaveRomList (void) {
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
	char fname[_MAX_FNAME],ext[_MAX_EXT];
	char FileName[_MAX_PATH];
	DWORD dwWritten;
	HANDLE hFile;
	int Size;

	GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
	_splitpath( path_buffer, drive, dir, fname, ext );
	sprintf(FileName,"%s%s%s",drive,dir,CacheFileName);
	
	hFile = CreateFile(FileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
	Size = sizeof(ROM_INFO);
	WriteFile(hFile,&Size,sizeof(Size),&dwWritten,NULL);
	WriteFile(hFile,&ItemList.ListCount,sizeof(ItemList.ListCount),&dwWritten,NULL);
	WriteFile(hFile,ItemList.List,Size * ItemList.ListCount,&dwWritten,NULL);
	CloseHandle(hFile);
}

void SelectRomDir (void) {
	char Buffer[MAX_PATH], Directory[255];
	LPITEMIDLIST pidl;
	BROWSEINFO bi;

	bi.hwndOwner = hMainWindow;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = Buffer;
	bi.lpszTitle = "Select current Rom Directory";
	bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = 0;
	if ((pidl = SHBrowseForFolder(&bi)) != NULL) {
		if (SHGetPathFromIDList(pidl, Directory)) {
			int len = strlen(Directory);

			if (Directory[len - 1] != '\\') {
				strcat(Directory,"\\");
			}
			SetRomDirectory(Directory, TRUE);
			RefreshRomBrowser();
		}
	}
}

void SetRomBrowserMaximized (BOOL Maximized) {
	long lResult;
	HKEY hKeyResults = 0;
	DWORD Disposition = 0;
	char String[200];

	sprintf(String,"Software\\N64 Emulation\\%s\\Page Setup",AppName);
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
	if (lResult == ERROR_SUCCESS) {
		RegSetValueEx(hKeyResults,"RomBrowser Maximized",0, REG_DWORD,(CONST BYTE *)(&Maximized),sizeof(DWORD));
	}
	RegCloseKey(hKeyResults);
}

void SetRomBrowserSize ( int nWidth, int nHeight ) {
	long lResult;
	HKEY hKeyResults = 0;
	DWORD Disposition = 0;
	char String[200];

	sprintf(String,"Software\\N64 Emulation\\%s\\Page Setup",AppName);
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
	if (lResult == ERROR_SUCCESS) {
		RegSetValueEx(hKeyResults,"Rom Browser Width",0, REG_DWORD,(CONST BYTE *)(&nWidth),sizeof(DWORD));
		RegSetValueEx(hKeyResults,"Rom Browser Height",0, REG_DWORD,(CONST BYTE *)(&nHeight),sizeof(DWORD));
	}
	RegCloseKey(hKeyResults);
}

void SetSortAscending (void) {
	long lResult;
	HKEY hKeyResults = 0;
	DWORD Disposition = 0;
	char String[200];

	sprintf(String,"Software\\N64 Emulation\\%s\\Page Setup",AppName);
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
	if (lResult == ERROR_SUCCESS) {
		RegSetValueEx(hKeyResults,"Sort Ascending",0, REG_DWORD,(CONST BYTE *)(&SortAscending),sizeof(DWORD));
	}
	RegCloseKey(hKeyResults);
}

void SetSortField (char * FieldName) {
	long lResult;
	HKEY hKeyResults = 0;
	DWORD Disposition = 0;
	char String[200];

	sprintf(String,"Software\\N64 Emulation\\%s\\Page Setup",AppName);
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
	if (lResult == ERROR_SUCCESS) {
		RegSetValueEx(hKeyResults,"Rom Browser Sort Field",0, REG_SZ,(CONST BYTE *)FieldName,strlen(FieldName));
	}
	RegCloseKey(hKeyResults);
}

void FillRomList (char * Directory) {
	char FullPath[MAX_PATH+1], FileName[MAX_PATH+1], SearchSpec[MAX_PATH+1];
	WIN32_FIND_DATA fd;
	HANDLE hFind;

	strcpy(SearchSpec,Directory);
	if (SearchSpec[strlen(Directory) - 1] != '\\') { strcat(SearchSpec,"\\"); }
	strcat(SearchSpec,"*.*");

	hFind = FindFirstFile(SearchSpec, &fd);
	if (hFind == INVALID_HANDLE_VALUE) { return; }
	do {
		char drive[_MAX_DRIVE] ,dir[_MAX_DIR], ext[_MAX_EXT];

		if (strcmp(fd.cFileName, ".") == 0) { continue; }
		if (strcmp(fd.cFileName, "..") == 0) { continue; }

		strcpy(FullPath,Directory);
		if (FullPath[strlen(Directory) - 1] != '\\') { strcat(FullPath,"\\"); }
		strcat(FullPath,fd.cFileName);
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			if (Rercursion) { FillRomList(FullPath); }
			continue;
		}
		_splitpath( FullPath, drive, dir, FileName, ext );
		if (_stricmp(ext, ".zip") == 0) { AddRomToList(FullPath); continue; }
		if (_stricmp(ext, ".v64") == 0) { AddRomToList(FullPath); continue; }
		if (_stricmp(ext, ".z64") == 0) { AddRomToList(FullPath); continue; }
		if (_stricmp(ext, ".n64") == 0) { AddRomToList(FullPath); continue; }
		if (_stricmp(ext, ".rom") == 0) { AddRomToList(FullPath); continue; }
		if (_stricmp(ext, ".jap") == 0) { AddRomToList(FullPath); continue; }
		if (_stricmp(ext, ".pal") == 0) { AddRomToList(FullPath); continue; }
		if (_stricmp(ext, ".usa") == 0) { AddRomToList(FullPath); continue; }
		if (_stricmp(ext, ".eur") == 0) { AddRomToList(FullPath); continue; }
		if (_stricmp(ext, ".bin") == 0) { AddRomToList(FullPath); continue; }
	} while (FindNextFile(hFind, &fd));

	FindClose(hFind);
}

void ShowRomList(HWND hParent) {
	DWORD X, Y, Width, Height;
	long Style;
	int iItem;

	if (hCPU != NULL) { return; }
	if (hRomList != NULL && IsWindowVisible(hRomList)) { return; }

	IgnoreMove = TRUE;
	ShowWindow(hMainWindow,SW_HIDE);
	SetupPlugins(hHiddenWin);
	if (hRomList == NULL) {
		CreateRomListControl(hParent);
	} else {
		EnableWindow(hRomList,TRUE);
	}
	if (GetRomBrowserSize(&Width,&Height)) {
		ChangeWinSize ( hMainWindow, Width, Height, NULL );
	} else {
		ChangeWinSize ( hMainWindow, 640, 480, hStatusWnd );
	}
	iItem = ListView_GetNextItem(hRomList, -1, LVNI_SELECTED);
	ListView_EnsureVisible(hRomList,iItem,FALSE);

	ShowWindow(hRomList,SW_SHOW);
	InvalidateRect(hParent,NULL,TRUE);
	if (GetStoredWinPos( "Main.RomList", &X, &Y ) ) {
		SetWindowPos(hMainWindow,NULL,X,Y,0,0, SWP_NOZORDER | SWP_NOSIZE);		 
	}		
	Style = GetWindowLong(hMainWindow,GWL_STYLE) | WS_SIZEBOX | WS_MAXIMIZEBOX;
	SetWindowLong(hMainWindow,GWL_STYLE,Style);
	if (IsRomBrowserMaximized()) { ShowWindow(hMainWindow,SW_MAXIMIZE); }
	ShowWindow(hMainWindow,SW_SHOW);
	IgnoreMove = FALSE;
	SetFocus(hRomList);
}
