!include "MUI2.nsh"

Name "Obsidian Fault Script"
!ifndef OUTPUT
	!define OUTPUT "ofs-windows-x64-installer.exe"
!endif
OutFile "${OUTPUT}"
InstallDir "$PROGRAMFILES64\Obsidian Fault Script"
InstallDirRegKey HKLM "Software\ObsidianFaultScript" "InstallDir"
RequestExecutionLevel admin

!define MUI_ICON "ofs.ico"
!define MUI_UNICON "ofs.ico"
Icon "ofs.ico"
UninstallIcon "ofs.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "Install"
	SetOutPath "$INSTDIR"
	DetailPrint "Installing OFS compiler executable"
	File "portable-staging\ofs.exe"
	DetailPrint "Installing command wrappers"
	File /nonfatal "portable-staging\infuse.cmd"
	File /nonfatal "portable-staging\uncover.cmd"
	File /nonfatal "portable-staging\reinfuse.cmd"
	File /nonfatal "portable-staging\ofs.ps1"
	File /nonfatal "portable-staging\infuse.ps1"
	File /nonfatal "portable-staging\uncover.ps1"
	File /nonfatal "portable-staging\reinfuse.ps1"
	DetailPrint "Installing runtime dependencies"
	File /nonfatal "portable-staging\*.dll"
	File /nonfatal "portable-staging\libofs_runtime.a"
	File /nonfatal "portable-staging\ofs_runtime.lib"
	DetailPrint "Installing application icon"
	File /oname=ofs.ico "ofs.ico"
	DetailPrint "Installing license terms"
	File /oname=LICENSE.txt "portable-staging\LICENSE.txt"
	DetailPrint "Installing standard library files"
	SetOutPath "$INSTDIR\stdlib"
	File /nonfatal "portable-staging\stdlib\*.ofs"
	SetOutPath "$INSTDIR"

	WriteRegStr HKLM "Software\ObsidianFaultScript" "InstallDir" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ObsidianFaultScript" "DisplayName" "Obsidian Fault Script"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ObsidianFaultScript" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ObsidianFaultScript" "DisplayIcon" "$INSTDIR\ofs.ico"
	WriteUninstaller "$INSTDIR\uninstall.exe"

	ReadRegStr $0 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
	StrCpy $1 "$0;$INSTDIR"
	WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path" "$1"
	System::Call 'user32::SendMessageTimeout(i 0xffff, i ${WM_SETTINGCHANGE}, i 0, t "Environment", i 0x2, i 5000, *i .r2)'
SectionEnd

Section "Uninstall"
	Delete "$INSTDIR\ofs.exe"
	Delete "$INSTDIR\infuse.cmd"
	Delete "$INSTDIR\uncover.cmd"
	Delete "$INSTDIR\reinfuse.cmd"
	Delete "$INSTDIR\ofs.ps1"
	Delete "$INSTDIR\infuse.ps1"
	Delete "$INSTDIR\uncover.ps1"
	Delete "$INSTDIR\reinfuse.ps1"
	Delete "$INSTDIR\*.dll"
	Delete "$INSTDIR\libofs_runtime.a"
	Delete "$INSTDIR\ofs_runtime.lib"
	Delete "$INSTDIR\ofs.ico"
	Delete "$INSTDIR\LICENSE.txt"
	Delete "$INSTDIR\uninstall.exe"
	RMDir /r "$INSTDIR\stdlib"
	RMDir "$INSTDIR"

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ObsidianFaultScript"
	DeleteRegKey HKLM "Software\ObsidianFaultScript"
SectionEnd