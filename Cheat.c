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
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include "main.h"
#include "cheats.h"
#include "cpu.h"
#include "resource.h"

#define UM_CHECKSTATECHANGE     (WM_USER + 100)
#define UM_CHANGECODEEXTENSION  (WM_USER + 101)
#define IDC_MYTREE				0x500

#define MaxCheats				300

#define SelectCheat				1
#define EditCheat				2
#define NewCheat 				3 

HWND hManageWindow = NULL;
HWND hSelectCheat, hAddCheat, hCheatTree;
CHEAT_CODES Codes[MaxCheats];
int NoOfCodes;

void GetCheatName          ( int CheatNo, char * CheatName, int CheatNameLen );
BOOL LoadCheatExt          ( char * CheatName, char * CheatExt, int MaxCheatExtLen);
void RefreshCheatManager   ( void );
void SaveCheatExt          ( char * CheatName, char * CheatExt );
BOOL TreeView_GetCheckState(HWND hwndTreeView, HTREEITEM hItem);
BOOL TreeView_SetCheckState(HWND hwndTreeView, HTREEITEM hItem, BOOL fCheck);

LRESULT CALLBACK ManageCheatsProc (HWND, UINT, WPARAM, LPARAM );

void ApplyCheats (void) {
	int count, count2;
	DWORD Address;
	
	for (count = 0; count < NoOfCodes; count ++) {
		for (count2 = 0; count2 < MaxGSEntries; count2 ++) {
			switch (Codes[count].Code[count2].Command & 0xFF000000) {
			case 0x80000000:
				Address = 0x80000000 | (Codes[count].Code[count2].Command & 0xFFFFFF);
				r4300i_SB_VAddr(Address,(BYTE)Codes[count].Code[count2].Value);
				break;
			case 0x81000000:
				Address = 0x80000000 | (Codes[count].Code[count2].Command & 0xFFFFFF);
				r4300i_SH_VAddr(Address,Codes[count].Code[count2].Value);
				break;
			case 0xA0000000:
				Address = 0xA0000000 | (Codes[count].Code[count2].Command & 0xFFFFFF);
				r4300i_SB_VAddr(Address,(BYTE)Codes[count].Code[count2].Value);
				break;
			case 0xA1000000:
				Address = 0xA0000000 | (Codes[count].Code[count2].Command & 0xFFFFFF);
				r4300i_SH_VAddr(Address,Codes[count].Code[count2].Value);
				break;
			case 0: count2 = MaxGSEntries; break;
			}
		}

	} 
}

BOOL CheatActive (char * Name) {
	char String[300], Identifier[100];
	HKEY hKeyResults = 0;
	long lResult;
	
	sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
	sprintf(String,"Software\\N64 Emulation\\%s\\Cheats\\%s",AppName,Identifier);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0, KEY_ALL_ACCESS,&hKeyResults);	
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Bytes, Active;
		char GameName[300];

		Bytes = sizeof(GameName);
		lResult = RegQueryValueEx(hKeyResults,"Name",0,&Type,(LPBYTE)GameName,&Bytes);
		Bytes = sizeof(Active);
		lResult = RegQueryValueEx(hKeyResults,Name,0,&Type,(LPBYTE)(&Active),&Bytes);
		RegCloseKey(hKeyResults);
		if (lResult == ERROR_SUCCESS) { return Active; }
	}
	return FALSE;
}

LRESULT CALLBACK CheatsCodeExProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static DWORD CheatNo;
	switch (uMsg) {
	case WM_INITDIALOG:
		CheatNo = lParam;
		{
			char * String = NULL, Identifier[100], CheatName[300],CheatExt[300], * ReadPos;
			LPSTR IniFileName;
			DWORD len;

			GetCheatName(CheatNo,CheatName,sizeof(CheatName));
			SetDlgItemText(hDlg,IDC_CHEAT_NAME,CheatName);
			LoadCheatExt(CheatName,CheatExt,sizeof(CheatExt));

			IniFileName = GetCheatIniFileName();
			sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
			sprintf(CheatName,"Cheat%d_O",CheatNo);
			_GetPrivateProfileString2(Identifier,CheatName,"",&String,IniFileName);
			ReadPos = String;
			while (strlen(ReadPos) > 0) {
				int index;
				
				if(strchr(ReadPos,',') == NULL) {
					len = strlen(ReadPos);					
				} else {
					len = strchr(ReadPos,',') - ReadPos;
				}
				if (len >= sizeof(CheatName)) { len = sizeof(CheatName) - 1; }
				strncpy(CheatName,ReadPos,len);
				CheatName[len] = 0;
				index = SendMessage(GetDlgItem(hDlg,IDC_CHEAT_LIST),LB_ADDSTRING,0,(LPARAM)CheatName);
				if (strcmp(CheatExt,CheatName) == 0) { 
					SendMessage(GetDlgItem(hDlg,IDC_CHEAT_LIST),LB_SETCURSEL,index,0);
				}
				if (strchr(ReadPos,',') == NULL) {
					ReadPos += strlen(ReadPos);
				} else {
					ReadPos = strchr(ReadPos,',') + 1;
				}
			}
			if (String) { free(String); }
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHEAT_LIST:
			if (HIWORD(wParam) == LBN_DBLCLK) { PostMessage(hDlg,WM_COMMAND,IDOK,0); break; }
			break;			
		case IDOK:
			{
				char CheatName[300], CheatExten[300];
				int index;

				index = SendMessage(GetDlgItem(hDlg,IDC_CHEAT_LIST),LB_GETCURSEL,0,0);
				if (index < 0) { index = 0; }
				GetDlgItemText(hDlg,IDC_CHEAT_NAME,CheatName,sizeof(CheatName));
				index = SendMessage(GetDlgItem(hDlg,IDC_CHEAT_LIST),LB_GETTEXT,index,(LPARAM)CheatExten);
				SaveCheatExt(CheatName,CheatExten);
				LoadCheats();
			}
			EndDialog(hDlg,0);
			break;
		case IDCANCEL:
			EndDialog(hDlg,0);
			break;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK CheatsCodeQuantProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static WORD Start, Stop, SelStart, SelStop;
	static DWORD CheatNo;


	switch (uMsg) {
	case WM_INITDIALOG:
		CheatNo = lParam;
		{
			char * String = NULL, Identifier[100], CheatName[300],CheatExt[300], * ReadPos;
			LPSTR IniFileName;

			IniFileName = GetCheatIniFileName();
			sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
			sprintf(CheatName,"Cheat%d_RN",CheatNo);
			_GetPrivateProfileString2(Identifier,CheatName,"",&String,IniFileName);
			SetDlgItemText(hDlg,IDC_NOTES,String);
			sprintf(CheatName,"Cheat%d_R",CheatNo);
			_GetPrivateProfileString2(Identifier,CheatName,"",&String,IniFileName);
			Start = (WORD)(String[0] == '$'?AsciiToHex(&String[1]):atol(String));
			ReadPos  = strrchr(String,'-');
			if (ReadPos != NULL) {
				Stop = (WORD)(ReadPos[1] == '$'?AsciiToHex(&ReadPos[2]):atol(&ReadPos[1]));			
			} else {
				Stop = 0;
			}
			
			GetCheatName(CheatNo,CheatName,sizeof(CheatName));
			SetDlgItemText(hDlg,IDC_CHEAT_NAME,CheatName);
			LoadCheatExt(CheatName,CheatExt,sizeof(CheatExt));
			SetDlgItemText(hDlg,IDC_VALUE,CheatExt);
			sprintf(CheatExt,"from $%X to $%X",Start,Stop);
			SetDlgItemText(hDlg,IDC_RANGE,CheatExt);
			if (String) { free(String); }
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_VALUE:
			if (HIWORD(wParam) == EN_UPDATE) {
				TCHAR szTmp[10], szTmp2[10];
				DWORD Value;
				GetDlgItemText(hDlg,IDC_VALUE,szTmp,sizeof(szTmp));
				Value = szTmp[0] =='$'?AsciiToHex(&szTmp[1]):AsciiToHex(szTmp);
				if (Value > Stop) { Value = Stop; }
				if (Value < Start) { Value = Start; }
				sprintf(szTmp2,"$%X",Value);
				if (strcmp(szTmp,szTmp2) != 0) {
					SetDlgItemText(hDlg,IDC_VALUE,szTmp2);
					if (SelStop == 0) { SelStop = strlen(szTmp2); SelStart = SelStop; }
					SendDlgItemMessage(hDlg,IDC_VALUE,EM_SETSEL,(WPARAM)SelStart,(LPARAM)SelStop);
				} else {
					WORD NewSelStart, NewSelStop;
					SendDlgItemMessage(hDlg,IDC_VALUE,EM_GETSEL,(WPARAM)&NewSelStart,(LPARAM)&NewSelStop);
					if (NewSelStart != 0) { SelStart = NewSelStart; SelStop = NewSelStop; }
				}
			}
			break;
		case IDOK:
			{
				TCHAR CheatName[300], CheatExten[300], szTmp[10];
				DWORD Value;

				GetDlgItemText(hDlg,IDC_VALUE,szTmp,sizeof(szTmp));
				Value = szTmp[0] =='$'?AsciiToHex(&szTmp[1]):AsciiToHex(szTmp);
				if (Value > Stop) { Value = Stop; }
				if (Value < Start) { Value = Start; }
				
				GetDlgItemText(hDlg,IDC_CHEAT_NAME,CheatName,sizeof(CheatName));
				sprintf(CheatExten,"$%X",Value);
				SaveCheatExt(CheatName,CheatExten);
				LoadCheats();
			}
			EndDialog(hDlg,0);
			break;
		case IDCANCEL:
			EndDialog(hDlg,0);
			break;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CheatUsesCodeExtensions (char * CheatString) {
	BOOL CodeExtension;
	DWORD count, len;

	char * ReadPos;

	if (strlen(CheatString) == 0){ return FALSE; }
	if (strchr(CheatString,'"') == NULL) { return FALSE; }
	len = strrchr(CheatString,'"') - strchr(CheatString,'"') - 1;

	ReadPos = strrchr(CheatString,'"') + 2;
	CodeExtension = FALSE;
	for (count = 0; count < MaxGSEntries && CodeExtension == FALSE; count ++) {
		if (strchr(ReadPos,' ') == NULL) { break; }
		ReadPos = strchr(ReadPos,' ') + 1;
		if (ReadPos[0] == '?' && ReadPos[1]== '?') { CodeExtension = TRUE; }
		if (ReadPos[2] == '?' && ReadPos[3]== '?') { CodeExtension = TRUE; }
		if (strchr(ReadPos,',') == NULL) { continue; }
		ReadPos = strchr(ReadPos,',') + 1;
	}
	return CodeExtension;
}

LRESULT CALLBACK CheatAddProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_INSERT:
			{
				TCHAR szTmp[10], szTmp2[10], szString[30];
				DWORD Value;

				GetDlgItemText(hDlg,IDC_MOD_CODE,szTmp,sizeof(szTmp));
				Value = AsciiToHex(szTmp);
				sprintf(szTmp,"%08X",Value);
				GetDlgItemText(hDlg,IDC_MOD_CODE2,szTmp2,sizeof(szTmp2));
				Value = AsciiToHex(szTmp2);
				sprintf(szTmp2,"%04X",(WORD)Value);
				sprintf(szString,"%s %s",szTmp,szTmp2);
				SendDlgItemMessage(hDlg,IDC_CHEAT_LIST,LB_ADDSTRING,0,(LPARAM)szString);
				SetDlgItemText(hDlg,IDC_MOD_CODE,"");
				SetDlgItemText(hDlg,IDC_MOD_CODE2,"");
			}
			break;
		case IDC_CLEAR: SendDlgItemMessage(hDlg,IDC_CHEAT_LIST,LB_RESETCONTENT,0,0); break;
		case IDC_MOD_CODE:
			if (HIWORD(wParam) == EN_UPDATE) {
				static WORD SelStart, SelStop;
				TCHAR szTmp[10], szTmp2[10];
				DWORD Value;
				GetDlgItemText(hDlg,LOWORD(wParam),szTmp,sizeof(szTmp));
				Value = szTmp[0] =='$'?AsciiToHex(&szTmp[1]):AsciiToHex(szTmp);
				sprintf(szTmp2,"%X",Value);
				if (strcmp(szTmp,szTmp2) != 0) {
					SetDlgItemText(hDlg,LOWORD(wParam),szTmp2);
					if (SelStop == 0) { SelStop = strlen(szTmp2); SelStart = SelStop; }
					SendDlgItemMessage(hDlg,LOWORD(wParam),EM_SETSEL,(WPARAM)SelStart,(LPARAM)SelStop);
				} else {
					WORD NewSelStart, NewSelStop;
					SendDlgItemMessage(hDlg,LOWORD(wParam),EM_GETSEL,(WPARAM)&NewSelStart,(LPARAM)&NewSelStop);
					if (NewSelStart != 0) { SelStart = NewSelStart; SelStop = NewSelStop; }
				}
			}
			break;
		case IDC_MOD_CODE2:
			if (HIWORD(wParam) == EN_UPDATE) {
				static WORD SelStart, SelStop;
				TCHAR szTmp[10], szTmp2[10];
				DWORD Value;
				GetDlgItemText(hDlg,LOWORD(wParam),szTmp,sizeof(szTmp));
				Value = szTmp[0] =='$'?AsciiToHex(&szTmp[1]):AsciiToHex(szTmp);
				if (Value > 0xFFFF) { Value = 0xFFFF; }
				sprintf(szTmp2,"%X",(WORD)Value);
				if (strcmp(szTmp,szTmp2) != 0) {
					SetDlgItemText(hDlg,LOWORD(wParam),szTmp2);
					if (SelStop == 0) { SelStop = strlen(szTmp2); SelStart = SelStop; }
					SendDlgItemMessage(hDlg,LOWORD(wParam),EM_SETSEL,(WPARAM)SelStart,(LPARAM)SelStop);
				} else {
					WORD NewSelStart, NewSelStop;
					SendDlgItemMessage(hDlg,LOWORD(wParam),EM_GETSEL,(WPARAM)&NewSelStart,(LPARAM)&NewSelStop);
					if (NewSelStart != 0) { SelStart = NewSelStart; SelStop = NewSelStop; }
				}
			}
			break;
		case IDC_ADD:
			{
				char Identifier[100], CheatName[200], NewCheatName[200], Line[20], * cheat;
				int ListCount, CheatLen, count, CheatNo;
				LPSTR IniFileName;

				GetDlgItemText(hDlg,IDC_CODE_NAME,NewCheatName,sizeof(NewCheatName));
				if (strlen(NewCheatName) == 0) { 
					MessageBox(hDlg,"You have to have a name for this cheat","Error",MB_OK|MB_ICONERROR);
					SetFocus(GetDlgItem(hDlg,IDC_CODE_NAME));
					break;
				}
				ListCount = SendDlgItemMessage(hDlg,IDC_CHEAT_LIST,LB_GETCOUNT,0,0);
				if (ListCount == 0) {
					MessageBox(hDlg,"You must have at least one entry","Error",MB_OK|MB_ICONERROR);
					SetFocus(GetDlgItem(hDlg,IDC_CODE_NAME));
					break;
				}
				for (count = 0; count < MaxCheats; count ++) {
					GetCheatName(count,CheatName,sizeof(CheatName));
					if (strlen(CheatName) == 0) {
						CheatNo = count;
						break;
					}
					if (strcmp(CheatName,NewCheatName) == 0) {
						MessageBox(hDlg,"Cheat Name is already in use","Error",MB_OK|MB_ICONERROR);
						SetFocus(GetDlgItem(hDlg,IDC_CODE_NAME));
						return TRUE;
					}
				}
				CheatLen = strlen(NewCheatName) + (ListCount * 14) + 100;
				cheat = malloc(CheatLen);
				sprintf(cheat,"\"%s\"",NewCheatName);
				for (count = 0; count < ListCount; count ++) {
					SendMessage(GetDlgItem(hDlg,IDC_CHEAT_LIST),LB_GETTEXT,count,(LPARAM)Line);
					strcat(cheat,",");
					strcat(cheat,Line);
				}
				//Add to ini
				IniFileName = GetCheatIniFileName();
				sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
				_WritePrivateProfileString(Identifier,"Name",RomName,IniFileName);
				sprintf(NewCheatName,"Cheat%d",CheatNo);
				_WritePrivateProfileString(Identifier,NewCheatName,cheat,GetCheatIniFileName());				
				if (cheat) { free(cheat); cheat = NULL; }
				CheatLen = SendDlgItemMessage(hDlg,IDC_NOTES,WM_GETTEXTLENGTH,0,0) + 5;
				if (CheatLen > 0) {
					cheat = malloc(CheatLen);
					GetDlgItemText(hDlg,IDC_NOTES,cheat,CheatLen);
					sprintf(NewCheatName,"Cheat%d_N",CheatNo);
					_WritePrivateProfileString(Identifier,NewCheatName,cheat,GetCheatIniFileName());				
					if (cheat) { free(cheat); cheat = NULL; }
				}
				MessageBox(hDlg,"Added","Error",MB_OK|MB_ICONERROR);
				RefreshCheatManager();
				SetDlgItemText(hDlg,IDC_CODE_NAME,"");
				SetDlgItemText(hDlg,IDC_MOD_CODE,"");
				SetDlgItemText(hDlg,IDC_MOD_CODE2,"");
				SendDlgItemMessage(hDlg,IDC_CHEAT_LIST,LB_RESETCONTENT,0,0);
				SetDlgItemText(hDlg,IDC_NOTES,"");
				
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK CheatListProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			DWORD Style;

			hCheatTree = CreateWindowEx(WS_EX_CLIENTEDGE,WC_TREEVIEW,"",
					WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | TVS_HASLINES | 
					TVS_HASBUTTONS | TVS_LINESATROOT  | TVS_DISABLEDRAGDROP |WS_TABSTOP|
					TVS_FULLROWSELECT, 11, 15, 268, 150, hDlg, (HMENU)IDC_MYTREE, hInst, NULL);
			Style = GetWindowLong(hCheatTree,GWL_STYLE);					
			SetWindowLong(hCheatTree,GWL_STYLE,TVS_CHECKBOXES |TVS_SHOWSELALWAYS| Style);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_UNMARK:
			{
				HTREEITEM hItem;
				hItem = TreeView_GetRoot(hCheatTree);
				while (hItem != NULL) {
					TreeView_SetCheckState(hCheatTree,hItem,FALSE);
					SendMessage(hDlg, UM_CHECKSTATECHANGE, TRUE, (LPARAM)hItem);
					hItem = TreeView_GetNextSibling(hCheatTree,hItem);
				}
			}
			break;
		case IDC_MARK:
			{
				HTREEITEM hItem;
				hItem = TreeView_GetRoot(hCheatTree);
				while (hItem != NULL) {
					TreeView_SetCheckState(hCheatTree,hItem,TRUE);
					SendMessage(hDlg, UM_CHECKSTATECHANGE, TRUE, (LPARAM)hItem);
					hItem = TreeView_GetNextSibling(hCheatTree,hItem);
				}
			}
			break;
		}
		break;
	case WM_NOTIFY:
		{
		   LPNMHDR lpnmh = (LPNMHDR) lParam;
    
		   if ((lpnmh->code  == NM_CLICK) && (lpnmh->idFrom == IDC_MYTREE))
		   {
				TVHITTESTINFO ht = {0};
				DWORD dwpos = GetMessagePos();

				// include <windowsx.h> and <windows.h> header files
				ht.pt.x = GET_X_LPARAM(dwpos);
				ht.pt.y = GET_Y_LPARAM(dwpos);
				MapWindowPoints(HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1);

				TreeView_HitTest(lpnmh->hwndFrom, &ht);

				if(TVHT_ONITEMSTATEICON & ht.flags)
				{
					PostMessage(hDlg, UM_CHECKSTATECHANGE, (WPARAM)TRUE, (LPARAM)ht.hItem);
				}
			}
		   if ((lpnmh->code  == NM_DBLCLK) && (lpnmh->idFrom == IDC_MYTREE))
		   {
				TVHITTESTINFO ht = {0};
				DWORD dwpos = GetMessagePos();

				// include <windowsx.h> and <windows.h> header files
				ht.pt.x = GET_X_LPARAM(dwpos);
				ht.pt.y = GET_Y_LPARAM(dwpos);
				MapWindowPoints(HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1);

				TreeView_HitTest(lpnmh->hwndFrom, &ht);

				if(TVHT_ONITEMLABEL & ht.flags)
				{
					PostMessage(hDlg, UM_CHANGECODEEXTENSION, 0, (LPARAM)ht.hItem);
				}
			}
			if ((lpnmh->code  == TVN_SELCHANGED) && (lpnmh->idFrom == IDC_MYTREE)) {
				HTREEITEM hItem;

				hItem = TreeView_GetSelection(hCheatTree);
				if (TreeView_GetChild(hCheatTree,hItem) == NULL) { 
					char * String = NULL, Lookup[40], Identifier[100];
					LPSTR IniFileName;
					TVITEM item;

					item.mask = TVIF_PARAM ;
					item.hItem = hItem;
					TreeView_GetItem(hCheatTree,&item);

					IniFileName = GetCheatIniFileName();
					sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));

					sprintf(Lookup,"Cheat%d_N",item.lParam);
					_GetPrivateProfileString2(Identifier,Lookup,"",&String,IniFileName);
					SetDlgItemText(hDlg,IDC_NOTES,String);
					if (String) { free(String); }
				} else {
					SetDlgItemText(hDlg,IDC_NOTES,"");
				}
			}
		}
		break;
	case UM_CHANGECODEEXTENSION:
		{
			char Identifier[100], * String = NULL, CheatName[500],CheatExt[300];
			HTREEITEM hItemChanged = (HTREEITEM)lParam;
			LPSTR IniFileName;
			TVITEM item;
	
			item.mask = TVIF_PARAM ;
			item.hItem = hItemChanged;
			TreeView_GetItem(hCheatTree,&item);

			IniFileName = GetCheatIniFileName();
			sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
			sprintf(CheatName,"Cheat%d",item.lParam);
			_GetPrivateProfileString2(Identifier,CheatName,"",&String,IniFileName);
			if (!CheatUsesCodeExtensions(String)) { 
				if (String) { free(String); }
				break; 
			}
			sprintf(CheatName,"Cheat%d_O",item.lParam);
			_GetPrivateProfileString2(Identifier,CheatName,"",&String,IniFileName);
			if (strlen(String) > 0) {
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_Cheats_CodeEx),hDlg,(DLGPROC)CheatsCodeExProc,item.lParam);
			} else {
				sprintf(CheatName,"Cheat%d_R",item.lParam);
				_GetPrivateProfileString2(Identifier,CheatName,"",&String,IniFileName);
				if (strlen(String) > 0) {
					DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_Cheats_Range),hDlg,(DLGPROC)CheatsCodeQuantProc,item.lParam);
				}
			}
			if (String) { free(String); }
			GetCheatName(item.lParam,CheatName,sizeof(CheatName));
			if (!LoadCheatExt(CheatName,CheatExt,sizeof(CheatExt))) {
				sprintf(CheatExt,"?");
			}
			item.mask = TVIF_PARAM | TVIF_TEXT;
			item.pszText = CheatName;
			item.cchTextMax = sizeof(CheatName);
			if (strrchr(CheatName,'\\') != NULL) {
				strcpy(CheatName,strrchr(CheatName,'\\') + 1);
			}
			sprintf(CheatName,"%s (=> %s)",CheatName,CheatExt);
			TreeView_SetItem(hCheatTree,&item);
		}
		break;
	case UM_CHECKSTATECHANGE:
		{
			HTREEITEM hChild,hParent, hItemChanged = (HTREEITEM)lParam;
			char String[300], Identifier[100];
			DWORD Disposition = 0;
			HKEY hKeyResults = 0;
			long lResult;
			int active;
			
			sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
			sprintf(String,"Software\\N64 Emulation\\%s\\Cheats\\%s",AppName,Identifier);
			lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"", REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
			active = TreeView_GetCheckState(hCheatTree,hItemChanged);
			if (lResult == ERROR_SUCCESS) {
				HTREEITEM hParent;
				char Name[300], Text[300];
				TVITEM item;

				item.mask = TVIF_TEXT|TVIF_PARAM;
				item.hItem = hItemChanged;
				item.pszText = Text;
				item.cchTextMax = sizeof(Text);
				TreeView_GetItem(hCheatTree,&item);
				
				if (TreeView_GetChild(hCheatTree,hItemChanged)) {
					hParent = TreeView_GetParent(hCheatTree,hItemChanged);
					strcpy(Name,Text);
					while (hParent != NULL) {
						char NewName[300];

						item.hItem = hParent;
						TreeView_GetItem(hCheatTree,&item);
						sprintf(NewName,"%s\\%s",Text,Name);
						hParent = TreeView_GetParent(hCheatTree,hParent);
						strcpy(Name,NewName);
					}
				} else {
					GetCheatName(item.lParam,Name,sizeof(Name));
				}
				RegSetValueEx(hKeyResults,"Name",0,REG_SZ,(CONST BYTE *)RomName,strlen(RomName));
				
				RegSetValueEx(hKeyResults,Name,0, REG_DWORD,(CONST BYTE *)(&active),sizeof(DWORD));
				RegCloseKey(hKeyResults);
			}
			if (wParam) {
				hChild = TreeView_GetChild(hCheatTree,hItemChanged);
				while (hChild != NULL) {
					TreeView_SetCheckState(hCheatTree,hChild,active);
					SendMessage(hDlg, UM_CHECKSTATECHANGE, wParam, (LPARAM)hChild);
					hChild = TreeView_GetNextSibling(hCheatTree,hChild);
				}
				LoadCheats();
			}
			if (!active) {
				hParent = TreeView_GetParent(hCheatTree,hItemChanged);
				if (hParent) {
					TreeView_SetCheckState(hCheatTree,hParent,active);
					SendMessage(hDlg, UM_CHECKSTATECHANGE, FALSE, (LPARAM)hParent);
				}
			}
		}
		break; 
	default:
		return FALSE;
	}
	return TRUE;
}

char * GetCheatIniFileName(void) {
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
	char fname[_MAX_FNAME],ext[_MAX_EXT];
	static char IniFileName[_MAX_PATH];

	GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
	_splitpath( path_buffer, drive, dir, fname, ext );
	sprintf(IniFileName,"%s%s%s",drive,dir,CheatIniName);
	return IniFileName;
}

void GetCheatName(int CheatNo, char * CheatName, int CheatNameLen) {
	char *String = NULL, Identifier[100];
	DWORD len;

	LPSTR IniFileName;
	IniFileName = GetCheatIniFileName();
	sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
	sprintf(CheatName,"Cheat%d",CheatNo);
	_GetPrivateProfileString2(Identifier,CheatName,"",&String,IniFileName);
	if (strlen(String) == 0) {
		memset(CheatName,0,CheatNameLen);
		if (String) { free(String); }
		return;		
	}
	len = strrchr(String,'"') - strchr(String,'"') - 1;
	memset(CheatName,0,CheatNameLen);
	strncpy(CheatName,strchr(String,'"') + 1,len);
	//if (strrchr(CheatName,'\\') != NULL) {
	//	strcpy(CheatName,strrchr(CheatName,'\\') + 1);
	//}
	if (String) { free(String); }
}

BOOL LoadCheatExt(char * CheatName, char * CheatExt, int MaxCheatExtLen) {
	char String[350], Identifier[100];
	HKEY hKeyResults = 0;
	long lResult;
	
	sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
	sprintf(String,"Software\\N64 Emulation\\%s\\Cheats\\%s",AppName,Identifier);

	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0, KEY_ALL_ACCESS,&hKeyResults);	
	if (lResult == ERROR_SUCCESS) {		
		DWORD Type, Bytes;

		sprintf(String,"%s exten",CheatName);
		Bytes = MaxCheatExtLen;
		lResult = RegQueryValueEx(hKeyResults,String,0,&Type,(LPBYTE)CheatExt,&Bytes);	
		RegCloseKey(hKeyResults);
		if (lResult == ERROR_SUCCESS) { return TRUE; }
	}
	return FALSE;
}

void LoadCheats (void) {
	DWORD len, count;
	LPSTR IniFileName;
	char * String = NULL;
	char Identifier[100];
	char CheatName[300];
	
	IniFileName = GetCheatIniFileName();
	sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
	NoOfCodes = 0;

	for (count = 0; count < MaxCheats; count ++ ) {
		char * ReadPos;
		int count2;

		sprintf(CheatName,"Cheat%d",count);
		_GetPrivateProfileString2(Identifier,CheatName,"",&String,IniFileName);
		if (strlen(String) == 0){ break; }
		if (strchr(String,'"') == NULL) { continue; }
		len = strrchr(String,'"') - strchr(String,'"') - 1;
		if ((int)len < 1) { continue; }
		memset(CheatName,0,sizeof(CheatName));
		strncpy(CheatName,strchr(String,'"') + 1,len);
		if (strlen(CheatName) == 0) { continue; }
		//if (strrchr(CheatName,'\\') != NULL) {
		//	strcpy(CheatName,strrchr(CheatName,'\\') + 1);
		//}		
		if (!CheatActive (CheatName)) { continue; }
		ReadPos = strrchr(String,'"') + 2;
		for (count2 = 0; count2 < MaxGSEntries; count2 ++) {
			char CheatExt[200];
			WORD Value;

			Codes[NoOfCodes].Code[count2].Command = AsciiToHex(ReadPos);
			if (strchr(ReadPos,' ') == NULL) { break; }
			ReadPos = strchr(ReadPos,' ') + 1;
			if (strncmp(ReadPos,"????",4) == 0) {
				if (LoadCheatExt(CheatName,CheatExt,sizeof(CheatExt))) {
					Value = CheatExt[0] == '$'?(WORD)AsciiToHex(&CheatExt[1]):(WORD)atol(CheatExt);
				} else {
					count2 = 0; break;
				}
				Codes[NoOfCodes].Code[count2].Value = Value;
			} else if (strncmp(ReadPos,"??",2) == 0) {
				Codes[NoOfCodes].Code[count2].Value = (BYTE)(AsciiToHex(ReadPos));
				if (LoadCheatExt(CheatName,CheatExt,sizeof(CheatExt))) {
					Value = CheatExt[0] == '$'?(BYTE)AsciiToHex(&CheatExt[1]):(BYTE)atol(CheatExt);
				} else {
					count2 = 0; break;
				}
				Codes[NoOfCodes].Code[count2].Value += (Value << 16);
			} else if (strncmp(&ReadPos[2],"??",2) == 0) {				
				Codes[NoOfCodes].Code[count2].Value = (WORD)(AsciiToHex(ReadPos) << 16);
				if (LoadCheatExt(CheatName,CheatExt,sizeof(CheatExt))) {
					Value = CheatExt[0] == '$'?(BYTE)AsciiToHex(&CheatExt[1]):(BYTE)atol(CheatExt);
				} else {
					count2 = 0; break;
				}
				Codes[NoOfCodes].Code[count2].Value += Value;
			} else {
				Codes[NoOfCodes].Code[count2].Value = (WORD)AsciiToHex(ReadPos);
			}
			if (strchr(ReadPos,',') == NULL) { continue; }
			ReadPos = strchr(ReadPos,',') + 1;
		}
		if (count2 == 0) { continue; }
		if (count2 < MaxGSEntries) {
			Codes[NoOfCodes].Code[count2].Command = 0;
			Codes[NoOfCodes].Code[count2].Value   = 0;
		}
		NoOfCodes += 1;
	}
	if (String) { free(String); }
}

void ManageCheats (HWND hParent) {
	if (hManageWindow != NULL) {
		SetForegroundWindow(hManageWindow);
		return;
	}
	DialogBox(hInst, MAKEINTRESOURCE(IDD_MANAGECHEATS), hParent, (DLGPROC)ManageCheatsProc);
	hManageWindow = NULL;
}

LRESULT CALLBACK ManageCheatsProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int CurrentPanel = SelectCheat;
	static RECT rcDisp;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			WINDOWPLACEMENT WndPlac;
			TC_ITEM item;
			HWND hTab;
			RECT * rc;

			hTab = GetDlgItem(hDlg,IDC_TAB);
			SendMessage(hTab, WM_SETFONT, (WPARAM)GetStockObject( DEFAULT_GUI_FONT ), 0);
			item.mask    = TCIF_TEXT | TCIF_PARAM;
			item.pszText = " Cheat List ";
			item.lParam  = SelectCheat;
			TabCtrl_InsertItem( hTab,0, &item);		
			item.pszText = " New ";
			item.lParam  = NewCheat;
			TabCtrl_InsertItem( hTab,2, &item);		
			WndPlac.length = sizeof(WndPlac);
			GetWindowPlacement(GetDlgItem(hDlg,IDC_DLGPOS),&WndPlac);
			rc = &WndPlac.rcNormalPosition;

			hSelectCheat = CreateDialog(hInst, MAKEINTRESOURCE(IDD_Cheats_List),hDlg,(DLGPROC)CheatListProc);
			SetWindowPos(hSelectCheat,HWND_TOP,rc->left,rc->top,rc->right - rc->left,rc->bottom - rc->top ,0);
			hAddCheat = CreateDialog(hInst, MAKEINTRESOURCE(IDD_Cheats_Add),hDlg,(DLGPROC)CheatAddProc);
			SetWindowPos(hAddCheat,HWND_TOP,rc->left,rc->top,rc->right - rc->left,rc->bottom - rc->top ,0);
			ShowWindow(hSelectCheat,SW_SHOW);
		}
		hManageWindow = hDlg;
		RefreshCheatManager();
		break;
	case WM_SIZE:
		GetClientRect( hDlg, &rcDisp);
		TabCtrl_AdjustRect( GetDlgItem(hDlg,IDC_TAB), FALSE, &rcDisp );
		break;
	case WM_NOTIFY:
		switch (((NMHDR *)lParam)->code) {
		case TCN_SELCHANGE:
			{
				TC_ITEM item;
				HWND hTab;

				hTab = GetDlgItem(hDlg,IDC_TAB);
				InvalidateRect( hTab, &rcDisp, TRUE );
				switch (CurrentPanel) {				
				case SelectCheat: ShowWindow(hSelectCheat,SW_HIDE); break;
				case NewCheat: ShowWindow(hAddCheat,SW_HIDE); break;
				}
				item.mask = TCIF_PARAM;
				TabCtrl_GetItem( hTab, TabCtrl_GetCurSel( hTab ), &item );
				CurrentPanel = item.lParam;
				switch (CurrentPanel) {
				case SelectCheat: ShowWindow(hSelectCheat,SW_SHOW); break;
				case NewCheat: ShowWindow(hAddCheat,SW_SHOW); break;
				}
				break;
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg,0);
			break;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

void RefreshCheatManager(void) {
	char CheatName[500], FullCheatName[500], CurrentName[300], *Layered, Identifier[100], * String = NULL;
	BOOL CodeExtension, IsCheatActive;
	TV_INSERTSTRUCT tv, tvold;
	DWORD count, len;
	LPSTR IniFileName;
	HTREEITEM hParent;

	if (hManageWindow == NULL) { return; }

	IniFileName = GetCheatIniFileName();
	sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
	NoOfCodes = 0;
	
	tv.hInsertAfter = TVI_SORT;
	tv.item.mask    = TVIF_TEXT | TVIF_PARAM;
	tvold.item.mask = TVIF_TEXT;
	tvold.item.pszText = CurrentName;
	tvold.item.cchTextMax = sizeof(CurrentName);

	TreeView_DeleteAllItems(hCheatTree);
	for (count = 0; count < MaxCheats; count ++ ) {
		char * ReadPos;

		sprintf(CheatName,"Cheat%d",count);
		_GetPrivateProfileString2(Identifier,CheatName,"",&String,IniFileName);
		if (strlen(String) == 0){ break; }
		if (strchr(String,'"') == NULL) { continue; }
		len = strrchr(String,'"') - strchr(String,'"') - 1;
		if (len >= sizeof(CheatName)) { len = sizeof(CheatName) - 1; }		
		memset(CheatName,0,sizeof(CheatName));
		strncpy(CheatName,strchr(String,'"') + 1,len);
		if (strlen(CheatName) == 0) { continue; }
		strcpy(FullCheatName,"");

		ReadPos = strrchr(String,'"') + 2;
		CodeExtension = CheatUsesCodeExtensions(String);

		Layered = strtok(CheatName,"\\");
		Layered += strlen(CheatName) + 1;
		tv.item.pszText = CheatName;
		tv.item.lParam  = count;
		tv.hParent      = TVI_ROOT;
		hParent = NULL;
		if (strlen(Layered) > 0) {
			tvold.item.hItem = TreeView_GetRoot(hCheatTree);
			while (hParent == NULL && tvold.item.hItem != NULL) {
				TreeView_GetItem(hCheatTree,&tvold.item);
				if (strcmp(CurrentName,CheatName) == 0) { hParent = tvold.item.hItem; }
				tvold.item.hItem = TreeView_GetNextSibling(hCheatTree,tvold.item.hItem);
			}
		}
		strcat(FullCheatName,CheatName);
		IsCheatActive = CheatActive (FullCheatName);
		if (CodeExtension && strlen(Layered) == 0) { 
			char CheatExt[200];
			if (!LoadCheatExt(CheatName,CheatExt,sizeof(CheatExt))) {
				sprintf(CheatExt,"?");
			}
			sprintf(CheatName,"%s (=> %s)",CheatName,CheatExt);
			Layered += strlen(Layered) + 1;
		}
		if (hParent == NULL) { hParent = TreeView_InsertItem(hCheatTree,&tv); }
		if (IsCheatActive) { 
			TreeView_SetCheckState(hCheatTree,hParent,TRUE); 
		}
		while (strlen(Layered) > 0) {
			HTREEITEM hNewParent;
			char * Text;

			Text = Layered;
			strtok(Layered,"\\");
			Layered += strlen(Layered) + 1;
			hNewParent = NULL;
			if (strlen(Layered) > 0) {
				tvold.item.hItem = TreeView_GetChild(hCheatTree,hParent);
				while (hNewParent == NULL && tvold.item.hItem != NULL) {
					TreeView_GetItem(hCheatTree,&tvold.item);
					if (strcmp(CurrentName,Text) == 0) { hNewParent = tvold.item.hItem; }
					tvold.item.hItem = TreeView_GetNextSibling(hCheatTree,tvold.item.hItem);
				}
			}
			strcat(FullCheatName,"\\");
			strcat(FullCheatName,Text);
			if (hNewParent == NULL) { 
				tv.hParent      = hParent;
				tv.item.pszText = Text;
				if (CodeExtension && strlen(Layered) == 0) { 
					char CheatExt[200];
					if (!LoadCheatExt(FullCheatName,CheatExt,sizeof(CheatExt))) {
						sprintf(CheatExt,"?");
					}
					sprintf(Text,"%s (=> %s)",Text,CheatExt);
					Layered += strlen(Layered) + 1;
				}
				hParent = TreeView_InsertItem(hCheatTree,&tv);
				if (CheatActive (FullCheatName)) { 
					TreeView_SetCheckState(hCheatTree,hParent,TRUE); 
				}
			} else {
				hParent = hNewParent;
			}
		}
	}
	if (String) { free(String); }
		
}

void SaveCheatExt(char * CheatName, char * CheatExt) {
	char String[300], Identifier[100];
	DWORD Disposition = 0;
	HKEY hKeyResults = 0;
	long lResult;
	
	sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
	sprintf(String,"Software\\N64 Emulation\\%s\\Cheats\\%s",AppName,Identifier);
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER, String,0,"", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,NULL, &hKeyResults,&Disposition);
	if (lResult == ERROR_SUCCESS) {		
		sprintf(String,"%s exten",CheatName);
		RegSetValueEx(hKeyResults,"Name",0,REG_SZ,(CONST BYTE *)RomName,strlen(RomName));				
		RegSetValueEx(hKeyResults,String,0,REG_SZ,(CONST BYTE *)CheatExt,strlen(CheatExt));
		RegCloseKey(hKeyResults);
	}
}

BOOL TreeView_GetCheckState(HWND hwndTreeView, HTREEITEM hItem)
{
    TVITEM tvItem;

    // Prepare to receive the desired information.
    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    // Request the information.
    TreeView_GetItem(hwndTreeView, &tvItem);

    // Return zero if it's not checked, or nonzero otherwise.
    return ((BOOL)(tvItem.state >> 12) -1);
}

BOOL TreeView_SetCheckState(HWND hwndTreeView, HTREEITEM hItem, BOOL fCheck)
{
    TVITEM tvItem;

    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    /*Image 1 in the tree-view check box image list is the
    unchecked box. Image 2 is the checked box.*/

    tvItem.state = INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1));

    return TreeView_SetItem(hwndTreeView, &tvItem);
}


