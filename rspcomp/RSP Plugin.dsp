# Microsoft Developer Studio Project File - Name="RSP Plugin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=RSP Plugin - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RSP Plugin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RSP Plugin.mak" CFG="RSP Plugin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RSP Plugin - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "RSP Plugin - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "RSP Plugin - Win32 Release External" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "Desktop"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RSP Plugin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RSP_Plugin___Win32_Release"
# PROP BASE Intermediate_Dir "RSP_Plugin___Win32_Release"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MD /W3 /GX /O2 /Ob2 /D "DLIST" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RSPPLUGIN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G5 /MD /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RSPPLUGIN_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"Plugin\MLE.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"Release\RSP.dll"

!ELSEIF  "$(CFG)" == "RSP Plugin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "RSP_Plugin___Win32_Debug"
# PROP BASE Intermediate_Dir "RSP_Plugin___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "DLIST" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RSPPLUGIN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RSPPLUGIN_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"Plugin\MLE.dll" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"Debug\RSP.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "RSP Plugin - Win32 Release External"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RSP_Plugin___Win32_Release_External"
# PROP BASE Intermediate_Dir "RSP_Plugin___Win32_Release_External"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_External"
# PROP Intermediate_Dir "Release_External"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MD /W3 /GX /O2 /Ob2 /D "DLIST" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RSPPLUGIN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G5 /MD /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RSPPLUGIN_EXPORTS" /D "EXTERNAL_RELEASE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"Plugin\MLE.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"Release_External\RSP.dll"

!ENDIF 

# Begin Target

# Name "RSP Plugin - Win32 Release"
# Name "RSP Plugin - Win32 Debug"
# Name "RSP Plugin - Win32 Release External"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "CPU Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Cpu.c
# End Source File
# Begin Source File

SOURCE=.\dma.c
# End Source File
# Begin Source File

SOURCE=".\Interpreter CPU.c"
# End Source File
# Begin Source File

SOURCE=".\Interpreter Ops.c"
# End Source File
# Begin Source File

SOURCE=.\memory.c

!IF  "$(CFG)" == "RSP Plugin - Win32 Release"

# ADD BASE CPP /FAs
# ADD CPP /FAs

!ELSEIF  "$(CFG)" == "RSP Plugin - Win32 Debug"

!ELSEIF  "$(CFG)" == "RSP Plugin - Win32 Release External"

# ADD BASE CPP /FAs
# ADD CPP /FAs

!ENDIF 

# End Source File
# Begin Source File

SOURCE=".\Recompiler Analysis.c"
# End Source File
# Begin Source File

SOURCE=".\Recompiler CPU.c"
# End Source File
# Begin Source File

SOURCE=".\Recompiler Ops.c"
# End Source File
# Begin Source File

SOURCE=".\Recompiler Sections.c"
# End Source File
# End Group
# Begin Group "X86 Opcodes Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Mmx.c
# End Source File
# Begin Source File

SOURCE=.\Sse.c
# End Source File
# Begin Source File

SOURCE=.\X86.c
# End Source File
# End Group
# Begin Group "Debugger Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\breakpoint.c
# End Source File
# Begin Source File

SOURCE=.\log.c
# End Source File
# Begin Source File

SOURCE=".\RSP Command.c"
# End Source File
# Begin Source File

SOURCE=".\RSP Register.c"
# End Source File
# End Group
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\Profiling.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Debugger Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\breakpoint.h
# End Source File
# Begin Source File

SOURCE=.\log.h
# End Source File
# Begin Source File

SOURCE=".\RSP Command.h"
# End Source File
# Begin Source File

SOURCE=".\RSP Registers.h"
# End Source File
# End Group
# Begin Group "CPU Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Cpu.h
# End Source File
# Begin Source File

SOURCE=.\dma.h
# End Source File
# Begin Source File

SOURCE=".\Interpreter CPU.h"
# End Source File
# Begin Source File

SOURCE=".\Interpreter Ops.h"
# End Source File
# Begin Source File

SOURCE=.\memory.h
# End Source File
# Begin Source File

SOURCE=.\OpCode.h
# End Source File
# Begin Source File

SOURCE=".\Recompiler Cpu.h"
# End Source File
# Begin Source File

SOURCE=".\Recompiler Ops.h"
# End Source File
# End Group
# Begin Group "X86 Opcode Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\X86.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Profiling.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\rsp.h
# End Source File
# Begin Source File

SOURCE=.\Types.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\RSP.rc
# End Source File
# End Group
# End Target
# End Project
