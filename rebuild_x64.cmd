@echo off
REM ============================================================
REM  Double-click to build the 64-bit MuMain client.
REM  - First run auto-configures the x64 build tree.
REM  - If Release\Data is missing, forces a full asset re-copy.
REM  - Recompiles only what changed.
REM  Output: out\build\windows-x64\src\Release\Main.exe
REM ============================================================
if not exist "%~dp0out\build\windows-x64\CMakeCache.txt" (
  echo First run - configuring x64 build tree...
  call "%~dp0build-env-x64.cmd" cmake --preset windows-x64
)
if not exist "%~dp0out\build\windows-x64\src\Release\Data" (
  echo Release\Data missing - forcing a full asset re-copy...
  del /q "%~dp0out\build\windows-x64\src\.assets_copied_Release.stamp" 2>nul
)
call "%~dp0build-env-x64.cmd" cmake --build --preset windows-x64-release
echo.
echo ============================================================
echo  Done. Client: out\build\windows-x64\src\Release\Main.exe
echo ============================================================
pause
