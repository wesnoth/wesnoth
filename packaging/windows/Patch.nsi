; This script can generate two types of patches for Battle for Wesnoth:
; a) self-contained patch installers
;    makensis /DVersion_M="1.12" /DPatch=5 Patch.nsi
; b) a tool to download the latest patch online, which allows Wesnoth to update automatically
;    makensis /DVersion_M="1.12" /DPatch=Download Patch.nsi
;    wesnoth-update.exe /i=1.12.4 /o=1.12.5 /MD5=[MD5sum]

; NSIS plugin dependencies:
; Nsis7z	http://nsis.sourceforge.net/Nsis7z_plug-in
; InetC		http://nsis.sourceforge.net/Inetc_plug-in	(only for downloader)
; MD5		http://nsis.sourceforge.net/MD5_plugin		(only for downloader)


!if ${Patch} == Download
  OutFile "wesnoth-update.exe"
!else
  OutFile "wesnoth-${Version_M}.${Patch}-patch.exe"
!endif

  Name $(LWesnoth)
  Unicode true
  SetOverwrite on
  !cd ..\..
  RequestExecutionLevel highest

  !include "MUI2.nsh"
  !include "LogicLib.nsh"
  !include "WinVer.nsh"

  Var Version
  Var Version_pre
  Var StartMenuFolder

  !define MUI_ICON "packaging\windows\wesnoth-icon.ico"
;  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_COMPONENTS
  !insertmacro MUI_UNPAGE_INSTFILES

  ; import translations
  !define MUI_INSTFILESPAGE        ; source of $(MUI_TEXT_ABORT_TITLE)
  !include "packaging\windows\translations.nsh"


Section

  SetOutPath "$INSTDIR"
!if ${Patch} == Download
  ; get latest patch online
  inetc::get http://www.domain.com/file patch.7z
  Pop $R0
  ${If} $R0 != "OK"
    MessageBox MB_OK "$(MUI_TEXT_ABORT_TITLE): $(LDownloadErr), $R0"
    Quit
  ${EndIf}
  md5dll::GetMD5File "$OUTDIR\patch.7z"
  Pop $R0
  ${If} $R0 != $MD5
    MessageBox MB_OK "$(MUI_TEXT_ABORT_TITLE): $(LDownloadErr)."
    Quit
  ${EndIf}
!else
  ; patch shipped with installer
  File patch.7z
!endif
  ; apply the patch - modified files will be overwritten
  Nsis7z::ExtractWithDetails "patch.7z" "$(LUpdate): %s"
  Delete "$OUTDIR\patch.7z"

  FileOpen $4 "$OUTDIR\deletelist.txt" r
  ClearErrors
  ${Do}
    FileRead $4 $1		; read until the end of line, including \r\n
;   ${IfThen} ${Errors} ${|} ${Break} ${|}
    StrCpy $1 $1 -2		; /r/n
    Delete "$OUTDIR\$1"	; doesn't throw if File doesn't exist
  ${LoopUntil} ${Errors}
  FileClose $4
  Delete "$OUTDIR\deletelist.txt"

  ${If} $StartMenuFolder != ""
    Call StartMenuShortcuts
  ${EndIf}

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; old uninstaller information
  DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Battle for Wesnoth $Version_pre"
  DeleteRegKey SHCTX "Software\Battle for Wesnoth\$Version_pre"

  ; new uninstaller information
  WriteRegStr SHCTX "Software\Battle for Wesnoth\$Version" "Installer Language" $LANGUAGE
  WriteRegStr SHCTX "Software\Battle for Wesnoth\$Version" "" $INSTDIR
  WriteRegStr SHCTX "Software\Battle for Wesnoth\$Version" "Start Menu Folder" $StartMenuFolder
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Battle for Wesnoth $Version" "DisplayName" "Battle for Wesnoth $Version" 
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Battle for Wesnoth $Version" "DisplayIcon" "$\"$INSTDIR\wesnoth.exe$\""
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Battle for Wesnoth $Version" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Battle for Wesnoth $Version" "QuietUninstallString" "$\"$INSTDIR\Uninstall.exe$\" /S"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Battle for Wesnoth $Version" "URLInfoAbout" "www.wesnoth.org"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Battle for Wesnoth $Version" "DisplayVersion" "$Version"
  WriteRegDWORD SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Battle for Wesnoth $Version" "NoModify" 1
  WriteRegDWORD SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Battle for Wesnoth $Version" "NoRepair" 1

SectionEnd

Function .onInit

!if ${Patch} == Download
  ; wesnoth-update.exe gets version numbers from wesnoth.exe
  Var /GLOBAL MD5
  ${GetParameters} $0
  ${GetOptions} $0 "/MD5=" $MD5
  ${GetOptions} $0 "/i=" $Version_pre
  ${GetOptions} $0 "/o=" $Version
  ${If} ${Errors}
    MessageBox MB_OK "$(MUI_TEXT_ABORT_TITLE): $(LWesnoth) ${Version_M} $(LVersionErr)"
    Quit
  ${EndIf}
!else
  ; new version number is hard-coded, get previous version number from registry
  StrCpy $Version "${Version_M}.${Patch}"
  ; Search HKCU & HKLM for compatible Wesnoth versions:
  Call ScanRegistry
  ${If} $Version_pre == ""
    Call CheckAdmin
    Call ScanRegistry
  ${EndIf}
  ${If} $Version_pre == ""
    MessageBox MB_OK "$(MUI_TEXT_ABORT_TITLE): $(LWesnoth) ${Version_M} $(LVersionErr)"
    Quit
  ${EndIf}
  StrCpy $Version_pre "${Version_M}.$Version_pre"
!endif

  ; initialize $LANGUAGE, $INSTDIR, $StartMenuFolder from registry
  ReadRegStr $LANGUAGE        SHCTX "Software\Battle for Wesnoth\$Version_pre" "Installer Language"
  ReadRegStr $INSTDIR         SHCTX "Software\Battle for Wesnoth\$Version_pre" ""
  ReadRegStr $StartMenuFolder SHCTX "Software\Battle for Wesnoth\$Version_pre" "Start Menu Folder"
  ; $INSTDIR: replace $Version_pre with $Version
  StrLen $1 "$Version_pre"
  StrCpy $0 $INSTDIR "" -$1
  ${If} $0 == "$Version_pre"
;   ${StrRep} "$0" "$INSTDIR" "$Version_pre" "$Version"
    StrCpy $0 $INSTDIR -$1
    StrCpy $0 "$0$Version"
    Rename $INSTDIR $0
    StrCpy $INSTDIR $0
  ${EndIf}

;   TODO: check if Wesnoth is running
;   Problem: needs another plugin to work on XP Home... worth the effort?
  ; MessageBox MB_OK "$(MUI_TEXT_ABORT_TITLE): $(LWesnoth) is currently running."
FunctionEnd

Function ScanRegistry
  ; iterate over version subkeys to determine current minor version number
  StrLen $3 "${Version_M}."		; hardcode $3 = 5 ?
  ${For} $0 0 10
    EnumRegKey $1 SHCTX "Software\Battle for Wesnoth" $0
    ${IfThen} $1 == "" ${|} ${Break} ${|}
    StrCpy $2 $1 $3
    ${If} $2 == "${Version_M}."
      StrCpy $2 $1 "" $3
      ${If} $2 < ${Patch}
      ${AndIf} $2 > $Version_pre
        StrCpy $Version_pre $2
      ${EndIf}
    ${EndIf}
  ${Next}
FunctionEnd

Function CheckAdmin
  ; set SHCTX from HKCU to HKLM, requires admin rights
  System::Call 'shell32::IsUserAnAdmin(v)i .r10'
  ${If} $R0 = 0
    MessageBox MB_OK "$(MUI_TEXT_ABORT_TITLE): $(LAdminErr)"
    SetErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
    Quit
  ${EndIf}
  SetShellVarContext all
FunctionEnd

Function StartMenuShortcuts
  DetailPrint $StartMenuFolder
  DetailPrint "Battle for Wesnoth $Version_pre"
  ${If} $StartMenuFolder == "Battle for Wesnoth $Version_pre"
    RMDir /r "$SMPROGRAMS\$StartMenuFolder"
    StrCpy $StartMenuFolder "Battle for Wesnoth $Version"
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  ${Else}
    Delete "$SMPROGRAMS\$StartMenuFolder\License.lnk"
    Delete "$SMPROGRAMS\$StartMenuFolder\Player's changelog.lnk"
    Delete "$SMPROGRAMS\$StartMenuFolder\Changelog.lnk"
    Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
    Delete "$SMPROGRAMS\$StartMenuFolder\Multiplayer server.lnk"
    Delete "$SMPROGRAMS\$StartMenuFolder\Map editor.lnk"
  ${EndIf}

  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Battle for Wesnoth.lnk" "$INSTDIR\wesnoth.exe"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Battle for Wesnoth (with console).lnk" "$INSTDIR\cwesnoth.cmd" "" "$INSTDIR\wesnoth.exe"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Manual.lnk" "$INSTDIR\manual\manual.$(LCode).html"


  ${If} ${IsNT}
  ${AndIf} ${AtMostWin7}
  ; Win7 or lower has collapsed start menu folders, activate additional shortcuts.
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Map editor.lnk" "$INSTDIR\wesnoth.exe" "-e" "$INSTDIR\wesnoth_editor-icon.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Changelog.lnk" "$INSTDIR\changelog.txt"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Player's changelog.lnk" "$INSTDIR\players_changelog.txt"
  ${EndIf}

  StrCpy $0 "$SMPROGRAMS\$StartMenuFolder\desktop.ini"
  ; Create desktop.ini file (allows to display localized start menu entries)
  ${IfNot} ${FileExists} $0
    FileOpen $9 $0 w
      FileWriteUTF16LE $9 ${U+FEFF} ; UTF16 BOM to enforce encoding
    FileClose $9
  ${EndIf}

  ; Add translations to desktop.ini
  ${If} $StartMenuFolder == "Battle for Wesnoth $Version"
    WriteIniStr $0 ".ShellClassInfo" "LocalizedResourceName" "$(LWesnoth) $Version"
  ${EndIf}
  WriteIniStr $0 "LocalizedFileNames" "Battle for Wesnoth.lnk" "$(LWesnoth)"
  WriteIniStr $0 "LocalizedFileNames" "Battle for Wesnoth (with console).lnk" "$(LWesnoth) (with console)"
  WriteIniStr $0 "LocalizedFileNames" "Manual.lnk" "$(LManual)"
  WriteIniStr $0 "LocalizedFileNames" "Map editor.lnk" "$(LEditor)"

  SetFileAttributes "$SMPROGRAMS\$StartMenuFolder" SYSTEM
  SetFileAttributes $0 HIDDEN|SYSTEM

FunctionEnd


;Uninstaller Section
; old uninstaller won't work because registry key includes hard-coded version number
; reuse %NSIS_UNINSTALL_FILES from the normal installer?







;--------------------------------
;Uninstaller Section

Section "un.$(LWesnoth)" un.BfW
  SectionIn RO  # read only (mandatory)

  Delete $INSTDIR\wesnoth.exe
  Delete $INSTDIR\wesnothd.exe
  Delete $INSTDIR\cwesnoth.cmd
  Delete $INSTDIR\*.dll
  Delete "$INSTDIR\wesnoth_editor-icon.ico"
  Delete $INSTDIR\std*.txt
  Delete $INSTDIR\README.txt
  Delete $INSTDIR\copyright.txt
  Delete $INSTDIR\COPYING.txt
  Delete $INSTDIR\changelog.txt
  Delete $INSTDIR\players_changelog.txt
  RMDir /r "$INSTDIR\data"
  RMDir /r "$INSTDIR\fonts"
  RMDir /r "$INSTDIR\images"
  RMDir /r "$INSTDIR\sounds"
  RMDir /r "$INSTDIR\translations"
  RMDir /r "$INSTDIR\manual"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

;  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

; If we created desktop.ini it'll start with a blank line
  FileOpen $9 "$SMPROGRAMS\$StartMenuFolder\desktop.ini" r
    FileReadUTF16LE $9 $1
  FileClose $9
  DetailPrint $1
  ${If} $1 == "$\r$\n"
    Delete "$SMPROGRAMS\$StartMenuFolder\desktop.ini"
  ${EndIf}

  Delete "$SMPROGRAMS\$StartMenuFolder\License.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Player's changelog.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Changelog.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Manual.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Multiplayer server.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Map editor.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Battle for Wesnoth (with console).lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Battle for Wesnoth.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  StrCpy $Version "${Version_M}.${Patch}"
  DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Battle for Wesnoth $Version"
  DeleteRegKey /ifempty SHCTX "Software\Battle for Wesnoth\$Version"
  DeleteRegKey /ifempty SHCTX "Software\Battle for Wesnoth"

SectionEnd

Section /o "un.$(LUserdata)" un.Userdata # optional
  RMDir /r "$DOCUMENTS\My Games\Wesnoth${Version_M}"
SectionEnd
