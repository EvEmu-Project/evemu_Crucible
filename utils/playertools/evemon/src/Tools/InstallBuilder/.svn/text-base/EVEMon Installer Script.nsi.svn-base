#
#  This is an NSIS Installer build script
#  for NSIS 2.46 with the UAC plugin installed.
#  Script is compatible with UAC v0.2.x.
#  (UAC.dll must be copied to NSIS\Plugins)
#

SetCompressor /solid lzma

RequestExecutionLevel admin

!include "UAC.nsh"
!include "Library.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"
!include "MUI.nsh"

Name "EVEMon"
OutFile "${OUTDIR}\EVEMon-install-${VERSION}.exe"
InstallDir "$PROGRAMFILES\EVEMon\"
InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon" "UninstallString"

VIAddVersionKey "ProductName" "EVEMon Installer"
VIAddVersionKey "CompanyName" "battleclinic.com"
VIAddVersionKey "LegalCopyright" "Copyright 2006, Timothy Fries and EVEMon Development Team"
VIAddVersionKey "FileDescription" "Installs EVEMon on your computer"
VIAddVersionKey "FileVersion" "${VERSION}"
VIProductVersion ${VERSION}

Var STARTMENU_FOLDER
Var MUI_TEMP

!define MUI_ABORTWARNING
!define MUI_ICON "..\..\..\..\..\EVEMon\EVEMon.ico"
!define MUI_UNICON "..\..\..\..\..\EVEMon\EVEMon.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\..\..\..\EVEMon.Common\Resources\Files\gpl.txt"
!insertmacro MUI_PAGE_DIRECTORY

# Start menu folder page configuration
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\EVEMon"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU "EVEMon" $STARTMENU_FOLDER
#-------------------------------------

!insertmacro MUI_PAGE_INSTFILES
# Finish page configuration
!define MUI_FINISHPAGE_RUN $INSTDIR\EVEMon.exe
!define MUI_FINISHPAGE_RUN_TEXT "Run EVEMon Now"
!insertmacro MUI_PAGE_FINISH
#-------------------------------------

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

!include "NETFrameworkCheck.nsh"

Function .onInit
	!insertmacro UAC_RunElevated 

	StrCmp 1223 $0 UAC_ElevationAborted ; UAC dialog aborted by user?
	StrCmp 0 $0 0 UAC_Err ; Error?
	StrCmp 1 $1 0 UAC_Success ; Are we the real deal or just the wrapper?
	Quit
	
	UAC_Err:
	MessageBox mb_iconstop "Unable to install EVEMon without Administrator permissions. (error $0)"
	Abort
	
	UAC_ElevationAborted:
	# elevation was aborted, we still run as normal
	
	UAC_Success:
	Call EnsureNotRunning
	
	# fix it so it only computes the space needed for EVEMon itself if .net is not installed
	SectionSetSize 0 0
	Call GetDotNETVersion
	Pop $0
	StrCmp $0 "" 0 .NetIsInstalled
		SectionSetSize 0 49268 ; The size of .NET v4 in KiB

	.NetIsInstalled:
	StrCmp "$INSTDIR" "$PROGRAMFILES\EVEMon\" checkForExeInDir
	StrCmp "$INSTDIR" "$PROGRAMFILES\EVEMon" checkForExeInDir
	Goto done

	checkForExeInDir:
	IfFileExists "$INSTDIR\EVEMon.exe" 0 done

    MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
    "Setup detected that another version of EVEMon is already installed. $\n$\n\
	Click `OK` to remove the existing version or `Cancel` to cancel this installation." \
    IDOK uninstall
	Abort

	;Remove previous installation
	uninstall:	
	Delete "$INSTDIR\*.*"
	RMDir /R "$INSTDIR\Microsoft.VC90.CRT"
	RMDir /R "$INSTDIR\Resources"
	RMDir $INSTDIR
	
	!insertmacro MUI_STARTMENU_GETFOLDER EVEMon $MUI_TEMP
	Delete "$SMPROGRAMS\$MUI_TEMP\EVEMon.lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall EVEMon.lnk"

	StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"

	startMenuDeleteLoop:
    ClearErrors
	RMDir $MUI_TEMP
	GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
	IfErrors startMenuDeleteLoopDone
	StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
		 
	startMenuDeleteLoopDone:
	DeleteRegKey /ifempty HKLM "Software\EVEMon"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon"
	
	done:
FunctionEnd

Function EnsureNotRunning
	IfFileExists "$INSTDIR\EVEMon.exe" 0 lbl_Done
	IntOp $1 0 + 0
	
	lbl_tryAgain:
	ClearErrors
	FileOpen $0 "$INSTDIR\EVEMon.exe" a
	IfErrors lbl_failedOpen
	FileClose $0
	goto lbl_Done

	lbl_failedOpen:
	IfSilent lbl_waitForIt
	MessageBox MB_RETRYCANCEL|MB_DEFBUTTON1|MB_ICONEXCLAMATION \
			"Please close EVEMon before continuing." /SD IDCANCEL IDRETRY lbl_tryAgain IDCANCEL lbl_abort
	goto lbl_tryAgain

	lbl_waitForIt:
	IntOp $1 $1 + 1
	IntCmp $1 10 0 0 lbl_failedToClose
	Sleep 500
	goto lbl_tryAgain

	lbl_failedToClose:
	Abort "EVEMon failed to close."

	lbl_abort:
	Abort "Operation cancelled by user."
 
	lbl_Done:
FunctionEnd

Function un.onInit
	!insertmacro UAC_RunElevated 

	StrCmp 1223 $0 UAC_ElevationAborted ; UAC dialog aborted by user?
	StrCmp 0 $0 0 UAC_Err ; Error?
	StrCmp 1 $1 0 UAC_Success ; Are we the real deal or just the wrapper?
	Quit
	
	UAC_Err:
	MessageBox mb_iconstop "Unable to uninstall EVEMon without Administrator permissions. (error $0)"
	Abort
	
	UAC_ElevationAborted:
	# elevation was aborted, we still run as normal
	UAC_Success:

	Call un.EnsureNotRunning
FunctionEnd

Function un.EnsureNotRunning
	IntOp $1 0 + 0
	lbl_tryAgain:
	ClearErrors
	FileOpen $0 "$INSTDIR\EVEMon.exe" a
	IfErrors lbl_failedOpen
	FileClose $0
	goto lbl_Done

	lbl_failedOpen:
	IfSilent lbl_waitForIt
	MessageBox MB_RETRYCANCEL|MB_DEFBUTTON1|MB_ICONEXCLAMATION \
			"Please close EVEMon before continuing." /SD IDCANCEL IDRETRY lbl_tryAgain IDCANCEL lbl_abort
	goto lbl_tryAgain

	lbl_waitForIt:
	IntOp $1 $1 + 1
	IntCmp $1 10 0 0 lbl_failedToClose
	Sleep 500
	goto lbl_tryAgain

	lbl_failedToClose:
	Abort "EVEMon failed to close."

	lbl_abort:
	Abort "Operation cancelled by user."
 
	lbl_Done:
FunctionEnd

 ; StrStr
 ; input, top of stack = string to search for
 ;				top of stack-1 = string to search in
 ; output, top of stack (replaces with the portion of the string remaining)
 ; modifies no other variables.
 ;
 ; Usage:
 ;	 Push "this is a long ass string"
 ;	 Push "ass"
 ;	 Call StrStr
 ;	 Pop $R0
 ;	($R0 at this point is "ass string")

 Function StrStr
	 Exch $R1 ; st=haystack,old$R1, $R1=needle
	 Exch		; st=old$R1,haystack
	 Exch $R2 ; st=old$R1,old$R2, $R2=haystack
	 Push $R3
	 Push $R4
	 Push $R5
	 StrLen $R3 $R1
	 StrCpy $R4 0
	 ; $R1=needle
	 ; $R2=haystack
	 ; $R3=len(needle)
	 ; $R4=cnt
	 ; $R5=tmp
	 loop:
		 StrCpy $R5 $R2 $R3 $R4
		 StrCmp $R5 $R1 done
		 StrCmp $R5 "" done
		 IntOp $R4 $R4 + 1
		 Goto loop
 done:
	 StrCpy $R1 $R2 "" $R4
	 Pop $R5
	 Pop $R4
	 Pop $R3
	 Pop $R2
	 Exch $R1
 FunctionEnd

Function .onInstSuccess

	; skip if not in silent mode
	IfSilent 0 lbl_skipRun

	; search for /AUTORUN on commandline and skip if not found
	Push $CMDLINE
	Push "/AUTORUN"
	Call StrStr
	Pop $0
	StrCmp $0 "" lbl_skipRun

	; Autorun if in silent mode and /AUTORUN is specified
	Exec "$INSTDIR\EVEMon.exe"

	lbl_skipRun:
FunctionEnd

Section "Install EVEMon" 
	ClearErrors
	ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{B3C090CF-5539-42EA-90EB-8648A79C7F8B}" \
				"UninstallString"
	IfErrors lbl_NoLegacyUninstall
	ExecWait "MsiExec.exe /quiet /x {B3C090CF-5539-42EA-90EB-8648A79C7F8B}"

	lbl_noLegacyUninstall:
	SetOutPath "$INSTDIR"
	File /r /x *vshost* /x *.config "..\..\..\..\..\EVEMon\bin\x86\Release\*.*" 

	WriteUninstaller "$INSTDIR\uninstall.exe"

	!insertmacro MUI_STARTMENU_WRITE_BEGIN EVEMon
	 SetShellVarContext current
		 CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
		 CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\EVEMon.lnk" "$INSTDIR\EVEMon.exe"
		 CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall EVEMon.lnk" "$INSTDIR\uninstall.exe"
	!insertmacro MUI_STARTMENU_WRITE_END

	# Add entry for Add/Remove Programs
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon" \
				 "DisplayName" "EVEMon"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon" \
				 "UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon" \
				 "DisplayIcon" "$INSTDIR\EVEMon.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon" \
				 "Publisher" "battleclinic.com"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon" \
				 "URLUpdateInfo" "http://evemon.battleclinic.com/"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon" \
				 "URLInfoAbout" "http://evemon.battleclinic.com/"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon" \
				 "DisplayVersion" "${VERSION}"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon" \
				 "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon" \
				 "NoRepair" 1
SectionEnd

Section "un.Uninstall EVEMon"
	SetShellVarContext current
	Delete "$INSTDIR\EVEMon.*"
	Delete "$INSTDIR\uninstall.exe"
	Delete "$INSTDIR\*.dll"
	Delete "$SMPROGRAMS\$STARTMENU_FOLDER\EVEMon.lnk"
	Delete "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall EVEMon.lnk"
	RMDir "$SMPROGRAMS\$STARTMENU_FOLDER"
	RMDir /R "$INSTDIR\Resources"

	IfRebootFlag rmDirWithReboot rmDirWithoutReboot

	rmDirWithReboot:
	RMDir /REBOOTOK $INSTDIR
	Goto afterRmDir
	
	rmDirWithoutReboot:
	RMDir $INSTDIR
	Goto afterRmDir

	afterRmDir:
	!insertmacro MUI_STARTMENU_GETFOLDER EVEMon $MUI_TEMP
	Delete "$SMPROGRAMS\$MUI_TEMP\EVEMon.lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall EVEMon.lnk"

	StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"

	startMenuDeleteLoop:
	ClearErrors
	RMDir $MUI_TEMP
	GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
	IfErrors startMenuDeleteLoopDone
	StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop

    startMenuDeleteLoopDone:
	DeleteRegKey /ifempty HKLM "Software\EVEMon"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EVEMon"
SectionEnd
