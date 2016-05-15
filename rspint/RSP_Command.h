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
char * RSPOpcodeName ( DWORD OpCode, DWORD PC );

void DumpRSPCode (void);
void Disable_RSP_Commands_Window ( void );
void Enable_RSP_Commands_Window ( void );
void Enter_RSP_Commands_Window ( void );
void RefreshRSPCommands ( void );
void SetRSPCommandToRunning ( void );
void SetRSPCommandToStepping ( void );
void SetRSPCommandViewto ( UINT NewLocation );

DWORD Stepping_Commands, WaitingForStep;
extern BOOL InRSPCommandsWindow;
