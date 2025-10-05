# Hype: The Time Quest Widescreen & FPS fix

## Installation

1. Create a new directory for the game (e.g. C:\Games\Hype). In the following this will be referred to as \<GameDir\>.
2. Copy the folders "DLL", "GameData" and "InstData" from the CD to \<GameDir\>.
3. Copy LangData\XXX\world (XXX is the game's language) from the CD to \<GameDir\>\GameData (there should already be a folder "world", just integrate the new one into it).
4. From the fix, copy Hype.asi, Hype.ini and ubi.ini to \<GameDir\>.
5. Copy the relevant version of  MaiD3Dvr_bleu.exe from the corresponding subfolder of the fix to \<GameDir\>.
   - <ins>English/German</ins>: Version A.
   - <ins>Spanish/Italian/Portuguese/Polish/Dutch</ins>: Version B.
   - <ins>Other languages</ins>: Either version A or version B is likely going to work. If you cannot start a new game or load a savegame, try the other version.
6. Download [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases) (Win32 version of **dinput.dll**), and extract it to \<GameDir\>.
7. Open \<GameDir\>\ubi.ini and change the line `Language=English` to `Language=XXX` (where XXX the same as in step 3). If you skip this step, there won't be any dialogue in-game.
8. Set up a Direct3D wrapper. Instructions for either dgVoodoo or DDrawCompat can be found below.
9. To change resolution or framerate, and for advanced options, edit \<GameDir\>\Hype.ini with a text editor.

## Setup of Direct3D Wrapper

### dgVoodoo 2

1. Download the latest stable version of dgVoodoo from here: https://github.com/dege-diosg/dgVoodoo2/releases
2. Copy dgVoodooCpl.exe & dgVoodoo.conf from the main directory as well as D3DImm.dll & DDraw.dll from the \MS\x86\ subdirectory to \<GameDir\>.
3. Open the configurator dgVoodooCpl.exe. In the "DirectX" tab under "Videocard", select "ATI Radeon 8500" from the dropdown menu. Click "Apply" to save the changes. This fixes some graphical glitches.
4. Still in the "DirectX" tab, under "Miscellaneous", uncheck "dgVoodoo Watermark" to remove the watermark. 
5. (Optional) To be able to Alt+Tab out of the game, do the following:
   - In the dgVoodoo configurator, right-click anywhere in the window and select "Show all sections of the configuration" (activates advanced options).
   - In the "DirectX" tab, under "Behavior", uncheck both both "Application controlled fullscreen/windowed state" and "Disable Alt+Enter to toggle screen state".
   - In the tab "GeneralExt", under "Fullscreen mode attributes", check "Fake".
   - Now you should be able to Alt+Tab out of the game IF you toggle window mode first (by pressing Alt+Enter). Once are back in the game, you can press Alt+Enter again to go back to fake-fullscreen mode.
   
### DDrawCompat
1. Download the latest release of [DDrawCompat](https://github.com/narzoul/DDrawCompat/releases).
2. Copy ddraw.dll to \<GameDir\>.
3. (Optional) To be able to Alt+Tab out of the game, do the following:
   - Download [DDrawCompat.ini](https://github.com/narzoul/DDrawCompat/blob/master/Tools/DDrawCompat.ini) and copy it to \<GameDir\>.
   - Open DDrawCompat.ini with a text editor, uncomment (remove the #) the line starting with "AltTabFix" and change it to `AltTabFix = noactivateapp(1)`.

## Using a game controller
The game natively supports DirectInput devices. Simply set up the controls through the in-game "configure" menu.
To use XInput devices, you can do the following:
1. Download the latest version of [Xidi](https://github.com/samuelgr/Xidi/releases).
2. Find the Win32 version of dinput.dll and rename it to dinputHooked.dll.
3. Copy dinputHooked.dll to \<GameDir\>.

## Known issues

### Double inputs during main menu navigation
This issue also occurs in the vanilla game, but is much more noticeable on high FPS. I have implemented a fix that you can activate by setting `FixDoubleInputs = true` in Hype.ini. Please note that this disables the in-game cheat codes.

### Missing grey HUD border elements
This seems to be an issue with version B (the non English version) of the game and is not related to the widescreen & FPS fix. On the bright side, I personally think that the HUD actually looks better in "centered" mode without these borders... that's why I added the `RemoveHUDBorders` option to the patch (works om both versions of the game, since version B still has parts of the grey borders left).

## FAQ / Troubleshooting

### The game crashes at startup
The most common reasons for this are:
- You are not using a DirectX wrapper (see above for instructions). When the game detects too much video memory, this causes an integer overflow resulting in an infinite loop at startup (the load bar hangs at around 50%). This also happens, if a very large amount of VRAM is selected in the dgVoodoo 2 configurator.
- *TMPFixMemory* was set too low.
- The resolution set in Hype.ini is not natively supported by your system. When the game detects a non-native resolution, it automatically reverts back to 640x480. Since the fix still manipulates the game to run at a higher resolution, this causes some memory corruption and ultimately crashes the game. If you really want to play the game at a non-native resolution, here are instructions for either dgVoodoo or DDrawCompat to "fake" support for other resolutions:
  - **dgVoodoo**: In the dgVoodoo configurator, activate advanced options (*right-click* -> *Show all sections...*). In the tab "DirectXExt", under "Enumerated resolution" after "Extras", enter the custom resolution (e.g. "640x360").
  - **DDrawCompat**: Open DDrawCompat.ini. Uncomment the line starting with "SupportedResolutions" and add the custom resolution (e.g. `SupportedResolutions = native, 640x480, 800x600, 1024x768, 640x360`).
- FMV playback is enabled in ubi.ini (see FAQ item below).

### The game crashes when starting a new game / loading a game / loading a new area
Here are some possible causes:
- *TMPLevelMemory* was set too low.
- Your localized version of the game is not compatible with the chosen version (A or B) of MaiD3Dvr_bleu.exe that comes with the fix. Please try the other one. If your version of the game does not work with the fix at all (i.e. you managed to run the fix with another localized version, but yours doesn't work with either version A or version B of the fix), please open an issue on GitHub.

### FMV playback
Getting FMV playback to work natively within the game engine is quite challenging. Therefore, FMV playback is disabled in \<GameDir\>\ubi.ini. Since there are only **two** FMVs in the entire game – one plays at the start and the other one at the very end – you might as well just watch them manually  (e.g. using VLC player) from \<GameDir\>\Gamedata\Videos\. If you want to try to enable in-game FMV playback anyway, I have listed some possible workarounds [here](https://www.pcgamingwiki.com/wiki/Hype:_The_Time_Quest#FMVs_do_not_play_.2F_game_crashes_at_startup).

### Why is the FPS value set to 63?
The game has a target FPS value hard-coded into its engine, which is normally set to 50 (probably, because it was released in the PAL region). The way that the in-game timing works is that it counts the milliseconds it takes to render a new frame. This value is stored as an integer. So in theory, the game tries to render a new frame every 1000/50 = 20 ms. However, for some reason, it always takes a tiny bit longer, and since the value is stored as an integer (i.e. only whole milliseconds are possible), the game *actually* renders a frame every 21 ms. This results in the vanilla game engine having a framerate of 1000/21 = 47.6 FPS. Now, setting the target FPS value to 62 would result in the game trying to render a new frame every ceil(1000/62) = 17 ms, which would result in an actual framerate of 1000/17 = 58.8 FPS. So to get above 60 FPS, we choose a target FPS value of 63, resulting in a new frame rendered every 16 ms or a framerate of 62.5 FPS.

### What is the PatchDeltaTiming option?
The game uses so-called delta timing to adjust the game's logic to the actual FPS. This way, the game doesn't slow down when the framerate drops and doesn't speed up when the framerate increases. As mentioned in the previous paragraph, the game does not measures its timing in FPS, but in ms per frame. The delta timing normally works, as long as it does not take more than 80 ms to render a frame (= 12.5 FPS) or less than 16 ms (= 62.5 FPS). Should the framerate drop below 12.5 FPS, the game will slow down, and should it rise above 62.5 FPS, the game will speed up. This option replaces the 16 ms floor by just 1 ms. This should, in theory, allow for framerates of up to 1000 FPS. Since a value of less than 16 ms per frame is only reached if the target FPS is set to more than 66, there is no reason to activate this option if TargetFPS is set to a value less than 67. Since I have no clue if this messes up anything else (e.g. jump physics), this feature is *experimental*.