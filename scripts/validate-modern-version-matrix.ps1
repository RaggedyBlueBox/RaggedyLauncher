param([string]$CatalogPath, [string]$PackagePath, [string]$AppxPath)
$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "common.ps1")
$root = Resolve-RepoRoot
if (-not $CatalogPath) { $CatalogPath = Join-Path $root $ProjectConfig.VersionCatalog }
if (-not $PackagePath) { $PackagePath = Get-ConfigPath "PackageContentDir" }
$targets = @(Import-Csv $CatalogPath -Delimiter "`t" | Where-Object { $_.loaderVersion -and $_.loaderVersion -ne 'selected' -and $_.loaderVersion -ne 'none' })
if ($targets.Count -ne 1 -or $targets[0].minecraftVersion -ne '26.3' -or $targets[0].loader -ne 'fabric' -or $targets[0].loaderVersion -ne '0.20.1') {
    throw 'Version catalog must contain only the 26.3 Fabric target'
}
if ($ProjectConfig.MinecraftVersion -ne '26.3' -or $ProjectConfig.FabricLoaderVersion -ne '0.20.1' -or $ProjectConfig.MinecraftAssetIndex -ne '34') {
    throw 'Build defaults must match the 26.3 Fabric target (minecraft 26.3, fabric 0.20.1, asset index 34)'
}
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip = $null
if ($AppxPath) { $zip = [IO.Compression.ZipFile]::OpenRead($AppxPath) }
try {
    function Require-Entry([string]$Relative) {
        if ($zip) {
            $entry = $zip.GetEntry($Relative.Replace('\', '/'))
            if (-not $entry -or $entry.Length -eq 0) { throw "Package entry missing or empty $Relative" }
        } else {
            $path = Join-Path $PackagePath $Relative
            if (-not (Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -eq 0) {
                throw "Package entry missing or empty $Relative"
            }
            if ($Relative.EndsWith('.jar') -and -not (Test-JarComplete $path)) { throw "Invalid packaged jar $Relative" }
        }
    }
    function Read-EntryBytes([string]$Relative) {
        if ($zip) {
            $entry = $zip.GetEntry($Relative.Replace('\', '/'))
            if (-not $entry) { throw "Package entry missing $Relative" }
            $stream = $entry.Open()
            try {
                $memory = New-Object System.IO.MemoryStream
                try {
                    $stream.CopyTo($memory)
                    return ,$memory.ToArray()
                } finally { $memory.Dispose() }
            } finally { $stream.Dispose() }
        }
        $path = Join-Path $PackagePath $Relative
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Package entry missing $Relative" }
        return ,[IO.File]::ReadAllBytes($path)
    }
    function Read-JarEntryBytes([byte[]]$JarBytes, [string]$Name) {
        $memory = New-Object System.IO.MemoryStream
        try {
            $memory.Write($JarBytes, 0, $JarBytes.Length)
            $memory.Position = 0
            $archive = New-Object System.IO.Compression.ZipArchive($memory, [IO.Compression.ZipArchiveMode]::Read)
            try {
                $entry = $archive.GetEntry($Name)
                if (-not $entry) { return $null }
                $stream = $entry.Open()
                try {
                    $copy = New-Object System.IO.MemoryStream
                    try {
                        $stream.CopyTo($copy)
                        return ,$copy.ToArray()
                    } finally { $copy.Dispose() }
                } finally { $stream.Dispose() }
            } finally { $archive.Dispose() }
        } finally { $memory.Dispose() }
    }
    function Require-Contains([string]$Text, [string]$Needle, [string]$Context) {
        if (-not $Text.Contains($Needle)) { throw "$Context is missing $Needle" }
    }
    Require-Entry "runtime\version_catalog.tsv"
    Require-Entry "runtime\recommended-mods.json"
    Require-Entry "runtime\performance-mods.json"
    Require-Entry "natives\SDL3.dll"
    $sdl3Text = [Text.Encoding]::ASCII.GetString((Read-EntryBytes "natives\SDL3.dll"))
    Require-Contains $sdl3Text "VCRUNTIME140_APP.dll" "UWP SDL3 runtime"
    Require-Contains $sdl3Text "MSVCP140_APP.dll" "UWP SDL3 runtime"
    $fabricJarRelative = "runtime\libraries\net\fabricmc\fabric-loader\0.20.1\fabric-loader-0.20.1.jar"
    Require-Entry $fabricJarRelative
    $fabricJarBytes = Read-EntryBytes $fabricJarRelative
    $accessModeClass = Read-JarEntryBytes $fabricJarBytes 'net/fabricmc/loader/impl/util/FileSystemUtil$AccessMode.class'
    if (-not $accessModeClass) { throw 'Fabric 0.20.1 AccessMode patch class is missing' }
    $fileSystemUtilClass = Read-JarEntryBytes $fabricJarBytes 'net/fabricmc/loader/impl/util/FileSystemUtil.class'
    if (-not $fileSystemUtilClass) { throw 'Fabric 0.20.1 FileSystemUtil patch class is missing' }
    $fileSystemUtilText = [Text.Encoding]::ASCII.GetString($fileSystemUtilClass)
    Require-Contains $fileSystemUtilText "getJarFileSystem" "Fabric 0.20.1 FileSystemUtil patch"
    Require-Contains $fileSystemUtilText 'Ljava/nio/file/Path;Z)Lnet/fabricmc/loader/impl/util/FileSystemUtil$FileSystemDelegate;' "Fabric 0.20.1 FileSystemUtil patch"
    Require-Contains $fileSystemUtilText 'Lnet/fabricmc/loader/impl/util/FileSystemUtil$AccessMode;' "Fabric 0.20.1 FileSystemUtil patch"
    $browserSource = Get-Content -LiteralPath (Join-Path $root 'MC.Xbox/mods/mods_browser.cpp') -Raw
    Require-Contains $browserSource 'g9O0WaGR' 'Featured modpack source'
    Require-Contains $browserSource 'f63ynpri' 'Featured modpack source'
    Require-Contains $browserSource 'PrependCuratedModpacks' 'Featured modpack source'
    $profileSource = Get-Content -LiteralPath (Join-Path $root 'MC.Xbox/profiles/profiles.cpp') -Raw
    Require-Contains $profileSource 'kReConsoleProfileId' 'Built-in profile source'
    Require-Contains $profileSource 'Simply Legacy' 'Built-in profile source'
    $appSource = Get-Content -LiteralPath (Join-Path $root 'MC.Xbox/App.cpp') -Raw
    Require-Contains $appSource 'SDL_OPENGL_LIBRARY' 'SDL OpenGL library hint source'
    Require-Contains $appSource 'L"graphics\\" + graphicsRuntime + L"\\opengl32.dll"' 'Package-relative SDL OpenGL hint source'
    $launchSource = Get-Content -LiteralPath (Join-Path $root 'MC.Xbox/launch/minecraft_launch.cpp') -Raw
    Require-Contains $launchSource 'org.lwjgl.opengl.libname' 'LWJGL OpenGL library source'
    Require-Contains $launchSource 'org.lwjgl.sdl.libname' 'LWJGL SDL3 library source'
    Require-Contains $launchSource 'org.lwjgl.system.SharedLibraryExtractPath' 'LWJGL shared library extract path source'
    Require-Contains $launchSource 'L"graphics\\" + graphicsRuntime + L"\\opengl32.dll"' 'Package-relative SDL OpenGL hint source'
    Require-Contains $appSource 'EnsureDefaultModpackInstalled' 'Default modpack bootstrap source'
    Require-Contains $browserSource 'EnsureDefaultModpackInstalled' 'Default modpack installer source'
    $sdlPrepareSource = Get-Content -LiteralPath (Join-Path $root 'scripts/prepare-sdl3-uwp.ps1') -Raw
    $sdlPatchSource = Get-Content -LiteralPath (Join-Path $root 'patches/sdl3-uwp-external-host.patch') -Raw
    Require-Contains $sdlPrepareSource 'b3852a0076c358f65ee19765e85ef9eded6e50df' 'Pinned UWP SDL3 source'
    Require-Contains $sdlPrepareSource '-DSDL_OPENGL=ON' 'UWP SDL3 desktop OpenGL config'
    Require-Contains $sdlPrepareSource '-DSDL_OPENGLES=OFF' 'UWP SDL3 EGL disable config'
    Require-Contains $sdlPatchSource 'src/loadso/windows/SDL_sysloadso.c' 'UWP SDL3 WinRT-aware shared-object loader source'
    Require-Contains $sdlPatchSource 'src/video/windows/SDL_windowsopengl.c' 'UWP SDL3 WGL source inclusion'
    Require-Contains $sdlPatchSource 'BanditSDL_WinRTAbsoluteLoadFixV1' 'UWP SDL3 absolute Mesa path normalization'
    foreach ($patchScript in @(
        'apply-sdl3-v7-wgl.ps1',
        'apply-sdl3-v8-provider-parity.ps1',
        'apply-sdl3-v9-stability.ps1',
        'apply-sdl3-v12-renderpearl-contract.ps1',
        'apply-sdl3-v13-input.ps1',
        'apply-sdl3-v14-input.ps1',
        'apply-sdl3-v15-input-contract.ps1',
        'apply-sdl3-v16-launcher-cursor.ps1',
        'apply-sdl3-v17-software-cursor.ps1')) {
        Require-Contains (Get-Content -LiteralPath (Join-Path $root "scripts/$patchScript") -Raw) 'SourceRoot' "UWP SDL3 $patchScript"
        Require-Contains $sdlPrepareSource $patchScript "UWP SDL3 $patchScript wired into preparer"
    }
    Require-Contains (Get-Content -LiteralPath (Join-Path $root 'MC.Xbox/SDL3-WinRTResources.rc') -Raw) '5000 CURSOR' 'Transparent WinRT cursor resource'
    Require-Contains (Get-Content -LiteralPath (Join-Path $root 'MC.Xbox/SDL3-WinRTResources.rc') -Raw) '5001 CURSOR' 'Launcher WinRT cursor resource'
    $buildSource = Get-Content -LiteralPath (Join-Path $root 'build.ps1') -Raw
    Require-Contains $buildSource 'SDL3-WinRTResources.rc' 'WinRT cursor resource build'
    Require-Contains $buildSource '"$mcResourceRes"' 'WinRT cursor resource link'
    Require-Contains $launchSource 'PreloadMesaGraphicsStackForSdl' 'Mesa support stack preload source'
    Require-Contains $launchSource 'SDL_VIDEO_DRIVER' 'SDL WinRT video driver source'
    Require-Contains $appSource 'L"mesa_glthread", L"false"' 'Mesa threading stability default'
    $manifestSource = Get-Content -LiteralPath (Join-Path $root 'MC.Xbox/Package.appxmanifest') -Raw
    Require-Contains $manifestSource 'ws2:longPathAware' 'Long path manifest'
    $commonSource = Get-Content -LiteralPath (Join-Path $root 'MC.Xbox/common/launcher_common.cpp') -Raw
    Require-Contains $commonSource 'ExtendedPath' 'Extended path support source'
    $modpackSource = Get-Content -LiteralPath (Join-Path $root 'MC.Xbox/mods/modpack_io.cpp') -Raw
    Require-Contains $modpackSource 'ExtendedPath(path)' 'Modpack long path extraction source'
    Require-Contains $modpackSource 'CreateFile2' 'Modpack long path extraction source'
    foreach ($inputFile in @(
        @{ Source = $CatalogPath; Relative = 'runtime/version_catalog.tsv' },
        @{ Source = (Join-Path $root 'config/recommended-mods.json'); Relative = 'runtime/recommended-mods.json' },
        @{ Source = (Join-Path $root 'config/performance-mods.json'); Relative = 'runtime/performance-mods.json' }
    )) {
        if ($zip) {
            $stream = $zip.GetEntry($inputFile.Relative).Open()
            $hasher = [Security.Cryptography.SHA256]::Create()
            try { $packagedHash = [BitConverter]::ToString($hasher.ComputeHash($stream)).Replace('-', '') }
            finally { $stream.Dispose(); $hasher.Dispose() }
        } else {
            $packagedHash = (Get-FileHash -LiteralPath (Join-Path $PackagePath $inputFile.Relative)).Hash
        }
        if ($packagedHash -ne (Get-FileHash -LiteralPath $inputFile.Source).Hash) {
            throw "Packaged input does not match source $($inputFile.Relative)"
        }
    }
    foreach ($target in $targets) {
        $id = "$($target.minecraftVersion)-$($target.loader)-$($target.loaderVersion)"
        Require-Entry "runtime\manifests\$id.tsv"
        if ($target.loader -eq 'fabric') {
            Require-Entry "runtime\libraries\net\fabricmc\fabric-loader\$($target.loaderVersion)\fabric-loader-$($target.loaderVersion).jar"
            Require-Entry "runtime\version-mods\$id\raggedy-xbox-compat-1.0.0.jar"
        }
        $runtimeDir = switch ($target.javaRuntime) { 'java17' { 'jre17' } 'java21' { 'jre21' } 'legacy' { 'jre8' } default { 'jre' } }
        Require-Entry "$runtimeDir\bin\server\jvm.dll"
        $suffix = switch ($runtimeDir) { 'jre17' { '-17' } 'jre21' { '-21' } 'jre8' { '-8' } default { '' } }
        foreach ($base in @('java-base-uwp-filesystem','java-zipfs-realpath','java-desktop-uwp-awt')) {
            Require-Entry "$base$suffix.jar"
        }
    }
    Write-Host "Package validation passed $($targets.Count) targets"
} finally { if ($zip) { $zip.Dispose() } }
