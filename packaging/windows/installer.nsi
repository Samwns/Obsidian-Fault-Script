!include "MUI2.nsh"

Name "Obsidian Fault Script"
!ifndef OUTPUT
	!define OUTPUT "ofs-windows-x64-installer.exe"
!endif
OutFile "${OUTPUT}"
InstallDir "$PROGRAMFILES64\Obsidian Fault Script"
InstallDirRegKey HKLM "Software\ObsidianFaultScript" "InstallDir"
RequestExecutionLevel admin

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "Install"
	SetOutPath "$INSTDIR"
	File "dist\ofs.exe"
	File "dist\libstdc++-6.dll"
	File "dist\libgcc_s_seh-1.dll"
	File "dist\libwinpthread-1.dll"

	WriteRegStr HKLM "Software\ObsidianFaultScript" "InstallDir" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ObsidianFaultScript" "DisplayName" "Obsidian Fault Script"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ObsidianFaultScript" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteUninstaller "$INSTDIR\uninstall.exe"

	# Add install dir to system PATH (append only).
	ReadRegStr $0 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
	StrCpy $1 "$0;$INSTDIR"
	WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path" "$1"
	System::Call 'user32::SendMessageTimeout(i 0xffff, i ${WM_SETTINGCHANGE}, i 0, t "Environment", i 0x2, i 5000, *i .r2)'
SectionEnd

Section "Uninstall"
	Delete "$INSTDIR\ofs.exe"
	Delete "$INSTDIR\libstdc++-6.dll"
	Delete "$INSTDIR\libgcc_s_seh-1.dll"
	Delete "$INSTDIR\libwinpthread-1.dll"
	Delete "$INSTDIR\uninstall.exe"
	RMDir "$INSTDIR"

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ObsidianFaultScript"
	DeleteRegKey HKLM "Software\ObsidianFaultScript"
SectionEnd
