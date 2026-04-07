@echo off
setlocal
set "OFS_EXE=%~dp0ofs.exe"
if exist "%OFS_EXE%" (
	"%OFS_EXE%" uncover %*
) else (
	ofs uncover %*
)
endlocal
