$DFTT_install_path = 'C:\Program Files\dftt'
$DFTT_exe_link = "https://github.com/ymich9963/dftt/releases/latest/download/dftt.exe"

if (Test-Path -Path $DFTT_install_path) {
    Write-Output "Removing previously installed executable."
    Remove-Item $DFTT_install_path -r # rm command
}

New-Item -Path $DFTT_install_path -ItemType Directory | Out-Null # make new dir and suppress output
curl -fsSLO $DFTT_exe_link
Move-Item dftt.exe $DFTT_install_path # mv command
Write-Output "Downloaded executable." # echo command

$Sys_Env_Path_Value = Get-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\Environment\' -Name Path 

# Change the backslashes to frontslashes so that -split can work
$DFTT_install_path_frontslash = $DFTT_install_path -replace "\\","/"
$Sys_Env_Path_Value_frontslash = $Sys_Env_Path_Value.Path -replace "\\", "/"

# Check if the install path exists by splitting the Path variable value
$DFTT_path_check = $Sys_Env_Path_Value_frontslash -split $DFTT_install_path_frontslash | Measure-Object 

if ($DFTT_path_check.Count -igt 1) {
    Write-Output "Detected previous DFTT installation."
    Write-Output "Nothing was added to the system Path variable."
} else {
    Write-Output "Detected no previous DFTT install."
    Write-Output "Adding executable to system Path environment variable."
    $New_Path_Value = $Sys_Env_Path_Value.Path + ";" + $DFTT_install_path + ";" 
    Set-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\Environment\' -Name Path -Value $New_Path_Value # set the system environment variable for DFTT 
}

Write-Output "Succesfully installed DFTT."

Read-Host -Prompt "Press Enter to exit"

Exit
