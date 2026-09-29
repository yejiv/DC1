@echo off
set ENGINE=E:\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe
set PROJECT=F:\UE_Projects\DC1\DC1
set MAP=F:\UE_Projects\DC1\DC1\Content\Maps\DevMap
set PORT=7777
echo Starting Dedicated Server on %MAP% port %PORT% ...
"%ENGINE%" "%PROJECT%" %MAP% -server -log -port=%PORT%
echo.
echo [Server exited. code=%ERRORLEVEL%]
pause
