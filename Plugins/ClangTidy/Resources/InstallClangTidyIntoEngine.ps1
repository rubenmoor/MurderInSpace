#
# This script patches UnrealBuildTool so that it will run clang-tidy on build.
#
param([Parameter(Mandatory=$false)][string] $ProjectDir)

$ErrorActionPreference = "Stop"

# Unreal Engine 5 messes up the PATH environment variable. Ensure we fix up PATH from the
# current system's settings.
$UserPath = ([System.Environment]::GetEnvironmentVariable("PATH", [System.EnvironmentVariableTarget]::User))
$ComputerPath = ([System.Environment]::GetEnvironmentVariable("PATH", [System.EnvironmentVariableTarget]::Machine))
if ($UserPath.Trim() -ne "") {
    $env:PATH = "$($UserPath);$($ComputerPath)"
} else {
    $env:PATH = $ComputerPath
}

function Set-RetryableContent([string] $Path, [string] $Value) {
    for ($i = 0; $i -lt 100; $i += 1) {
        try {
            Set-Content -Force -Path $Path -Value $Value
            return
        } catch {
            if ($_.ToString().Contains("Stream was not readable.")) {
                Start-Sleep -Milliseconds 10
                continue
            }
        }
    }
    Write-Error "Could not write to file: $Path"
}

$EngineDir = (Resolve-Path "$((Get-Location).Path)\..\..").Path

$InstalledFolderName = "ClangTidyForUnrealEngine"
if (!(Test-Path "$env:PROGRAMDATA\$InstalledFolderName")) {
    New-Item -ItemType Directory -Path "$env:PROGRAMDATA\$InstalledFolderName" | Out-Null
}
Get-ChildItem -Path "$env:PROGRAMDATA\$InstalledFolderName" -Recurse -Filter "*" | % {
    if ($_.IsReadOnly) {
        $_.IsReadOnly = $false
    }
}
$Acl = Get-Acl "$env:PROGRAMDATA\$InstalledFolderName"
$AccessRule = New-Object System.Security.AccessControl.FileSystemAccessRule("Everyone", "FullControl", "Allow")
$Acl.SetAccessRule($AccessRule)
$Acl | Set-Acl "$env:PROGRAMDATA\$InstalledFolderName"

$global:ProgressPreference = 'SilentlyContinue'

# Check to make sure ClangTidySystem.lua (which contains the plugin provided checks)
# is up to date.
$ExistingContent = ""
$TargetContent = Get-Content -Raw -Path "$PSScriptRoot\ClangTidySystem.lua"
if (Test-Path "$env:PROGRAMDATA\$InstalledFolderName\ClangTidySystem.lua") {
    $ExistingContent = Get-Content -Raw -Path "$env:PROGRAMDATA\$InstalledFolderName\ClangTidySystem.lua"
}
if ($null -eq $ExistingContent -or $null -eq $TargetContent -or $ExistingContent.Trim() -ne $TargetContent.Trim()) {
    Set-RetryableContent -Path "$env:PROGRAMDATA\$InstalledFolderName\ClangTidySystem.lua" -Value $TargetContent
}

# Check to make sure LICENSE.txt is up to date.
$ExistingContent = ""
$TargetContent = Get-Content -Raw -Path "$PSScriptRoot\LICENSE.txt"
if (Test-Path "$env:PROGRAMDATA\$InstalledFolderName\LICENSE.txt") {
    $ExistingContent = Get-Content -Raw -Path "$env:PROGRAMDATA\$InstalledFolderName\LICENSE.txt"
}
if ($null -eq $ExistingContent -or $null -eq $TargetContent -or $ExistingContent.Trim() -ne $TargetContent.Trim()) {
    Set-RetryableContent -Path "$env:PROGRAMDATA\$InstalledFolderName\LICENSE.txt" -Value $TargetContent
}

# Create the registry key if needed.
if (!(Test-Path "HKCU:\SOFTWARE\RedpointGames")) {
    New-Item -Path HKCU:\SOFTWARE\RedpointGames | Out-Null
}
if (!(Test-Path "HKCU:\SOFTWARE\RedpointGames\LicenseManager")) {
    New-Item -Path HKCU:\SOFTWARE\RedpointGames\LicenseManager | Out-Null
}
if (!(Test-Path "HKCU:\SOFTWARE\RedpointGames\LicenseManager\ClangTidy")) {
    New-Item -Path HKCU:\SOFTWARE\RedpointGames\LicenseManager\ClangTidy | Out-Null
}

# Check if the binaries are installed or out-of-date.
$OutOfDateBinaries = $false
$CurrentVersion = ""
if (!(Test-Path "$env:PROGRAMDATA\$InstalledFolderName\version")) {
    $OutOfDateBinaries = $true
} else {
    $CurrentVersion = (Get-Content -Raw -Path "$env:PROGRAMDATA\$InstalledFolderName\version").Trim()
    $NextVersionCheck = (Get-ItemProperty -Path "HKCU:\SOFTWARE\RedpointGames\LicenseManager\ClangTidy" -Name NextVersionCheck -ErrorAction SilentlyContinue).NextVersionCheck
    if (($null -eq $NextVersionCheck) -or ((Get-Date).Ticks -gt $NextVersionCheck)) {
        $OutOfDateBinaries = $true
    }
}

if ($OutOfDateBinaries) {
    $LicenseServerURL = (Get-ItemProperty -ErrorAction SilentlyContinue -Path "HKCU:\SOFTWARE\RedpointGames\LicenseManager\ClangTidy" -Name "LicenseServerURL").LicenseServerURL
    if ($LicenseServerURL -eq $null) {
        $LicenseServerURL = "https://licensing.redpoint.games"
    }

    Write-Output "Checking that the clang-tidy installation is up-to-date..."
    $Metadata = $null
    try {
        $Metadata = (Invoke-WebRequest -UseBasicParsing -Uri "$LicenseServerURL/api/licenses/download/metadata" -Method Post -Body @{
            productId = "clang-tidy";
        }).Content | ConvertFrom-Json
    } catch {
        Write-Output "warning: Unable to install the clang-tidy binaries from the License Manager."
    }

    if ($Metadata -ne $null) {
        if ($Metadata.version -ne $CurrentVersion) {
            Write-Output "A new version of clang-tidy is available, downloading and installing..."
            foreach ($Entry in $Metadata.entries) {
                $FileHash = ""
                if (Test-Path "$env:PROGRAMDATA\$InstalledFolderName\$($Entry.name)") {
                    $FileHash = (Get-FileHash -Algorithm SHA1 -Path "$env:PROGRAMDATA\$InstalledFolderName\$($Entry.name)").Hash.ToLowerInvariant()
                }
                if ($FileHash -ne $Entry.hash.ToLowerInvariant()) {
                    Write-Output " - $($Entry.name) ..."
                    Invoke-WebRequest -UseBasicParsing -Uri "$($Entry.uri)" -OutFile "$env:PROGRAMDATA\$InstalledFolderName\$($Entry.name)" | Out-Null
                }
            }
            Set-RetryableContent -Path "$env:PROGRAMDATA\$InstalledFolderName\version" -Value $Metadata.version
        }
        Set-ItemProperty -Path "HKCU:\SOFTWARE\RedpointGames\LicenseManager\ClangTidy" -Name NextVersionCheck -Value ((Get-Date).AddHours(24).Ticks)
    }
}

# Check to make sure the UBT source files are up-to-date.
Get-ChildItem -Path "$EngineDir\Engine\Source\Programs\UnrealBuildTool" -Recurse -Filter "*" | % {
    if ($_.IsReadOnly) {
        $_.IsReadOnly = $false
    }
}
Get-ChildItem -Path "$EngineDir\Engine\Source\Programs\DotNETCommon" -Recurse -Filter "*" | % {
    if ($_.IsReadOnly) {
        $_.IsReadOnly = $false
    }
}
if (!(Test-Path "$EngineDir\Engine\Source\Programs\UnrealBuildTool\Platform\Redpoint")) {
    New-Item -Force -ItemType Directory "$EngineDir\Engine\Source\Programs\UnrealBuildTool\Platform\Redpoint" | Out-Null
}
$FilesToInstall = Get-ChildItem -Path $PSScriptRoot -Filter *.cs
$NeedsRebuild = $false
foreach ($File in $FilesToInstall) {
    $TargetPath = "$EngineDir\Engine\Source\Programs\UnrealBuildTool\Platform\Redpoint\$($File.Name)"
    $NeedsWrite = $false
    $LocalContent = (Get-Content -Raw $File.FullName)
    if (!(Test-Path $TargetPath)) {
        $NeedsWrite = $true
    } else {
        $TargetContent = (Get-Content -Raw $TargetPath)
        if ($LocalContent.Trim() -ne $TargetContent.Trim()) {
            $NeedsWrite = $true
        }
    }
    if ($NeedsWrite) {
        Set-RetryableContent -Path $TargetPath -Value $LocalContent
        $NeedsRebuild = $true
    }
}

# Check if we need to patch UEBuildPlatform.cs
$UEBP = (Get-Content -Raw "$EngineDir\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildPlatform.cs")
$OriginalUEBP = $UEBP
if (!$UEBP.Contains("foreach (Type CheckType in AllTypes.OrderBy(x => x.FullName))")) {
    $UEBP = $UEBP.Replace(
        "foreach (Type CheckType in AllTypes)",
        "foreach (Type CheckType in AllTypes.OrderBy(x => x.FullName))"
    )
}
if ($UEBP.Trim() -ne $OriginalUEBP.Trim()) {
    if (!(Test-Path "$EngineDir\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildPlatform.cs.backup")) {
        Copy-Item -Force "$EngineDir\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildPlatform.cs" "$EngineDir\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildPlatform.cs.backup"
    }
    Set-RetryableContent -Path "$EngineDir\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildPlatform.cs" -Value $UEBP
    $NeedsRebuild = $true
}

# Check to see if we need to patch UnrealBuildTool.csproj.
$UBTProj = (Get-Content -Raw "$EngineDir\Engine\Source\Programs\UnrealBuildTool\UnrealBuildTool.csproj")
$OriginalUBTProj = $UBTProj
if (!$UBTProj.Contains("<!-- CLANG_BEGIN -->")) {
    $UBTProj = $UBTProj.Replace(
        "<Compile Include=`"System\Action.cs`" />",
        "<Compile Include=`"System\Action.cs`" /><!-- CLANG_BEGIN --><!-- CLANG_END -->"
    )
}
$Regex = [regex]"CLANG_BEGIN.+CLANG_END"
$CompileActions = ""
foreach ($File in $FilesToInstall) {
    $CompileActions += "<Compile Include=`"Platform\Redpoint\$($File.Name)`" />"
}
$UBTProj = $Regex.Replace($UBTProj, "CLANG_BEGIN -->$($CompileActions)<!-- CLANG_END")
if (!$UBTProj.Contains("__REDPOINT_MODIFICATIONS__")) {
    $ExtraDefines = ""
    $UBTProj = $UBTProj.Replace(
        "<RootNamespace>UnrealBuildTool</RootNamespace>", 
        "<RootNamespace>UnrealBuildTool</RootNamespace><DefineConstants>`$(DefineConstants);__REDPOINT_MODIFICATIONS__$ExtraDefines</DefineConstants>")
}
if ($UBTProj -ne $OriginalUBTProj) {
    if (!(Test-Path "$EngineDir\Engine\Source\Programs\UnrealBuildTool\UnrealBuildTool.csproj.backup")) {
        Copy-Item -Force "$EngineDir\Engine\Source\Programs\UnrealBuildTool\UnrealBuildTool.csproj" "$EngineDir\Engine\Source\Programs\UnrealBuildTool\UnrealBuildTool.csproj.backup"
    }
    Set-RetryableContent -Path "$EngineDir\Engine\Source\Programs\UnrealBuildTool\UnrealBuildTool.csproj" -Value $UBTProj
    $NeedsRebuild = $true
}

# Check if we need to patch EpicGames.Perforce.csproj
$PerforceProjPath = "$EngineDir\Engine\Source\Programs\Shared\EpicGames.Perforce\EpicGames.Perforce.csproj"
if (Test-Path $PerforceProjPath) {
    $PerforceProj = (Get-Content -Raw $PerforceProjPath)
    $PerforceProjOrig = $PerforceProj
    if ($PerforceProj.Contains("<None Include=`"..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\win-x64\`$(WindowsNativeConfig)\EpicGames.Perforce.Native.dll`" Visible=`"false`">")) {
        $PerforceProj = $PerforceProj.Replace(
            "<None Include=`"..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\win-x64\`$(WindowsNativeConfig)\EpicGames.Perforce.Native.dll`" Visible=`"false`">",
            "<None Include=`"..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\win-x64\`$(WindowsNativeConfig)\EpicGames.Perforce.Native.dll`" Visible=`"false`" Condition=`"Exists('..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\win-x64\`$(WindowsNativeConfig)\EpicGames.Perforce.Native.dll')`">");
    }
    if ($PerforceProj.Contains("<None Include=`"..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\linux-x64\Release\EpicGames.Perforce.Native.so`" Visible=`"false`">")) {
        $PerforceProj = $PerforceProj.Replace(
            "<None Include=`"..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\linux-x64\Release\EpicGames.Perforce.Native.so`" Visible=`"false`">",
            "<None Include=`"..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\linux-x64\Release\EpicGames.Perforce.Native.so`" Visible=`"false`" Condition=`"Exists('..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\linux-x64\Release\EpicGames.Perforce.Native.so')`">");
    }
    if ($PerforceProj.Contains("<None Include=`"..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\mac-x64\Release\EpicGames.Perforce.Native.dylib`" Visible=`"false`">")) {
        $PerforceProj = $PerforceProj.Replace(
            "<None Include=`"..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\mac-x64\Release\EpicGames.Perforce.Native.dylib`" Visible=`"false`">",
            "<None Include=`"..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\mac-x64\Release\EpicGames.Perforce.Native.dylib`" Visible=`"false`" Condition=`"Exists('..\..\..\..\Binaries\DotNET\EpicGames.Perforce.Native\mac-x64\Release\EpicGames.Perforce.Native.dylib')`">");
    }
    if ($PerforceProj -ne $PerforceProjOrig) {
        if (!(Test-Path "$PerforceProjPath.backup")) {
            Copy-Item -Force "$PerforceProjPath" "$PerforceProjPath.backup"
        }
        Set-RetryableContent -Path "$PerforceProjPath" -Value $PerforceProj
        $NeedsRebuild = $true
    }
}

function Get-MSBuildFromRegistry($Path, $Key, $Suffix) {
    $PathsToTry = @(
        "HKCU:\SOFTWARE\$Path",
        "HKLM:\SOFTWARE\$Path",
        "HKCU:\SOFTWARE\Wow6432Node\$Path",
        "HKLM:\SOFTWARE\Wow6432Node\$Path"
    )
    foreach ($PathToTry in $PathsToTry) {
        if (Test-Path "$PathToTry") {
            $Property = Get-ItemProperty -Path "$PathToTry" -Name $Key -ErrorAction SilentlyContinue
            if ($Property -ne $null) {
                $PropertyValue = $Property."$Key"
                if ($PropertyValue -ne $null) {
                    if (Test-Path "$PropertyValue$Suffix") {
                        return "$PropertyValue$Suffix"
                    }
                }
            }
        }
    }
    return $null
}
function Get-MSBuildPath() {
    if ($global:IsMacOS) {
        if (Test-Path "/Library/Frameworks/Mono.framework/Versions/Current/Commands/msbuild") {
            return "/Library/Frameworks/Mono.framework/Versions/Current/Commands/msbuild"
        }
        if (Test-Path "/Library/Frameworks/Mono.framework/Versions/Current/Commands/xbuild") {
            return "/Library/Frameworks/Mono.framework/Versions/Current/Commands/xbuild"
        }
        return $null
    }
    if (Test-Path "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe") {
        foreach ($VsInstallPath in (& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -prerelease -sort -products * -property installationPath)) {
            if ($null -ne $VsInstallPath) {
                $VsInstallPath = $VsInstallPath.Trim()
                if (Test-Path "$VsInstallPath\MSBuild\Current\Bin\MSBuild.exe") {
                    return "$VsInstallPath\MSBuild\Current\Bin\MSBuild.exe"
                }
                if (Test-Path "$VsInstallPath\MSBuild\15.0\Bin\MSBuild.exe") {
                    return "$VsInstallPath\MSBuild\15.0\Bin\MSBuild.exe"
                }
            }
        }
    }
    $local:ProposedPath = (Get-MSBuildFromRegistry "Microsoft\VisualStudio\SxS\VS7" "15.0" "MSBuild\15.0\bin\MSBuild.exe")
    if ($null -ne $local:ProposedPath) {
        return $local:ProposedPath
    }
    if (Test-Path "${env:ProgramFiles(x86)}\MSBuild\14.0\bin\MSBuild.exe") {
        return "${env:ProgramFiles(x86)}\MSBuild\14.0\bin\MSBuild.exe"
    }
    $local:ProposedPath = (Get-MSBuildFromRegistry "Microsoft\MSBuild\ToolsVersions\14.0" "MSBuildToolsPath" "MSBuild.exe")
    if ($null -ne $local:ProposedPath) {
        return $local:ProposedPath
    }
    $local:ProposedPath = (Get-MSBuildFromRegistry "Microsoft\MSBuild\ToolsVersions\12.0" "MSBuildToolsPath" "MSBuild.exe")
    if ($null -ne $local:ProposedPath) {
        return $local:ProposedPath
    }
    return $null
}

if ($NeedsRebuild) {
    $Success = $false

    try {
        $MSBuildPath = Get-MSBuildPath
        if ($null -eq $MSBuildPath) {
            Write-Output "Unable to locate MSBuild!"
            exit 1
        }

        $UnrealBuildToolProject = "$EngineDir\Engine\Source\Programs\UnrealBuildTool\UnrealBuildTool.csproj"
        $UnrealBuildToolProjectContent = (Get-Content -Raw "$UnrealBuildToolProject")

        $IsUnrealEngine5 = $false
        if ($UnrealBuildToolProjectContent.Contains("netcoreapp") -or $UnrealBuildToolProjectContent.Contains("net6.0")) {
            Write-Host "Detected Unreal Engine 5."
            $IsUnrealEngine5 = $true
        }

        if ($IsUnrealEngine5) {
            Get-ChildItem -Path "$EngineDir\Engine\Source\Programs\Shared\EpicGames.Core" -Recurse -Filter "*" | % {
                if ($_.IsReadOnly) {
                    $_.IsReadOnly = $false
                }
            }

            if (Test-Path "$env:ProgramFiles\dotnet\dotnet.exe") {
                $Sources = (& "$env:ProgramFiles\dotnet\dotnet.exe" nuget list source | Out-String)
                if (!$Sources.Contains("https://api.nuget.org/v3/index.json")) {
                    # Make sure we have the NuGet source for restore.
                    & "$env:ProgramFiles\dotnet\dotnet.exe" nuget add source -n nuget.org "https://api.nuget.org/v3/index.json"
                }
            }

            Write-Host "Restoring packages..."
            & "$MSBuildPath" "/nologo" "/verbosity:quiet" "$UnrealBuildToolProject" "/property:Configuration=Development" "/property:Platform=AnyCPU" "/p:WarningLevel=0" "/target:Restore"
            if ($LastExitCode -ne 0) {
                exit $LastExitCode
            }
        }
        
        & "$MSBuildPath" "/nologo" "/verbosity:quiet" "$UnrealBuildToolProject" "/property:Configuration=Development" "/property:Platform=AnyCPU" "/property:OutputPath=$EngineDir\Engine\Binaries\DotNET_CLANGPatched" "/property:ReferencePath=$EngineDir\Engine\Binaries\DotNET\UnrealBuildTool"
        if ($LastExitCode -ne 0) {
            exit $LastExitCode
        }
        
        $CopyFiles = @(
            "UnrealBuildTool.exe",
            "UnrealBuildTool.exe.config",
            "UnrealBuildTool.dll",
            "UnrealBuildTool.dll.config",
            "UnrealBuildTool.deps.json",
            "UnrealBuildTool.runtimeconfig.dev.json",
            "UnrealBuildTool.runtimeconfig.json",
            "UnrealBuildTool.pdb",
            "UnrealBuildTool.xml"
        )
        foreach ($CopyFile in $CopyFiles) {
            if ($IsUnrealEngine5) {
                if ((Test-Path "$EngineDir\Engine\Binaries\DotNET\UnrealBuildTool\$($CopyFile)") -or $CopyFile.EndsWith("UnrealBuildTool.pdb")) {
                    Move-Item -Force "$EngineDir\Engine\Binaries\DotNET\UnrealBuildTool\$($CopyFile)" "$EngineDir\Engine\Binaries\DotNET\UnrealBuildTool\$($CopyFile).old"
                    Copy-Item -Force "$EngineDir\Engine\Binaries\DotNET_CLANGPatched\$($CopyFile)" "$EngineDir\Engine\Binaries\DotNET\UnrealBuildTool\$($CopyFile)"
                }
                if ((Test-Path "$EngineDir\Engine\Binaries\DotNET\AutomationTool\$($CopyFile)") -or $CopyFile.EndsWith("UnrealBuildTool.pdb")) {
                    Move-Item -Force "$EngineDir\Engine\Binaries\DotNET\AutomationTool\$($CopyFile)" "$EngineDir\Engine\Binaries\DotNET\AutomationTool\$($CopyFile).old"
                    Copy-Item -Force "$EngineDir\Engine\Binaries\DotNET_CLANGPatched\$($CopyFile)" "$EngineDir\Engine\Binaries\DotNET\AutomationTool\$($CopyFile)"
                }
                if ((Test-Path "$EngineDir\Engine\Binaries\DotNET\AutomationTool\AutomationUtils\$($CopyFile)") -or $CopyFile.EndsWith("UnrealBuildTool.pdb")) {
                    Move-Item -Force "$EngineDir\Engine\Binaries\DotNET\AutomationTool\AutomationUtils\$($CopyFile)" "$EngineDir\Engine\Binaries\DotNET\AutomationTool\AutomationUtils\$($CopyFile).old"
                    Copy-Item -Force "$EngineDir\Engine\Binaries\DotNET_CLANGPatched\$($CopyFile)" "$EngineDir\Engine\Binaries\DotNET\AutomationTool\AutomationUtils\$($CopyFile)"
                }
            } else {
                if (Test-Path "$EngineDir\Engine\Binaries\DotNET\$($CopyFile)") {
                    Move-Item -Force "$EngineDir\Engine\Binaries\DotNET\$($CopyFile)" "$EngineDir\Engine\Binaries\DotNET\$($CopyFile).old"
                    Copy-Item -Force "$EngineDir\Engine\Binaries\DotNET_CLANGPatched\$($CopyFile)" "$EngineDir\Engine\Binaries\DotNET\$($CopyFile)"
                }
            }
        }
        $Success = $true
    } finally {
        if (!$Success) {
            # Restore original script.
            if (Test-Path "$EngineDir\Engine\Source\Programs\UnrealBuildTool\UnrealBuildTool.csproj.backup") {
                Set-RetryableContent -Path "$EngineDir\Engine\Source\Programs\UnrealBuildTool\UnrealBuildTool.csproj" -Value (Get-Content -Raw -Path "$EngineDir\Engine\Source\Programs\UnrealBuildTool\UnrealBuildTool.csproj.backup")
            }

            # Remove Redpoint files (this is important for .NET Core / UE5 which will implicitly include files).
            if (Test-Path "$EngineDir\Engine\Source\Programs\UnrealBuildTool\Platform\Redpoint") {
                Remove-Item -Force -Recurse "$EngineDir\Engine\Source\Programs\UnrealBuildTool\Platform\Redpoint"
            }
        }
    }

    $StopBuildOnUBTModify = (Get-ItemProperty -Path "HKCU:\SOFTWARE\RedpointGames\LicenseManager\ClangTidy" -Name StopBuildOnUBTModify -ErrorAction SilentlyContinue).StopBuildOnUBTModify
    if ($StopBuildOnUBTModify -eq "true") {
        Write-Host "error: clang-tidy for Unreal Engine had to patch your engine's UnrealBuildTool. The build was stopped because you have 'StopBuildOnUBTModify' set in the registry."
        exit 1
    } else {
        Write-Host "warning: clang-tidy for Unreal Engine had to patch your engine's UnrealBuildTool. clang-tidy might not run until you next build the project."
        exit 0
    }
}

exit 0