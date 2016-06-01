  SetCompressor /SOLID lzma
# Variables
  !define Product "Lambda HTML Editor"
  !define Version "3.0.1"
  !define FileName "LambdaEditor.exe"

# Configuration
  Name "${Product} ${Version}"
  OutFile "LambdaEditorSetup.exe"
  InstallDir "$PROGRAMFILES\${Product}"
  InstallDirRegKey HKLM "Software\${Product}" "Install Dir"
  XPStyle On

# Modern UI Configuration
  !include "MUI.nsh"
  !define MUI_ICON "images\install.ico"
  !define MUI_UNICON "images\uninstall.ico"
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "images\installbanner.bmp"
  !define MUI_HEADERIMAGE_UNBITMAP "images\uninstallbanner.bmp"
  !define MUI_HEADERIMAGE_RIGHT
  !define MUI_WELCOMEFINISHPAGE_BITMAP "images\install.bmp"
  !define MUI_ABORTWARNING
  !define MUI_WELCOMEPAGE_TITLE_3LINES
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "license.txt"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_LANGUAGE "English"

# Begin Section
Section "Lambda HTML Editor" SecMain
SectionIn RO
  ; Add files
  SetOutPath "$INSTDIR"
  SetOverWrite On
  File "${FileName}"
  File "license.txt"
  SetOutPath "$INSTDIR\Templates"
  File "templates\HTML Document.html"
  File "templates\HTML Stylesheet.css"
  File "templates\JavaScript Unit.js"
  File "templates\PHP Document.php"
  SetOutPath "$INSTDIR"

  ; Create start-menu items
  CreateShortCut "$SMPROGRAMS\${Product}.lnk" "$INSTDIR\${FileName}" "" "$INSTDIR\${FileName}" 0
  CreateShortCut "$DESKTOP\${Product}.lnk" "$INSTDIR\${FileName}" "" "$INSTDIR\${FileName}" 0

  ; Write file association information to the registry
  ReadRegStr $0 HKCR ".html" ""
  WriteRegStr HKCR "$0\shell\edit\command" "" '$INSTDIR\${FileName} "%1"'

  ; Write IE editor information to the registry
  WriteRegStr HKLM "Software\Microsoft\Internet Explorer\View Source Editor\${Product}" "" '$INSTDIR\${FileName} "%1"'

  ; Write installation information to the registry
  WriteRegStr HKLM "Software\${Product}" "Install Dir" "$INSTDIR"
  WriteRegDWORD HKLM "Software\${Product}" "Install Language" $LANGUAGE

  ; Write uninstall information to the registry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${Product}" "DisplayIcon" "$INSTDIR\${FileName},0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${Product}" "DisplayName" "${Product}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${Product}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${Product}" "ModifyPath" "$INSTDIR"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${Product}" "NoModify" 1

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

# Uninstaller Section
Section "Uninstall"
  ;Delete Files And Directory
  Delete "$INSTDIR\*.*"
  Delete "$INSTDIR\templates\*.*"
  RmDir "$INSTDIR"

  ;Delete Shortcuts
  Delete "$SMPROGRAMS\${Product}.lnk"
  Delete "$DESKTOP\${Product}.lnk"

  ;Delete IE editor information from registry
  DeleteRegKey HKLM "Software\Microsoft\Internet Explorer\View Source Editor\${Product}"

  ;Delete Uninstaller And Uninstall Registry Entries
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${Product}"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${Product}"
SectionEnd

;--------------------------------
; Uninstaller Functions

;Function un.onInit 
;  !insertmacro MUI_UNGETLANGUAGE
;FunctionEnd
