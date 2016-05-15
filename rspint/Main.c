/*
 * RSP Interpreter plug in for Project 64 (A Nintendo 64 emulator).
 *
 * (c) Copyright 2001 zilmar (zilmar@emulation64.com) 
 *
 * RSP Interpreter plug in homepage: www.pj64.net
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
 * RSP Interpreter plug in is freeware for PERSONAL USE only. 
 * Commercial users should seek permission of the copyright holders first. 
 * Commercial use includes charging money for RSP Interpreter plug in or 
 * software derived from RSP Interpreter plug in.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so if they want them.
 *
 */
#include <Windows.h>
#include <stdio.h>
#include "Rsp.h"
#include "Rsp_CPU.h"
#include "Rsp_Command.h"
#include "Rsp_Registers.h"
#include "breakpoint.h"
#include "resource.h"

void ProcessMenuItem(int ID);

DEBUG_INFO DebugInfo;
RSP_INFO RSPInfo;
HINSTANCE hinstDLL;
HMENU hRSPMenu = NULL;

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

void DisplayError (char * Message, ...) {
	char Msg[400];
	va_list ap;

	va_start( ap, Message );
	vsprintf( Msg, Message, ap );
	va_end( ap );
	MessageBox(NULL,Msg,"Error",MB_OK|MB_ICONERROR);
}

/******************************************************************
  Function: CloseDLL
  Purpose:  This function is called when the emulator is closing
            down allowing the dll to de-initialise.
  input:    none
  output:   none
*******************************************************************/ 
__declspec(dllexport) void CloseDLL (void) {
}

/******************************************************************
  Function: DllAbout
  Purpose:  This function is optional function that is provided
            to give further information about the DLL.
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
__declspec(dllexport) void DllAbout ( HWND hParent ) {
	MessageBox(hParent,AboutMsg,"About",MB_OK | MB_ICONINFORMATION );
}

BOOL WINAPI DllMain(  HINSTANCE hinst, DWORD fdwReason, LPVOID lpvReserved ){ 
	hinstDLL = hinst;
	return TRUE;
}
/******************************************************************
  Function: GetDllInfo
  Purpose:  This function allows the emulator to gather information
            about the dll by filling in the PluginInfo structure.
  input:    a pointer to a PLUGIN_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/ 
__declspec(dllexport) void GetDllInfo ( PLUGIN_INFO * PluginInfo ) {
	PluginInfo->Version = 0x0101;
	PluginInfo->Type = PLUGIN_TYPE_RSP;
#if ( (defined(DLIST) & defined(ALIST)) )
	sprintf(PluginInfo->Name,"High level emulation Plugin");
#endif
#if ( (defined(DLIST) & !defined(ALIST)) )
	sprintf(PluginInfo->Name,"Middle level emulation Plugin");
#endif
#if ( (!defined(DLIST) & defined(ALIST)) )
	sprintf(PluginInfo->Name,"Graphics Low level emulation Plugin");
#endif
#if ( (!defined(DLIST) & !defined(ALIST)) )
	sprintf(PluginInfo->Name,"Low level emulation Plugin");
#endif
	PluginInfo->NormalMemory = FALSE;
	PluginInfo->MemoryBswaped = TRUE;
}

/******************************************************************
  Function: GetRspDebugInfo
  Purpose:  This function allows the emulator to gather information
            about the debug capabilities of the dll by filling in
			the DebugInfo structure.
  input:    a pointer to a RSPDEBUG_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/ 
__declspec(dllexport) void GetRspDebugInfo ( RSPDEBUG_INFO * DebugInfo ) {
#if ( !(defined(DLIST) & defined(ALIST)) )
	HKEY hKeyResults = 0;

	hRSPMenu = LoadMenu(hinstDLL,"RspMenu");
	DebugInfo->hRSPMenu = hRSPMenu;
	DebugInfo->ProcessMenuItem = ProcessMenuItem;

	DebugInfo->UseBPoints = TRUE;
	sprintf(DebugInfo->BPPanelName," RSP ");
	DebugInfo->Add_BPoint = Add_BPoint;
	DebugInfo->CreateBPPanel = CreateBPPanel;
	DebugInfo->HideBPPanel = HideBPPanel;
	DebugInfo->PaintBPPanel = PaintBPPanel;
	DebugInfo->RefreshBpoints = RefreshBpoints;
	DebugInfo->RemoveAllBpoint = RemoveAllBpoint;
	DebugInfo->RemoveBpoint = RemoveBpoint;
	DebugInfo->ShowBPPanel = ShowBPPanel;
	
	DebugInfo->Enter_RSP_Commands_Window = Enter_RSP_Commands_Window;

#endif
}

#if ( !(defined(DLIST) & defined(ALIST)) )
int GetStoredWinPos( char * WinName, DWORD * X, DWORD * Y ) {
	long lResult;
	HKEY hKeyResults = 0;
	char String[200];

	sprintf(String,"Software\\N64 Emulation\\DLL\\%s\\Page Setup",DllName);
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
#endif
/******************************************************************
  Function: InitiateRSP
  Purpose:  This function is called when the DLL is started to give
            information from the emulator that the n64 RSP 
			interface needs
  input:    Rsp_Info is passed to this function which is defined
            above.
			CycleCount is the number of cycles between switching
			control between teh RSP and r4300i core.
  output:   none
*******************************************************************/ 
__declspec(dllexport) void InitiateRSP ( RSP_INFO Rsp_Info, DWORD * CycleCount) {
	RSPInfo = Rsp_Info;
#if ( !(defined(DLIST) & defined(ALIST)) )
	*CycleCount = 0;
	InitilizeRSPRegisters();
	Build_RSP();
#endif
}

/******************************************************************
  Function: InitiateRSPDebugger
  Purpose:  This function is called when the DLL is started to give
            information from the emulator that the n64 RSP 
			interface needs to intergrate the debugger with the
			rest of the emulator.
  input:    DebugInfo is passed to this function which is defined
            above.
  output:   none
*******************************************************************/ 
__declspec(dllexport) void InitiateRSPDebugger ( DEBUG_INFO Debug_Info) {
	DebugInfo = Debug_Info;
}

#if ( !(defined(DLIST) & defined(ALIST)) )
void ProcessMenuItem(int ID) {
	switch (ID) {
	case ID_RSPCOMMANDS: Enter_RSP_Commands_Window(); break;
	case ID_RSPREGISTERS: Enter_RSP_Register_Window(); break;
	case ID_DUMP_RSPCODE: DumpRSPCode(); break;
	case ID_DUMP_DMEM: DumpRSPCode(); break;
	}
}
#endif
/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/ 
__declspec(dllexport) void RomClosed (void) {
#if ( !(defined(DLIST) & defined(ALIST)) )
	InitilizeRSPRegisters();
#endif
}


