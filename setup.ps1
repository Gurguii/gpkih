#!/usr/bin/env pwsh

$required_tools = @(
    "msbuild", "cmake"
)

$exec = @{
    msbuild = "";
    cmake = "";
}

[array]$exec_find_paths = ("C:\Program Files\", "C:\Program Files (x86)\");

[string]$root = $PSScriptRoot

[string]$build_dir = "$root\build"

[string]$binpaths = "$root\.tools"

# Check binpaths
if(Get-Item "$binpaths" -ErrorAction SilentlyContinue){
    # File exists
    [array]$contents = Get-Content "$binpaths"
    foreach($line in $contents){
        [string]$exec_name = $line.split('=')[0];
        [string]$exec_path = $line.split('=')[1];
        foreach($tool in $required_tools){
            if("$tool" -eq "$exec_name"){
                $exec[$tool] = "$exec_path"
            }
        }
    }    
}

# Look for required tools
foreach($executable in $required_tools){
    if($exec[$executable]){
        # Already set with .tools file
        continue
    } 

    if(Get-Command -Name "$executable" -ErrorAction SilentlyContinue){
        # Found executable in PATH
        $exec[$executable] = (Get-Command -Name "$executable").Source
        Write-Host "-- $executable found in PATH"

        # Write executable entry to .tools
        Write-Host "-- Adding entry to $binpaths"
        Write-Output "$executable=$($exec[$executable])" | Out-File -Append "$binpaths"
        
        continue
    }else{
        # Tool not found in PATH, look in common paths
        Write-Host "-- $executable.exe not found in PATH or .tools, looking for it in C:\Program Files\ and C:\Program Files (x86)\"
        foreach($find_path in $exec_find_paths){
            $found = Get-ChildItem -Path "$find_path" -Recurse -Name "$executable.exe" | Select-Object -First 1;
            if($found.Length -gt 0){
                # Found executable
                $fullpath = "$find_path$found"
                $exec[$executable] = "$fullpath"
                Write-Output "$executable=$fullpath" | Out-File -Append "$binpaths"
                $found = ""
                break
            }
            else{
                Write-Host "$executable not found, exiting..."
            }
        }
    }
} 


if (!(Get-Item "$build_dir" -ErrorAction SilentlyContinue)){
    # Create build dir
    New-Item -ItemType Directory -Path "$build_dir" | Out-Null
    if(!(Get-Item "$build_dir")){
        Write-Host -ForegroundColor Red "Couldn't create build directory"
        exit 0
    }
}

# Get into build\ to run commands
Set-Location "$build_dir"

Write-Host "[ Using executables ]"
foreach($k in $exec.Keys){
    Write-Host "[ $k.exe ] : $($exec[$k])"
}

# run 'cmake ..' inside build directory
$cmake_executable = $exec["cmake"]
$cmake_command_args = "-DCMAKE_EXPORT_COMPILE_COMMANDS=1 .."

#if($debug){
#    $cmake_command_args += " -DDEBUG=ON"
#}else{
#    $cmake_command_args += " -DDEBUG=OFF"
#}

$cmake_process = Start-Process -FilePath "$cmake_executable" -ArgumentList "$cmake_command_args" -PassThru -Wait -NoNewWindow

if($cmake_process.ExitCode -ne 0){
    Write-Host "Command '$cmake_executable $cmake_command_args' returned $exitcode"
    exit $cmake_process.ExitCode
}
Write-Host -ForegroundColor Green " -- cmake command succesful"

$msbuild_executable = "$($exec["msbuild"])"
$msbuild_command_args = "-verbosity:minimal -maxCpuCount:4 -t:gpkih `"$build_dir\gpkih.sln`""

$build_process = Start-Process -FilePath "$msbuild_executable" -ArgumentList "$msbuild_command_args" -PassThru -Wait -NoNewWindow
if($build_process.ExitCode -ne 0){
    Write-Host -ForegroundColor Red "[error]MSBuild returned $($build_process.ExitCode)"
    exit $cmake_process.ExitCode    
}

[string]$exe_src = "$build_dir\Debug\gpkih.exe"
[string]$exe_dst = "$root\gpkih.exe"

Write-host -ForegroundColor Green " -- msbuild command succesful"
Write-Host " -- Copying $exe_src to $exe_dst"

Copy-Item -Path "$exe_src" -Destination "$exe_dst" -Force