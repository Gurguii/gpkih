#!/usr/bin/env pwsh

$required_tools = ("msbuild", "cmake")
$exec = @{
    "msbuild" = "";
    "cmake" = "";
}

[array]$exec_find_paths = ("C:\Program Files\", "C:\Program Files (x86)");

# Look for required tools
foreach($executable in $required_tools){
    if(Get-Command -Name "$executable" -ErrorAction SilentlyContinue){
        # Found executable in PATH
        $exec[$executable] = (Get-Command -Name "$executable").Source
        write-host "got it in path: $executable -> $($exec[$executable])"
        continue
    }else{
        # Not found in PATH
        Write-Host "[info] $executable.exe not found in PATH, looking for it in C:\Program Files\ and C:\Program Files (x86)\"
        foreach($find_path in $exec_find_paths){
            $found = (Get-ChildItem -Path "C:\Program Files (x86)\" -Recurse 
            | Where-Object -Property Name -Like "msbuild.exe" 
            | Select-Object -ExpandProperty fullname -ErrorAction SilentlyContinue)[0];

            if($found.Length -gt 0){
                $exec[$executable] = $found
            }else{
                Write-Host "$executable not found, exiting..."
            }
        }
    } 
}

$root = $PSScriptRoot
$build_dir = "$root\build"
$stderr = "$root\.err.log"

if (!(Get-Item "$build_dir" -ErrorAction SilentlyContinue)){
    # Create build dir
    New-Item -ItemType Directory -Path "$build_dir"
}

Set-Location "$build_dir"

# run 'cmake ..' inside build directory
$last = $Error.Count

$cmake_executable = $exec["cmake"]
$cmake_command_args = ".."
& "$cmake_executable" "$cmake_command_args"
if($last -lt $Error.Count){
    # Got an error
    Write-Host -ForegroundColor Red "Encountered an error running '$($exec["cmake"]) ..'"
    exit 0
}

$msbuild_executable = $exec["msbuild"]
$msbuild_command_args = "$build_dir\gpkih.sln"
& "$msbuild_executable" "$msbuild_command_args"

if($last -lt $Error.Count){
    # Got an error
    Write-Host -ForegroundColor Red "Encountered an error running '$msbuild_command'"
    exit 0
}

Copy-Item -Path "$build_dir\Debug\gpkih.exe" -Destination "$root\gpkih.exe" -Force 
Write-Host -ForegroundColor Green "gpkih succesfully compiled"