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
10. To change resolution and for advanced options, edit \<GameDir\>\Alex.ini with a text editor. For resolutions above Full HD (1920x1080), see the instructions below.

## Setup of Direct3D Wrapper

### dgVoodoo 2

1. Download the latest stable version of dgVoodoo from here: https://github.com/dege-diosg/dgVoodoo2/releases
2. Copy dgVoodooCpl.exe & dgVoodoo.conf from the main directory as well as D3DImm.dll & DDraw.dll from the \MS\x86\ subdirectory to \<GameDir\>.
3. Open the configurator dgVoodooCpl.exe. In the "DirectX" tab under "Videocard", select "ATI Radeon 8500" from the dropdown menu. Click "Apply" to save the changes. This may fix some graphical glitches.
4. Still in the "DirectX" tab, under "Miscellaneous", uncheck "dgVoodoo Watermark" to remove the watermark. 
   
### DDrawCompat
1. Download the latest release of [DDrawCompat](https://github.com/narzoul/DDrawCompat/releases).
2. Copy ddraw.dll to \<GameDir\>.
3. (Optional) For better alt+tab support:
   - Download [DDrawCompat.ini](https://github.com/narzoul/DDrawCompat/blob/master/Tools/DDrawCompat.ini) and copy it to \<GameDir\>.
   - Open DDrawCompat.ini with a text editor, uncomment (remove the #) the line starting with "AltTabFix" and change it to `AltTabFix = noactivateapp(1)`.
   
## Resolutions above Full HD (> 1920x1080)
If the resolution in Alex.ini is set too high (e.g. 2560x1440), the game will crash upon pressing escape and exiting the menu. This has something to do with the game creating a screenshot for the savegame preview image, which likely causes some kind of memory corruption due to insufficiently allocated memory.
If you still want to play at high resolutions, please choose **either** of the following two workarounds:

### (A) Recommended: Set a lower resolution in Alex.ini and scale with the Direct3D wrapper
In Alex.ini, set the resolution to a smaller value with the same aspect ratio. For example, if you want to play at 2560x1440, you may choose 1280x720 (both are 16:9) in Alex.ini. **Hint**: Make sure that `xRes` is at least 640 and `yRes` is at least 480 to avoid down-scaling of the background images.
Next, configure the Direct3D wrapper to scale up to the desired resolution.

*dgVoodoo 2*:
1. Run dgVoodooCpl.exe. In the "DirectX" tab under "Resolution", select "Desktop".
2. *If your system does not natively support the resolution set in Alex.ini or if you are unsure*: Right-click anywhere in the dgVoodoo configurator and select "Show all sections of the configuration" (activates advanced options). In the tab "DirectXExt", under "Enumerated resolution" after "Extras", enter the resolution from Alex.ini (e.g. `1280x720`).

*dDDrawCompat*:
1. Open DDrawCompat.ini. Uncomment (remove #) the line *ResolutionScale* and set `ResolutionScale = display(1)`.
2. *If your system does not natively support the resolution set in Alex.ini or if you are unsure*: Uncomment (remove #) the line *SupportedResolutions* and add the resolution from Alex.ini, e.g. `SupportedResolutions = native, 640x480, 800x600, 1024x768, 1280x720`.

### (B) Disable savegame snapshots
In Alex.ini, set `DisableSaveGameSnapshots = true`. This will prevent the crashes, but will result in the savegame preview images being black.

## FAQ / Troubleshooting

### The game crashes at startup
The most common reasons for this are:
- You are not using a DirectX wrapper (see above for instructions). When the game detects too much video memory, this causes an integer overflow resulting in an infinite loop at startup (the load bar hangs at around 50%). This also happens, if a very large amount of VRAM is selected in the dgVoodoo 2 configurator.
- The resolution set in Alex.ini is not natively supported by your system. When the game detects a non-native resolution, it automatically reverts back to 640x480. Since the fix still manipulates the game to run at a higher resolution, this causes some memory corruption and ultimately crashes the game. Please see the section "Resolutions above Full HD" to fix this.
- You are using the Pentium III EXE with the original version or vice versa. Please choose the right EXE for your version.

### The game crashes after exiting the menu / after the introductory cutscene
A high resolution (> 1920x1080) was set in Alex.ini. Please see the section "Resolutions above Full HD".