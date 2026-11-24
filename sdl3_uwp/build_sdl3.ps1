param(
    [string]$OutputDir
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "..\scripts\common.ps1")

$repoUrl = "https://github.com/XboxEmulationHub/SDL3-uwp.git"
$repoRef = "uwp-3.4.20"
$commit = "ec2e0e7c77ec9a04437eb3ae884d62ae0e6129b4"
$cacheRoot = Join-Path (Get-ConfigPath "CacheDir") "SDL3-uwp"
$sourceDir = Join-Path $cacheRoot "src"
$buildDir = Join-Path $cacheRoot "build"
$installDir = Join-Path $cacheRoot "install"
$stampPath = Join-Path $cacheRoot "build.stamp"
$stamp = "SDL3-UWP|$commit|x64|WindowsStore|10.0|mainready|wgl"

if (-not $OutputDir) {
    $OutputDir = Join-Path (Get-ConfigPath "BuildDir") "sdl3-uwp"
}
Ensure-Dir $OutputDir
$dllPath = Join-Path $OutputDir "SDL3.dll"

$git = Get-Command git -ErrorAction SilentlyContinue
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $git) { throw "git is required to fetch SDL3-UWP." }
if (-not $cmake) { throw "CMake is required to build SDL3-UWP." }

$sdkRoot = "${env:ProgramFiles(x86)}\Windows Kits\10\Lib"
$sdkVersions = @(Get-ChildItem -LiteralPath $sdkRoot -Directory -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match '^\d+\.\d+\.\d+\.\d+$' } |
    Sort-Object Name)
if ($sdkVersions.Count -eq 0) { throw "Windows 10 SDK was not found." }
$sdkVersion = $sdkVersions[-1].Name

if (-not (Test-Path (Join-Path $sourceDir "CMakeLists.txt"))) {
    Ensure-Dir $cacheRoot
    & $git.Source clone --branch $repoRef --depth 1 $repoUrl $sourceDir
    if ($LASTEXITCODE -ne 0) { throw "Could not clone SDL3-UWP." }
}

$head = (& $git.Source -C $sourceDir rev-parse HEAD).Trim()
if ($head -ne $commit) {
    & $git.Source -C $sourceDir fetch --depth 1 origin $commit
    if ($LASTEXITCODE -ne 0) { throw "Could not fetch pinned SDL3-UWP commit." }
    & $git.Source -C $sourceDir checkout --detach $commit
    if ($LASTEXITCODE -ne 0) { throw "Could not checkout pinned SDL3-UWP commit." }
}

$sdlCPath = Join-Path $sourceDir 'src\SDL.c'
$sdlCSource = [IO.File]::ReadAllText($sdlCPath)
if ($sdlCSource.Contains('static bool SDL_MainIsReady = false;')) {
    Write-Host "Patching SDL3-UWP main-ready behavior..."
    $sdlCSource = $sdlCSource.Replace('static bool SDL_MainIsReady = false;', 'static bool SDL_MainIsReady = true;')
    [IO.File]::WriteAllText($sdlCPath, $sdlCSource)
} elseif (-not $sdlCSource.Contains('static bool SDL_MainIsReady = true;')) {
    throw "Could not apply SDL3-UWP main-ready patch."
}

$winrtVideoPath = Join-Path $sourceDir 'src\video\winrt\SDL_winrtvideo.cpp'
$winrtVideoLines = [IO.File]::ReadAllLines($winrtVideoPath)
$glStart = -1
for ($i = 0; $i -lt ($winrtVideoLines.Length - 1); $i++) {
    if ($winrtVideoLines[$i] -eq '#ifdef SDL_VIDEO_OPENGL_EGL' -and
        $winrtVideoLines[$i + 1] -match 'WIN(RT_GLES|GL)_LoadLibrary') {
        $glStart = $i
        break
    }
}
if ($glStart -lt 0) { throw "Could not locate WinRT SDL OpenGL dispatch." }
if ($winrtVideoLines[$glStart + 1] -match 'WINRT_GLES_LoadLibrary') {
    Write-Host "Patching SDL3-UWP WinRT GL dispatch to WGL..."
    $glEnd = $glStart
    while ($glEnd -lt $winrtVideoLines.Length -and $winrtVideoLines[$glEnd] -ne '#endif') { $glEnd++ }
    if ($glEnd -ge $winrtVideoLines.Length) { throw "Could not locate WinRT SDL OpenGL dispatch end." }
    $wglDispatch = @(
        '#ifdef SDL_VIDEO_OPENGL_WGL',
        '    device->GL_LoadLibrary = WIN_GL_LoadLibrary;',
        '    device->GL_GetProcAddress = WIN_GL_GetProcAddress;',
        '    device->GL_UnloadLibrary = WIN_GL_UnloadLibrary;',
        '    device->GL_CreateContext = WIN_GL_CreateContext;',
        '    device->GL_MakeCurrent = WIN_GL_MakeCurrent;',
        '    device->GL_SetSwapInterval = WIN_GL_SetSwapInterval;',
        '    device->GL_GetSwapInterval = WIN_GL_GetSwapInterval;',
        '    device->GL_SwapWindow = WIN_GL_SwapWindow;',
        '    device->GL_DestroyContext = WIN_GL_DestroyContext;',
        '#elif SDL_VIDEO_OPENGL_EGL',
        '    device->GL_LoadLibrary = WINRT_GLES_LoadLibrary;',
        '    device->GL_GetProcAddress = WINRT_GLES_GetProcAddress;',
        '    device->GL_UnloadLibrary = WINRT_GLES_UnloadLibrary;',
        '    device->GL_CreateContext = WINRT_GLES_CreateContext;',
        '    device->GL_MakeCurrent = WINRT_GLES_MakeCurrent;',
        '    device->GL_SetSwapInterval = WINRT_GLES_SetSwapInterval;',
        '    device->GL_GetSwapInterval = WINRT_GLES_GetSwapInterval;',
        '    device->GL_SwapWindow = WINRT_GLES_SwapWindow;',
        '    device->GL_DestroyContext = WINRT_GLES_DestroyContext;',
        '#endif'
    )
    $beforeDispatch = @($winrtVideoLines[0..($glStart - 1)])
    $afterDispatch = if ($glEnd + 1 -lt $winrtVideoLines.Length) { @($winrtVideoLines[($glEnd + 1)..($winrtVideoLines.Length - 1)]) } else { @() }
    $winrtVideoLines = $beforeDispatch + $wglDispatch + $afterDispatch
    [IO.File]::WriteAllLines($winrtVideoPath, $winrtVideoLines)
} elseif ($winrtVideoLines[$glStart + 1] -notmatch 'WIN_GL_LoadLibrary') {
    throw "Could not apply SDL3-UWP WGL dispatch patch."
}

$windowsGlPath = Join-Path $sourceDir 'src\video\windows\SDL_windowsopengl.c'
$windowsGlLines = [IO.File]::ReadAllLines($windowsGlPath)
if (-not ($windowsGlLines | Where-Object { $_.Contains("SDL_strchr(path, ':')") })) {
    $glLoadLine = -1
    for ($i = 0; $i -lt ($windowsGlLines.Length - 4); $i++) {
        if ($windowsGlLines[$i] -eq 'bool WIN_GL_LoadLibrary(SDL_VideoDevice *_this, const char *path)' -and
            $windowsGlLines[$i + 4] -eq '    if (path == NULL) {') {
            $glLoadLine = $i + 4
            break
        }
    }
    if ($glLoadLine -lt 0) { throw "Could not apply SDL3-UWP WGL loader patch." }
    Write-Host "Patching SDL3-UWP WGL loader to use the package-relative OpenGL hint..."
    $windowsGlLines[$glLoadLine] = '    if ((path == NULL) || (SDL_strchr(path, '':'') != NULL)) {'
    [IO.File]::WriteAllLines($windowsGlPath, $windowsGlLines)
}

if ((Test-Path $dllPath) -and (Test-Path $stampPath) -and ([IO.File]::ReadAllText($stampPath).Trim() -eq $stamp)) {
    Write-Host "SDL3-UWP is up to date -> $dllPath"
    return
}

Remove-Item -Recurse -Force $buildDir,$installDir -ErrorAction SilentlyContinue
Ensure-Dir $buildDir
Ensure-Dir $installDir

& $cmake.Source -S $sourceDir -B $buildDir -G "Visual Studio 18 2026" -A "x64,version=$sdkVersion" "-DCMAKE_SYSTEM_NAME=WindowsStore" "-DCMAKE_SYSTEM_VERSION=10.0" "-DWINDOWS_STORE=ON" "-DSDL_SHARED=ON" "-DSDL_STATIC=OFF" "-DSDL_TESTS=OFF" "-DSDL_TEST_LIBRARY=OFF" "-DSDL_EXAMPLES=OFF" "-DSDL_INSTALL=ON"
if ($LASTEXITCODE -ne 0) { throw "SDL3-UWP CMake configuration failed." }

& $cmake.Source --build $buildDir --config Release
if ($LASTEXITCODE -ne 0) { throw "SDL3-UWP build failed." }

& $cmake.Source --install $buildDir --config Release --prefix $installDir
if ($LASTEXITCODE -ne 0) { throw "SDL3-UWP install failed." }

$builtDll = Get-ChildItem $installDir -Filter "SDL3.dll" -File -Recurse | Select-Object -First 1
if (-not $builtDll) {
    $builtDll = Get-ChildItem $buildDir -Filter "SDL3.dll" -File -Recurse | Select-Object -First 1
}
if (-not $builtDll) { throw "SDL3.dll was not produced by the SDL3-UWP build." }

Copy-Item $builtDll.FullName $dllPath -Force
[IO.File]::WriteAllText($stampPath, $stamp)
Write-Host "SDL3-UWP built OK -> $dllPath"
