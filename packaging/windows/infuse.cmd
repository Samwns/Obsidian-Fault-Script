@echo off
setlocal
set "OFS_EXE=%~dp0ofs.exe"
if exist "%OFS_EXE%" (
	"%OFS_EXE%" infuse %*
) else (
	ofs infuse %*
)
endlocal
