@echo off
REM ============================================================
REM  DC1 Dedicated Server + Client 접속 흐름 테스트
REM  - 서버: DevMap 을 -server 로 (언쿡 데디, Server 타겟 불필요)
REM  - 클라: Logo 에서 시작 (-game 스탠드얼론)
REM  경로/맵/포트만 바꾸면 재사용 가능
REM ============================================================

set ENGINE="E:\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
set PROJECT="F:\UE_Projects\DC1\DC1\DC1.uproject"
set SERVER_MAP=DevMap
set CLIENT_MAP=Logo
set PORT=7777

echo [1/2] Dedicated Server 실행 (%SERVER_MAP%, port %PORT%) ...
start "DC1 Server" %ENGINE% %PROJECT% %SERVER_MAP% -server -log -port=%PORT%

REM 서버가 뜰 시간 잠깐 대기
timeout /t 3 /nobreak >nul

echo [2/2] Client 실행 (%CLIENT_MAP%) ...
start "DC1 Client" %ENGINE% %PROJECT% %CLIENT_MAP% -game -log -windowed -resx=1280 -resy=720

echo.
echo 두 창이 떴으면:
echo   - Server 창: 접속 로그 확인
echo   - Client 창: 콘솔(~) 열고  open 127.0.0.1:%PORT%  로 접속 테스트
echo     (또는 IP창/ClientTravel 로직 붙인 뒤 버튼으로)
exit
