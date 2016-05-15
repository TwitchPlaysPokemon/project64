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
#include <shlobj.h>
#include <stdio.h>
#include "main.h"
#include "cheats.h"
#include "cpu.h"
#include "plugin.h"
#include "debugger.h"
#include "Settings.h"
#include "htmlHelp.h"
#include "resource.h"

LARGE_INTEGER Frequency, Frames[NoOfFrames], LastFrame;
BOOL HaveDebugger, AutoLoadMapFile, ShowUnhandledMemory, ShowTLBMisses, 
	ShowDListAListCount, ShowCompMem, Profiling, IndvidualBlock, AutoStart, 
	AutoSleep, DisableRegCaching, UseIni, UseTlb, UseLinking, RomBrowser,
	IgnoreMove, Rercursion, ShowPifRamErrors, LimitFPS, ShowCPUPer, AutoZip, 
	AutoFullScreen, SystemABL;
DWORD CurrentFrame, CPU_Type, SystemCPU_Type, SelfModCheck, SystemSelfModCheck, 
	RomsToRemember, RomDirsToRemember;
HWND hMainWindow, hHiddenWin, hStatusWnd;
char CurrentSave[256];
HMENU hMainMenu;
HINSTANCE hInst;

void RomInfo   ( void );
void SelectLang( HWND hWnd, int LangMenuID );
void SetupMenu ( HWND hWnd );
void SetupMenuTitle ( HMENU hMenu, int MenuPos, char * ShotCut, char * Title, char * Language, char *  LangFile);

LRESULT CALLBACK AboutBoxProc    ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK AboutIniBoxProc ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK Main_Proc       ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK RomInfoProc     ( HWND, UINT, WPARAM, LPARAM );

void AboutBox (void) {
	DialogBox(hInst, MAKEINTRESOURCE(IDD_About), hMainWindow, (DLGPROC)AboutBoxProc);
}

void AboutIniBox (void) {
	DialogBox(hInst, MAKEINTRESOURCE(IDD_About_Ini), hMainWindow, (DLGPROC)AboutIniBoxProc);
}

LRESULT CALLBACK AboutBoxProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	#define NoOfGreets 4
	char * GREETS[NoOfGreets] = {
		"Thanks and greets to: Anarko, BreakPoint",
		"cricket, _demo_, Duddie, F|res, Icepir8",
		"Lac, Lemmy, rcp, schibo, slacka, smiff",
		"and any one I missed"
	};
	PAINTSTRUCT ps;
	HFONT hOldFont;
	int OldBkMode;
	LOGFONT lf;
	HFONT fMain;

	switch (uMsg) {
	case WM_INITDIALOG:
		break;
	case WM_PAINT:
		BeginPaint( hDlg, &ps );
	
		memset(&lf, 0, sizeof(lf));
		lf.lfHeight         = 20;
		lf.lfWidth			= 0;
		lf.lfWeight         = FW_NORMAL;
		lf.lfCharSet        = ANSI_CHARSET;
		lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
		lf.lfQuality        = PROOF_QUALITY;
		lf.lfPitchAndFamily = FF_DONTCARE|DEFAULT_PITCH;
		lstrcpy(lf.lfFaceName, "Arial");
		fMain = CreateFontIndirect(&lf);
		if (fMain == NULL) {
			DisplayError("Failed to create an Arial font");
			return FALSE;
		}


		OldBkMode = SetBkMode( ps.hdc, TRANSPARENT );
		SetTextColor(ps.hdc, RGB(0,0,0));
		hOldFont = (HFONT)SelectObject(ps.hdc, fMain);
			
		TextOut( ps.hdc, 10,130,AppVer,strlen(AppVer));
		{
			int count, pos;
			for (count = 0, pos = 170; count < NoOfGreets; count ++, pos+=20){
				TextOut( ps.hdc, 10,pos,GREETS[count],strlen(GREETS[count]));
			}
		}
		SelectObject( ps.hdc,hOldFont );
		SetBkMode( ps.hdc, OldBkMode );
		
		EndPaint( hDlg, &ps );
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg,0);
			break;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK AboutIniBoxProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static char RDBHomePage[300], CHTHomePage[300], RDXHomePage[300];
	
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			char * IniFile, Language[100], String[200],String2[200];

			//Language
			IniFile = GetLangFileName();
			_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),IniFile);
			_GetPrivateProfileString(Language,"Author","",String,sizeof(String),IniFile);
			if (strlen(String) != 0) {
				sprintf(String2,"Author: %s",String);
				SetDlgItemText(hDlg,IDC_LAN_AUTHOR,String2);
				_GetPrivateProfileString(Language,"Version","",String,sizeof(String),IniFile);
				sprintf(String2,"Version: %s",String);
				SetDlgItemText(hDlg,IDC_LAN_VERSION,String2);
				_GetPrivateProfileString(Language,"Date","",String,sizeof(String),IniFile);
				sprintf(String2,"Date: %s",String);
				SetDlgItemText(hDlg,IDC_LAN_DATE,String2);
			} else {
				EnableWindow(GetDlgItem(hDlg,IDC_LAN),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LAN_AUTHOR),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LAN_VERSION),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_LAN_DATE),FALSE);
			}
			
			//RDB
			IniFile = GetIniFileName();
			_GetPrivateProfileString("Meta","Author","",String,sizeof(String),IniFile);
			if (strlen(String) != 0) {
				sprintf(String2,"Author: %s",String);
				SetDlgItemText(hDlg,IDC_RDB_AUTHOR,String2);
				_GetPrivateProfileString("Meta","Version","",String,sizeof(String),IniFile);
				sprintf(String2,"Version: %s",String);
				SetDlgItemText(hDlg,IDC_RDB_VERSION,String2);
				_GetPrivateProfileString("Meta","Date","",String,sizeof(String),IniFile);
				sprintf(String2,"Date: %s",String);
				SetDlgItemText(hDlg,IDC_RDB_DATE,String2);
				_GetPrivateProfileString("Meta","Homepage","",RDBHomePage,sizeof(RDBHomePage),IniFile);
				if (strlen(RDBHomePage) == 0) {
					EnableWindow(GetDlgItem(hDlg,IDC_RDB_HOME),FALSE);
				}
			} else {
				EnableWindow(GetDlgItem(hDlg,IDC_RDB),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RDB_AUTHOR),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RDB_VERSION),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RDB_DATE),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RDB_HOME),FALSE);
			}

			//Cheat
			IniFile = GetCheatIniFileName();
			_GetPrivateProfileString("Meta","Author","",String,sizeof(String),IniFile);
			if (strlen(String) != 0) {
				sprintf(String2,"Author: %s",String);
				SetDlgItemText(hDlg,IDC_CHT_AUTHOR,String2);
				_GetPrivateProfileString("Meta","Version","",String,sizeof(String),IniFile);
				sprintf(String2,"Version: %s",String);
				SetDlgItemText(hDlg,IDC_CHT_VERSION,String2);
				_GetPrivateProfileString("Meta","Date","",String,sizeof(String),IniFile);
				sprintf(String2,"Date: %s",String);
				SetDlgItemText(hDlg,IDC_CHT_DATE,String2);
				_GetPrivateProfileString("Meta","Homepage","",CHTHomePage,sizeof(CHTHomePage),IniFile);
				if (strlen(CHTHomePage) == 0) {
					EnableWindow(GetDlgItem(hDlg,IDC_CHT_HOME),FALSE);
				}
			} else {
				EnableWindow(GetDlgItem(hDlg,IDC_CHT),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_CHT_AUTHOR),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_CHT_VERSION),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_CHT_DATE),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_CHT_HOME),FALSE);
			}

			//Extended Info
			IniFile = GetExtIniFileName();
			_GetPrivateProfileString("Meta","Author","",String,sizeof(String),IniFile);
			if (strlen(String) != 0) {
				sprintf(String2,"Author: %s",String);
				SetDlgItemText(hDlg,IDC_RDX_AUTHOR,String2);
				_GetPrivateProfileString("Meta","Version","",String,sizeof(String),IniFile);
				sprintf(String2,"Version: %s",String);
				SetDlgItemText(hDlg,IDC_RDX_VERSION,String2);
				_GetPrivateProfileString("Meta","Date","",String,sizeof(String),IniFile);
				sprintf(String2,"Date: %s",String);
				SetDlgItemText(hDlg,IDC_RDX_DATE,String2);
				_GetPrivateProfileString("Meta","Homepage","",RDXHomePage,sizeof(CHTHomePage),IniFile);
				if (strlen(RDXHomePage) == 0) {
					EnableWindow(GetDlgItem(hDlg,IDC_RDX_HOME),FALSE);
				}
			} else {
				EnableWindow(GetDlgItem(hDlg,IDC_RDX),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RDX_AUTHOR),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RDX_VERSION),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RDX_DATE),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_RDX_HOME),FALSE);
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RDB_HOME: ShellExecute(NULL,"open",RDBHomePage,NULL,NULL,SW_SHOWNORMAL); break;
		case IDC_CHT_HOME: ShellExecute(NULL,"open",CHTHomePage,NULL,NULL,SW_SHOWNORMAL); break;
		case IDC_RDX_HOME: ShellExecute(NULL,"open",RDXHomePage,NULL,NULL,SW_SHOWNORMAL); break;
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg,0);
			break;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

DWORD AsciiToHex (char * HexValue) {
	DWORD Count, Finish, Value = 0;

	Finish = strlen(HexValue);
	if (Finish > 8 ) { Finish = 8; }

	for (Count = 0; Count < Finish; Count++){
		Value = (Value << 4);
		switch( HexValue[Count] ) {
		case '0': break;
		case '1': Value += 1; break;
		case '2': Value += 2; break;
		case '3': Value += 3; break;
		case '4': Value += 4; break;
		case '5': Value += 5; break;
		case '6': Value += 6; break;
		case '7': Value += 7; break;
		case '8': Value += 8; break;
		case '9': Value += 9; break;
		case 'A': Value += 10; break;
		case 'a': Value += 10; break;
		case 'B': Value += 11; break;
		case 'b': Value += 11; break;
		case 'C': Value += 12; break;
		case 'c': Value += 12; break;
		case 'D': Value += 13; break;
		case 'd': Value += 13; break;
		case 'E': Value += 14; break;
		case 'e': Value += 14; break;
		case 'F': Value += 15; break;
		case 'f': Value += 15; break;
		default: 
			Value = (Value >> 4);
			Count = Finish;
		}
	}
	return Value;
}

void ChangeWinSize ( HWND hWnd, long width, long height, HWND hStatusBar ) {
	WINDOWPLACEMENT wndpl;
    RECT rc1, swrect;

	wndpl.length = sizeof(wndpl);
	GetWindowPlacement( hWnd, &wndpl);

	if ( hStatusBar != NULL ) {
		GetClientRect( hStatusBar, &swrect );
	    SetRect( &rc1, 0, 0, width, height + swrect.bottom );
	} else {
	    SetRect( &rc1, 0, 0, width, height );
	}


    AdjustWindowRectEx( &rc1,GetWindowLong( hWnd, GWL_STYLE ),
		GetMenu( hWnd ) != NULL, GetWindowLong( hWnd, GWL_EXSTYLE ) ); 

    MoveWindow( hWnd, wndpl.rcNormalPosition.left, wndpl.rcNormalPosition.top, 
		rc1.right - rc1.left, rc1.bottom - rc1.top, TRUE );
}

void __cdecl DisplayError (char * Message, ...) {
	char Msg[400];
	va_list ap;

	va_start( ap, Message );
	vsprintf( Msg, Message, ap );
	va_end( ap );
	MessageBox(NULL,Msg,"Error",MB_OK|MB_ICONERROR|MB_SETFOREGROUND);
	SetActiveWindow(hMainWindow);
}

void DisplayFPS (void) {
	if (CurrentFrame > (NoOfFrames << 3)) {
		LARGE_INTEGER Total;
		char Message[100];
		int count;
		
		Total.QuadPart = 0;
		for (count = 0; count < NoOfFrames; count ++) {
			Total.QuadPart += Frames[count].QuadPart;
		}
		sprintf(Message, "FPS: %.2f", Frequency.QuadPart/ ((double)Total.QuadPart / (NoOfFrames << 3)));
		SendMessage( hStatusWnd, SB_SETTEXT, 1, (LPARAM)Message );
	} else {
		SendMessage( hStatusWnd, SB_SETTEXT, 1, (LPARAM)"FPS: -.--" );
	}
}

void FixMenuLang (HWND hWnd) {
	char * LangFile, Language[100];
	HMENU hMenu = GetMenu(hWnd), hSubMenu;

	LangFile = GetLangFileName();
	_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);
	
	SetupMenuTitle(hMenu,0,NULL,"MenuTitle1",Language,LangFile);
	SetupMenuTitle(hMenu,1,NULL,"MenuTitle2",Language,LangFile);
	SetupMenuTitle(hMenu,2,NULL,"MenuTitle3",Language,LangFile);
#ifdef EXTERNAL_RELEASE
	SetupMenuTitle(hMenu,3,NULL,"MenuTitle5",Language,LangFile);
	SetupMenuTitle(hMenu,4,NULL,"MenuTitle4",Language,LangFile);
#else
	SetupMenuTitle(hMenu,4,NULL,"MenuTitle5",Language,LangFile);
	SetupMenuTitle(hMenu,5,NULL,"MenuTitle4",Language,LangFile);
#endif 
	
	//File
	hSubMenu = GetSubMenu(hMenu,0);
	SetupMenuTitle(hSubMenu,0,"Ctrl+O","FileItem1",Language,LangFile);
	SetupMenuTitle(hSubMenu,1,NULL,"FileItem2",Language,LangFile);
	SetupMenuTitle(hSubMenu,3,NULL,"FileItem3",Language,LangFile);
	SetupMenuTitle(hSubMenu,4,NULL,"FileItem4",Language,LangFile);
	SetupMenuTitle(hSubMenu,6,NULL,"FileItem6",Language,LangFile);
	SetupMenuTitle(hSubMenu,7,"F5","FileItem7",Language,LangFile);
	SetupMenuTitle(hSubMenu,9,NULL,"FileItem8",Language,LangFile);
	SetupMenuTitle(hSubMenu,10,NULL,"FileItem9",Language,LangFile);
	SetupMenuTitle(hSubMenu,12,NULL,"FileItem5",Language,LangFile);

	//System
	hSubMenu = GetSubMenu(hMenu,1);
	SetupMenuTitle(hSubMenu,0,"F1","SystemItem1",Language,LangFile);
	SetupMenuTitle(hSubMenu,1,"F2","SystemItem2",Language,LangFile);
	SetupMenuTitle(hSubMenu,2,"F3","SystemItem3",Language,LangFile);
	SetupMenuTitle(hSubMenu,4,"F4","SystemItem9",Language,LangFile);
	SetupMenuTitle(hSubMenu,6,"F5","SystemItem4",Language,LangFile);
	SetupMenuTitle(hSubMenu,7,"Ctrl+A","SystemItem7",Language,LangFile);
	SetupMenuTitle(hSubMenu,8,"F7","SystemItem5",Language,LangFile);
	SetupMenuTitle(hSubMenu,9,"Ctrl+L","SystemItem8",Language,LangFile);
	SetupMenuTitle(hSubMenu,11,NULL,"SystemItem6",Language,LangFile);

	//Options
	hSubMenu = GetSubMenu(hMenu,2);
	SetupMenuTitle(hSubMenu,0,"Alt+Enter","OptionsItem1",Language,LangFile);
	SetupMenuTitle(hSubMenu,2,NULL,"OptionsItem2",Language,LangFile);
	SetupMenuTitle(hSubMenu,3,NULL,"OptionsItem3",Language,LangFile);
	SetupMenuTitle(hSubMenu,4,NULL,"OptionsItem4",Language,LangFile);
	SetupMenuTitle(hSubMenu,5,NULL,"OptionsItem5",Language,LangFile);
	SetupMenuTitle(hSubMenu,7,NULL,"OptionsItem9",Language,LangFile);
	SetupMenuTitle(hSubMenu,8,NULL,"OptionsItem6",Language,LangFile);
	SetupMenuTitle(hSubMenu,9,"Ctrl+S","OptionsItem7",Language,LangFile);

	//Help Menu
#ifdef EXTERNAL_RELEASE
	hSubMenu = GetSubMenu(hMenu,4);
#else
	hSubMenu = GetSubMenu(hMenu,5);
#endif 
#ifdef BETA_VERSION
	SetupMenuTitle(hSubMenu,2,NULL,"HelpItem2",Language,LangFile);
	SetupMenuTitle(hSubMenu,3,NULL,"HelpItem3",Language,LangFile);
	SetupMenuTitle(hSubMenu,5,NULL,"HelpItem4",Language,LangFile);
	SetupMenuTitle(hSubMenu,6,NULL,"HelpItem1",Language,LangFile);
#else
	SetupMenuTitle(hSubMenu,0,NULL,"HelpItem2",Language,LangFile);
	SetupMenuTitle(hSubMenu,1,NULL,"HelpItem3",Language,LangFile);
	SetupMenuTitle(hSubMenu,3,NULL,"HelpItem4",Language,LangFile);
	SetupMenuTitle(hSubMenu,4,NULL,"HelpItem1",Language,LangFile);
#endif

	hSubMenu = GetSubMenu(hMenu,1);
	hSubMenu = GetSubMenu(hSubMenu,11);
	SetupMenuTitle(hSubMenu,0,"~","SaveSlotItem1",Language,LangFile);
	SetupMenuTitle(hSubMenu,2,"1","SaveSlotItem2",Language,LangFile);
	SetupMenuTitle(hSubMenu,3,"2","SaveSlotItem3",Language,LangFile);
	SetupMenuTitle(hSubMenu,4,"3","SaveSlotItem4",Language,LangFile);
	SetupMenuTitle(hSubMenu,5,"4","SaveSlotItem5",Language,LangFile);
	SetupMenuTitle(hSubMenu,6,"5","SaveSlotItem6",Language,LangFile);
	SetupMenuTitle(hSubMenu,7,"6","SaveSlotItem7",Language,LangFile);
	SetupMenuTitle(hSubMenu,8,"7","SaveSlotItem8",Language,LangFile);
	SetupMenuTitle(hSubMenu,9,"8","SaveSlotItem9",Language,LangFile);
	SetupMenuTitle(hSubMenu,10,"9","SaveSlotItem10",Language,LangFile);
	SetupMenuTitle(hSubMenu,11,"0","SaveSlotItem11",Language,LangFile);
	AddRecentFile(hWnd,NULL);
	AddRecentDir(hWnd,NULL);
	DrawMenuBar(hWnd);


}

char * GetExtIniFileName(void) {
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
	char fname[_MAX_FNAME],ext[_MAX_EXT];
	static char IniFileName[_MAX_PATH];

	GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
	_splitpath( path_buffer, drive, dir, fname, ext );
	sprintf(IniFileName,"%s%s%s",drive,dir,ExtIniName);
	return IniFileName;
}

char * GetIniFileName(void) {
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
	char fname[_MAX_FNAME],ext[_MAX_EXT];
	static char IniFileName[_MAX_PATH];

	GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
	_splitpath( path_buffer, drive, dir, fname, ext );
	sprintf(IniFileName,"%s%s%s",drive,dir,IniName);
	return IniFileName;
}

char * GetLangFileName(void) {
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
	char fname[_MAX_FNAME],ext[_MAX_EXT];
	static char IniFileName[_MAX_PATH];

	GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
	_splitpath( path_buffer, drive, dir, fname, ext );
	sprintf(IniFileName,"%s%s%s",drive,dir,LangFileName);
	return IniFileName;
}

char * GetNotesIniFileName(void) {
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
	char fname[_MAX_FNAME],ext[_MAX_EXT];
	static char IniFileName[_MAX_PATH];

	GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
	_splitpath( path_buffer, drive, dir, fname, ext );
	sprintf(IniFileName,"%s%s%s",drive,dir,NotesIniName);
	return IniFileName;
}

int GetStoredWinPos( char * WinName, DWORD * X, DWORD * Y ) {
	long lResult;
	HKEY hKeyResults = 0;
	char String[200];

	sprintf(String,"Software\\N64 Emulation\\%s\\Page Setup",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0, KEY_ALL_ACCESS,&hKeyResults);
	
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Value, Bytes = 4;

		sprintf(String,"%s Top",WinName);
		lResult = RegQueryValueEx(hKeyResults,String,0,&Type,(LPBYTE)(&Value),&Bytes);
		if (Type == REG_DWORD && lResult == ERROR_SUCCESS) { 
			*Y = Value;
		} else {
			RegCloseKey(hKeyResults);
			return FALSE;
		}
	
		sprintf(String,"%s Left",WinName);
		lResult = RegQueryValueEx(hKeyResults,String,0,&Type,(LPBYTE)(&Value),&Bytes);
		if (Type == REG_DWORD && lResult == ERROR_SUCCESS) { 
			*X = Value;
		} else {
			RegCloseKey(hKeyResults);
			return FALSE;
		}
		RegCloseKey(hKeyResults);
		return TRUE;
	}
	return FALSE;
}

int InitalizeApplication ( HINSTANCE hInstance ) {
	INITCOMMONCONTROLSEX IntComStruct;
	HKEY hKeyResults = 0;
	char String[200];
	long lResult;

	IntComStruct.dwSize = sizeof(IntComStruct);
	IntComStruct.dwICC = ICC_TREEVIEW_CLASSES;
	InitCommonControls();	
	InitCommonControlsEx(&IntComStruct);
	hInst = hInstance;

	if (!Allocate_Memory()) { 
		DisplayError("Failed to allocate Memory");
		return FALSE; 
	}

	hPauseMutex = CreateMutex(NULL,FALSE,NULL);
#if (!defined(EXTERNAL_RELEASE))
	HaveDebugger = FALSE;
	sprintf(String,"Software\\N64 Emulation\\%s",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0, KEY_ALL_ACCESS,&hKeyResults);	
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Value, Bytes = 4;

		lResult = RegQueryValueEx(hKeyResults,"Debugger",0,&Type,(LPBYTE)(&Value),&Bytes);
		if (Type == REG_DWORD && lResult == ERROR_SUCCESS) { 
			if (Value == 0x9348ae97) {
				HaveDebugger = TRUE;
			}
		}
	}
#endif
	
	IgnoreMove = FALSE;
	CPU_Type = Default_CPU;
	SystemCPU_Type = Default_CPU;
	SystemSelfModCheck = Default_SelfModCheck;
	SystemRdramSize = Default_RdramSize;
	SystemABL = Default_AdvancedBlockLink;
	AutoStart = Default_AutoStart;
	AutoSleep = Default_AutoSleep;
	DisableRegCaching = Default_DisableRegCaching;
	UseIni = Default_UseIni;
	AutoZip = Default_AutoZip;
	AutoFullScreen = FALSE;
	RomsToRemember = Default_RomsToRemember;
	RomDirsToRemember = Default_RomsDirsToRemember;
	AutoLoadMapFile = Default_AutoMap;
	ShowUnhandledMemory = Default_ShowUnhandledMemory;
	ShowCPUPer = Default_ShowCPUPer;
	LimitFPS = Default_LimitFPS;
	ShowTLBMisses = Default_ShowTLBMisses;
	Profiling = Default_ProfilingOn;
	IndvidualBlock = Default_IndvidualBlock;
	RomBrowser = Default_UseRB;
	Rercursion = Default_Rercursion;

	sprintf(String,"Software\\N64 Emulation\\%s",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0,KEY_ALL_ACCESS,
		&hKeyResults);
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Bytes = 4;

		lResult = RegQueryValueEx(hKeyResults,"Roms To Remember",0,&Type,(BYTE *)(&RomsToRemember),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { RomsToRemember = Default_RomsToRemember; }

		lResult = RegQueryValueEx(hKeyResults,"Rom Dirs To Remember",0,&Type,(BYTE *)(&RomDirsToRemember),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { RomDirsToRemember = Default_RomsDirsToRemember; }

		lResult = RegQueryValueEx(hKeyResults,"Start Emulation when rom is opened",0,&Type,(BYTE *)(&AutoStart),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { AutoStart = Default_AutoStart; }

		lResult = RegQueryValueEx(hKeyResults,"Use Rom Browser",0,&Type,(BYTE *)(&RomBrowser),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { RomBrowser = Default_UseRB; }
	
		lResult = RegQueryValueEx(hKeyResults,"Use Recursion",0,&Type,(BYTE *)(&Rercursion),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { Rercursion = Default_Rercursion; }

		lResult = RegQueryValueEx(hKeyResults,"Pause emulation when window is not active",0,&Type,(BYTE *)(&AutoSleep),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { AutoSleep = Default_AutoSleep; }

		lResult = RegQueryValueEx(hKeyResults,"Always overwrite default settings with ones from ini?",0,&Type,(BYTE *)(&UseIni),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { UseIni = Default_UseIni; }

		lResult = RegQueryValueEx(hKeyResults,"Automatically compress instant saves",0,&Type,(BYTE *)(&AutoZip),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { AutoZip = Default_AutoZip; }

		lResult = RegQueryValueEx(hKeyResults,"On open rom go full screen",0,&Type,(BYTE *)(&AutoFullScreen),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { AutoFullScreen = FALSE; }

		lResult = RegQueryValueEx(hKeyResults,"CPU Type",0,&Type,(BYTE *)(&CPU_Type),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { CPU_Type = Default_CPU; }
		SystemCPU_Type = CPU_Type;

		lResult = RegQueryValueEx(hKeyResults,"Self modifying code method",0,&Type,(LPBYTE)(&SelfModCheck),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { SelfModCheck = Default_SelfModCheck; }
		SystemSelfModCheck = SelfModCheck;

		lResult = RegQueryValueEx(hKeyResults,"Advanced Block Linking",0,&Type,(LPBYTE)(&SystemABL),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { SystemABL = Default_AdvancedBlockLink; }

		lResult = RegQueryValueEx(hKeyResults,"Default RDRAM Size",0,&Type,(LPBYTE)(&SystemRdramSize),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { SystemRdramSize = Default_RdramSize; }

		lResult = RegQueryValueEx(hKeyResults,"Show CPU %",0,&Type,(LPBYTE)(&ShowCPUPer),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { ShowCPUPer = Default_ShowCPUPer;	}

		lResult = RegQueryValueEx(hKeyResults,"Limit FPS",0,&Type,(LPBYTE)(&LimitFPS),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { LimitFPS = Default_LimitFPS;	}

#if (!defined(EXTERNAL_RELEASE))
		if (HaveDebugger) {
			lResult = RegQueryValueEx(hKeyResults,"Auto Load Map File",0,&Type,(BYTE *)(&AutoLoadMapFile),&Bytes);
			if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { AutoLoadMapFile = Default_AutoMap; }

			lResult = RegQueryValueEx(hKeyResults,"Show Unhandled Memory Accesses",0,&Type,(LPBYTE)(&ShowUnhandledMemory),&Bytes);
			if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { ShowUnhandledMemory = Default_ShowUnhandledMemory;	}

			lResult = RegQueryValueEx(hKeyResults,"Show Load/Store TLB Misses",0,&Type,(LPBYTE)(&ShowTLBMisses),&Bytes);
			if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { ShowTLBMisses = Default_ShowTLBMisses;	}

			lResult = RegQueryValueEx(hKeyResults,"Show Dlist/Alist Count",0,&Type,(LPBYTE)(&ShowDListAListCount),&Bytes);
			if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { ShowDListAListCount = Default_ShowDlistCount;	}

			lResult = RegQueryValueEx(hKeyResults,"Show Compile Memory",0,&Type,(LPBYTE)(&ShowCompMem),&Bytes);
			if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { ShowCompMem = Default_ShowCompileMemory;	}

			lResult = RegQueryValueEx(hKeyResults,"Show Pif Ram Errors",0,&Type,(LPBYTE)(&ShowPifRamErrors),&Bytes);
			if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { ShowPifRamErrors = Default_ShowPifRamErrors;	}

			lResult = RegQueryValueEx(hKeyResults,"Profiling On",0,&Type,(LPBYTE)(&Profiling),&Bytes);
			if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { Profiling = Default_ProfilingOn; }

			lResult = RegQueryValueEx(hKeyResults,"Log Indvidual Blocks",0,&Type,(LPBYTE)(&IndvidualBlock),&Bytes);
			if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { IndvidualBlock = Default_IndvidualBlock; }
		
		}
#endif
		RegCloseKey(hKeyResults);
	}
	

	LoadRomBrowserColoumnInfo ();
	SetupRegisters(&Registers);
	QueryPerformanceFrequency(&Frequency);
#if (!defined(EXTERNAL_RELEASE))
	LoadLogOptions(&LogOptions, FALSE);
	StartLog();
#endif
	return TRUE;
}

LRESULT CALLBACK Main_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char * LangFile, Language[100], String[256];
	DWORD Disposition;
	HKEY hKeyResults;
	long lResult;
	UINT uState;
	HMENU hMenu;

	switch (uMsg) {	
	case WM_CREATE:
		if ( hHiddenWin ) { 
			hStatusWnd = CreateStatusWindow( WS_CHILD | WS_VISIBLE, "", hWnd, 100 );
			SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"" );
			ChangeWinSize ( hWnd, 640, 480, hStatusWnd );
			SetupMenu(hWnd);
		} 
		hMainMenu = GetMenu(hWnd);
		break;	
	case WM_SYSCOMMAND:
		switch (wParam) {
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			//SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"Screen saver start" );
			return 0;
		case SC_MAXIMIZE:
			SetRomBrowserMaximized(TRUE);
			return DefWindowProc(hWnd,uMsg,wParam,lParam);
		default:
			return DefWindowProc(hWnd,uMsg,wParam,lParam);
		}
		break;
	case WM_PAINT:
		if (RomListVisible()) { ValidateRect(hWnd,NULL); break; }
		if (hWnd == hHiddenWin) { ValidateRect(hWnd,NULL); break; }
		__try {
			if (DrawScreen != NULL) { DrawScreen(); }
		} __except( r4300i_CPU_MemoryFilter( GetExceptionCode(), GetExceptionInformation()) ) {
			DisplayError("Unknown memory action in trying to update the screen\n\nEmulation stop");
			ExitThread(0);
		}
		ValidateRect(hWnd,NULL);
		break;
	case WM_MOVE:
		if (IgnoreMove) { break; }
		if (hWnd == hHiddenWin) { break; }		
		if (MoveScreen != NULL) { 
			MoveScreen((int)(short) LOWORD(lParam), (int)(short) HIWORD(lParam)); 
		}
		if (IsIconic(hWnd)) { 
			if (!CPU_Paused) { PauseCpu(); break; } 
			break; 
		} else {
			if (!ManualPaused && (CPU_Paused || CPU_Action.Pause)) { PauseCpu(); break; }  
		}
		//DisplayError("WM_MOVE\n%d\n%d",RomListVisible(),IsRomBrowserMaximized());
		if (!RomListVisible() || (RomListVisible() && !IsRomBrowserMaximized())) {
			StoreCurrentWinPos( RomListVisible()?"Main.RomList":"Main", hWnd );
		}
		break;
	case WM_SIZE:
		if (hWnd == hHiddenWin) { break; }
		{
			RECT clrect, swrect;
			int Parts[2];
		
			GetClientRect( hWnd, &clrect );
			GetClientRect( hStatusWnd, &swrect );

			Parts[0] = (LOWORD( lParam) - (int)( clrect.right * 0.25 ));
			Parts[1] = LOWORD( lParam);

			SendMessage( hStatusWnd, SB_SETPARTS, 2, (LPARAM)&Parts[0] );
			MoveWindow ( hStatusWnd, 0, clrect.bottom - swrect.bottom,
				LOWORD( lParam ), HIWORD( lParam ), TRUE );
			DisplayFPS();
		}
		ResizeRomListControl(LOWORD(lParam),HIWORD( lParam ));
		if (!IgnoreMove) {
			if (wParam == SIZE_RESTORED && RomListVisible()) {
				SetRomBrowserMaximized(FALSE); 
				SetRomBrowserSize(LOWORD(lParam),HIWORD( lParam ));
			}
		}
		break;
	case WM_SETFOCUS: 
		if (hWnd == hHiddenWin) { break; }
		if (AutoSleep && !ManualPaused && (CPU_Paused || CPU_Action.Pause)) { PauseCpu(); }  
		break;
	case WM_KILLFOCUS: 
		if (hWnd == hHiddenWin) { break; }
		if (AutoSleep && !CPU_Paused) { PauseCpu(); } 
		break;
	case WM_NOTIFY:
		if (wParam == IDC_ROMLIST) { RomListNotify((LPNMHDR)lParam); }
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
		break;
	case WM_DRAWITEM:
		if (wParam == IDC_ROMLIST) { RomListDrawItem((LPDRAWITEMSTRUCT)lParam); }
		break;
	case WM_MENUSELECT:
		LangFile = GetLangFileName();
		_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);
		switch (LOWORD(wParam)) {
		case ID_FILE_OPEN_ROM: _GetPrivateProfileString(Language,"MenuDes1","",String,sizeof(String),LangFile); break;
		case ID_FILE_ROM_INFO: _GetPrivateProfileString(Language,"MenuDes2","",String,sizeof(String),LangFile); break;
		case ID_FILE_STARTEMULATION: _GetPrivateProfileString(Language,"MenuDes3","",String,sizeof(String),LangFile); break;
		case ID_FILE_ENDEMULATION: _GetPrivateProfileString(Language,"MenuDes4","",String,sizeof(String),LangFile); break;
		case ID_FILE_EXIT: _GetPrivateProfileString(Language,"MenuDes5","",String,sizeof(String),LangFile); break;
		case ID_CPU_RESET: _GetPrivateProfileString(Language,"MenuDes6","",String,sizeof(String),LangFile); break;
		case ID_CPU_PAUSE: _GetPrivateProfileString(Language,"MenuDes7","",String,sizeof(String),LangFile); break;
		case ID_CPU_SAVE: _GetPrivateProfileString(Language,"MenuDes8","",String,sizeof(String),LangFile); break;
		case ID_CPU_RESTORE: _GetPrivateProfileString(Language,"MenuDes9","",String,sizeof(String),LangFile); break;
		case ID_CPU_LOAD: _GetPrivateProfileString(Language,"MenuDes12","",String,sizeof(String),LangFile); break;
		case ID_HELP_ABOUT: _GetPrivateProfileString(Language,"MenuDes10","",String,sizeof(String),LangFile); break;
		case ID_FILE_REFRESHROMLIST: _GetPrivateProfileString(Language,"MenuDes13","",String,sizeof(String),LangFile); break;
		case ID_SYSTEM_GENERATEBITMAP: _GetPrivateProfileString(Language,"MenuDes15","",String,sizeof(String),LangFile); break;
		case ID_SYSTEM_LIMITFPS: _GetPrivateProfileString(Language,"MenuDes16","",String,sizeof(String),LangFile); break;
		case ID_CPU_SAVEAS:  _GetPrivateProfileString(Language,"MenuDes17","",String,sizeof(String),LangFile); break;
		case ID_OPTIONS_FULLSCREEN: _GetPrivateProfileString(Language,"MenuDes18","",String,sizeof(String),LangFile); break;
		case ID_OPTIONS_CONFIG_GFX: _GetPrivateProfileString(Language,"MenuDes19","",String,sizeof(String),LangFile); break;
		case ID_OPTIONS_CONFIG_RSP: _GetPrivateProfileString(Language,"MenuDes20","",String,sizeof(String),LangFile); break;
		case ID_OPTIONS_CONFIG_AUDIO: _GetPrivateProfileString(Language,"MenuDes21","",String,sizeof(String),LangFile); break;
		case ID_OPTIONS_CONFIG_CONTROL: _GetPrivateProfileString(Language,"MenuDes22","",String,sizeof(String),LangFile); break;
		case ID_OPTIONS_SHOWCPUUSAGE: _GetPrivateProfileString(Language,"MenuDes23","",String,sizeof(String),LangFile); break;
		case ID_OPTIONS_CHEATS: _GetPrivateProfileString(Language,"MenuDes24","",String,sizeof(String),LangFile); break;
		case ID_OPTIONS_SETTINGS: _GetPrivateProfileString(Language,"MenuDes25","",String,sizeof(String),LangFile); break;
		case ID_FILE_ROMDIRECTORY: _GetPrivateProfileString(Language,"MenuDes26","",String,sizeof(String),LangFile); break;
		default: 
			if (LOWORD(wParam) >= ID_FILE_RECENT_FILE && LOWORD(wParam) <= (ID_FILE_RECENT_FILE + RomsToRemember)) {
				_GetPrivateProfileString(Language,"MenuDes11","",String,sizeof(String),LangFile);
			} else if (LOWORD(wParam) >= ID_FILE_RECENT_DIR && LOWORD(wParam) <= (ID_FILE_RECENT_DIR + RomDirsToRemember)) {
				_GetPrivateProfileString(Language,"MenuDes14","",String,sizeof(String),LangFile);
			} else {
				String[0] = 0; 
			}
			break;
		}
		SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)String );
		break;
	case WM_KEYDOWN: 
		if (hWnd == hHiddenWin) { break; }		
		if (WM_KeyDown) { WM_KeyDown(wParam, lParam); };
		break;
	case WM_KEYUP: 
		if (hWnd == hHiddenWin) { break; }		
		if (WM_KeyUp) { WM_KeyUp(wParam, lParam); }; 
		break;
	//case WM_ERASEBKGND: break;
	case WM_USER + 10: 
		if (hWnd == hHiddenWin) { break; }		
		if (!wParam) {
			while (ShowCursor(FALSE) >= 0) { Sleep(0); }
		} else {
			while (ShowCursor(TRUE) < 0) { Sleep(0); }
		}
		break;
	case WM_USER + 17:  SetFocus(hMainWindow); break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_FILE_OPEN_ROM: OpenN64Image(); break;
		case ID_FILE_ROM_INFO: RomInfo(); break;
		case ID_FILE_STARTEMULATION: 
			HideRomBrowser();
			StartEmulation(); 
			break;
		case ID_FILE_ENDEMULATION: 
			CloseCpu();
			hMenu = GetMenu(hMainWindow);
			EnableMenuItem(hMenu,ID_FILE_STARTEMULATION,MFS_ENABLED|MF_BYCOMMAND);
			if (DrawScreen != NULL) { DrawScreen(); }
			if (RomBrowser) { 
				ShowRomList(hMainWindow); 
			}
			break;
		case ID_FILE_ROMDIRECTORY: SelectRomDir(); break; 
		case ID_FILE_REFRESHROMLIST: RefreshRomBrowser(); break; 
		case ID_FILE_EXIT: DestroyWindow(hWnd);	break;		
		case ID_CPU_RESET: 
			CloseCpu(); 
			StartEmulation();
			break;
		case ID_CPU_PAUSE: ManualPaused = TRUE; PauseCpu(); break;
		case ID_CPU_SAVE: 
			CPU_Action.SaveState = TRUE;
			CPU_Action.DoSomething = TRUE;
			break;
		case ID_CPU_SAVEAS:
			{
				char drive[_MAX_DRIVE] ,dir[_MAX_DIR], fname[_MAX_FNAME],ext[_MAX_EXT];
				char Directory[255], SaveFile[255];
				OPENFILENAME openfilename;

				memset(&SaveFile, 0, sizeof(SaveFile));
				memset(&openfilename, 0, sizeof(openfilename));

				GetInstantSaveDir( Directory );

				openfilename.lStructSize  = sizeof( openfilename );
				openfilename.hwndOwner    = hMainWindow;
				openfilename.lpstrFilter  = "PJ64 Saves (*.zip, *.pj)\0*.pj?;*.pj;*.zip;";
				openfilename.lpstrFile    = SaveFile;
				openfilename.lpstrInitialDir    = Directory;
				openfilename.nMaxFile     = MAX_PATH;
				openfilename.Flags        = OFN_HIDEREADONLY;

				if (GetSaveFileName (&openfilename)) {
					_splitpath( SaveFile, drive, dir, fname, ext );
   					_makepath( SaveFile, drive, dir, fname, NULL );
					strcpy(SaveAsFileName,SaveFile);
					CPU_Action.SaveState = TRUE;
					CPU_Action.DoSomething = TRUE;
				}
			}
			break;
		case ID_CPU_RESTORE: 
			CPU_Action.RestoreState = TRUE;
			CPU_Action.DoSomething = TRUE;
			break;
		case ID_CPU_LOAD:
			{
				char Directory[255], SaveFile[255];
				OPENFILENAME openfilename;

				memset(&SaveFile, 0, sizeof(SaveFile));
				memset(&openfilename, 0, sizeof(openfilename));

				GetInstantSaveDir( Directory );

				openfilename.lStructSize  = sizeof( openfilename );
				openfilename.hwndOwner    = hMainWindow;
				openfilename.lpstrFilter  = "PJ64 Saves (*.zip, *.pj)\0*.pj?;*.pj;*.zip;";
				openfilename.lpstrFile    = SaveFile;
				openfilename.lpstrInitialDir    = Directory;
				openfilename.nMaxFile     = MAX_PATH;
				openfilename.Flags        = OFN_HIDEREADONLY;

				if (GetOpenFileName (&openfilename)) {
					strcpy(LoadFileName,SaveFile);
					CPU_Action.RestoreState = TRUE;
					CPU_Action.DoSomething = TRUE;
				}
			}
			break;
		case ID_SYSTEM_GENERATEBITMAP:
			if (CaptureScreen) {
				char Directory[255];				
				GetSnapShotDir(Directory);
				CaptureScreen(Directory);
				//DisplayError(Directory);
			}
			break;
		case ID_SYSTEM_LIMITFPS:			
			uState = GetMenuState( hMainMenu, ID_SYSTEM_LIMITFPS, MF_BYCOMMAND);			
			hKeyResults = 0;
			Disposition = 0;

			if ( uState & MFS_CHECKED ) {								
				CheckMenuItem( hMainMenu, ID_SYSTEM_LIMITFPS, MF_BYCOMMAND | MFS_UNCHECKED );
				LimitFPS = FALSE;
			} else {
				CheckMenuItem( hMainMenu, ID_SYSTEM_LIMITFPS, MF_BYCOMMAND | MFS_CHECKED );
				LimitFPS = TRUE;
			}
		
			sprintf(String,"Software\\N64 Emulation\\%s",AppName);
			lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
				REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
		
			if (lResult == ERROR_SUCCESS) { 
				RegSetValueEx(hKeyResults,"Limit FPS",0,REG_DWORD,(BYTE *)&LimitFPS,sizeof(DWORD));
			}
			RegCloseKey(hKeyResults);
			break;
		case ID_CURRENTSAVE_DEFAULT: SetCurrentSaveState(hWnd,LOWORD(wParam)); break;
		case ID_CURRENTSAVE_0: SetCurrentSaveState(hWnd,LOWORD(wParam)); break;
		case ID_CURRENTSAVE_1: SetCurrentSaveState(hWnd,LOWORD(wParam)); break;
		case ID_CURRENTSAVE_2: SetCurrentSaveState(hWnd,LOWORD(wParam)); break;
		case ID_CURRENTSAVE_3: SetCurrentSaveState(hWnd,LOWORD(wParam)); break;
		case ID_CURRENTSAVE_4: SetCurrentSaveState(hWnd,LOWORD(wParam)); break;
		case ID_CURRENTSAVE_5: SetCurrentSaveState(hWnd,LOWORD(wParam)); break;
		case ID_CURRENTSAVE_6: SetCurrentSaveState(hWnd,LOWORD(wParam)); break;
		case ID_CURRENTSAVE_7: SetCurrentSaveState(hWnd,LOWORD(wParam)); break;
		case ID_CURRENTSAVE_8: SetCurrentSaveState(hWnd,LOWORD(wParam)); break;
		case ID_CURRENTSAVE_9: SetCurrentSaveState(hWnd,LOWORD(wParam)); break;
		case ID_OPTIONS_FULLSCREEN: 
			CPU_Action.ChangeWindow = TRUE;
			CPU_Action.DoSomething = TRUE;
			break;
		case ID_OPTIONS_CONFIG_GFX: GFXDllConfig(hWnd); break;
		case ID_OPTIONS_CONFIG_AUDIO: AiDllConfig(hWnd); break;
		case ID_OPTIONS_CONFIG_RSP: RSPDllConfig(hWnd); break;
		case ID_OPTIONS_CONFIG_CONTROL: ContConfig(hWnd); break;
		case ID_OPTIONS_SETTINGS: ChangeSettings(hWnd); break;
		case ID_OPTIONS_CHEATS: ManageCheats(NULL); break;
		case ID_OPTIONS_SHOWCPUUSAGE:
			hMenu = GetMenu( hWnd );
			uState = GetMenuState( hMenu, ID_OPTIONS_SHOWCPUUSAGE, MF_BYCOMMAND);			
			hKeyResults = 0;
			Disposition = 0;

			if ( uState & MFS_CHECKED ) {								
				CheckMenuItem( hMenu, ID_OPTIONS_SHOWCPUUSAGE, MF_BYCOMMAND | MFS_UNCHECKED );
				ShowCPUPer = FALSE;
			} else {
				CheckMenuItem( hMenu, ID_OPTIONS_SHOWCPUUSAGE, MF_BYCOMMAND | MFS_CHECKED );
				ShowCPUPer = TRUE;
			}
		
			sprintf(String,"Software\\N64 Emulation\\%s",AppName);
			lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
				REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
		
			if (lResult == ERROR_SUCCESS) { 
				RegSetValueEx(hKeyResults,"Show CPU %",0,REG_DWORD,(BYTE *)&ShowCPUPer,sizeof(DWORD));
			}
			RegCloseKey(hKeyResults);
			break;
#if (!defined(EXTERNAL_RELEASE))
		case ID_OPTIONS_PROFILING_ON:
		case ID_OPTIONS_PROFILING_OFF:
			if (HaveDebugger) {
				hMenu = GetMenu( hWnd );
				uState = GetMenuState( hMenu, ID_OPTIONS_PROFILING_ON, MF_BYCOMMAND);			
				hKeyResults = 0;
				Disposition = 0;

				if ( uState & MFS_CHECKED ) {								
					CheckMenuItem( hMenu, ID_OPTIONS_PROFILING_ON, MF_BYCOMMAND | MFS_UNCHECKED );
					CheckMenuItem( hMenu, ID_OPTIONS_PROFILING_OFF, MF_BYCOMMAND | MFS_CHECKED );
					Profiling = FALSE;
					ResetTimerList();
				} else {
					CheckMenuItem( hMenu, ID_OPTIONS_PROFILING_ON, MF_BYCOMMAND | MFS_CHECKED );
					CheckMenuItem( hMenu, ID_OPTIONS_PROFILING_OFF, MF_BYCOMMAND | MFS_UNCHECKED );
					ResetTimerList();
					Profiling = TRUE;
				}
			
				sprintf(String,"Software\\N64 Emulation\\%s",AppName);
				lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
				if (lResult == ERROR_SUCCESS) { 
					RegSetValueEx(hKeyResults,"Profiling On",0,REG_DWORD,(BYTE *)&Profiling,sizeof(DWORD));
				}
				RegCloseKey(hKeyResults);
			}
			break;
		case ID_OPTIONS_PROFILING_RESETSTATS: 
			if (HaveDebugger) { ResetTimerList(); }
			break;
		case ID_OPTIONS_PROFILING_GENERATELOG: 
			if (HaveDebugger) { GenerateTimerResults(); }
			break;
		case ID_OPTIONS_PROFILING_LOGINDIVIDUALBLOCKS:
			if (HaveDebugger) {
				hMenu = GetMenu( hWnd );
				uState = GetMenuState( hMenu, ID_OPTIONS_PROFILING_LOGINDIVIDUALBLOCKS, MF_BYCOMMAND);			
				hKeyResults = 0;
				Disposition = 0;

				if ( uState & MFS_CHECKED ) {								
					CheckMenuItem( hMenu, ID_OPTIONS_PROFILING_LOGINDIVIDUALBLOCKS, MF_BYCOMMAND | MFS_UNCHECKED );
					IndvidualBlock = FALSE;
				} else {
					CheckMenuItem( hMenu, ID_OPTIONS_PROFILING_LOGINDIVIDUALBLOCKS, MF_BYCOMMAND | MFS_CHECKED );
					IndvidualBlock = TRUE;
				}
			
				sprintf(String,"Software\\N64 Emulation\\%s",AppName);
				lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
				if (lResult == ERROR_SUCCESS) { 
					RegSetValueEx(hKeyResults,"Log Indvidual Blocks",0,REG_DWORD,
						(BYTE *)&IndvidualBlock,sizeof(DWORD));
				}
				RegCloseKey(hKeyResults);
			}
			break;
		case ID_OPTIONS_MAPPINGS_OPENMAPFILE: ChooseMapFile(hWnd); break;
		case ID_OPTIONS_MAPPINGS_CLOSEMAPFILE: ResetMappings(); break;
		case ID_OPTIONS_MAPPINGS_AUTOLOADMAPFILE:
			if (HaveDebugger) {
				hMenu = GetMenu( hWnd );
				uState = GetMenuState( hMenu, ID_OPTIONS_MAPPINGS_AUTOLOADMAPFILE, MF_BYCOMMAND);			
				hKeyResults = 0;
				Disposition = 0;

				if ( uState & MFS_CHECKED ) {								
					CheckMenuItem( hMenu, ID_OPTIONS_MAPPINGS_AUTOLOADMAPFILE, MF_BYCOMMAND | MFS_UNCHECKED );
					AutoLoadMapFile = FALSE;
				} else {
					CheckMenuItem( hMenu, ID_OPTIONS_MAPPINGS_AUTOLOADMAPFILE, MF_BYCOMMAND | MFS_CHECKED );
					AutoLoadMapFile = TRUE;
				}
			
				sprintf(String,"Software\\N64 Emulation\\%s",AppName);
				lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
				if (lResult == ERROR_SUCCESS) { 
					RegSetValueEx(hKeyResults,"Auto Load Map File",0,REG_DWORD,
						(BYTE *)&AutoLoadMapFile,sizeof(DWORD));
				}
				RegCloseKey(hKeyResults);
			}
			break;
		case ID_DEBUGGER_SETBREAKPOINT: Enter_BPoint_Window(); break;
		case ID_DEBUGGER_R4300ICOMMANDS: Enter_R4300i_Commands_Window(); break;
		case ID_DEBUGGER_R4300IREGISTERS: Enter_R4300i_Register_Window(); break;
		case ID_DEBUGGER_LOGOPTIONS: EnterLogOptions(hWnd); break;
		case ID_DEBUGGER_GENERATELOG:
			if (HaveDebugger) {
				hMenu = GetMenu( hWnd );
				uState = GetMenuState( hMenu, ID_DEBUGGER_GENERATELOG, MF_BYCOMMAND);			
				hKeyResults = 0;
				Disposition = 0;

				if ( uState & MFS_CHECKED ) {								
					CheckMenuItem( hMenu, ID_DEBUGGER_GENERATELOG, MF_BYCOMMAND | MFS_UNCHECKED );
					LogOptions.GenerateLog = FALSE;
				} else {
					CheckMenuItem( hMenu, ID_DEBUGGER_GENERATELOG, MF_BYCOMMAND | MFS_CHECKED );
					LogOptions.GenerateLog = TRUE;
				}
			
				sprintf(String,"Software\\N64 Emulation\\%s\\Logging",AppName);
				lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
				if (lResult == ERROR_SUCCESS) { 
					RegSetValueEx(hKeyResults,"Generate Log File",0,
						REG_DWORD,(BYTE *)&LogOptions.GenerateLog,sizeof(DWORD));
				}
				RegCloseKey(hKeyResults);
				LoadLogOptions(&LogOptions, FALSE);
#ifndef EXTERNAL_RELEASE
				StartLog();
#endif
			}
			break;
		case ID_DEBUGGER_MEMORY: Enter_Memory_Window(); break;
		case ID_DEBUGGER_TLBENTRIES: Enter_TLB_Window(); break;
		case ID_DEBUGGER_SHOWUNHANDLEDMEMORYACCESSES:
			if (HaveDebugger) {
				hMenu = GetMenu( hWnd );
				uState = GetMenuState( hMenu, ID_DEBUGGER_SHOWUNHANDLEDMEMORYACCESSES, MF_BYCOMMAND);			
				hKeyResults = 0;
				Disposition = 0;

				if ( uState & MFS_CHECKED ) {								
					CheckMenuItem( hMenu, ID_DEBUGGER_SHOWUNHANDLEDMEMORYACCESSES, MF_BYCOMMAND | MFS_UNCHECKED );
					ShowUnhandledMemory = FALSE;
				} else {
					CheckMenuItem( hMenu, ID_DEBUGGER_SHOWUNHANDLEDMEMORYACCESSES, MF_BYCOMMAND | MFS_CHECKED );
					ShowUnhandledMemory = TRUE;
				}
			
				sprintf(String,"Software\\N64 Emulation\\%s",AppName);
				lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
				if (lResult == ERROR_SUCCESS) { 
					RegSetValueEx(hKeyResults,"Show Unhandled Memory Accesses",0,
						REG_DWORD,(BYTE *)&ShowUnhandledMemory,sizeof(DWORD));
				}
				RegCloseKey(hKeyResults);
			}
			break;
		case ID_DEBUGGER_SHOWTLBMISSES:
			if (HaveDebugger) {
				hMenu = GetMenu( hWnd );
				uState = GetMenuState( hMenu, ID_DEBUGGER_SHOWTLBMISSES, MF_BYCOMMAND);			
				hKeyResults = 0;
				Disposition = 0;

				if ( uState & MFS_CHECKED ) {								
					CheckMenuItem( hMenu, ID_DEBUGGER_SHOWTLBMISSES, MF_BYCOMMAND | MFS_UNCHECKED );
					ShowTLBMisses = FALSE;
				} else {
					CheckMenuItem( hMenu, ID_DEBUGGER_SHOWTLBMISSES, MF_BYCOMMAND | MFS_CHECKED );
					ShowTLBMisses = TRUE;
				}
			
				sprintf(String,"Software\\N64 Emulation\\%s",AppName);
				lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
				if (lResult == ERROR_SUCCESS) { 
					RegSetValueEx(hKeyResults,"Show Load/Store TLB Misses",0,
						REG_DWORD,(BYTE *)&ShowTLBMisses,sizeof(DWORD));
				}
				RegCloseKey(hKeyResults);
			}
			break;
		case ID_DEBUGGER_SHOWDLISTALISTCOUNT:
			if (HaveDebugger) {
				hMenu = GetMenu( hWnd );
				uState = GetMenuState( hMenu, ID_DEBUGGER_SHOWDLISTALISTCOUNT, MF_BYCOMMAND);			
				hKeyResults = 0;
				Disposition = 0;

				if ( uState & MFS_CHECKED ) {								
					CheckMenuItem( hMenu, ID_DEBUGGER_SHOWDLISTALISTCOUNT, MF_BYCOMMAND | MFS_UNCHECKED );
					ShowDListAListCount = FALSE;
				} else {
					CheckMenuItem( hMenu, ID_DEBUGGER_SHOWDLISTALISTCOUNT, MF_BYCOMMAND | MFS_CHECKED );
					ShowDListAListCount = TRUE;
				}
			
				sprintf(String,"Software\\N64 Emulation\\%s",AppName);
				lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
				if (lResult == ERROR_SUCCESS) { 
					RegSetValueEx(hKeyResults,"Show Dlist/Alist Count",0,
						REG_DWORD,(BYTE *)&ShowDListAListCount,sizeof(DWORD));
				}
				RegCloseKey(hKeyResults);
			}
			break;
		case ID_DEBUGGER_SHOWCOMPMEM:
			if (HaveDebugger) {
				hMenu = GetMenu( hWnd );
				uState = GetMenuState( hMenu, ID_DEBUGGER_SHOWCOMPMEM, MF_BYCOMMAND);			
				hKeyResults = 0;
				Disposition = 0;

				if ( uState & MFS_CHECKED ) {								
					CheckMenuItem( hMenu, ID_DEBUGGER_SHOWCOMPMEM, MF_BYCOMMAND | MFS_UNCHECKED );
					ShowCompMem = FALSE;
				} else {
					CheckMenuItem( hMenu, ID_DEBUGGER_SHOWCOMPMEM, MF_BYCOMMAND | MFS_CHECKED );
					ShowCompMem = TRUE;
				}
			
				sprintf(String,"Software\\N64 Emulation\\%s",AppName);
				lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
				if (lResult == ERROR_SUCCESS) { 
					RegSetValueEx(hKeyResults,"Show Compile Memory",0,
						REG_DWORD,(BYTE *)&ShowCompMem,sizeof(DWORD));
				}
				RegCloseKey(hKeyResults);
			}
			break;
		case ID_DEBUGGER_SHOWPIFRAMERRORS:
			if (HaveDebugger) {
				hMenu = GetMenu( hWnd );
				uState = GetMenuState( hMenu, ID_DEBUGGER_SHOWPIFRAMERRORS, MF_BYCOMMAND);			
				hKeyResults = 0;
				Disposition = 0;

				if ( uState & MFS_CHECKED ) {								
					CheckMenuItem( hMenu, ID_DEBUGGER_SHOWPIFRAMERRORS, MF_BYCOMMAND | MFS_UNCHECKED );
					ShowPifRamErrors = FALSE;
				} else {
					CheckMenuItem( hMenu, ID_DEBUGGER_SHOWPIFRAMERRORS, MF_BYCOMMAND | MFS_CHECKED );
					ShowPifRamErrors = TRUE;
				}
			
				sprintf(String,"Software\\N64 Emulation\\%s",AppName);
				lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
					REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
				if (lResult == ERROR_SUCCESS) { 
					RegSetValueEx(hKeyResults,"Show Pif Ram Errors",0,
						REG_DWORD,(BYTE *)&ShowPifRamErrors,sizeof(DWORD));
				}
				RegCloseKey(hKeyResults);
			}
			break;
#endif
		case ID_HELP_CONTENTS:
			{
				char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
				char fname[_MAX_FNAME],ext[_MAX_EXT], HelpFileName[_MAX_PATH];

				GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
				_splitpath( path_buffer, drive, dir, fname, ext );
   				_makepath( HelpFileName, drive, dir, "Project64", "chm" );
				HtmlHelp(hWnd,HelpFileName,HH_DISPLAY_TOPIC,0);
			}
			break;
		case ID_HELPMNU_INDEX:
			{
				char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
				char fname[_MAX_FNAME],ext[_MAX_EXT], HelpFileName[_MAX_PATH];

				GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
				_splitpath( path_buffer, drive, dir, fname, ext );
   				_makepath( HelpFileName, drive, dir, "Project64", "chm" );
				HtmlHelp(hWnd,HelpFileName,HH_DISPLAY_INDEX,0);
			}
			break;
		case ID_HELP_ABOUT: AboutBox(); break;
		case ID_HELP_ABOUTSETTINGFILES: AboutIniBox(); break;
		default:
			if (LOWORD(wParam) >= ID_FILE_RECENT_FILE && LOWORD(wParam) <= (ID_FILE_RECENT_FILE + RomsToRemember)) {
				LoadRecentRom(LOWORD(wParam));
			} else if (LOWORD(wParam) >= ID_FILE_RECENT_DIR && LOWORD(wParam) <= (ID_FILE_RECENT_DIR + RomDirsToRemember)) {
				SetRecentRomDir(LOWORD(wParam));
			} else if (LOWORD(wParam) >= ID_LANG_SELECT && LOWORD(wParam) <= (ID_LANG_SELECT + 100)) {
				SelectLang(hWnd,LOWORD(wParam));
				FixMenuLang(hWnd);
				FixRomBrowserColoumnLang();
			} else if (LOWORD(wParam) > 5000 && LOWORD(wParam) <= 5100 ) { 
				if (RspDebug.ProcessMenuItem != NULL) {
					RspDebug.ProcessMenuItem(LOWORD(wParam));
				}
			} else if (LOWORD(wParam) > 5100 && LOWORD(wParam) <= 5200 ) { 
				if (GFXDebug.ProcessMenuItem != NULL) {
					GFXDebug.ProcessMenuItem(LOWORD(wParam));
				}
			}
		}
		break;
	case WM_DESTROY:
		SaveRomBrowserColoumnInfo();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return TRUE;
}

void SelectLang(HWND hWnd, int LangMenuID) {
	char * LangFile, String[800];
	HMENU hMenu = GetMenu(hWnd);
	static int LastLang = -1;
	MENUITEMINFO menuinfo;

	menuinfo.cbSize = sizeof(MENUITEMINFO);
	menuinfo.fMask = MIIM_TYPE;
	menuinfo.fType = MFT_STRING;
	menuinfo.dwTypeData = String;
	menuinfo.cch = sizeof(String);
	GetMenuItemInfo(hMenu,LangMenuID,FALSE,&menuinfo);

	LangFile = GetLangFileName();
	_WritePrivateProfileString("Default","Language",String,LangFile);
	if (LastLang != -1) {
		CheckMenuItem( hMenu, LastLang, MF_BYCOMMAND | MFS_UNCHECKED );
	}
	LastLang = LangMenuID;
	CheckMenuItem( hMenu, LastLang, MF_BYCOMMAND | MFS_CHECKED );
}

void RegisterExtension ( char * Extension, BOOL RegisterWithPj64 ) {
	char ShortAppName[] = { "PJ64" }; 
	char sKeyValue[] = { "Project 64" };
  	char app_path[_MAX_PATH];
	
	char String[200];
	DWORD Disposition = 0;
	HKEY hKeyResults = 0;
	long lResult;

	//Find Application name
	GetModuleFileName(NULL,app_path,sizeof(app_path));
 
	//creates a Root entry for sKeyName
	lResult = RegCreateKeyEx( HKEY_CLASSES_ROOT, ShortAppName,0,"", REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
	RegSetValueEx(hKeyResults,"",0,REG_SZ,(BYTE *)sKeyValue,sizeof(sKeyValue));
	RegCloseKey(hKeyResults);

	// Set the command line for "MyApp".
	sprintf(String,"%s\\DefaultIcon",ShortAppName);
	lResult = RegCreateKeyEx( HKEY_CLASSES_ROOT, String,0,"", REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
	sprintf(String,"%s",app_path);
	RegSetValueEx(hKeyResults,"",0,REG_SZ,(BYTE *)String,strlen(String));
	RegCloseKey(hKeyResults);
	
	//set the icon for the file extension
	sprintf(String,"%s\\shell\\open\\command",ShortAppName);
	lResult = RegCreateKeyEx( HKEY_CLASSES_ROOT, String,0,"", REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
	sprintf(String,"%s %%1",app_path);
	RegSetValueEx(hKeyResults,"",0,REG_SZ,(BYTE *)String,strlen(String));
	RegCloseKey(hKeyResults);

	// creates a Root entry for passed associated with sKeyName
	lResult = RegCreateKeyEx( HKEY_CLASSES_ROOT, Extension,0,"", REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
	if (RegisterWithPj64) {
		RegSetValueEx(hKeyResults,"",0,REG_SZ,(BYTE *)ShortAppName,sizeof(ShortAppName));
	} else {
		RegSetValueEx(hKeyResults,"",0,REG_SZ,(BYTE *)"",1);
	}
 
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);	
}

int RegisterWinClass ( void ) {
	WNDCLASS wcl;

	wcl.style			= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wcl.cbClsExtra		= 0;
	wcl.cbWndExtra		= 0;
	wcl.hIcon			= LoadIcon(hInst,"ICON");
	wcl.hCursor			= NULL;	
	wcl.hInstance		= hInst;

	wcl.lpfnWndProc		= (WNDPROC)Main_Proc;
	wcl.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcl.lpszMenuName	= MAKEINTRESOURCE(MAIN_MENU);
	wcl.lpszClassName	= AppName;
	if (RegisterClass(&wcl)  == 0) return FALSE;

	return TRUE;
}

void RomInfo (void) {
	DialogBox(hInst, "ROM_INFO_DIALOG", hMainWindow, (DLGPROC)RomInfoProc);
}

LRESULT CALLBACK RomInfoProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char String[255], count;
	DWORD CRC;

	switch (uMsg) {
	case WM_INITDIALOG:
		
		memcpy(&String[1],(void *)(ROM + 0x20),20);
		for( count = 1 ; count < 21; count += 4 ) {
			String[count] ^= String[count+3];
			String[count + 3] ^= String[count];
			String[count] ^= String[count+3];			
			String[count + 1] ^= String[count + 2];
			String[count + 2] ^= String[count + 1];
			String[count + 1] ^= String[count + 2];			
		}
		String[0] = ' ';
		String[21] = '\0';
		SetDlgItemText(hDlg,IDC_INFO_ROMNAME,String);

		strcpy(&String[1],CurrentFileName);
		SetDlgItemText(hDlg,IDC_INFO_FILENAME,String);
		
		sprintf(&String[1],"%.1f MBit",(float)RomFileSize/0x20000);
		SetDlgItemText(hDlg,IDC_INFO_ROMSIZE,String);

		String[1] = *(ROM + 0x3F);
		String[2] = *(ROM + 0x3E);
		String[3] = '\0';
		SetDlgItemText(hDlg,IDC_INFO_CARTID,String);
		
		switch (*(ROM + 0x38)) {
		case 'N':
			SetDlgItemText(hDlg,IDC_INFO_MANUFACTURER," Nintendo");
			break;
		case 0:
			SetDlgItemText(hDlg,IDC_INFO_MANUFACTURER," None");
			break;
		default:
			SetDlgItemText(hDlg,IDC_INFO_MANUFACTURER," (Unknown)");
			break;
		}

		switch (*(ROM + 0x3D)) {
		case 'D': SetDlgItemText(hDlg,IDC_INFO_COUNTRY," Germany"); break;
		case 'E': SetDlgItemText(hDlg,IDC_INFO_COUNTRY," USA"); break;
		case 'F': SetDlgItemText(hDlg,IDC_INFO_COUNTRY," French"); break;
		case 'J': SetDlgItemText(hDlg,IDC_INFO_COUNTRY," Japan"); break;
		case 'I': SetDlgItemText(hDlg,IDC_INFO_COUNTRY," Italian"); break;
		case 'P': SetDlgItemText(hDlg,IDC_INFO_COUNTRY," Europe"); break;
		case 'S': SetDlgItemText(hDlg,IDC_INFO_COUNTRY," Spanish"); break;
		case 'U': SetDlgItemText(hDlg,IDC_INFO_COUNTRY," Australia"); break;
		case 0: SetDlgItemText(hDlg,IDC_INFO_COUNTRY," None"); break;
		default:
			sprintf(&String[1]," Unknown %c (%02X)",*(ROM + 0x3D),*(ROM + 0x3D));
			SetDlgItemText(hDlg,IDC_INFO_COUNTRY,String);
		}
		
		CRC = *(ROM + 0x13);
		CRC = (CRC << 8) +  *(ROM + 0x12);
		CRC = (CRC << 8) +  *(ROM + 0x11);
		CRC = (CRC << 8) +  *(ROM + 0x10);
		sprintf(&String[1],"0x%08X",CRC);
		SetDlgItemText(hDlg,IDC_INFO_CRC1,String);

		CRC = *(ROM + 0x17);
		CRC = (CRC << 8) +  *(ROM + 0x16);
		CRC = (CRC << 8) +  *(ROM + 0x15);
		CRC = (CRC << 8) +  *(ROM + 0x14);
		sprintf(&String[1],"0x%08X",CRC);
		SetDlgItemText(hDlg,IDC_INFO_CRC2,String);

		if (GetCicChipID(ROM) < 0) { 
			sprintf(&String[1],"Unknown");
		} else {
			sprintf(&String[1],"CIC-NUS-610%d",GetCicChipID(ROM));
		}
		SetDlgItemText(hDlg,IDC_INFO_CIC,String);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CLOSE_BUTTON:
			EndDialog(hDlg,0);
			break;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL TestExtensionRegistered ( char * Extension ) {
	char ShortAppName[] = { "PJ64" }; 
	HKEY hKeyResults = 0;
	char Association[100];
	long lResult;
	DWORD Type, Bytes;

	lResult = RegOpenKey( HKEY_CLASSES_ROOT,Extension,&hKeyResults);	
	if (lResult != ERROR_SUCCESS) { return FALSE; }
	
	Bytes = sizeof(Association);
	lResult = RegQueryValueEx(hKeyResults,"",0,&Type,(LPBYTE)(&Association),&Bytes);
	RegCloseKey(hKeyResults);
	if (lResult != ERROR_SUCCESS) { return FALSE;  }

	if (strcmp(Association,ShortAppName) != 0) { return FALSE; }
	return TRUE;
}

void SetupMenu ( HWND hWnd ) {
	char * LangFile, String[800], Language[100], *RP;
	HMENU hMenu = GetMenu(hWnd), hSubMenu;
	MENUITEMINFO menuinfo;
	int count;

	FixMenuLang(hWnd);

	LangFile = GetLangFileName();
#ifdef EXTERNAL_RELEASE
	hSubMenu = GetSubMenu(hMenu,3);
#else
	hSubMenu = GetSubMenu(hMenu,4);
#endif 
	_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);
	menuinfo.cbSize = sizeof(MENUITEMINFO);
	menuinfo.fMask = MIIM_TYPE|MIIM_ID;
	menuinfo.fType = MFT_STRING;
	menuinfo.fState = MFS_ENABLED;
	menuinfo.dwTypeData = String;
	menuinfo.cch = sizeof(String);
	GetMenuItemInfo(hSubMenu,0,TRUE,&menuinfo);
	menuinfo.wID = ID_LANG_SELECT;
	SetMenuItemInfo(hSubMenu,0,TRUE,&menuinfo);

	memset(String,0,sizeof(String));
	GetPrivateProfileSectionNames(String,sizeof(String),LangFile);	
	for (count = 1,RP = String; *RP != 0; RP += strlen(RP) + 1) {
		if (strcmp(RP,"English") == 0) { 
			if (strcmp(RP,Language) == 0) { SelectLang(hWnd,ID_LANG_SELECT); }
			continue; 
		}
		if (strcmp(RP,"Default") == 0) { continue; }
		menuinfo.dwTypeData = RP;
		menuinfo.cch = strlen(RP) + 1;
		menuinfo.wID = ID_LANG_SELECT + count;
		InsertMenuItem(hSubMenu, count++, TRUE, &menuinfo);
		if (strcmp(RP,Language) == 0) { SelectLang(hWnd,menuinfo.wID); }
	}
	

	LoadRecentFiles(hWnd);
	LoadRecentDirs(hWnd);
	SetCurrentSaveState(hWnd,ID_CURRENTSAVE_DEFAULT);

	if (LimitFPS) {
		CheckMenuItem( hMenu, ID_SYSTEM_LIMITFPS, MF_BYCOMMAND | MFS_CHECKED );
	}
	if (ShowCPUPer) {
		CheckMenuItem( hMenu, ID_OPTIONS_SHOWCPUUSAGE, MF_BYCOMMAND | MFS_CHECKED );
	}
#if (!defined(EXTERNAL_RELEASE))
	if (HaveDebugger) {
		if (AutoLoadMapFile) {
			CheckMenuItem( hMenu, ID_OPTIONS_MAPPINGS_AUTOLOADMAPFILE, MF_BYCOMMAND | MFS_CHECKED );
		}
		if (LogOptions.GenerateLog) {
			CheckMenuItem( hMenu, ID_DEBUGGER_GENERATELOG, MF_BYCOMMAND | MFS_CHECKED );
		}
		if (ShowUnhandledMemory) {
			CheckMenuItem( hMenu, ID_DEBUGGER_SHOWUNHANDLEDMEMORYACCESSES, MF_BYCOMMAND | MFS_CHECKED );
		}
		if (ShowTLBMisses) {
			CheckMenuItem( hMenu, ID_DEBUGGER_SHOWTLBMISSES, MF_BYCOMMAND | MFS_CHECKED );
		}
		if (ShowDListAListCount) {
			CheckMenuItem( hMenu, ID_DEBUGGER_SHOWDLISTALISTCOUNT, MF_BYCOMMAND | MFS_CHECKED );
		}
		if (ShowCompMem) {
			CheckMenuItem( hMenu, ID_DEBUGGER_SHOWCOMPMEM, MF_BYCOMMAND | MFS_CHECKED );
		}
		if (ShowPifRamErrors) {
			CheckMenuItem( hMenu, ID_DEBUGGER_SHOWPIFRAMERRORS, MF_BYCOMMAND | MFS_CHECKED );
		}
		if (IndvidualBlock) {
			CheckMenuItem( hMenu, ID_OPTIONS_PROFILING_LOGINDIVIDUALBLOCKS, MF_BYCOMMAND | MFS_CHECKED );
		}
		
		if (Profiling) {
			CheckMenuItem( hMenu, ID_OPTIONS_PROFILING_ON, MF_BYCOMMAND | MFS_CHECKED );
		} else {
			CheckMenuItem( hMenu, ID_OPTIONS_PROFILING_OFF, MF_BYCOMMAND | MFS_CHECKED );
		}
	}
	
	/* Debugger Menu */
	if (!HaveDebugger) { DeleteMenu(hMenu,3,MF_BYPOSITION);  }
#endif 
#ifdef BETA_VERSION
	{
		MENUITEMINFO menuinfo;
		char String[256];

		hSubMenu = GetSubMenu(hMenu,4);
		memset(&menuinfo, 0, sizeof(MENUITEMINFO));
		menuinfo.cbSize = sizeof(MENUITEMINFO);		
		menuinfo.fMask = MIIM_TYPE;
		menuinfo.fType = MFT_STRING;
		menuinfo.fState = MFS_ENABLED;
		menuinfo.dwTypeData = String;
		menuinfo.cch = sizeof(String);
		strcpy(String,"Beta For: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
		InsertMenuItem(hSubMenu, 0, TRUE, &menuinfo);
		strcpy(String,"Email: ?????????????????????????????????????????????????????????????????????????????????");
		InsertMenuItem(hSubMenu, 1, TRUE, &menuinfo);
	}
#endif
}

void SetupMenuTitle ( HMENU hMenu, int MenuPos, char * ShotCut, char * Title, char * Language, char *  LangFile) {
	MENUITEMINFO MenuInfo;
	char String[256];

	memset(&MenuInfo, 0, sizeof(MENUITEMINFO));
	MenuInfo.cbSize = sizeof(MENUITEMINFO);
	MenuInfo.fMask = MIIM_TYPE;
	MenuInfo.fType = MFT_STRING;
	MenuInfo.fState = MFS_ENABLED;
	MenuInfo.dwTypeData = String;
	MenuInfo.cch = 256;

	GetMenuItemInfo(hMenu,MenuPos,TRUE,&MenuInfo);
	if (strchr(String,'\t') != NULL) { *(strchr(String,'\t')) = '\0'; }
	_GetPrivateProfileString(Language,Title,String,String,sizeof(String),LangFile);
	if (ShotCut) { sprintf(String,"%s\t%s",String,ShotCut); }
	SetMenuItemInfo(hMenu,MenuPos,TRUE,&MenuInfo);
}


void SetCurrentSaveState (HWND hWnd, int State) {
	char String[256];
	HMENU hMenu;
	
	hMenu = GetMenu(hWnd);
	if (hCPU == NULL) { State = ID_CURRENTSAVE_DEFAULT; }

	CheckMenuItem( hMenu, ID_CURRENTSAVE_DEFAULT, MF_BYCOMMAND | MFS_UNCHECKED );
	CheckMenuItem( hMenu, ID_CURRENTSAVE_0, MF_BYCOMMAND | MFS_UNCHECKED );
	CheckMenuItem( hMenu, ID_CURRENTSAVE_1, MF_BYCOMMAND | MFS_UNCHECKED );
	CheckMenuItem( hMenu, ID_CURRENTSAVE_2, MF_BYCOMMAND | MFS_UNCHECKED );
	CheckMenuItem( hMenu, ID_CURRENTSAVE_3, MF_BYCOMMAND | MFS_UNCHECKED );
	CheckMenuItem( hMenu, ID_CURRENTSAVE_4, MF_BYCOMMAND | MFS_UNCHECKED );
	CheckMenuItem( hMenu, ID_CURRENTSAVE_5, MF_BYCOMMAND | MFS_UNCHECKED );
	CheckMenuItem( hMenu, ID_CURRENTSAVE_6, MF_BYCOMMAND | MFS_UNCHECKED );
	CheckMenuItem( hMenu, ID_CURRENTSAVE_7, MF_BYCOMMAND | MFS_UNCHECKED );
	CheckMenuItem( hMenu, ID_CURRENTSAVE_8, MF_BYCOMMAND | MFS_UNCHECKED );
	CheckMenuItem( hMenu, ID_CURRENTSAVE_9, MF_BYCOMMAND | MFS_UNCHECKED );
	CheckMenuItem( hMenu, State, MF_BYCOMMAND | MFS_CHECKED );
	if (strlen(RomName) == 0) { return; }
	
	strcpy(CurrentSave,RomName);
	
	switch (State) {
	case ID_CURRENTSAVE_DEFAULT: strcat(CurrentSave,".pj"); break;
	case ID_CURRENTSAVE_0: strcat(CurrentSave,".pj0"); break;
	case ID_CURRENTSAVE_1: strcat(CurrentSave,".pj1"); break;
	case ID_CURRENTSAVE_2: strcat(CurrentSave,".pj2"); break;
	case ID_CURRENTSAVE_3: strcat(CurrentSave,".pj3"); break;
	case ID_CURRENTSAVE_4: strcat(CurrentSave,".pj4"); break;
	case ID_CURRENTSAVE_5: strcat(CurrentSave,".pj5"); break;
	case ID_CURRENTSAVE_6: strcat(CurrentSave,".pj6"); break;
	case ID_CURRENTSAVE_7: strcat(CurrentSave,".pj7"); break;
	case ID_CURRENTSAVE_8: strcat(CurrentSave,".pj8"); break;
	case ID_CURRENTSAVE_9: strcat(CurrentSave,".pj9"); break;
	}

	sprintf(String,"Save state slot: %s",CurrentSave);
	SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)String ); 
}


void ShutdownApplication ( void ) {
	CloseCpu();
	if (TargetInfo != NULL) { VirtualFree(TargetInfo,0,MEM_RELEASE); }
	ShutdownPlugins();
	SaveRecentFiles();
	Release_Memory();
#if (!defined(EXTERNAL_RELEASE))
	StopLog();
#endif
	CloseHandle(hPauseMutex);
}

void StoreCurrentWinPos (  char * WinName, HWND hWnd ) {
	long lResult;
	HKEY hKeyResults = 0;
	DWORD Disposition = 0;
	RECT WinRect;
	char String[200];

	GetWindowRect(hWnd, &WinRect );
	sprintf(String,"Software\\N64 Emulation\\%s\\Page Setup",AppName);
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
	if (lResult == ERROR_SUCCESS) {
		sprintf(String,"%s Top",WinName);
		RegSetValueEx(hKeyResults,String,0, REG_DWORD,(CONST BYTE *)(&WinRect.top),
			sizeof(DWORD));
		sprintf(String,"%s Left",WinName);
		RegSetValueEx(hKeyResults,String,0, REG_DWORD,(CONST BYTE *)(&WinRect.left),
			sizeof(DWORD));
	}
	RegCloseKey(hKeyResults);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgs, int nWinMode) {
#define WindowWidth  640
#define WindowHeight 480
	HACCEL Accel, AccelCPU, AccelRB;
	DWORD X, Y;
	MSG msg;

	//DisplayError("%X",r4300i_COP1_MF);
	if ( !InitalizeApplication (hInstance) ) { return FALSE; }
	if ( !RegisterWinClass () ) { return FALSE; }
	if ( !GetStoredWinPos( "Main", &X, &Y ) ) {
  		X = (GetSystemMetrics( SM_CXSCREEN ) - WindowWidth) / 2;
		Y = (GetSystemMetrics( SM_CYSCREEN ) - WindowHeight) / 2;
	}

	Accel = LoadAccelerators(hInst,MAKEINTRESOURCE(IDR_ACCELERATOR));	
	AccelCPU = LoadAccelerators(hInst,MAKEINTRESOURCE(IDR_CPU));	
	AccelRB = LoadAccelerators(hInst,MAKEINTRESOURCE(IDR_ROMBROWSER));	
	
	hHiddenWin = CreateWindow( AppName, AppName, WS_OVERLAPPED | WS_CLIPCHILDREN | 
		WS_CLIPSIBLINGS | WS_SYSMENU | WS_MINIMIZEBOX,X,Y,WindowWidth,WindowHeight,
		NULL,NULL,hInst,NULL
	);

	if ( !hHiddenWin ) { return FALSE; }

	hMainWindow = CreateWindow( AppName, AppName, WS_OVERLAPPED | WS_CLIPCHILDREN | 
		WS_CLIPSIBLINGS | WS_SYSMENU | WS_MINIMIZEBOX,X,Y,WindowWidth,WindowHeight,
		NULL,NULL,hInst,NULL
	);
	
	if ( !hMainWindow ) { return FALSE; }
	if (strlen(lpszArgs) > 0) {
		DWORD ThreadID;

		SetupPlugins(hMainWindow); 
		ShowWindow(hMainWindow, nWinMode);
		strcpy(CurrentFileName,lpszArgs);
		CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)OpenChosenFile,NULL,0, &ThreadID);	
	} else {		
		if (RomBrowser) { 
			ShowRomList(hMainWindow); 
		} else {
			SetupPlugins(hMainWindow); 
			ShowWindow(hMainWindow, nWinMode);
		}
	}

	while (GetMessage(&msg,NULL,0,0)) {
		if (hCPU == NULL && TranslateAccelerator(hMainWindow,AccelRB,&msg)) { continue; }
		if (hCPU != NULL && TranslateAccelerator(hMainWindow,AccelCPU,&msg)) { continue; }
		if (!TranslateAccelerator(hMainWindow,Accel,&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	ShutdownApplication ();
	return msg.wParam;
}
