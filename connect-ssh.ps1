# ============================
#  Configuration SSH automatique
# ============================

Write-Host "Installation et configuration du serveur SSH..."

# Installer OpenSSH Server si absent
Add-WindowsCapability -Online -Name OpenSSH.Server~~~~0.0.1.0 -ErrorAction SilentlyContinue | Out-Null

# Démarrer le service SSH
Start-Service sshd -ErrorAction SilentlyContinue

# Le mettre en automatique
Set-Service -Name sshd -StartupType Automatic

Write-Host "Service SSH activé et configuré."

# ============================
#  Ajout de la clé publique
# ============================

# ⚠️ Mets ta clé publique ici :
$publicKey = "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICNLhMKE5S0lqDhoi13c1nQgnwiGgBsuQue5CTALVGVj goxos@osaka"

# Chemin du dossier .ssh
$sshPath = "$env:USERPROFILE\.ssh"

# Création du dossier si nécessaire
if (!(Test-Path $sshPath)) {
    New-Item -ItemType Directory -Path $sshPath | Out-Null
}

# Chemin du fichier authorized_keys
$authFile = "$sshPath\authorized_keys"

# Création du fichier si nécessaire
if (!(Test-Path $authFile)) {
    New-Item -ItemType File -Path $authFile | Out-Null
}

# Ajout de la clé si elle n'existe pas déjà
$content = Get-Content $authFile -ErrorAction SilentlyContinue
if ($content -notcontains $publicKey) {
    Add-Content -Path $authFile -Value $publicKey
    Write-Host "Clé publique ajoutée."
} else {
    Write-Host "Clé déjà présente, rien à faire."
}

# ============================
#  Affichage de l'adresse IP
# ============================

Write-Host ""
Write-Host "Récupération de l'adresse IP locale..."
$ip = (Get-NetIPAddress -AddressFamily IPv4 `
       | Where-Object { $_.IPAddress -like '192.168.*' -or $_.IPAddress -like '10.*' } `
       | Select-Object -First 1 -ExpandProperty IPAddress)

Write-Host "Adresse IP locale détectée : $ip"
Write-Host ""
Write-Host "Configuration terminée."