@echo off
cd /D "%~dp0"
build\ddsom\ddsom.exe --rom "%~dp0rom\ddsom"
exit /b %ERRORLEVEL%
