@echo off
setlocal
set "PATH=%~dp0;%PATH%"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0ofs.ps1" %*
exit /b %ERRORLEVEL%
