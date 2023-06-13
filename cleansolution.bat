echo off

del *.sln
rmdir /s /q .vs
rmdir /s /q Binaries
rmdir /s /q Intermediate
rmdir /s /q Saved
rmdir /s /q DerivedDataCache


set MyUVS="C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe"
set MyUBT="F:\ue\UE_5.1\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"

rem change to your own project name
set MyFullPath="%cd%\MurderInSpace"


%MyUVS% /projectfiles %MyFullPath%.uproject

%MyUBT% Development Win64 -Project=%MyFullPath%.uproject -TargetType=Editor -Progress -NoEngineChanges -NoHotReloadFromIDE

%MyFullPath%.uproject

%MyFullPath%.sln