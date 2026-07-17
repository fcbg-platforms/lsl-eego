param(
    [Parameter(Mandatory = $true)]
    [string]$Passphrase
)

$ErrorActionPreference = 'Stop'

function Encrypt-File {
    param(
        [Parameter(Mandatory = $true)]
        [string]$InputFile,
        [Parameter(Mandatory = $true)]
        [string]$OutputFile
    )

    if (-not (Test-Path $InputFile)) {
        throw "Missing input file: $InputFile"
    }

    openssl enc -aes-256-cbc -pbkdf2 -iter 200000 -salt -in $InputFile -out $OutputFile -pass pass:$Passphrase

    if (-not (Test-Path $OutputFile)) {
        throw "Failed to create encrypted file: $OutputFile"
    }
}

Encrypt-File -InputFile '.\eego-SDK.dll' -OutputFile '.\eego-SDK.dll.enc'
Encrypt-File -InputFile '.\eego-SDK.lib' -OutputFile '.\eego-SDK.lib.enc'
Encrypt-File -InputFile '.\libeego-SDK.so' -OutputFile '.\libeego-SDK.so.enc'

Write-Host 'Encrypted SDK files created:'
Write-Host ' - eego-SDK.dll.enc'
Write-Host ' - eego-SDK.lib.enc'
Write-Host ' - libeego-SDK.so.enc'
