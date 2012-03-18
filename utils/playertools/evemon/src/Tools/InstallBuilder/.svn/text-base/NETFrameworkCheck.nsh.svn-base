Function GetDotNETVersion
  ReadRegStr $0 HKLM "SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full" "TargetVersion"
  push $0
FunctionEnd

Var DOTNET_RETURN_CODE

Section "Microsoft .NET Framework v4.0"
  SectionIn RO
  
  ; search for /SKIPDOTNET on commandline and skip if found
  Push $CMDLINE
  Push "/SKIPDOTNET"
  Call StrStr
  Pop $0
  StrCmp $0 "" lbl_notSkipped
  goto lbl_Done
  
  lbl_notSkipped:
  Call GetDotNETVersion
  Pop $0
  strCmp $0 "4.0.0" lbl_Done
  strCmp $0 "" lbl_StartInstallNotFound
  goto lbl_Done

  lbl_StartInstallNotFound:
  MessageBox MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 "Microsoft .NET Framework 4 is required, and does not appear to be installed.$\nYou must \
                    install it before continuing.$\nIf you choose to continue, you will need to be connected \
                    to the internet before proceeding.$\nWould you like to continue with the installation?" /SD IDNO IDYES lbl_Confirmed IDNO lbl_Cancelled

  lbl_Cancelled:                  
  Abort "Microsoft .NET Framework 4 is required."

  lbl_Confirmed:
  nsisdl::download \
         /TIMEOUT=120000 "http://download.microsoft.com/download/9/5/A/95A9616B-7A37-4AF6-BC36-D6EA96C8DAAE/dotNetFx40_Full_x86_x64.exe" "$PLUGINSDIR\dotnetfx.exe"
  Pop $0
  StrCmp "$0" "success" lbl_continue
  Abort ".NET Framework 4 download failed: $0"

  lbl_continue:
  Banner::show /NOUNLOAD "Installing .NET 4: Please Wait"
  nsExec::ExecToStack '"$PLUGINSDIR\dotnetfx.exe" /q /c:"install.exe /q"'
  pop $DOTNET_RETURN_CODE
  Banner::destroy
  SetRebootFlag true

  StrCmp "$DOTNET_RETURN_CODE" "" lbl_NoError
  StrCmp "$DOTNET_RETURN_CODE" "0" lbl_NoError
  StrCmp "$DOTNET_RETURN_CODE" "3010" lbl_NoError
  StrCmp "$DOTNET_RETURN_CODE" "8192" lbl_NoError
  StrCmp "$DOTNET_RETURN_CODE" "error" lbl_Error
  StrCmp "$DOTNET_RETURN_CODE" "timeout" lbl_TimeOut
  Goto lbl_Error

  lbl_TimeOut:
  Abort "The .NET Framework download timed out."

  lbl_Error:
  Abort "The .NET Framework install failed (error code $DOTNET_RETURN_CODE)."
  
  lbl_NoError:
  lbl_Done:
SectionEnd


