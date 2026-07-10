@echo off
REM ============================================================
REM  Double-click to rebuild the MuMain client after changing
REM  assets (src\bin\Data\...) or source code.
REM  - Recompiles only what changed; re-copies changed assets.
REM  - If the Release\Data folder is missing (e.g. you deleted the
REM    Release folder), it clears the asset "stamp" so the full
REM    Data set is copied again.
REM  Output: out\build\windows-x86\src\Release\Main.exe
REM ============================================================
if not exist "%~dp0out\build\windows-x86\src\Release\Data" (
  echo Release\Data missing - forcing a full asset re-copy...
  del /q "%~dp0out\build\windows-x86\src\.assets_copied_Release.stamp" 2>nul
)
call "%~dp0build-env.cmd" cmake --build --preset windows-x86-release
echo.
echo ============================================================
echo  Done. Client: out\build\windows-x86\src\Release\Main.exe
echo ============================================================
pause
