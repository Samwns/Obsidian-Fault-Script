@echo off
setlocal
set "OFS_CMD=%~dp0ofs.cmd"
set "PATH=%~dp0;%PATH%"
if exist "C:\Program Files\LLVM\bin" set "PATH=C:\Program Files\LLVM\bin;%PATH%"
if exist "C:\Program Files\Git\mingw64\bin" set "PATH=C:\Program Files\Git\mingw64\bin;%PATH%"
if exist "C:\msys64\ucrt64\bin" set "PATH=C:\msys64\ucrt64\bin;%PATH%"
if exist "%OFS_CMD%" (
	"%OFS_CMD%" uncover %*
) else (
	ofs uncover %*
)
endlocal
