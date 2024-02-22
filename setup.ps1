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
        Write-Host "[info] $executable found in PATH"
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
$stderr_log = "$root\.error.log"
$stdout_log = "$root\.out.log"

if (!(Get-Item "$build_dir" -ErrorAction SilentlyContinue)){
    # Create build dir
    New-Item -ItemType Directory -Path "$build_dir"
}

Set-Location "$build_dir"

Write-Host "[ Using executables ]"
foreach($k in $exec.Keys){
    Write-Host "[ $k.exe ] : $($exec[$k])"
}

# run 'cmake ..' inside build directory

$cmake_executable = $exec["cmake"]
$cmake_command_args = ".."
$cmake_process = Start-Process -FilePath "$cmake_executable" -ArgumentList "$cmake_command_args" -PassThru -Wait -NoNewWindow

if($cmake_process.ExitCode -ne 0){
    Write-Host "Command '$cmake_executable $cmake_command_args' returned $exitcode"
    exit $cmake_process.ExitCode
}

$msbuild_executable = "$($exec["msbuild"])"
$msbuild_command_args = "-verbosity:minimal -maxCpuCount:4 -t:gpkih `"$build_dir\gpkih.sln`""

$build_process = Start-Process -FilePath "$msbuild_executable" -ArgumentList "$msbuild_command_args" -PassThru -Wait -NoNewWindow

if($build_process.ExitCode -ne 0){
    Write-Host -ForegroundColor "[error]MSBuild returned $($build_process.ExitCode)"
    exit $cmake_process.ExitCode    
}

Copy-Item -Path "$build_dir\Debug\gpkih.exe" -Destination "$root\gpkih.exe" -Force 