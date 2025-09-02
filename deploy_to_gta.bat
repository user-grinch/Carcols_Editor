@echo off
if defined GTA_SA_DIR (
    taskkill /IM gta_sa.exe /F /FI "STATUS eq RUNNING"
    xcopy /Y "%~dp0\$(TargetFileName)" "%GTA_SA_DIR%"
)
