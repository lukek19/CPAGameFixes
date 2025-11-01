# Donald Duck: Goin' Quackers Widescreen & Compatibility Fix

## Installation

**Note**: This fix is intended for the retail version of the game only. DO NOT USE IT WITH ANY PRE-PATCHED EXE FILES.

The official Russian "portable" edition (the one with bugfixes.dll in the game directory) was *not tested*, but should be compatible with the fix. It simply has a few of the same patches hard-coded into the game's exe. In this case, please set `CompatibilityPatch`, `NoCD` and `MakePortable` to `false` (although it shouldn't do any harm to patch the code twice). Please note that the Russian version is not really "portable", as the game stores its savegames and configuration in %appdata%\Disney\DonaldDuck.

1. Copy the folder "Donald" from the CD anywhere on your harddrive (e.g. C:\Games\Donald). In the following this will be referred to as \<GameDir\>.
2. Copy the folder "Data" from the CD to \<GameDir\>.
3. From the fix, copy Donald.asi, Donald.ini & ubi.ini to \<GameDir\>. Also copy \dll\GliDX6vf.dll to \<GameDir\>\dll\.
4. Download [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases) (Win32 version of **dinput.dll**), and extract it to \<GameDir\>.
5. Set up a Direct3D wrapper. Instructions for either dgVoodoo or DDrawCompat can be found below.
6. To change aspect ratio and for advanced options, edit \<GameDir\>\Alex.ini with a text editor.

## Setup of Direct3D Wrapper

### dgVoodoo 2

1. Download the latest stable version of dgVoodoo from here: https://github.com/dege-diosg/dgVoodoo2/releases
2. Copy dgVoodooCpl.exe & dgVoodoo.conf from the main directory as well as D3DImm.dll & DDraw.dll from the \MS\x86\ subdirectory to \<GameDir\>.
3. Open the configurator dgVoodooCpl.exe. In the "DirectX" tab under "Resolution", choose "Desktop". Under "Miscellanneous", uncheck "dgVoodoo Watermark".
4. Click "Apply" to save the changes.

   
### DDrawCompat
1. Download the latest release of [DDrawCompat](https://github.com/narzoul/DDrawCompat/releases). Also Download [DDrawCompat.ini](https://github.com/narzoul/DDrawCompat/blob/master/Tools/DDrawCompat.ini).
2. Copy ddraw.dll and DDrawCompat.ini to \<GameDir\>.
3. Open DDrawCompat.ini with a text editor. Uncomment (remove the "#") the relevant rows and set the following parameters:
	- `AlternatePixelCenter = on`
	- `DisplayAspectRatio = display`
	- `LogLevel = none`
	- `ResolutionScale = display(1)`
   
## Using a game controller
To use XInput devices:
1. Download the latest version of [Xidi](https://github.com/samuelgr/Xidi/releases).
2. Find the Win32 version of dinput.dll and rename it to dinputHooked.dll.
3. Copy dinputHooked.dll to \<GameDir\>.
4. Download the pre-configured [Xidi.ini](https://github.com/samuelgr/XidiGameConfigurations/blob/master/GameConfigurations/Donald%20Duck%20-%20Quack%20Attack/Xidi.ini) and place it in \<GameDir\>. Adjust the the configuration to your own taste.
 

## FAQ / Known Issues

### Weird looking symbols in the top left corner
This is actually not a bug. The symbols are actual HUD elements, stored in this position when they are not needed. At 4:3 aspect ratio they are off-screen, but at wider aspect ratios (which the developers never intended) they become visible. If you press "9" on the keyboard, they will magically teleport to their proper position in the HUD.
To fix this, one would likely need to edit \Data\World\Levels\MapMonde\MapMonde.sna, which is currently out-of-scope for this patch.

### Background elements in 2D sidescrolling levels
In the sidescrolling levels, the 3D background elements will sometimes clip in and out of view. This is likely related to the game's draw distance and I currently do not have a fix for this.

### GliDX6vf.dll
The included file GliDX6vf.dll has been patched to fix the game's internal frame limiter sometimes altering between 60 and 30 FPS. It is almost identical to the one created by Dege (the author of dgVoodoo) for Rayman 2 and Goin' Quackers. However, I have updated it so that it also works with DDrawCompat.

### Disable internal frame limiter
To disable the game's internal frame synchro completely, you may set `DisableInternalSync = true` in Donald.ini. In this case, there is no need to replace the game's original GliDX6vf.dll with the one provided in this fix. Please make sure that you are using an external frame limiter (e.g. dgVoodoo/DDrawCompat's built-in functionality) and limit the framerate to 60 FPS to avoid speed-up and crashes.

### FMVs are too small and stretched
Plese refer to the separate [FMV Troubleshooting Guide](https://github.com/lukek19/CPAGameFixes/blob/main/Donald/FMVTroubleshooting.md).