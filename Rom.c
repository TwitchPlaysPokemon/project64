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
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "main.h"
#include "cpu.h"
#include "memory.h"
#include "debugger.h"
#include "plugin.h"
#include "unzip.h"
#include "resource.h" 

#define MenuLocOfUsedFiles	9
#define MenuLocOfUsedDirs	(MenuLocOfUsedFiles + 1)

DWORD RomFileSize, RomRamSize, RomSaveUsing, RomCPUType, RomSelfMod, 
	RomUseTlb, RomUseLinking, RomCF, RomUseLargeBuffer, RomUseCache,
	RomReadRomDirect;
char CurrentFileName[MAX_PATH+1], RomName[MAX_PATH+1], RomHeader[64];
char LastRoms[10][MAX_PATH+1], LastDirs[10][MAX_PATH+1];

BOOL IsValidRomImage ( BYTE Test[4] );

void AddRecentDir(HWND hWnd, char * addition) {
	char * LangFile, Language[100];
	HMENU hMenu, hSubMenu;
	MENUITEMINFO menuinfo;
	char String[256];
	DWORD count;

	memset(&menuinfo, 0, sizeof(MENUITEMINFO));
	menuinfo.cbSize = sizeof(MENUITEMINFO);
	menuinfo.fMask = MIIM_TYPE|MIIM_ID;
	menuinfo.fType = MFT_STRING;
	menuinfo.fState = MFS_ENABLED;
	menuinfo.dwTypeData = String;
	menuinfo.cch = 256;


    hMenu = GetMenu(hWnd);
	hSubMenu = GetSubMenu(hMenu,0);
	DeleteMenu(hSubMenu, MenuLocOfUsedDirs, MF_BYPOSITION);
	sprintf(String,"None");
	InsertMenuItem(hSubMenu, MenuLocOfUsedDirs, TRUE, &menuinfo);
	hSubMenu = CreateMenu();

	if (addition != NULL && RomDirsToRemember > 0) {
		char Dir[MAX_PATH+1];
		BOOL bFound = FALSE;

		strcpy(Dir,addition);
		for (count = 0; count < RomDirsToRemember && !bFound; count ++ ) {
			if (strcmp(addition, LastDirs[count]) == 0) { 
				if (count != 0) {
					memmove(&LastDirs[1],&LastDirs[0],sizeof(LastDirs[0]) * count);
				}
				bFound = TRUE;
			}
		}

		if (bFound == FALSE) { memmove(&LastDirs[1],&LastDirs[0],sizeof(LastDirs[0]) * (RomDirsToRemember - 1)); }
		strcpy(LastDirs[0],Dir);
		SaveRecentDirs();
	}
		
	if (strlen(LastDirs[0]) == 0) { 
		menuinfo.wID = ID_FILE_RECENT_DIR;
		sprintf(String,"None");
		InsertMenuItem(hSubMenu, 0, TRUE, &menuinfo);
	}
	menuinfo.fMask = MIIM_TYPE|MIIM_ID;
	for (count = 0; count < RomDirsToRemember; count ++ ) {
		if (strlen(LastDirs[count]) == 0) { break; }
		menuinfo.wID = ID_FILE_RECENT_DIR + count;
		sprintf(String,"&%d %s",(count + 1) % 10,LastDirs[count]);
		InsertMenuItem(hSubMenu, count, TRUE, &menuinfo);
	}
	LangFile = GetLangFileName();
	_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);
	ModifyMenu(GetSubMenu(hMenu,0),MenuLocOfUsedDirs,MF_POPUP|MF_BYPOSITION,(DWORD)hSubMenu,"Recent Rom Directories");
	SetupMenuTitle(GetSubMenu(hMenu,0),MenuLocOfUsedDirs,NULL,"FileItem9",Language,LangFile);
	if (strlen(LastDirs[0]) == 0) { 
		EnableMenuItem(GetSubMenu(hMenu,0),MenuLocOfUsedDirs,MF_BYPOSITION|MFS_DISABLED);
	} else {
		EnableMenuItem(GetSubMenu(hMenu,0),MenuLocOfUsedDirs,MF_BYPOSITION|MFS_ENABLED);
	}
}

void AddRecentFile(HWND hWnd, char * addition) {
	char * LangFile, Language[100];
	HMENU hMenu, hSubMenu;
	MENUITEMINFO menuinfo;
	char String[256];
	DWORD count;

	memset(&menuinfo, 0, sizeof(MENUITEMINFO));
	menuinfo.cbSize = sizeof(MENUITEMINFO);
	menuinfo.fMask = MIIM_TYPE|MIIM_ID;
	menuinfo.fType = MFT_STRING;
	menuinfo.fState = MFS_ENABLED;
	menuinfo.dwTypeData = String;
	menuinfo.cch = 256;


    hMenu = GetMenu(hWnd);
	hSubMenu = GetSubMenu(hMenu,0);
	DeleteMenu(hSubMenu, MenuLocOfUsedFiles, MF_BYPOSITION);
	sprintf(String,"None");
	InsertMenuItem(hSubMenu, MenuLocOfUsedFiles, TRUE, &menuinfo);
	hSubMenu = CreateMenu();

	if (addition != NULL && RomsToRemember > 0) {
		char Rom[MAX_PATH+1];
		BOOL bFound = FALSE;

		strcpy(Rom,addition);
		for (count = 0; count < RomsToRemember && !bFound; count ++ ) {
			if (strcmp(addition, LastRoms[count]) == 0) { 
				if (count != 0) {
					memmove(&LastRoms[1],&LastRoms[0],sizeof(LastRoms[0]) * count);
				}
				bFound = TRUE;
			}
		}

		if (bFound == FALSE) { memmove(&LastRoms[1],&LastRoms[0],sizeof(LastRoms[0]) * (RomsToRemember - 1)); }
		strcpy(LastRoms[0],Rom);
		SaveRecentFiles();
	}
		
	if (strlen(LastRoms[0]) == 0) { 
		menuinfo.wID = ID_FILE_RECENT_DIR;
		sprintf(String,"None");
		InsertMenuItem(hSubMenu, 0, TRUE, &menuinfo);
	}
	menuinfo.fMask = MIIM_TYPE|MIIM_ID;
	for (count = 0; count < RomsToRemember; count ++ ) {
		if (strlen(LastRoms[count]) == 0) { break; }
		menuinfo.wID = ID_FILE_RECENT_FILE + count;
		sprintf(String,"&%d %s",(count + 1) % 10,LastRoms[count]);
		InsertMenuItem(hSubMenu, count, TRUE, &menuinfo);
	}

	LangFile = GetLangFileName();
	_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);
	ModifyMenu(GetSubMenu(hMenu,0),MenuLocOfUsedFiles,MF_POPUP|MF_BYPOSITION,(DWORD)hSubMenu,"Recent Rom");
	SetupMenuTitle(GetSubMenu(hMenu,0),MenuLocOfUsedFiles,NULL,"FileItem8",Language,LangFile);
	if (strlen(LastRoms[0]) == 0) { 
		EnableMenuItem(GetSubMenu(hMenu,0),MenuLocOfUsedFiles,MF_BYPOSITION|MFS_DISABLED);
	} else {
		EnableMenuItem(GetSubMenu(hMenu,0),MenuLocOfUsedFiles,MF_BYPOSITION|MFS_ENABLED);
	}
}

void ByteSwapRom (void) {
	DWORD count;

	SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"Byte swapping image" );
	switch (*((DWORD *)&ROM[0])) {
	case 0x12408037:
		for( count = 0 ; count < RomFileSize; count += 4 ) {
			ROM[count] ^= ROM[count+2];
			ROM[count + 2] ^= ROM[count];
			ROM[count] ^= ROM[count+2];			
			ROM[count + 1] ^= ROM[count + 3];
			ROM[count + 3] ^= ROM[count + 1];
			ROM[count + 1] ^= ROM[count + 3];			
		}
		break;
	case 0x40123780:
		for( count = 0 ; count < RomFileSize; count += 4 ) {
			ROM[count] ^= ROM[count+3];
			ROM[count + 3] ^= ROM[count];
			ROM[count] ^= ROM[count+3];			
			ROM[count + 1] ^= ROM[count + 2];
			ROM[count + 2] ^= ROM[count + 1];
			ROM[count + 1] ^= ROM[count + 2];			
		}
		break;
	case 0x80371240: break;
	default:
		DisplayError("ByteSwapRom: %X",ROM[0]);
	}
}

int ChooseN64RomToOpen ( void ) {
	OPENFILENAME openfilename;
	char FileName[256],Directory[255];

	memset(&FileName, 0, sizeof(FileName));
	memset(&openfilename, 0, sizeof(openfilename));

	GetRomDirectory( Directory );

	openfilename.lStructSize  = sizeof( openfilename );
	openfilename.hwndOwner    = hMainWindow;
	openfilename.lpstrFilter  = "N64 ROMs (*.zip, *.?64, *.rom, *.usa, *.jap, *.eur, *.bin)\0*.?64;*.zip;*.bin;*.rom;*.usa;*.jap;*.eur\0All files (*.*)\0*.*\0";
	openfilename.lpstrFile    = FileName;
	openfilename.lpstrInitialDir    = Directory;
	openfilename.nMaxFile     = MAX_PATH;
	openfilename.Flags        = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (GetOpenFileName (&openfilename)) {							
		char drive[_MAX_DRIVE] ,dir[_MAX_DIR];
		char fname[_MAX_FNAME],ext[_MAX_EXT];
		_splitpath( FileName, drive, dir, fname, ext );
		sprintf(Directory,"%s%s",drive,dir);
		SetRomDirectory( Directory, FALSE );
		strcpy(CurrentFileName,FileName);
		return TRUE;
	}
	
	return FALSE;
}

void EnableOpenMenuItems (void) {
	HMENU hMenu = GetMenu(hMainWindow);	
	int count;

	EnableMenuItem(hMenu,ID_FILE_OPEN_ROM,MFS_ENABLED|MF_BYCOMMAND);
	for (count = 0; count < (int)RomsToRemember; count ++ ) {
		if (strlen(LastRoms[count]) == 0) { break; }
		EnableMenuItem(hMenu,ID_FILE_RECENT_FILE + count,MFS_ENABLED|MF_BYCOMMAND);
	}
}

void GetRomDirectory ( char * Directory ) {
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
	char fname[_MAX_FNAME],ext[_MAX_EXT];
	char Dir[255], Group[200];
	long lResult;
	HKEY hKeyResults = 0;
	
	GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
	_splitpath( path_buffer, drive, dir, fname, ext );

	sprintf(Group,"Software\\N64 Emulation\\%s",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,Group,0,KEY_ALL_ACCESS,
		&hKeyResults);
	sprintf(Directory,"%s%s",drive,dir);

	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Bytes = sizeof(Dir);
		lResult = RegQueryValueEx(hKeyResults,"Rom Directory",0,&Type,(LPBYTE)Dir,&Bytes);
		if (lResult == ERROR_SUCCESS) { strcpy(Directory,Dir); }
	}
	RegCloseKey(hKeyResults);	
}

BOOL IsValidRomImage ( BYTE Test[4] ) {
	if ( *((DWORD *)&Test[0]) == 0x40123780 ) { return TRUE; }
	if ( *((DWORD *)&Test[0]) == 0x12408037 ) { return TRUE; }
	if ( *((DWORD *)&Test[0]) == 0x80371240 ) { return TRUE; }
	return FALSE;
}

BOOL LoadDataFromRomFile(char * FileName,BYTE * Data,int DataLen, int * RomSize) {
	BYTE Test[4];
	int count;

	if (strnicmp(&FileName[strlen(FileName)-4], ".ZIP",4) == 0 ){ 
		int len, port = 0, FoundRom;
	    unz_file_info info;
		char zname[132];
		unzFile file;
		file = unzOpen(FileName);
		if (file == NULL) { return FALSE; }

		port = unzGoToFirstFile(file);
		FoundRom = FALSE; 
		while(port == UNZ_OK && FoundRom == FALSE) {
			unzGetCurrentFileInfo(file, &info, zname, 128, NULL,0, NULL,0);
		    if (unzLocateFile(file, zname, 1) != UNZ_OK ) {
				unzClose(file);
				return FALSE;
			}
			if( unzOpenCurrentFile(file) != UNZ_OK ) {
				unzClose(file);
				return FALSE;
			}
			unzReadCurrentFile(file,Test,4);
			if (IsValidRomImage(Test)) {
				FoundRom = TRUE;
				RomFileSize = info.uncompressed_size;
				memcpy(Data,Test,4);
				len = unzReadCurrentFile(file,&Data[4],DataLen - 4) + 4;

				if ((int)DataLen != len) {
					unzCloseCurrentFile(file);
					unzClose(file);
					return FALSE;
				}
				*RomSize = info.uncompressed_size;
				if(unzCloseCurrentFile(file) == UNZ_CRCERROR) {
					unzClose(file);
					return FALSE;
				}
				unzClose(file);
			}
			if (FoundRom == FALSE) {
				unzCloseCurrentFile(file);
				port = unzGoToNextFile(file);
			}

		}
		if (FoundRom == FALSE) {
			return FALSE;
		}
	} else {
		DWORD dwRead;
		HANDLE hFile;
		
		hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,
			OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
			NULL);
		
		if (hFile == INVALID_HANDLE_VALUE) {  return FALSE; }

		SetFilePointer(hFile,0,0,FILE_BEGIN);
		ReadFile(hFile,Test,4,&dwRead,NULL);
		if (!IsValidRomImage(Test)) { CloseHandle( hFile ); return FALSE; }
		SetFilePointer(hFile,0,0,FILE_BEGIN);
		if (!ReadFile(hFile,Data,DataLen,&dwRead,NULL)) { CloseHandle( hFile ); return FALSE; }
		*RomSize = GetFileSize(hFile,NULL);
		CloseHandle( hFile ); 		
	}	

	switch (*((DWORD *)&Data[0])) {
	case 0x12408037:
		for( count = 0 ; count < DataLen; count += 4 ) {
			Data[count] ^= Data[count+2];
			Data[count + 2] ^= Data[count];
			Data[count] ^= Data[count+2];			
			Data[count + 1] ^= Data[count + 3];
			Data[count + 3] ^= Data[count + 1];
			Data[count + 1] ^= Data[count + 3];			
		}
		break;
	case 0x40123780:
		for( count = 0 ; count < DataLen; count += 4 ) {
			Data[count] ^= Data[count+3];
			Data[count + 3] ^= Data[count];
			Data[count] ^= Data[count+3];			
			Data[count + 1] ^= Data[count + 2];
			Data[count + 2] ^= Data[count + 1];
			Data[count + 1] ^= Data[count + 2];			
		}
		break;
	case 0x80371240: break;
	}
	return TRUE;
}

void LoadRecentDirs (HWND hParent) {
	HKEY hKeyResults = 0;
	char String[256];
	long lResult;
	DWORD count;

	sprintf(String,"Software\\N64 Emulation\\%s",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0,KEY_ALL_ACCESS,&hKeyResults);
	
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Bytes;

		for (count = 0; count < RomDirsToRemember; count++) {
			Bytes = sizeof(LastDirs[count]);
			sprintf(String,"RecentDir%d",count+1);			
			lResult = RegQueryValueEx(hKeyResults,String,0,&Type,(LPBYTE)LastDirs[count],&Bytes);
			if (lResult != ERROR_SUCCESS) { 
				memset(LastDirs[count],0,sizeof(LastDirs[count]));
				break;
			}
		}
		RegCloseKey(hKeyResults);	
	}
	AddRecentDir(hParent,NULL);
}

void LoadRecentFiles (HWND hParent) {
	HKEY hKeyResults = 0;
	char String[256];
	long lResult;
	DWORD count;

	sprintf(String,"Software\\N64 Emulation\\%s",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0,KEY_ALL_ACCESS,&hKeyResults);
	
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Bytes;

		for (count = 0; count < RomsToRemember; count++) {
			Bytes = sizeof(LastRoms[count]);
			sprintf(String,"RecentFile%d",count+1);			
			lResult = RegQueryValueEx(hKeyResults,String,0,&Type,(LPBYTE)LastRoms[count],&Bytes);
			if (lResult != ERROR_SUCCESS) { 
				memset(LastRoms[count],0,sizeof(LastRoms[count]));
				break;
			}
		}
		RegCloseKey(hKeyResults);	
	}
	AddRecentFile(hParent,NULL);
}

void LoadRecentRom (DWORD Index) {
	DWORD ThreadID;

	Index -= ID_FILE_RECENT_FILE;
	if (Index < 0 || Index > RomsToRemember) { return; }
	strcpy(CurrentFileName,LastRoms[Index]);
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)OpenChosenFile,NULL,0, &ThreadID);	
}


void LoadRomOptions ( void ) {
	DWORD NewRamSize;

	ReadRomOptions();

	NewRamSize = RomRamSize;
	if ((int)RomRamSize < 0) { NewRamSize = SystemRdramSize; }

	if (RomReadRomDirect) {
		if(VirtualAlloc(N64MEM + 0x10000000, RomFileSize, MEM_COMMIT, PAGE_READWRITE)==NULL) {
			DisplayError("Not enough memory for N64MEM!");
			ExitThread(0);
		}
		memcpy(N64MEM + 0x10000000,ROM,RomFileSize);
		if (VirtualAlloc((BYTE *)DelaySlotTable + (0x10000000 >> 0xA), (RomFileSize >> 0xA), MEM_COMMIT, PAGE_READWRITE)==NULL) {
			DisplayError("Failed to Extend Delay Slot Table to 8mb");
			ExitThread(0);
		}
	}

	if (RomUseLargeBuffer) {
		if (VirtualAlloc(RecompCode, LargeCompileBufferSize, MEM_COMMIT, PAGE_READWRITE)==NULL) {
			DisplayError("Failed to allocate for Recomp Buffer");
			ExitThread(0);
		}
	} else {
		VirtualFree(RecompCode, LargeCompileBufferSize,MEM_DECOMMIT);
		if (VirtualAlloc(RecompCode, NormalCompileBufferSize, MEM_COMMIT, PAGE_READWRITE)==NULL) {
			DisplayError("Failed to Recomp Buffer");
			ExitThread(0);
		}
	}
	if (NewRamSize != RdramSize) {
		if (RdramSize == 0x400000) { 
			if (VirtualAlloc(N64MEM + 0x400000, 0x400000, MEM_COMMIT, PAGE_READWRITE)==NULL) {
				DisplayError("Failed to Extend memory to 8mb");
				ExitThread(0);
			}
			if (VirtualAlloc((BYTE *)JumpTable + 0x400000, 0x400000, MEM_COMMIT, PAGE_READWRITE)==NULL) {
				DisplayError("Failed to Extend Jump Table to 8mb");
				ExitThread(0);
			}
			if (VirtualAlloc((BYTE *)DelaySlotTable + (0x400000 >> 0xA), (0x400000 >> 0xA), MEM_COMMIT, PAGE_READWRITE)==NULL) {
				DisplayError("Failed to Extend Delay Slot Table to 8mb");
				ExitThread(0);
			}
		} else {
			VirtualFree(N64MEM + 0x400000, 0x400000,MEM_DECOMMIT);
			VirtualFree((BYTE *)JumpTable + 0x400000, 0x400000,MEM_DECOMMIT);
			VirtualFree((BYTE *)DelaySlotTable + (0x400000 >> 0xA), (0x400000 >> 0xA),MEM_DECOMMIT);
		}
	}
	RdramSize = NewRamSize;
	CPU_Type = SystemCPU_Type;
	if (RomCPUType != CPU_Default) { CPU_Type = RomCPUType; }
	CountPerOp = RomCF;
	if (CountPerOp < 1)  { CountPerOp = Default_CountPerOp; }
	if (CountPerOp > 6)  { CountPerOp = Default_CountPerOp; }
	
	SaveUsing = RomSaveUsing;
	SelfModCheck = SystemSelfModCheck;
	if (RomSelfMod != ModCode_Default) { SelfModCheck = RomSelfMod; }
	UseTlb = RomUseTlb;
	UseLinking = SystemABL;
	DisableRegCaching = !RomUseCache;
	if (UseIni && RomUseLinking == 0 ) { UseLinking = TRUE; }
	if (UseIni && RomUseLinking == 1 ) { UseLinking = FALSE; }
	switch (*(ROM + 0x3D)) {
	case 0x44: //Germany
	case 0x46: //french
	case 0x49: //Italian
	case 0x50: //Europe
	case 0x53: //Spanish
	case 0x55: //Australia
	case 0x58: // X (PAL)
	case 0x59: // X (PAL)
		Timer_Initialize((double)50);
		break;
	case 0x37: // 7 (Beta)
	case 0x41: // A (NTSC)
	case 0x45: //USA
	case 0x4A: //Japan
	default:
		Timer_Initialize((double)60);
		break;
	}
}

void RemoveRecentDirList (HWND hWnd) {
	HMENU hMenu;
	DWORD count;

    hMenu = GetMenu(hWnd);
	for (count = 0; count < RomDirsToRemember; count ++ ) {
		DeleteMenu(hMenu, ID_FILE_RECENT_DIR + count, MF_BYCOMMAND);
	}
	memset(LastRoms[0],0,sizeof(LastRoms[0]));
}

void RemoveRecentList (HWND hWnd) {
	HMENU hMenu;
	DWORD count;

    hMenu = GetMenu(hWnd);
	for (count = 0; count < RomsToRemember; count ++ ) {
		DeleteMenu(hMenu, ID_FILE_RECENT_FILE + count, MF_BYCOMMAND);
	}
	memset(LastRoms[0],0,sizeof(LastRoms[0]));
}

void ReadRomOptions (void) {
	RomRamSize        = -1;
	RomSaveUsing      = Auto;
	RomCF             = -1;
	RomCPUType        = CPU_Default;
	RomSelfMod        = ModCode_Default;
	RomUseTlb         = TRUE;
	RomUseCache       = TRUE;
	RomUseLargeBuffer = FALSE;
	RomReadRomDirect  = FALSE;
	RomUseLinking     = -1;

	if (strlen(RomName) != 0) {
		char Identifier[100];
		LPSTR IniFileName;
		char String[100];

		IniFileName = GetIniFileName();
		sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
	
		if (UseIni) { RomRamSize = _GetPrivateProfileInt(Identifier,"RDRAM Size",-1,IniFileName); }
		if (RomRamSize == 4 || RomRamSize == 8) { 
			RomRamSize *= 0x100000; 
		} else {
			RomRamSize = -1; 
		}

		RomCF = _GetPrivateProfileInt(Identifier,"Counter Factor",-1,IniFileName);
		if (RomCF > 6) { RomCF = -1; }

		_GetPrivateProfileString(Identifier,"Save Type","",String,sizeof(String),IniFileName);
		if (strcmp(String,"4kbit Eeprom") == 0)       { RomSaveUsing = Eeprom_4K; } 
		else if (strcmp(String,"16kbit Eeprom") == 0) { RomSaveUsing = Eeprom_16K; } 
		else if (strcmp(String,"Sram") == 0)          { RomSaveUsing = Sram; } 
		else if (strcmp(String,"FlashRam") == 0)      { RomSaveUsing = FlashRam; } 
		else                                          { RomSaveUsing = Auto; } 

		if (UseIni) {
			_GetPrivateProfileString(Identifier,"CPU Type","",String,sizeof(String),IniFileName);
			if (strcmp(String,"Interpreter") == 0)       { RomCPUType = CPU_Interpreter; } 
			else if (strcmp(String,"Recompiler") == 0)   { RomCPUType = CPU_Recompiler; } 
			else if (strcmp(String,"SyncCores") == 0)    { RomCPUType = CPU_SyncCores; } 
			else                                         { RomCPUType = CPU_Default; } 

			_GetPrivateProfileString(Identifier,"Self-modifying code Method","",String,sizeof(String),IniFileName);
			if (strcmp(String,"None") == 0)                      { RomSelfMod = ModCode_None; } 
			else if (strcmp(String,"Cache") == 0)                { RomSelfMod = ModCode_Cache; } 
			else if (strcmp(String,"Protected Memory") == 0)     { RomSelfMod = ModCode_ProtectedMemory; } 
			else if (strcmp(String,"Check Memory") == 0)         { RomSelfMod = ModCode_CheckMemoryCache; } 
			else if (strcmp(String,"Check Memory & cache") == 0) { RomSelfMod = ModCode_CheckMemoryCache; } 
			else if (strcmp(String,"Change Memory") == 0)        { RomSelfMod = ModCode_ChangeMemory; } 
			else                                                 { RomSelfMod = ModCode_Default; } 
		}
		_GetPrivateProfileString(Identifier,"Use TLB","",String,sizeof(String),IniFileName);
		if (strcmp(String,"No") == 0) { RomUseTlb = FALSE; } 
		_GetPrivateProfileString(Identifier,"Reg Cache","",String,sizeof(String),IniFileName);
		if (strcmp(String,"No") == 0) { RomUseCache = FALSE; } 
		_GetPrivateProfileString(Identifier,"Use Large Buffer","",String,sizeof(String),IniFileName);
		if (strcmp(String,"Yes") == 0) { RomUseLargeBuffer = TRUE; } 
		_GetPrivateProfileString(Identifier,"Linking","",String,sizeof(String),IniFileName);
		if (strcmp(String,"On") == 0) { RomUseLinking = 0; } 
		if (strcmp(String,"Off") == 0) { RomUseLinking = 1; } 
	}
}

void OpenN64Image ( void ) {
	DWORD ThreadID;
	HMENU hMenu;

	hMenu = GetMenu(hMainWindow);
	EnableMenuItem(hMenu,ID_FILE_OPEN_ROM,MFS_DISABLED|MF_BYCOMMAND);
	SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"Choosing N64 image" );
	if (ChooseN64RomToOpen()) {
		CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)OpenChosenFile,NULL,0, &ThreadID);	
	} else {
		EnableOpenMenuItems();
	}
}

void OpenChosenFile ( void ) {
#define ReadFromRomSection	0x400000
	char drive[_MAX_DRIVE] ,FileName[_MAX_DIR],dir[_MAX_DIR], ext[_MAX_EXT];
	char WinTitle[70], MapFile[_MAX_PATH];
	char Message[100];
	BYTE Test[4];
	int count;

	EnableMenuItem(hMainMenu,ID_FILE_OPEN_ROM,MFS_DISABLED|MF_BYCOMMAND);
	for (count = 0; count < (int)RomsToRemember; count ++ ) {
		if (strlen(LastRoms[count]) == 0) { break; }
		EnableMenuItem(hMainMenu,ID_FILE_RECENT_FILE + count,MFS_DISABLED|MF_BYCOMMAND);
	}
	HideRomBrowser();
	Sleep(1000);
	CloseCpu();
#if (!defined(EXTERNAL_RELEASE))
	ResetMappings();
#endif
	strcpy(MapFile,CurrentFileName);

	if (strnicmp(&CurrentFileName[strlen(CurrentFileName)-4], ".ZIP",4) == 0 ){ 
		int len, port = 0, FoundRom;
	    unz_file_info info;
		char zname[132];
		unzFile file;
		file = unzOpen(CurrentFileName);
		if (file == NULL) {
			DisplayError("Zip Error: unzOpen(%s) failed\n",CurrentFileName);
			EnableOpenMenuItems();			
			ShowRomList(hMainWindow);
			return;
		}

		port = unzGoToFirstFile(file);
		FoundRom = FALSE; 
		while(port == UNZ_OK && FoundRom == FALSE) {
			unzGetCurrentFileInfo(file, &info, zname, 128, NULL,0, NULL,0);
		    if (unzLocateFile(file, zname, 1) != UNZ_OK ) {
				unzClose(file);
				DisplayError("Zip Error: unzLocateFile() failed\n");
				EnableOpenMenuItems();			
				ShowRomList(hMainWindow);
				return;
			}
			if( unzOpenCurrentFile(file) != UNZ_OK ) {
				unzClose(file);
				DisplayError("Error in zip file\n");
				EnableOpenMenuItems();			
				ShowRomList(hMainWindow);
				return;
			}
			unzReadCurrentFile(file,Test,4);
			if (IsValidRomImage(Test)) {
				FoundRom = TRUE;
				RomFileSize = info.uncompressed_size;
				if (!Allocate_ROM()) {
					unzCloseCurrentFile(file);
					unzClose(file);
					DisplayError("Not enough memory for rom");
					EnableOpenMenuItems();			
					ShowRomList(hMainWindow);
					return;
				}
				memcpy(ROM,Test,4);
				//len = unzReadCurrentFile(file,&ROM[4],RomFileSize - 4) + 4;
				len = 4;
				for (count = 4; count < (int)RomFileSize; count += ReadFromRomSection) {
					len += unzReadCurrentFile(file,&ROM[count],ReadFromRomSection);
					sprintf(Message,"Loaded: %.2f%c",((float)len/(float)RomFileSize) * 100.0f,'%');
					SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)Message );
					Sleep(100);
				}
				if ((int)RomFileSize != len) {
					unzCloseCurrentFile(file);
					unzClose(file);
					switch (len) {
					case UNZ_ERRNO:
						DisplayError("Errno:...");
						break;
					case UNZ_EOF:
						DisplayError("Unexpected end of file.");
						break;
					case UNZ_PARAMERROR:
						DisplayError("Parameter error.");
						break;
					case UNZ_BADZIPFILE:
						DisplayError("Bad zipfile.");
						break;
					case UNZ_INTERNALERROR:
						DisplayError("Internal error.");
						break;
					case UNZ_CRCERROR:
						DisplayError("CRC error.");
						break;
					}
					EnableOpenMenuItems();			
					ShowRomList(hMainWindow);
					return;
				}
				if(unzCloseCurrentFile(file) == UNZ_CRCERROR) {
					unzClose(file);
					DisplayError("CRC error in zip file.\n");
					EnableOpenMenuItems();			
					ShowRomList(hMainWindow);
					return;
				}
				AddRecentFile(hMainWindow,CurrentFileName);
				_splitpath( CurrentFileName, drive, dir, FileName, ext );
				unzClose(file);
			}
			if (FoundRom == FALSE) {
				unzCloseCurrentFile(file);
				port = unzGoToNextFile(file);
			}
		}
		if (FoundRom == FALSE) {
		    DisplayError("Zip Error: file contains no n64 image\n");
		    unzClose(file);
			EnableOpenMenuItems();			
			ShowRomList(hMainWindow);
			return;
		}
#if (!defined(EXTERNAL_RELEASE))
		if (AutoLoadMapFile) {
			OpenZipMapFile(MapFile);
		}
#endif
	} else {
		DWORD dwRead, dwToRead, TotalRead;
		HANDLE hFile;
		
		hFile = CreateFile(CurrentFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
			OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
			NULL);
		
		if (hFile == INVALID_HANDLE_VALUE) { 
			SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"" );
			DisplayError("Failed to open file");
			EnableOpenMenuItems();			
			ShowRomList(hMainWindow);
			return;
		}

		SetFilePointer(hFile,0,0,FILE_BEGIN);
		ReadFile(hFile,Test,4,&dwRead,NULL);
		if (!IsValidRomImage(Test)) {
			CloseHandle( hFile );
			SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"" );
			DisplayError("Not a valid n64 Image");
			EnableOpenMenuItems();			
			ShowRomList(hMainWindow);
			return;
		}
		RomFileSize = GetFileSize(hFile,NULL);

		if (!Allocate_ROM()) {
			CloseHandle( hFile );
			SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"" );
			DisplayError("Not enough memory for rom");
			EnableOpenMenuItems();			
				ShowRomList(hMainWindow);
			return;
		}

		SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"Loading image" );
		SetFilePointer(hFile,0,0,FILE_BEGIN);
		
		TotalRead = 0;
		for (count = 0; count < (int)RomFileSize; count += ReadFromRomSection) {
			dwToRead = RomFileSize - count;
			if (dwToRead > ReadFromRomSection) { dwToRead = ReadFromRomSection; }

			if (!ReadFile(hFile,&ROM[count],dwToRead,&dwRead,NULL)) {
				CloseHandle( hFile );
				SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"" );
				DisplayError("Failed to copy rom to memory");
				EnableOpenMenuItems();			
				ShowRomList(hMainWindow);
				return;
			}
			TotalRead += dwRead;
			sprintf(Message,"Loaded: %.2f%c",((float)TotalRead/(float)RomFileSize) * 100.0f,'%');
			SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)Message );
			Sleep(100);
		}
		dwRead = TotalRead;

		if (RomFileSize != dwRead) {
			CloseHandle( hFile );
			SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"" );
			DisplayError("Failed to copy rom to memory");
			EnableOpenMenuItems();			
			ShowRomList(hMainWindow);
			return;
		}
		CloseHandle( hFile ); 		
		AddRecentFile(hMainWindow,CurrentFileName);
		_splitpath( CurrentFileName, drive, dir, FileName, ext );
	}
	ByteSwapRom();
	SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"" );
	memcpy(RomHeader,ROM,sizeof(RomHeader));
#if (!defined(EXTERNAL_RELEASE))
	if (AutoLoadMapFile) {
		char *p;

		p = strrchr(MapFile,'.');
		if (p != NULL) {
			*p = '\0';
		}
		strcat(MapFile,".cod");
		if (OpenMapFile(MapFile)) {
			p = strrchr(MapFile,'.');
			if (p != NULL) {
				*p = '\0';
			}
			strcat(MapFile,".map");
			OpenMapFile(MapFile);
		}
	}
#endif
	
	memcpy(&RomName[0],(void *)(ROM + 0x20),20);
	for( count = 0 ; count < 20; count += 4 ) {
		RomName[count] ^= RomName[count+3];
		RomName[count + 3] ^= RomName[count];
		RomName[count] ^= RomName[count+3];			
		RomName[count + 1] ^= RomName[count + 2];
		RomName[count + 2] ^= RomName[count + 1];
		RomName[count + 1] ^= RomName[count + 2];			
	}
	for( count = 19 ; count >= 0; count -- ) {
		if (RomName[count] == ' ') {
			RomName[count] = '\0';
		} else if (RomName[count] == '\0') {
		} else {
			count = -1;
		}
	}
	RomName[20] = '\0';
	if (strlen(RomName) == 0) { strcpy(RomName,FileName); }
	sprintf( WinTitle, "%s - %s", RomName, AppName);
	
	for( count = 0 ; count < (int)strlen(RomName); count ++ ) {
		switch (RomName[count]) {
		case '/':
		case '\\':
			RomName[count] = '-';
			break;
		case ':':
			RomName[count] = ';';
			break;
		}
	}
	SetWindowText(hMainWindow,WinTitle);

	EnableOpenMenuItems();			
	EnableMenuItem(hMainMenu,ID_FILE_ROM_INFO,MFS_ENABLED|MF_BYCOMMAND);
	EnableMenuItem(hMainMenu,ID_FILE_STARTEMULATION,MFS_ENABLED|MF_BYCOMMAND);	
	EnableMenuItem(hMainMenu,ID_OPTIONS_CHEATS,MFS_ENABLED|MF_BYCOMMAND);
	if (HaveDebugger) {
		EnableMenuItem(hMainMenu,ID_DEBUGGER_MEMORY,MFS_ENABLED|MF_BYCOMMAND);
		EnableMenuItem(hMainMenu,ID_DEBUGGER_TLBENTRIES,MFS_ENABLED|MF_BYCOMMAND);
		EnableMenuItem(hMainMenu,ID_DEBUGGER_R4300IREGISTERS,MFS_ENABLED|MF_BYCOMMAND);
		if (IsMenu(RspDebug.hRSPMenu)) {
			EnableMenuItem(hMainMenu,(DWORD)RspDebug.hRSPMenu,MFS_ENABLED|MF_BYCOMMAND);
		}
		if (IsMenu(GFXDebug.hGFXMenu)) {
			EnableMenuItem(hMainMenu,(DWORD)GFXDebug.hGFXMenu,MFS_ENABLED|MF_BYCOMMAND);
		}
	}
	SetCurrentSaveState(hMainWindow,ID_CURRENTSAVE_DEFAULT);
	if (RomBrowser) { SetupPlugins(hMainWindow); }
	sprintf(WinTitle,"Loaded - [ %s ]",FileName);
	SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)WinTitle );	
	if (AutoFullScreen) {
		char Status[100], Identifier[100], result[100];
	
		sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
		_GetPrivateProfileString(Identifier,"Status",Default_RomStatus,Status,sizeof(Status),GetIniFileName());
		strcat(Status,".AutoFullScreen");
		_GetPrivateProfileString("Rom Status",Status,"True",result,sizeof(result),GetIniFileName());
		if (strcmp(result,"True") == 0) {
			if (ChangeWindow) { ChangeWindow(); } 
		}
	}
	if (AutoStart) { StartEmulation(); }
}

void SaveRecentDirs (void) {
	long lResult;
	HKEY hKeyResults = 0;
	DWORD Disposition = 0;
	char String[200];

	sprintf(String,"Software\\N64 Emulation\\%s",AppName);
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"",REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
	if (lResult == ERROR_SUCCESS) {
		DWORD count;

		for (count = 0; count < RomDirsToRemember; count++) {
			if (strlen(LastDirs[count]) == 0) { break; }
			sprintf(String,"RecentDir%d",count+1);			
			RegSetValueEx(hKeyResults,String,0,REG_SZ,(LPBYTE)LastDirs[count],strlen(LastDirs[count]));
		}
		RegCloseKey(hKeyResults);
	}
}

void SaveRecentFiles (void) {
	long lResult;
	HKEY hKeyResults = 0;
	DWORD Disposition = 0;
	char String[200];

	sprintf(String,"Software\\N64 Emulation\\%s",AppName);
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"",REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
	if (lResult == ERROR_SUCCESS) {
		DWORD count;

		for (count = 0; count < RomsToRemember; count++) {
			if (strlen(LastRoms[count]) == 0) { break; }
			sprintf(String,"RecentFile%d",count+1);			
			RegSetValueEx(hKeyResults,String,0,REG_SZ,(LPBYTE)LastRoms[count],strlen(LastRoms[count]));
		}
		RegCloseKey(hKeyResults);
	}
}

void SaveRomOptions (void) {
	char Identifier[100];
	LPSTR IniFileName;
	char String[100];

	if (strlen(RomName) == 0) { return; }

	IniFileName = GetIniFileName();
	sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
	_WritePrivateProfileString(Identifier,"Internal Name",RomName,IniFileName);

	switch (RomRamSize) {
	case 0x400000: strcpy(String,"4"); break;
	case 0x800000: strcpy(String,"8"); break;
	default: strcpy(String,"Default"); break;
	}
	_WritePrivateProfileString(Identifier,"RDRAM Size",String,GetIniFileName());

	switch (RomCF) {
	case 1: case 2: case 3: case 4: case 5: case 6: sprintf(String,"%d",RomCF); break;
	default: sprintf(String,"Default"); break;
	}
	_WritePrivateProfileString(Identifier,"Counter Factor",String,GetIniFileName());

	switch (RomSaveUsing) {
	case Eeprom_4K: sprintf(String,"4kbit Eeprom"); break;
	case Eeprom_16K: sprintf(String,"16kbit Eeprom"); break;
	case Sram: sprintf(String,"Sram"); break;
	case FlashRam: sprintf(String,"FlashRam"); break;
	default: sprintf(String,"First Save Type"); break;
	}
	_WritePrivateProfileString(Identifier,"Save Type",String,GetIniFileName());

	switch (RomCPUType) {
	case CPU_Interpreter: sprintf(String,"Interpreter"); break;
	case CPU_Recompiler: sprintf(String,"Recompiler"); break;
	case CPU_SyncCores: sprintf(String,"SyncCores"); break;
	default: sprintf(String,"Default"); break;
	}
	_WritePrivateProfileString(Identifier,"CPU Type",String,GetIniFileName());
	
	switch (RomSelfMod) {
	case ModCode_None: sprintf(String,"None"); break;
	case ModCode_Cache: sprintf(String,"Cache"); break;
	case ModCode_ProtectedMemory: sprintf(String,"Protected Memory"); break;
	case ModCode_CheckMemoryCache: sprintf(String,"Check Memory & cache"); break;
	case ModCode_ChangeMemory: sprintf(String,"Change Memory"); break;
	default: sprintf(String,"Default"); break;
	}
	_WritePrivateProfileString(Identifier,"Self-modifying code Method",String,GetIniFileName());

	_WritePrivateProfileString(Identifier,"Reg Cache",RomUseCache?"Yes":"No",GetIniFileName());
	_WritePrivateProfileString(Identifier,"Use TLB",RomUseTlb?"Yes":"No",GetIniFileName());
	_WritePrivateProfileString(Identifier,"Use Large Buffer",RomUseLargeBuffer?"Yes":"No",GetIniFileName());
	_WritePrivateProfileString(Identifier,"Linking","Global",GetIniFileName()); 
	if (RomUseLinking == 0) { _WritePrivateProfileString(Identifier,"Linking","On",GetIniFileName()); }
	if (RomUseLinking == 1) { _WritePrivateProfileString(Identifier,"Linking","Off",GetIniFileName()); }
}

void SetRecentRomDir (DWORD Index) {
	Index -= ID_FILE_RECENT_DIR;
	if (Index < 0 || Index > RomDirsToRemember) { return; }
	SetRomDirectory(LastDirs[Index], TRUE);
	RefreshRomBrowser();
}

void SetRomDirectory ( char * Directory, BOOL IgnoreDefaultDir ) {
	long lResult;
	HKEY hKeyResults = 0;
	DWORD Disposition = 0;
	char Group[200];

	sprintf(Group,"Software\\N64 Emulation\\%s",AppName);
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER, Group,0,"",REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Value, Bytes;

		Bytes = 4;
		if (IgnoreDefaultDir) {
			Value = TRUE;
			RegSetValueEx(hKeyResults,"Use Default Rom Dir",0,REG_DWORD,(BYTE *)&Value,sizeof(DWORD));
		}
		lResult = RegQueryValueEx(hKeyResults,"Use Default Rom Dir",0,&Type,(LPBYTE)(&Value),&Bytes);
		if ((lResult == ERROR_SUCCESS && Value == TRUE) || lResult != ERROR_SUCCESS) {
			RegSetValueEx(hKeyResults,"Rom Directory",0,REG_SZ,(LPBYTE)Directory,strlen(Directory));
			AddRecentDir(hMainWindow,Directory);
		} 
		RegCloseKey(hKeyResults);
	}
}
