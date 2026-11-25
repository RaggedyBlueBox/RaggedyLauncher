param(
    [Parameter(Mandatory = $true)][string]$SourceRoot
)
$ErrorActionPreference = "Stop"
$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
function Read-Sdl { param([string]$Path) return ([System.IO.File]::ReadAllText($Path) -replace "`r`n", "`n") }
function Write-Sdl { param([string]$Path,[string]$Text) [System.IO.File]::WriteAllText($Path, $Text, $Utf8NoBom) }
function Replace-SdlOnce {
    param([string]$Text,[string]$Old,[string]$New,[string]$Label)
    $i=$Text.IndexOf($Old,[System.StringComparison]::Ordinal)
    if($i -lt 0){ throw "V18 SDL patch anchor not found: $Label" }
    if($Text.IndexOf($Old,$i+$Old.Length,[System.StringComparison]::Ordinal) -ge 0){ throw "V18 SDL patch non-unique anchor: $Label" }
    return $Text.Substring(0,$i) + $New + $Text.Substring($i+$Old.Length)
}
$videoPath = Join-Path $SourceRoot 'src\video\winrt\SDL_winrtvideo.cpp'
$appPath = Join-Path $SourceRoot 'src\core\winrt\SDL_winrtapp_direct3d.cpp'
foreach($p in @($videoPath,$appPath)){
    if(-not(Test-Path -LiteralPath $p)){throw "Missing SDL WinRT source for V18: $p"}
}
$video=Read-Sdl $videoPath
$app=Read-Sdl $appPath
$old=@'
extern "C" __declspec(dllexport) int BanditSDL_WinRTSoftwareCursorV17(void)
{
    return 17;
}
'@
$new=@'
extern "C" __declspec(dllexport) int BanditSDL_WinRTSoftwareCursorV17(void)
{
    return 17;
}

extern "C" __declspec(dllexport) int BanditSDL_WinRTBackKeyContractV18(void)
{
    return 18;
}
'@
$video=Replace-SdlOnce $video $old $new 'V18 runtime marker'

$old=@'
template <typename BackButtonEventArgs>
static void WINRT_OnBackButtonPressed(BackButtonEventArgs ^ args)
{
    SDL_SendKeyboardKey(0, SDL_GLOBAL_KEYBOARD_ID, 0, SDL_SCANCODE_AC_BACK, true);
    SDL_SendKeyboardKey(0, SDL_GLOBAL_KEYBOARD_ID, 0, SDL_SCANCODE_AC_BACK, false);

    if (SDL_GetHintBoolean(SDL_HINT_WINRT_HANDLE_BACK_BUTTON, false)) {
        args->Handled = true;
    }
}
'@
$new=@'
template <typename BackButtonEventArgs>
static void WINRT_OnBackButtonPressed(BackButtonEventArgs ^ args)
{
    static bool banditBackKeyContractLogged = false;
    if (!banditBackKeyContractLogged) {
        banditBackKeyContractLogged = true;
        SDL_Log("Bandit SDL V18: BackRequested stays gamepad B; synthetic AC_BACK suppressed");
    }

    if (SDL_GetHintBoolean(SDL_HINT_WINRT_HANDLE_BACK_BUTTON, false)) {
        args->Handled = true;
    }
}
'@
$app=Replace-SdlOnce $app $old $new 'BackRequested synthetic AC_BACK removal'
if($app.Contains('SDL_SCANCODE_AC_BACK')){ throw 'V18 SDL patch left a synthetic AC_BACK keyboard path' }
Write-Sdl $videoPath $video
Write-Sdl $appPath $app
Write-Host 'Applied V18 back-key contract patch: gamepad B no longer injects AC_BACK keyboard input.'
