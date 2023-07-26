@echo off
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -ExecutionPolicy Bypass -File "%~dp0\InstallClangTidyIntoEngine.ps1" -ProjectDir %1
exit %errorlevel%