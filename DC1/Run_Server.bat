@echo off
set ENGINE=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe
set PROJECT=C:\Hilde\DC1\DC1\DC1.uproject
set MAP=/Game/Maps/DevMap
set PORT=7777
echo Starting Dedicated Server on %MAP% port %PORT% ...
"%ENGINE%" "%PROJECT%" %MAP% -server -log -port=%PORT%
echo.
echo [Server exited. code=%ERRORLEVEL%]
pause
