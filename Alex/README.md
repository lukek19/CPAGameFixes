# Alex Builds His Farm Widescreen Fix

## Installation

1. Create a new directory for the game (e.g. C:\Games\Alex). In the following this will be referred to as \<GameDir\>.
2. Copy the folders "DLL" and "GameData" and from the CD to \<GameDir\>.
3. Copy LangData\XXX\World (XXX is the desired language) from the CD to \<GameDir\>\GameData (there should already be a folder "World", just integrate the new one into it).
4. *Only Non-Pentium III Edition*: Copy (or move) the folder \<GameDir\>\gamedata\world\soundMemory\XXX (XXX is the desired language) to \<GameDir\>\gamedata\world\Sound (again, there should already be a folder XXX, just integrate it).
5. From the fix, copy Alex.asi and Alex.ini \<GameDir\>.
6. Depending on whether you have the original version or the Pentium III version of the game, copy Alex_D3D.exe/Alex.exe and ubi.ini from the corresponding subfolder to \<GameDir\>.
*Note*: Some international releases of the game do not have SafeDisc encrypted EXEs. In this case, you may also use your original EXE from the exe\d3d folder on your CD. The EXE is not encrypted if there is no ICD file in the folder and the size of the EXE is ~2MB/2.5MB.
7. Download [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases) (Win32 version of **dinput.dll**), and extract it to \<GameDir\>.
8. Open \<GameDir\>\ubi.ini and change the line `Language=English` to `Language=XXX` (where XXX the same as in step 3).
9. Set up a Direct3D wrapper. Instructions for either dgVoodoo or DDrawCompat can be found below.
10. To change resolution and for advanced options, edit \<GameDir\>\Alex.ini with a text editor.

## Setup of Direct3D Wrapper

### dgVoodoo 2

1. Download the latest stable version of dgVoodoo from here: https://github.com/dege-diosg/dgVoodoo2/releases
2. Copy dgVoodooCpl.exe & dgVoodoo.conf from the main directory as well as D3DImm.dll & DDraw.dll from the \MS\x86\ subdirectory to \<GameDir\>.
3. Open the configurator dgVoodooCpl.exe. In the "DirectX" tab under "Videocard", select "ATI Radeon 8500" from the dropdown menu. Click "Apply" to save the changes. This may fix some graphical glitches.
4. Still in the "DirectX" tab, under "Miscellaneous", uncheck "dgVoodoo Watermark" to remove the watermark. 
   
### DDrawCompat
1. Download the latest release of [DDrawCompat](https://github.com/narzoul/DDrawCompat/releases).
2. Copy ddraw.dll to \<GameDir\>.
3. (Optional) To be able to Alt+Tab out of the game, do the following:
   - Download [DDrawCompat.ini](https://github.com/narzoul/DDrawCompat/blob/master/Tools/DDrawCompat.ini) and copy it to \<GameDir\>.
   - Open DDrawCompat.ini with a text editor, uncomment (remove the #) the line starting with "AltTabFix" and change it to `AltTabFix = noactivateapp(1)`.

## FAQ / Troubleshooting

### The game crashes at startup
The most common reasons for this are:
- You are not using a DirectX wrapper (see above for instructions). When the game detects too much video memory, this causes an integer overflow resulting in an infinite loop at startup (the load bar hangs at around 50%). This also happens, if a very large amount of VRAM is selected in the dgVoodoo 2 configurator.
- *TMPFixMemory* was set too low. Try a larger value in Alex.ini.
- The resolution set in Alex.ini is not natively supported by your system. When the game detects a non-native resolution, it automatically reverts back to 640x480. Since the fix still manipulates the game to run at a higher resolution, this causes some memory corruption and ultimately crashes the game. If you really want to play the game at a non-native resolution, here are instructions for either dgVoodoo or DDrawCompat to "fake" support for other resolutions:
  - **dgVoodoo**: In the dgVoodoo configurator, activate advanced options (*right-click* -> *Show all sections...*). In the tab "DirectXExt", under "Enumerated resolution" after "Extras", enter the custom resolution (e.g. "640x360").
  - **DDrawCompat**: Open DDrawCompat.ini. Uncomment the line starting with "SupportedResolutions" and add the custom resolution (e.g. `SupportedResolutions = native, 640x480, 800x600, 1024x768, 640x360`).
- You are using the Pentium III EXE with the original version or vice versa. Please choose the right EXE for your version.

### The game crashes when starting a new game / loading a game / loading a new area
Here are some possible causes:
- *TMPLevelMemory* was set too low. Try a larger value in Alex.ini.