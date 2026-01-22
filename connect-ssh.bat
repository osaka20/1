@echo off
:: Vérifier si le script est lancé en admin
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo [*] Le script n'est pas en mode administrateur.
    echo [*] Relance automatique en mode admin...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

echo [*] Script lancé en mode administrateur.
echo.

:: Lancer PowerShell pour configurer SSH
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
 "Write-Host 'Installation du serveur SSH...'; ^
  Add-WindowsCapability -Online -Name OpenSSH.Server~~~~0.0.1.0 -ErrorAction SilentlyContinue | Out-Null; ^
  Start-Service sshd -ErrorAction SilentlyContinue; ^
  Set-Service -Name sshd -StartupType Automatic; ^
  Write-Host 'Service SSH configure.'; ^
  Write-Host ''; ^
  Write-Host 'Ajout de la cle publique...'; ^
  $publicKey = 'ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICNLhMKE5S0lqDhoi13c1nQgnwiGgBsuQue5CTALVGVj goxos@osaka'; ^
  $sshPath = \"$env:USERPROFILE\.ssh\"; ^
  if (!(Test-Path $sshPath)) { New-Item -ItemType Directory -Path $sshPath | Out-Null }; ^
  $authFile = \"$sshPath\authorized_keys\"; ^
  if (!(Test-Path $authFile)) { New-Item -ItemType File -Path $authFile | Out-Null }; ^
  $content = Get-Content $authFile -ErrorAction SilentlyContinue; ^
  if ($content -notcontains $publicKey) { Add-Content -Path $authFile -Value $publicKey; Write-Host 'Cle ajoutee.' } else { Write-Host 'Cle deja presente.' }; ^
  Write-Host ''; ^
  Write-Host 'Recuperation de votre IP locale...'; ^
  $ip = (Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.IPAddress -like '192.168.*' -or $_.IPAddress -like '10.*' } | Select-Object -First 1 -ExpandProperty IPAddress); ^
  Write-Host 'Adresse IP locale :' $ip; ^
  Write-Host ''; ^
  Write-Host 'Configuration terminee.'"

pause