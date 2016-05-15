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
#include <shlwapi.h>
#include <shlobj.h>
#include <stdio.h>
#include "main.h"
#include "cpu.h"
#include "plugin.h"
#include "debugger.h"
#include "resource.h"

LRESULT CALLBACK DefaultOptionsProc   ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK DirSelectProc        ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK PluginSelectProc     ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK RomBrowserProc       ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK RomSettingsProc      ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK ShellIntegrationProc ( HWND, UINT, WPARAM, LPARAM );

void ChangeSettings(HWND hwndOwner) {
	char * LangFile, Language[100], String[6][256];
    PROPSHEETPAGE psp[6];
    PROPSHEETHEADER psh;

	LangFile = GetLangFileName();
	_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE;
    psp[0].hInstance = hInst;
    psp[0].pszTemplate = MAKEINTRESOURCE(PLUGIN_SELECT);
    psp[0].pfnDlgProc = PluginSelectProc;
	_GetPrivateProfileString(Language,"SettingsTab1","Plugins",String[0],sizeof(String[0]),LangFile);
    psp[0].pszTitle = String[0];
    psp[0].lParam = 0;
    psp[0].pfnCallback = NULL;

    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USETITLE;
    psp[1].hInstance = hInst;
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_Settings_Directory);
    psp[1].pfnDlgProc = DirSelectProc;
	_GetPrivateProfileString(Language,"SettingsTab2","Directories",String[1],sizeof(String[1]),LangFile);
    psp[1].pszTitle = String[1];
    psp[1].lParam = 0;
    psp[1].pfnCallback = NULL;

    psp[2].dwSize = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags = PSP_USETITLE;
    psp[2].hInstance = hInst;
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_Settings_RomBrowser);
    psp[2].pfnDlgProc = RomBrowserProc;
	_GetPrivateProfileString(Language,"SettingsTab5","Rom Selection",String[2],sizeof(String[2]),LangFile);
    psp[2].pszTitle = String[2];
    psp[2].lParam = 0;
    psp[2].pfnCallback = NULL;

    psp[3].dwSize = sizeof(PROPSHEETPAGE);
    psp[3].dwFlags = PSP_USETITLE;
    psp[3].hInstance = hInst;
    psp[3].pszTemplate = MAKEINTRESOURCE(IDD_Settings_Options);
    psp[3].pfnDlgProc = DefaultOptionsProc;
	_GetPrivateProfileString(Language,"SettingsTab3","Options",String[3],sizeof(String[3]),LangFile);
    psp[3].pszTitle = String[3];
    psp[3].lParam = 0;
    psp[3].pfnCallback = NULL;

    psp[4].dwSize = sizeof(PROPSHEETPAGE);
    psp[4].dwFlags = PSP_USETITLE;
    psp[4].hInstance = hInst;
    psp[4].pszTemplate = MAKEINTRESOURCE(IDD_Settings_Rom);
    psp[4].pfnDlgProc = RomSettingsProc;
	_GetPrivateProfileString(Language,"SettingsTab4","Rom Settings",String[4],sizeof(String[4]),LangFile);
    psp[4].pszTitle = String[4];
    psp[4].lParam = 0;
    psp[4].pfnCallback = NULL;

    psp[5].dwSize = sizeof(PROPSHEETPAGE);
    psp[5].dwFlags = PSP_USETITLE;
    psp[5].hInstance = hInst;
    psp[5].pszTemplate = MAKEINTRESOURCE(IDD_Settings_ShellIntegration);
    psp[5].pfnDlgProc = ShellIntegrationProc;
	_GetPrivateProfileString(Language,"SettingsTab6","Shell Integration",String[5],sizeof(String[5]),LangFile);
    psp[5].pszTitle = String[5];
    psp[5].lParam = 0;
    psp[5].pfnCallback = NULL;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndOwner;
    psh.hInstance = hInst;
    psh.pszCaption = (LPSTR) "Settings";
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = 0;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;
    psh.pfnCallback = NULL;

	PropertySheet(&psh);
	return;
}

LRESULT CALLBACK DefaultOptionsProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char * LangFile, Language[100], String[256],OrigString[256];
	int indx;

	switch (uMsg) {
	case WM_INITDIALOG:		
		LangFile = GetLangFileName();
		_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);
		
		GetDlgItemText(hDlg,IDC_TEXT20,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText1",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_TEXT20,String);	
		GetDlgItemText(hDlg,IDC_START_ON_ROM_OPEN,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText2",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_START_ON_ROM_OPEN,String);	
		GetDlgItemText(hDlg,IDC_AUTOSLEEP,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText3",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_AUTOSLEEP,String);	
		GetDlgItemText(hDlg,IDC_USEINI,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText5",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_USEINI,String);	
		GetDlgItemText(hDlg,IDC_TEXT2,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText6",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_TEXT2,String);	
		GetDlgItemText(hDlg,IDC_TEXT3,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText7",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_TEXT3,String);	
		GetDlgItemText(hDlg,IDC_TEXT4,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText8",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_TEXT4,String);	
		GetDlgItemText(hDlg,IDC_TEXT5,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText14",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_TEXT5,String);	
		GetDlgItemText(hDlg,IDC_TEXT6,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText10",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_TEXT6,String);	
		GetDlgItemText(hDlg,IDC_TEXT7,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText11",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_TEXT7,String);
		GetDlgItemText(hDlg,IDC_ZIP,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText12",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_ZIP,String);	
		GetDlgItemText(hDlg,IDC_LOAD_FULLSCREEN,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"OptionsText13",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_LOAD_FULLSCREEN,String);	
		
		_GetPrivateProfileString(Language,"CPUType2","Interpreter",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETITEMDATA,indx,CPU_Interpreter);
		SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETCURSEL,indx,0);
		_GetPrivateProfileString(Language,"CPUType3","Recompiler",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETITEMDATA,indx,CPU_Recompiler);
		if (SystemCPU_Type == CPU_Recompiler) { SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETCURSEL,indx,0); }
		if (HaveDebugger) {
			indx = SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_ADDSTRING,0,(LPARAM)"Synchronise cores");
			SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETITEMDATA,indx,CPU_SyncCores);
			if (SystemCPU_Type == CPU_SyncCores) { SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETCURSEL,indx,0); }
		}

		_GetPrivateProfileString(Language,"SelfModMethod2","None",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETITEMDATA,indx,ModCode_None);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETCURSEL,indx,0);
		_GetPrivateProfileString(Language,"SelfModMethod3","Cache",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETITEMDATA,indx,ModCode_Cache);
		if (SystemSelfModCheck == ModCode_Cache) { SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETCURSEL,indx,0); }		
		_GetPrivateProfileString(Language,"SelfModMethod4","Protect Memory",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETITEMDATA,indx,ModCode_ProtectedMemory);
		if (SystemSelfModCheck == ModCode_ProtectedMemory) { SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETCURSEL,indx,0); }
		_GetPrivateProfileString(Language,"SelfModMethod6","Check Memory & Cache",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETITEMDATA,indx,ModCode_CheckMemoryCache);
		if (SystemSelfModCheck == ModCode_CheckMemoryCache) { SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETCURSEL,indx,0); }
		_GetPrivateProfileString(Language,"SelfModMethod7","Change Memory & cache",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETITEMDATA,indx,ModCode_ChangeMemory);
		if (SystemSelfModCheck == ModCode_ChangeMemory) { SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETCURSEL,indx,0); }		
		
		_GetPrivateProfileString(Language,"RDRAMSize2","4 MB",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_SETITEMDATA,indx,0x400000);
		SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_SETCURSEL,indx,0);
		_GetPrivateProfileString(Language,"RDRAMSize3","8 MB",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_SETITEMDATA,indx,0x800000);
		if (SystemRdramSize == 0x800000) { SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_SETCURSEL,indx,0); }


		_GetPrivateProfileString(Language,"AdvBlokLink2","On",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_ABL),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_ABL),CB_SETITEMDATA,indx,TRUE);
		SendMessage(GetDlgItem(hDlg,IDC_ABL),CB_SETCURSEL,indx,0);
		_GetPrivateProfileString(Language,"AdvBlokLink3","Off",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_ABL),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_ABL),CB_SETITEMDATA,indx,FALSE);
		if (SystemABL == FALSE) { SendMessage(GetDlgItem(hDlg,IDC_ABL),CB_SETCURSEL,indx,0); }

		if (AutoStart) { SendMessage(GetDlgItem(hDlg,IDC_START_ON_ROM_OPEN),BM_SETCHECK, BST_CHECKED,0); }
		if (AutoSleep) { SendMessage(GetDlgItem(hDlg,IDC_AUTOSLEEP),BM_SETCHECK, BST_CHECKED,0); }
		if (UseIni) { SendMessage(GetDlgItem(hDlg,IDC_USEINI),BM_SETCHECK, BST_CHECKED,0); }
		if (AutoZip) { SendMessage(GetDlgItem(hDlg,IDC_ZIP),BM_SETCHECK, BST_CHECKED,0); }
		if (AutoFullScreen) { SendMessage(GetDlgItem(hDlg,IDC_LOAD_FULLSCREEN),BM_SETCHECK, BST_CHECKED,0); }
		
		break;	
	case WM_NOTIFY:
		if (((NMHDR FAR *) lParam)->code == PSN_APPLY) {
			long lResult;
			HKEY hKeyResults = 0;
			DWORD Disposition = 0;
			char String[200];
		
			sprintf(String,"Software\\N64 Emulation\\%s",AppName);
			lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"", REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
			if (lResult == ERROR_SUCCESS) {
				AutoStart = SendMessage(GetDlgItem(hDlg,IDC_START_ON_ROM_OPEN),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
				RegSetValueEx(hKeyResults,"Start Emulation when rom is opened",0,REG_DWORD,(BYTE *)&AutoStart,sizeof(DWORD));

				AutoSleep = SendMessage(GetDlgItem(hDlg,IDC_AUTOSLEEP),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
				RegSetValueEx(hKeyResults,"Pause emulation when window is not active",0,REG_DWORD,(BYTE *)&AutoSleep,sizeof(DWORD));
				
				UseIni = SendMessage(GetDlgItem(hDlg,IDC_USEINI),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
				RegSetValueEx(hKeyResults,"Always overwrite default settings with ones from ini?",0,REG_DWORD,(BYTE *)&UseIni,sizeof(DWORD));

				AutoZip = SendMessage(GetDlgItem(hDlg,IDC_ZIP),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
				RegSetValueEx(hKeyResults,"Automatically compress instant saves",0,REG_DWORD,(BYTE *)&AutoZip,sizeof(DWORD));

				AutoFullScreen = SendMessage(GetDlgItem(hDlg,IDC_LOAD_FULLSCREEN),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
				RegSetValueEx(hKeyResults,"On open rom go full screen",0,REG_DWORD,(BYTE *)&AutoFullScreen,sizeof(DWORD));

				indx = SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_GETCURSEL,0,0); 
				SystemCPU_Type = SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_GETITEMDATA,indx,0);
				RegSetValueEx(hKeyResults,"CPU Type",0,REG_DWORD,(BYTE *)&SystemCPU_Type,sizeof(DWORD));

				indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_GETCURSEL,0,0); 
				SystemSelfModCheck = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_GETITEMDATA,indx,0);
				RegSetValueEx(hKeyResults,"Self modifying code method",0,REG_DWORD,(BYTE *)&SystemSelfModCheck,sizeof(DWORD));

				indx = SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_GETCURSEL,0,0); 
				SystemRdramSize = SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_GETITEMDATA,indx,0);
				RegSetValueEx(hKeyResults,"Default RDRAM Size",0,REG_DWORD,(BYTE *)&SystemRdramSize,sizeof(DWORD));
				
				indx = SendMessage(GetDlgItem(hDlg,IDC_ABL),CB_GETCURSEL,0,0); 
				SystemABL = SendMessage(GetDlgItem(hDlg,IDC_ABL),CB_GETITEMDATA,indx,0);
				RegSetValueEx(hKeyResults,"Advanced Block Linking",0,REG_DWORD,(BYTE *)&SystemABL,sizeof(DWORD));
			}
			RegCloseKey(hKeyResults);
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK DirSelectProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			HKEY hKeyResults = 0;
			char * LangFile, Language[100], String[256], String2[256];
			char Directory[255];
			long lResult;
		
			sprintf(String,"Software\\N64 Emulation\\%s",AppName);
			lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0, KEY_ALL_ACCESS,&hKeyResults);	
			if (lResult == ERROR_SUCCESS) {
				DWORD Type, Value, Bytes = 4;
	
				lResult = RegQueryValueEx(hKeyResults,"Use Default Plugin Dir",0,&Type,(LPBYTE)(&Value),&Bytes);
				if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { Value = TRUE; }
				SendMessage(GetDlgItem(hDlg,Value?IDC_PLUGIN_DEFAULT:IDC_PLUGIN_OTHER),BM_SETCHECK, BST_CHECKED,0);

				lResult = RegQueryValueEx(hKeyResults,"Use Default Rom Dir",0,&Type,(LPBYTE)(&Value),&Bytes);
				if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { Value = TRUE; }
				SendMessage(GetDlgItem(hDlg,Value?IDC_ROM_DEFAULT:IDC_ROM_OTHER),BM_SETCHECK, BST_CHECKED,0);

				lResult = RegQueryValueEx(hKeyResults,"Use Default Auto Save Dir",0,&Type,(LPBYTE)(&Value),&Bytes);
				if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { Value = TRUE; }
				SendMessage(GetDlgItem(hDlg,Value?IDC_AUTO_DEFAULT:IDC_AUTO_OTHER),BM_SETCHECK, BST_CHECKED,0);

				lResult = RegQueryValueEx(hKeyResults,"Use Default Instant Save Dir",0,&Type,(LPBYTE)(&Value),&Bytes);
				if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { Value = TRUE; }
				SendMessage(GetDlgItem(hDlg,Value?IDC_INSTANT_DEFAULT:IDC_INSTANT_OTHER),BM_SETCHECK, BST_CHECKED,0);

				lResult = RegQueryValueEx(hKeyResults,"Use Default Snap Shot Dir",0,&Type,(LPBYTE)(&Value),&Bytes);
				if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { Value = TRUE; }
				SendMessage(GetDlgItem(hDlg,Value?IDC_SNAP_DEFAULT:IDC_SNAP_OTHER),BM_SETCHECK, BST_CHECKED,0);

				Bytes = sizeof(Directory);
				lResult = RegQueryValueEx(hKeyResults,"Plugin Directory",0,&Type,(LPBYTE)Directory,&Bytes);
				if (lResult != ERROR_SUCCESS) { GetPluginDir(Directory ); }
				SetDlgItemText(hDlg,IDC_PLUGIN_DIR,Directory);

				Bytes = sizeof(Directory);
				lResult = RegQueryValueEx(hKeyResults,"Instant Save Directory",0,&Type,(LPBYTE)Directory,&Bytes);
				if (lResult != ERROR_SUCCESS) { GetInstantSaveDir(Directory); }
				SetDlgItemText(hDlg,IDC_INSTANT_DIR,Directory);

				Bytes = sizeof(Directory);
				lResult = RegQueryValueEx(hKeyResults,"Auto Save Directory",0,&Type,(LPBYTE)Directory,&Bytes);
				if (lResult != ERROR_SUCCESS) { GetAutoSaveDir(Directory); }
				SetDlgItemText(hDlg,IDC_AUTO_DIR,Directory);

				Bytes = sizeof(Directory);
				lResult = RegQueryValueEx(hKeyResults,"Snap Shot Directory",0,&Type,(LPBYTE)Directory,&Bytes);
				if (lResult != ERROR_SUCCESS) { GetSnapShotDir(Directory); }
				SetDlgItemText(hDlg,IDC_SNAP_DIR,Directory);
			} else {
				SendMessage(GetDlgItem(hDlg,IDC_PLUGIN_DEFAULT),BM_SETCHECK, BST_CHECKED,0);
				SendMessage(GetDlgItem(hDlg,IDC_ROM_DEFAULT),BM_SETCHECK, BST_CHECKED,0);
				SendMessage(GetDlgItem(hDlg,IDC_AUTO_DEFAULT),BM_SETCHECK, BST_CHECKED,0);
				SendMessage(GetDlgItem(hDlg,IDC_INSTANT_DEFAULT),BM_SETCHECK, BST_CHECKED,0);
				SendMessage(GetDlgItem(hDlg,IDC_SNAP_DEFAULT),BM_SETCHECK, BST_CHECKED,0);
				GetPluginDir(Directory );
				SetDlgItemText(hDlg,IDC_PLUGIN_DIR,Directory);
				GetInstantSaveDir(Directory);
				SetDlgItemText(hDlg,IDC_INSTANT_DIR,Directory);
				GetAutoSaveDir(Directory);
				SetDlgItemText(hDlg,IDC_AUTO_DIR,Directory);
				GetSnapShotDir(Directory);
				SetDlgItemText(hDlg,IDC_SNAP_DIR,Directory);
			}			
			GetRomDirectory( Directory );
			SetDlgItemText(hDlg,IDC_ROM_DIR,Directory);

			LangFile = GetLangFileName();
			_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);
			_GetPrivateProfileString(Language,"StoreDirName1","Plugin Directoy",String,sizeof(String),LangFile);
			sprintf(String2," %s: ",String);
			SetDlgItemText(hDlg,IDC_DIR_FRAME1,String2);
			_GetPrivateProfileString(Language,"StoreDirName2","Rom Directory",String,sizeof(String),LangFile);
			sprintf(String2," %s: ",String);
			SetDlgItemText(hDlg,IDC_DIR_FRAME2,String2);
			_GetPrivateProfileString(Language,"StoreDirName3","N64 Auto saves",String,sizeof(String),LangFile);
			sprintf(String2," %s: ",String);
			SetDlgItemText(hDlg,IDC_DIR_FRAME3,String2);
			_GetPrivateProfileString(Language,"StoreDirName4","Instant saves",String,sizeof(String),LangFile);
			sprintf(String2," %s: ",String);
			SetDlgItemText(hDlg,IDC_DIR_FRAME4,String2);
			_GetPrivateProfileString(Language,"StoreDirName5","Screen Shots",String,sizeof(String),LangFile);
			sprintf(String2," %s: ",String);
			SetDlgItemText(hDlg,IDC_DIR_FRAME5,String2);
			
			_GetPrivateProfileString(Language,"DefaultRomDir","Last folder that a rom was open from.",String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_ROM_DEFAULT,String);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SELECT_PLUGIN_DIR:
		case IDC_SELECT_ROM_DIR:
		case IDC_SELECT_INSTANT_DIR:
		case IDC_SELECT_AUTO_DIR:
		case IDC_SELECT_SNAP_DIR:
			{
				char Buffer[MAX_PATH], Directory[255], Title[255];
				LPITEMIDLIST pidl;
				BROWSEINFO bi;

				switch (LOWORD(wParam)) {
				case IDC_SELECT_PLUGIN_DIR: strcpy(Title,"Select Plugin Directory"); break;
				case IDC_SELECT_ROM_DIR: strcpy(Title,"Select Rom Directory"); break;
				case IDC_SELECT_AUTO_DIR: strcpy(Title,"Select Automatic save Directory"); break;
				case IDC_SELECT_INSTANT_DIR: strcpy(Title,"Select Instant save Directory"); break;
				case IDC_SELECT_SNAP_DIR: strcpy(Title,"Select snap shot Directory"); break;
				}

				bi.hwndOwner = hDlg;
				bi.pidlRoot = NULL;
				bi.pszDisplayName = Buffer;
				bi.lpszTitle = Title;
				bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
				bi.lpfn = NULL;
				bi.lParam = 0;
				if ((pidl = SHBrowseForFolder(&bi)) != NULL) {
					if (SHGetPathFromIDList(pidl, Directory)) {
						int len = strlen(Directory);

						if (Directory[len - 1] != '\\') { strcat(Directory,"\\"); }
						switch (LOWORD(wParam)) {
						case IDC_SELECT_PLUGIN_DIR: 
							SetDlgItemText(hDlg,IDC_PLUGIN_DIR,Directory);
							SendMessage(GetDlgItem(hDlg,IDC_PLUGIN_DEFAULT),BM_SETCHECK, BST_UNCHECKED,0);
							SendMessage(GetDlgItem(hDlg,IDC_PLUGIN_OTHER),BM_SETCHECK, BST_CHECKED,0);
							break;
						case IDC_SELECT_ROM_DIR: 
							SetDlgItemText(hDlg,IDC_ROM_DIR,Directory);
							SendMessage(GetDlgItem(hDlg,IDC_ROM_DEFAULT),BM_SETCHECK, BST_UNCHECKED,0);
							SendMessage(GetDlgItem(hDlg,IDC_ROM_OTHER),BM_SETCHECK, BST_CHECKED,0);
							break;
						case IDC_SELECT_INSTANT_DIR: 
							SetDlgItemText(hDlg,IDC_INSTANT_DIR,Directory);
							SendMessage(GetDlgItem(hDlg,IDC_INSTANT_DEFAULT),BM_SETCHECK, BST_UNCHECKED,0);
							SendMessage(GetDlgItem(hDlg,IDC_INSTANT_OTHER),BM_SETCHECK, BST_CHECKED,0);
							break;
						case IDC_SELECT_AUTO_DIR: 
							SetDlgItemText(hDlg,IDC_AUTO_DIR,Directory);
							SendMessage(GetDlgItem(hDlg,IDC_AUTO_DEFAULT),BM_SETCHECK, BST_UNCHECKED,0);
							SendMessage(GetDlgItem(hDlg,IDC_AUTO_OTHER),BM_SETCHECK, BST_CHECKED,0);
							break;
						case IDC_SELECT_SNAP_DIR: 
							SetDlgItemText(hDlg,IDC_SNAP_DIR,Directory);
							SendMessage(GetDlgItem(hDlg,IDC_SNAP_DEFAULT),BM_SETCHECK, BST_UNCHECKED,0);
							SendMessage(GetDlgItem(hDlg,IDC_SNAP_OTHER),BM_SETCHECK, BST_CHECKED,0);
							break;
						}
					}
				}
			}
			break;
		}
		break;
	case WM_NOTIFY:
		if (((NMHDR FAR *) lParam)->code == PSN_APPLY) { 
			long lResult;
			HKEY hKeyResults = 0;
			DWORD Disposition = 0;
			char String[200];
		
			sprintf(String,"Software\\N64 Emulation\\%s",AppName);
			lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"", REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
			if (lResult == ERROR_SUCCESS) {
				DWORD Value;
							
				Value = SendMessage(GetDlgItem(hDlg,IDC_PLUGIN_DEFAULT),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
				RegSetValueEx(hKeyResults,"Use Default Plugin Dir",0,REG_DWORD,(BYTE *)&Value,sizeof(DWORD));
				if (Value == FALSE) {
					GetDlgItemText(hDlg,IDC_PLUGIN_DIR,String,sizeof(String));
					RegSetValueEx(hKeyResults,"Plugin Directory",0,REG_SZ,(CONST BYTE *)String,strlen(String));
				}

				Value = SendMessage(GetDlgItem(hDlg,IDC_ROM_DEFAULT),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
				RegSetValueEx(hKeyResults,"Use Default Rom Dir",0,REG_DWORD,(BYTE *)&Value,sizeof(DWORD));
				if (Value == FALSE) {
					GetDlgItemText(hDlg,IDC_ROM_DIR,String,sizeof(String));
					RegSetValueEx(hKeyResults,"Rom Directory",0,REG_SZ,(CONST BYTE *)String,strlen(String));
				}

				Value = SendMessage(GetDlgItem(hDlg,IDC_AUTO_DEFAULT),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
				RegSetValueEx(hKeyResults,"Use Default Auto Save Dir",0,REG_DWORD,(BYTE *)&Value,sizeof(DWORD));
				if (Value == FALSE) {
					GetDlgItemText(hDlg,IDC_AUTO_DIR,String,sizeof(String));
					RegSetValueEx(hKeyResults,"Auto Save Directory",0,REG_SZ,(CONST BYTE *)String,strlen(String));
				}
				
				Value = SendMessage(GetDlgItem(hDlg,IDC_INSTANT_DEFAULT),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
				RegSetValueEx(hKeyResults,"Use Default Instant Save Dir",0,REG_DWORD,(BYTE *)&Value,sizeof(DWORD));
				if (Value == FALSE) {
					GetDlgItemText(hDlg,IDC_INSTANT_DIR,String,sizeof(String));
					RegSetValueEx(hKeyResults,"Instant Save Directory",0,REG_SZ,(CONST BYTE *)String,strlen(String));
				}
				
				Value = SendMessage(GetDlgItem(hDlg,IDC_SNAP_DEFAULT),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
				RegSetValueEx(hKeyResults,"Use Default Snap Shot Dir",0,REG_DWORD,(BYTE *)&Value,sizeof(DWORD));
				if (Value == FALSE) {
					GetDlgItemText(hDlg,IDC_SNAP_DIR,String,sizeof(String));
					RegSetValueEx(hKeyResults,"Snap Shot Directory",0,REG_SZ,(CONST BYTE *)String,strlen(String));
				}
			}
			RegCloseKey(hKeyResults);
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL PluginsChanged ( HWND hDlg ) {
	DWORD index;

#ifndef EXTERNAL_RELEASE
	index = SendMessage(GetDlgItem(hDlg,RSP_LIST),CB_GETCURSEL,0,0);
	index = SendMessage(GetDlgItem(hDlg,RSP_LIST),CB_GETITEMDATA,(WPARAM)index,0);
	if ((int)index >= 0) {
		if(_stricmp(RspDLL,PluginNames[index]) != 0) { return TRUE; }
	}
#endif

	index = SendMessage(GetDlgItem(hDlg,GFX_LIST),CB_GETCURSEL,0,0);
	index = SendMessage(GetDlgItem(hDlg,GFX_LIST),CB_GETITEMDATA,(WPARAM)index,0);
	if ((int)index >= 0) {
		if(_stricmp(GfxDLL,PluginNames[index]) != 0) { return TRUE; }
	}

	index = SendMessage(GetDlgItem(hDlg,AUDIO_LIST),CB_GETCURSEL,0,0);
	index = SendMessage(GetDlgItem(hDlg,AUDIO_LIST),CB_GETITEMDATA,(WPARAM)index,0);
	if ((int)index >= 0) {
		if(_stricmp(AudioDLL,PluginNames[index]) != 0) { return TRUE; }
	}

	index = SendMessage(GetDlgItem(hDlg,CONT_LIST),CB_GETCURSEL,0,0);
	index = SendMessage(GetDlgItem(hDlg,CONT_LIST),CB_GETITEMDATA,(WPARAM)index,0);
	if ((int)index >= 0) {
		if(_stricmp(ControllerDLL,PluginNames[index]) != 0) { return TRUE; }
	}
	return FALSE;
}

LRESULT CALLBACK PluginSelectProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char Plugin[300];
	HANDLE hLib;
	DWORD count, index;

	switch (uMsg) {
	case WM_INITDIALOG:
		SetupPluginScreen(hDlg);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {		
#ifndef EXTERNAL_RELEASE
		case RSP_LIST:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				index = SendMessage(GetDlgItem(hDlg,RSP_LIST),CB_GETCURSEL,0,0);
				index = SendMessage(GetDlgItem(hDlg,RSP_LIST),CB_GETITEMDATA,(WPARAM)index,0);

				GetPluginDir(Plugin);
				strcat(Plugin,PluginNames[index]);
				hLib = LoadLibrary(Plugin);		
				if (hLib == NULL) { DisplayError("Failed to load: %s",Plugin); }
				RSPDllAbout = (void (__cdecl *)(HWND))GetProcAddress( hLib, "DllAbout" );
				EnableWindow(GetDlgItem(hDlg,RSP_ABOUT),RSPDllAbout != NULL ? TRUE:FALSE);
			}
			break;
#endif
		case GFX_LIST:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				index = SendMessage(GetDlgItem(hDlg,GFX_LIST),CB_GETCURSEL,0,0);
				index = SendMessage(GetDlgItem(hDlg,GFX_LIST),CB_GETITEMDATA,(WPARAM)index,0);

				GetPluginDir(Plugin);
				strcat(Plugin,PluginNames[index]);
				hLib = LoadLibrary(Plugin);		
				if (hLib == NULL) { DisplayError("Failed to load: %s",Plugin); }
				GFXDllAbout = (void (__cdecl *)(HWND))GetProcAddress( hLib, "DllAbout" );
				EnableWindow(GetDlgItem(hDlg,GFX_ABOUT),GFXDllAbout != NULL ? TRUE:FALSE);
			}
			break;
		case AUDIO_LIST:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				index = SendMessage(GetDlgItem(hDlg,AUDIO_LIST),CB_GETCURSEL,0,0);
				index = SendMessage(GetDlgItem(hDlg,AUDIO_LIST),CB_GETITEMDATA,(WPARAM)index,0);

				GetPluginDir(Plugin);
				strcat(Plugin,PluginNames[index]);
				hLib = LoadLibrary(Plugin);		
				if (hLib == NULL) { DisplayError("Failed to load: %s",Plugin); }
				AiDllAbout = (void (__cdecl *)(HWND))GetProcAddress( hLib, "DllAbout" );
				EnableWindow(GetDlgItem(hDlg,GFX_ABOUT),GFXDllAbout != NULL ? TRUE:FALSE);
			}
			break;
		case CONT_LIST:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				index = SendMessage(GetDlgItem(hDlg,CONT_LIST),CB_GETCURSEL,0,0);
				index = SendMessage(GetDlgItem(hDlg,CONT_LIST),CB_GETITEMDATA,(WPARAM)index,0);

				GetPluginDir(Plugin);
				strcat(Plugin,PluginNames[index]);
				hLib = LoadLibrary(Plugin);		
				if (hLib == NULL) { DisplayError("Failed to load: %s",Plugin); }
				ContDllAbout = (void (__cdecl *)(HWND))GetProcAddress( hLib, "DllAbout" );
				EnableWindow(GetDlgItem(hDlg,CONT_ABOUT),ContDllAbout != NULL ? TRUE:FALSE);
			}
			break;
#ifndef EXTERNAL_RELEASE
		case RSP_ABOUT: RSPDllAbout(hDlg); break;
#endif
		case GFX_ABOUT: GFXDllAbout(hDlg); break;
		case CONT_ABOUT: ContDllAbout(hDlg); break;
		case AUDIO_ABOUT: AiDllAbout(hDlg); break;
		}
		break;
	case WM_NOTIFY:
		if (((NMHDR FAR *) lParam)->code == PSN_APPLY) { 
			long lResult;
			HKEY hKeyResults = 0;
			DWORD Disposition = 0;
			char String[200];

			if (PluginsChanged(hDlg) == FALSE) { break; }

			if (hCPU != NULL) { 
				char String[256];
				int Response;

				ShowWindow(hDlg,SW_HIDE);
				strcpy(String,"Changing the plugins will reset the current rom.\n\nChange Plugins?");
				Response = MessageBox(hMainWindow,String,"Change Plugins",MB_YESNO|MB_ICONQUESTION);
				if (Response != IDYES) { break; }
			}

			sprintf(String,"Software\\N64 Emulation\\%s\\Dll",AppName);
			lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"", REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
			if (lResult == ERROR_SUCCESS) {
				DWORD index;

#ifndef EXTERNAL_RELEASE
				index = SendMessage(GetDlgItem(hDlg,RSP_LIST),CB_GETCURSEL,0,0);
				index = SendMessage(GetDlgItem(hDlg,RSP_LIST),CB_GETITEMDATA,(WPARAM)index,0);
				sprintf(String,"%s",PluginNames[index]);
				RegSetValueEx(hKeyResults,"RSP Dll",0,REG_SZ,(CONST BYTE *)String,
					strlen(String));
#endif
				index = SendMessage(GetDlgItem(hDlg,GFX_LIST),CB_GETCURSEL,0,0);
				index = SendMessage(GetDlgItem(hDlg,GFX_LIST),CB_GETITEMDATA,(WPARAM)index,0);
				sprintf(String,"%s",PluginNames[index]);
				RegSetValueEx(hKeyResults,"Graphics Dll",0,REG_SZ,(CONST BYTE *)String,
					strlen(String));
				index = SendMessage(GetDlgItem(hDlg,AUDIO_LIST),CB_GETCURSEL,0,0);
				index = SendMessage(GetDlgItem(hDlg,AUDIO_LIST),CB_GETITEMDATA,(WPARAM)index,0);
				sprintf(String,"%s",PluginNames[index]);
				RegSetValueEx(hKeyResults,"Audio Dll",0,REG_SZ,(CONST BYTE *)String,
					strlen(String));
				index = SendMessage(GetDlgItem(hDlg,CONT_LIST),CB_GETCURSEL,0,0);
				index = SendMessage(GetDlgItem(hDlg,CONT_LIST),CB_GETITEMDATA,(WPARAM)index,0);
				sprintf(String,"%s",PluginNames[index]);
				RegSetValueEx(hKeyResults,"Controller Dll",0,REG_SZ,(CONST BYTE *)String,
					strlen(String));
			}
			RegCloseKey(hKeyResults);
			if (hCPU != NULL) { 
				CloseCpu();
				ShutdownPlugins();
				SetupPlugins(hMainWindow);				
				StartEmulation();
			} else {
				ShutdownPlugins();
				if (!RomBrowser) { SetupPlugins(hMainWindow); }
				if (RomBrowser) { SetupPlugins(hHiddenWin); }
			}
		}
		if (((NMHDR FAR *) lParam)->code == PSN_RESET) { 
			for (count = 0; count <	PluginCount; count ++ ) { free(PluginNames[count]); }
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

void RomAddFieldToList (HWND hDlg, char * Name, int Pos) {
	int listCount;

	if (Pos < 0) { 
		SendDlgItemMessage(hDlg,IDC_AVALIABLE,LB_ADDSTRING,0,(LPARAM)Name);
		return;
	}
	listCount = SendDlgItemMessage(hDlg,IDC_USING,LB_GETCOUNT,0,0);
	if (Pos > listCount) { Pos = listCount; }
	SendDlgItemMessage(hDlg,IDC_USING,LB_INSERTSTRING,Pos,(LPARAM)Name);
}

LRESULT CALLBACK RomBrowserProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:		
		if (RomBrowser) { SendMessage(GetDlgItem(hDlg,IDC_USE_ROMBROWSER),BM_SETCHECK, BST_CHECKED,0); }
		if (Rercursion) { SendMessage(GetDlgItem(hDlg,IDC_RECURSION),BM_SETCHECK, BST_CHECKED,0); }
		{ 
			int count;

			for (count = 0; count < NoOfFields; count ++) {
				RomAddFieldToList(hDlg,RomBrowserFields[count].Name,RomBrowserFields[count].Pos);
			}
		}
		{
			char * LangFile, Language[100], String[256];
			sprintf(String,"%d",RomsToRemember);
			SetDlgItemText(hDlg,IDC_REMEMBER,String);
			sprintf(String,"%d",RomDirsToRemember);
			SetDlgItemText(hDlg,IDC_REMEMBERDIR,String);

			LangFile = GetLangFileName();
			_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);
			
			GetDlgItemText(hDlg,IDC_ROMSEL_TEXT1,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText1",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_ROMSEL_TEXT1,String);
			GetDlgItemText(hDlg,IDC_ROMSEL_TEXT2,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText2",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_ROMSEL_TEXT2,String);
			GetDlgItemText(hDlg,IDC_ROMSEL_TEXT3,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText3",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_ROMSEL_TEXT3,String);
			GetDlgItemText(hDlg,IDC_ROMSEL_TEXT4,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText4",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_ROMSEL_TEXT4,String);
			GetDlgItemText(hDlg,IDC_USE_ROMBROWSER,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText5",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_USE_ROMBROWSER,String);
			GetDlgItemText(hDlg,IDC_RECURSION,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText6",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_RECURSION,String);
			GetDlgItemText(hDlg,IDC_ROMSEL_TEXT5,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText7",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_ROMSEL_TEXT5,String);
			GetDlgItemText(hDlg,IDC_ROMSEL_TEXT6,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText8",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_ROMSEL_TEXT6,String);
			GetDlgItemText(hDlg,IDC_ADD,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText9",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_ADD,String);
			GetDlgItemText(hDlg,IDC_REMOVE,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText10",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_REMOVE,String);
			GetDlgItemText(hDlg,IDC_UP,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText11",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_UP,String);
			GetDlgItemText(hDlg,IDC_DOWN,String,sizeof(String));
			_GetPrivateProfileString(Language,"RomSelText12",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_DOWN,String);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ADD:
			{
				char String[100];
				int index, listCount;

				index = SendMessage(GetDlgItem(hDlg,IDC_AVALIABLE),LB_GETCURSEL,0,0);
				if (index < 0) { break; }
				SendMessage(GetDlgItem(hDlg,IDC_AVALIABLE),LB_GETTEXT,index,(LPARAM)String);
				SendDlgItemMessage(hDlg,IDC_USING,LB_ADDSTRING,0,(LPARAM)String);
				SendDlgItemMessage(hDlg,IDC_AVALIABLE,LB_DELETESTRING,index,0);
				listCount = SendDlgItemMessage(hDlg,IDC_AVALIABLE,LB_GETCOUNT,0,0);
				if (index >= listCount) { index -= 1;}
				SendDlgItemMessage(hDlg,IDC_AVALIABLE,LB_SETCURSEL,index,0);
			}
			break;
		case IDC_REMOVE:
			{
				char String[100];
				int index, listCount;

				index = SendMessage(GetDlgItem(hDlg,IDC_USING),LB_GETCURSEL,0,0);
				if (index < 0) { break; }
				SendMessage(GetDlgItem(hDlg,IDC_USING),LB_GETTEXT,index,(LPARAM)String);
				SendDlgItemMessage(hDlg,IDC_AVALIABLE,LB_ADDSTRING,0,(LPARAM)String);
				SendDlgItemMessage(hDlg,IDC_USING,LB_DELETESTRING,index,0);
				listCount = SendDlgItemMessage(hDlg,IDC_USING,LB_GETCOUNT,0,0);
				if (index >= listCount) { index -= 1;}
				SendDlgItemMessage(hDlg,IDC_USING,LB_SETCURSEL,index,0);
			}
			break;
		case IDC_UP:
			{
				char String[100];
				int index;

				index = SendMessage(GetDlgItem(hDlg,IDC_USING),LB_GETCURSEL,0,0);
				if (index <= 0) { break; }
				SendMessage(GetDlgItem(hDlg,IDC_USING),LB_GETTEXT,index,(LPARAM)String);
				SendDlgItemMessage(hDlg,IDC_USING,LB_DELETESTRING,index,0);
				SendDlgItemMessage(hDlg,IDC_USING,LB_INSERTSTRING,index - 1,(LPARAM)String);
				SendDlgItemMessage(hDlg,IDC_USING,LB_SETCURSEL,index - 1,0);
			}
			break;
		case IDC_DOWN:
			{
				char String[100];
				int index,listCount;

				index = SendMessage(GetDlgItem(hDlg,IDC_USING),LB_GETCURSEL,0,0);
				listCount = SendDlgItemMessage(hDlg,IDC_USING,LB_GETCOUNT,0,0);
				if ((index + 1) == listCount) { break; }
				SendMessage(GetDlgItem(hDlg,IDC_USING),LB_GETTEXT,index,(LPARAM)String);
				SendDlgItemMessage(hDlg,IDC_USING,LB_DELETESTRING,index,0);
				SendDlgItemMessage(hDlg,IDC_USING,LB_INSERTSTRING,index + 1,(LPARAM)String);
				SendDlgItemMessage(hDlg,IDC_USING,LB_SETCURSEL,index + 1,0);
			}
			break;
		}
		break;
		
	case WM_NOTIFY:
		if (((NMHDR FAR *) lParam)->code == PSN_APPLY) { 
			char String[200], szIndex[10];
			int index, listCount;
			DWORD Disposition = 0;
			HKEY hKeyResults = 0;
			LPSTR IniFileName;
			long lResult;

			RomBrowser = SendMessage(GetDlgItem(hDlg,IDC_USE_ROMBROWSER),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
			Rercursion = SendMessage(GetDlgItem(hDlg,IDC_RECURSION),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;

			sprintf(String,"Software\\N64 Emulation\\%s",AppName);
			lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"", REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
			if (lResult == ERROR_SUCCESS) {
				RegSetValueEx(hKeyResults,"Use Rom Browser",0,REG_DWORD,(BYTE *)&RomBrowser,sizeof(DWORD));
				RegSetValueEx(hKeyResults,"Use Recursion",0,REG_DWORD,(BYTE *)&Rercursion,sizeof(DWORD));
			}
						
			SaveRomBrowserColoumnInfo();
			IniFileName = GetIniFileName();
			listCount = SendDlgItemMessage(hDlg,IDC_USING,LB_GETCOUNT,0,0);
			for (index = 0; index < listCount; index ++ ){
				SendMessage(GetDlgItem(hDlg,IDC_USING),LB_GETTEXT,index,(LPARAM)String);
				sprintf(szIndex,"%d",index);
				_WritePrivateProfileString("Rom Browser",String,szIndex,IniFileName);
			}
			listCount = SendDlgItemMessage(hDlg,IDC_AVALIABLE,LB_GETCOUNT,0,0);
			strcpy(szIndex,"-1");
			for (index = 0; index < listCount; index ++ ){
				SendMessage(GetDlgItem(hDlg,IDC_AVALIABLE),LB_GETTEXT,index,(LPARAM)String);
				_WritePrivateProfileString("Rom Browser",String,szIndex,IniFileName);
			}			
			LoadRomBrowserColoumnInfo();
			ResetRomBrowserColomuns();
			if (RomBrowser) { ShowRomList(hMainWindow); }
			if (!RomBrowser) { HideRomBrowser(); }

			RemoveRecentList(hMainWindow);
			RomsToRemember = GetDlgItemInt(hDlg,IDC_REMEMBER,NULL,FALSE);
			if (RomsToRemember < 0) { RomsToRemember = 0; }
			if (RomsToRemember > 10) { RomsToRemember = 10; }
			RegSetValueEx(hKeyResults,"Roms To Remember",0,REG_DWORD,(BYTE *)&RomsToRemember,sizeof(DWORD));
			LoadRecentFiles(hMainWindow);

			RemoveRecentList(hMainWindow);
			RomDirsToRemember = GetDlgItemInt(hDlg,IDC_REMEMBERDIR,NULL,FALSE);
			if (RomDirsToRemember < 0) { RomDirsToRemember = 0; }
			if (RomDirsToRemember > 10) { RomDirsToRemember = 10; }
			RegSetValueEx(hKeyResults,"Rom Dirs To Remember",0,REG_DWORD,(BYTE *)&RomDirsToRemember,sizeof(DWORD));
			LoadRecentDirs(hMainWindow);
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK RomSettingsProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char * LangFile, Language[100], String[256], OrigString[256];
	int indx;

	switch (uMsg) {
	case WM_INITDIALOG:		
		ReadRomOptions();
		LangFile = GetLangFileName();
		_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);

		GetDlgItemText(hDlg,IDC_MEMORY_SIZE_TEXT,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"RomOptionsText1",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_MEMORY_SIZE_TEXT,String);	
		GetDlgItemText(hDlg,IDC_SAVE_TYPE_TEXT,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"RomOptionsText2",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_SAVE_TYPE_TEXT,String);	
		GetDlgItemText(hDlg,IDC_CPU_TYPE_TEXT,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"RomOptionsText3",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_CPU_TYPE_TEXT,String);	
		GetDlgItemText(hDlg,IDC_SELFMOD_TEXT,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"RomOptionsText4",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_SELFMOD_TEXT,String);	
		GetDlgItemText(hDlg,IDC_USE_TLB,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"RomOptionsText6",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_USE_TLB,String);
		GetDlgItemText(hDlg,IDC_COUNTFACT_TEXT,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"RomOptionsText7",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_COUNTFACT_TEXT,String);
		GetDlgItemText(hDlg,IDC_BLOCK_LINKING_TEXT,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"RomOptionsText8",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_BLOCK_LINKING_TEXT,String);
		GetDlgItemText(hDlg,IDC_LARGE_COMPILE_BUFFER,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"RomOptionsText9",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_LARGE_COMPILE_BUFFER,String);
		GetDlgItemText(hDlg,IDC_ROM_REGCACHE,OrigString,sizeof(OrigString));
		_GetPrivateProfileString(Language,"RomOptionsText10",OrigString,String,sizeof(String),LangFile);
		SetDlgItemText(hDlg,IDC_ROM_REGCACHE,String);
		


		_GetPrivateProfileString(Language,"RDRAMSize1","Default",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_SETITEMDATA,indx,-1);
		SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_SETCURSEL,indx,0);
		_GetPrivateProfileString(Language,"RDRAMSize2","4 MB",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_SETITEMDATA,indx,0x400000);
		if (RomRamSize == 0x400000) { SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_SETCURSEL,indx,0); }
		_GetPrivateProfileString(Language,"RDRAMSize3","8 MB",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_SETITEMDATA,indx,0x800000);
		if (RomRamSize == 0x800000) { SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_SETCURSEL,indx,0); }
		
		_GetPrivateProfileString(Language,"SaveType1","Use First Used Save Type",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_SETITEMDATA,indx,Auto);
		SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_SETCURSEL,indx,0);
		_GetPrivateProfileString(Language,"SaveType2","4kbit Eeprom",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_SETITEMDATA,indx,Eeprom_4K);
		if (RomSaveUsing == Eeprom_4K) { SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_SETCURSEL,indx,0); }
		_GetPrivateProfileString(Language,"SaveType3","16kbit Eeprom",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_SETITEMDATA,indx,Eeprom_16K);
		if (RomSaveUsing == Eeprom_16K) { SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_SETCURSEL,indx,0); }
		_GetPrivateProfileString(Language,"SaveType4","32kbytes SRAM",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_SETITEMDATA,indx,Sram);
		if (RomSaveUsing == Sram) { SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_SETCURSEL,indx,0); }
		_GetPrivateProfileString(Language,"SaveType5","Flashram",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_SETITEMDATA,indx,FlashRam);
		if (RomSaveUsing == FlashRam) { SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_SETCURSEL,indx,0); }

		_GetPrivateProfileString(Language,"CounterFactor1","Default",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETITEMDATA,indx,-1);
		SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETCURSEL,indx,0);
		indx = SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_ADDSTRING,0,(LPARAM)"1");
		SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETITEMDATA,indx,1);
		if (RomCF == 1) { SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETCURSEL,indx,0); }
		indx = SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_ADDSTRING,0,(LPARAM)"2");
		SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETITEMDATA,indx,2);
		if (RomCF == 2) { SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETCURSEL,indx,0); }
		indx = SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_ADDSTRING,0,(LPARAM)"3");
		SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETITEMDATA,indx,3);
		if (RomCF == 3) { SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETCURSEL,indx,0); }
		indx = SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_ADDSTRING,0,(LPARAM)"4");
		SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETITEMDATA,indx,4);
		if (RomCF == 4) { SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETCURSEL,indx,0); }
		indx = SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_ADDSTRING,0,(LPARAM)"5");
		SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),IDC_COUNTFACT,indx,5);
		if (RomCF == 5) { SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETCURSEL,indx,0); }
		indx = SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_ADDSTRING,0,(LPARAM)"6");
		SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETITEMDATA,indx,6);
		if (RomCF == 6) { SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_SETCURSEL,indx,0); }

		_GetPrivateProfileString(Language,"CPUType1","Default",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETITEMDATA,indx,CPU_Default);
		SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETCURSEL,indx,0);
		_GetPrivateProfileString(Language,"CPUType2","Interpreter",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETITEMDATA,indx,CPU_Interpreter);
		if (RomCPUType == CPU_Interpreter) { SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETCURSEL,indx,0); }
		_GetPrivateProfileString(Language,"CPUType3","Recompiler",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETITEMDATA,indx,CPU_Recompiler);
		if (RomCPUType == CPU_Recompiler) { SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETCURSEL,indx,0); }
		if (HaveDebugger) {
			indx = SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_ADDSTRING,0,(LPARAM)"Synchronise cores");
			SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETITEMDATA,indx,CPU_SyncCores);
			if (RomCPUType == CPU_SyncCores) { SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_SETCURSEL,indx,0); }
		}
		
		_GetPrivateProfileString(Language,"SelfModMethod1","Default",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETITEMDATA,indx,ModCode_Default);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETCURSEL,indx,0);
		_GetPrivateProfileString(Language,"SelfModMethod2","None",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETITEMDATA,indx,ModCode_None);
		if (RomSelfMod == ModCode_None) { SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETCURSEL,indx,0); }		
		_GetPrivateProfileString(Language,"SelfModMethod3","Cache",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETITEMDATA,indx,ModCode_Cache);
		if (RomSelfMod == ModCode_Cache) { SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETCURSEL,indx,0); }		
		_GetPrivateProfileString(Language,"SelfModMethod4","Protect Memory",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETITEMDATA,indx,ModCode_ProtectedMemory);
		if (RomSelfMod == ModCode_ProtectedMemory) { SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETCURSEL,indx,0); }
		_GetPrivateProfileString(Language,"SelfModMethod6","Check Memory & Cache",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETITEMDATA,indx,ModCode_CheckMemoryCache);
		if (RomSelfMod == ModCode_CheckMemoryCache) { SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETCURSEL,indx,0); }
		_GetPrivateProfileString(Language,"SelfModMethod7","Change Memory & cache",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETITEMDATA,indx,ModCode_ChangeMemory);
		if (RomSelfMod == ModCode_ChangeMemory) { SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_SETCURSEL,indx,0); }

		_GetPrivateProfileString(Language,"AdvBlokLink1","Global Setting",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_BLOCK_LINKING),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_BLOCK_LINKING),CB_SETITEMDATA,indx,-1);
		SendMessage(GetDlgItem(hDlg,IDC_BLOCK_LINKING),CB_SETCURSEL,indx,0);
		_GetPrivateProfileString(Language,"AdvBlokLink2","On",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_BLOCK_LINKING),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_BLOCK_LINKING),CB_SETITEMDATA,indx,0);
		if (RomUseLinking == 0) { SendMessage(GetDlgItem(hDlg,IDC_BLOCK_LINKING),CB_SETCURSEL,indx,0); }
		_GetPrivateProfileString(Language,"AdvBlokLink3","Off",String,sizeof(String),LangFile);
		indx = SendMessage(GetDlgItem(hDlg,IDC_BLOCK_LINKING),CB_ADDSTRING,0,(LPARAM)String);
		SendMessage(GetDlgItem(hDlg,IDC_BLOCK_LINKING),CB_SETITEMDATA,indx,1);
		if (RomUseLinking == 1) { SendMessage(GetDlgItem(hDlg,IDC_BLOCK_LINKING),CB_SETCURSEL,indx,0); }

		if (RomUseTlb) { SendMessage(GetDlgItem(hDlg,IDC_USE_TLB),BM_SETCHECK, BST_CHECKED,0); }
		if (RomUseCache) { SendMessage(GetDlgItem(hDlg,IDC_ROM_REGCACHE),BM_SETCHECK, BST_CHECKED,0); }
		if (RomUseLargeBuffer) { SendMessage(GetDlgItem(hDlg,IDC_LARGE_COMPILE_BUFFER),BM_SETCHECK, BST_CHECKED,0); }

		if (strlen(RomName) != 0) { 
			LPSTR NotesIniFileName = GetNotesIniFileName();
			char Identifier[100];
			
			sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
			_GetPrivateProfileString(Identifier,"Note","",String,sizeof(String),NotesIniFileName);
			SetDlgItemText(hDlg,IDC_NOTES,String);
		}

		if (strlen(RomName) == 0) {
			EnableWindow(GetDlgItem(hDlg,IDC_MEMORY_SIZE_TEXT),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_RDRAM_SIZE),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_SAVE_TYPE_TEXT),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_SAVE_TYPE),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_COUNTFACT_TEXT),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_COUNTFACT),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_CPU_TYPE_TEXT),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_CPU_TYPE),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_SELFMOD_TEXT),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_SELFMOD),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_USE_TLB),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_ROM_REGCACHE),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_BLOCK_LINKING_TEXT),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_BLOCK_LINKING),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_LARGE_COMPILE_BUFFER),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_NOTES),FALSE);
		}
		break;
	case WM_NOTIFY:
		if (((NMHDR FAR *) lParam)->code == PSN_APPLY) { 
			LPSTR NotesIniFileName = GetNotesIniFileName();
			char Identifier[100];
			
			if (strlen(RomName) == 0) { break; }
			sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
			_WritePrivateProfileString(Identifier,"Internal Name",RomName,NotesIniFileName);
			GetDlgItemText(hDlg,IDC_NOTES,String,sizeof(String));
			_WritePrivateProfileString(Identifier,"Note",String,NotesIniFileName);

			if (!UseIni) { break; }
			indx = SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_GETCURSEL,0,0); 
			RomRamSize = SendMessage(GetDlgItem(hDlg,IDC_RDRAM_SIZE),CB_GETITEMDATA,indx,0);
			indx = SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_GETCURSEL,0,0); 
			RomSaveUsing = SendMessage(GetDlgItem(hDlg,IDC_SAVE_TYPE),CB_GETITEMDATA,indx,0);
			indx = SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_GETCURSEL,0,0); 
			RomCF = SendMessage(GetDlgItem(hDlg,IDC_COUNTFACT),CB_GETITEMDATA,indx,0);
			indx = SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_GETCURSEL,0,0); 
			RomCPUType = SendMessage(GetDlgItem(hDlg,IDC_CPU_TYPE),CB_GETITEMDATA,indx,0);
			indx = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_GETCURSEL,0,0); 
			RomSelfMod = SendMessage(GetDlgItem(hDlg,IDC_SELFMOD),CB_GETITEMDATA,indx,0);
			indx = SendMessage(GetDlgItem(hDlg,IDC_BLOCK_LINKING),CB_GETCURSEL,0,0); 
			RomUseLinking = SendMessage(GetDlgItem(hDlg,IDC_BLOCK_LINKING),CB_GETITEMDATA,indx,0);
			RomUseTlb = SendMessage(GetDlgItem(hDlg,IDC_USE_TLB),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
			RomUseCache = SendMessage(GetDlgItem(hDlg,IDC_ROM_REGCACHE),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;
			RomUseLargeBuffer = SendMessage(GetDlgItem(hDlg,IDC_LARGE_COMPILE_BUFFER),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE;			
			SaveRomOptions();
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK ShellIntegrationProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:		
		{ 
			char * LangFile, Language[100], String[256];
			LangFile = GetLangFileName();
			_GetPrivateProfileString("Default","Language","English",Language,sizeof(Language),LangFile);

			GetDlgItemText(hDlg,IDC_SHELL_INT_TEXT,String,sizeof(String));
			_GetPrivateProfileString(Language,"ShellIntegration1",String,String,sizeof(String),LangFile);
			SetDlgItemText(hDlg,IDC_SHELL_INT_TEXT,String);	
		}
		if (TestExtensionRegistered(".v64")) { SendMessage(GetDlgItem(hDlg,IDC_V64),BM_SETCHECK, BST_CHECKED,0); }
		if (TestExtensionRegistered(".z64")) { SendMessage(GetDlgItem(hDlg,IDC_Z64),BM_SETCHECK, BST_CHECKED,0); }
		if (TestExtensionRegistered(".n64")) { SendMessage(GetDlgItem(hDlg,IDC_N64),BM_SETCHECK, BST_CHECKED,0); }
		if (TestExtensionRegistered(".rom")) { SendMessage(GetDlgItem(hDlg,IDC_ROM),BM_SETCHECK, BST_CHECKED,0); }
		if (TestExtensionRegistered(".jap")) { SendMessage(GetDlgItem(hDlg,IDC_JAP),BM_SETCHECK, BST_CHECKED,0); }
		if (TestExtensionRegistered(".pal")) { SendMessage(GetDlgItem(hDlg,IDC_PAL),BM_SETCHECK, BST_CHECKED,0); }
		if (TestExtensionRegistered(".usa")) { SendMessage(GetDlgItem(hDlg,IDC_USA),BM_SETCHECK, BST_CHECKED,0); }
		if (TestExtensionRegistered(".eur")) { SendMessage(GetDlgItem(hDlg,IDC_EUR),BM_SETCHECK, BST_CHECKED,0); }
		break;
	case WM_NOTIFY:
		if (((NMHDR FAR *) lParam)->code == PSN_APPLY) { 
			RegisterExtension(".v64",SendMessage(GetDlgItem(hDlg,IDC_V64),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE);
			RegisterExtension(".z64",SendMessage(GetDlgItem(hDlg,IDC_Z64),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE);
			RegisterExtension(".n64",SendMessage(GetDlgItem(hDlg,IDC_N64),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE);
			RegisterExtension(".rom",SendMessage(GetDlgItem(hDlg,IDC_ROM),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE);
			RegisterExtension(".jap",SendMessage(GetDlgItem(hDlg,IDC_JAP),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE);
			RegisterExtension(".pal",SendMessage(GetDlgItem(hDlg,IDC_PAL),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE);
			RegisterExtension(".usa",SendMessage(GetDlgItem(hDlg,IDC_USA),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE);
			RegisterExtension(".eur",SendMessage(GetDlgItem(hDlg,IDC_EUR),BM_GETSTATE, 0,0) == BST_CHECKED?TRUE:FALSE);
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}