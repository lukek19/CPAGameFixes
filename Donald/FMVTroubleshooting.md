# FMV Troubleshooting Guide

If the game's FMVs are not playing, this is likely related to the Indeo 5 codec. Microsoft has partially disabled Indeo 5 playback due to security concerns. If FMVs are not playing for you, please choose *either* of the following fixes.

## Convert the FMVs to the more compatible Cinepak format

1. Download [ffmpeg](https://ffmpeg.org/download.html) (the "essentials" build should suffice).
2. Move the four AVI files from \<GameDir\> to the bin subfolder of ffmpeg.
3. Open a command prompt and navigate to the bin subfolder of ffmpeg.
4. Run the following commands:

- `ffmpeg -i "EndGame.avi" -c:a copy -c:v cinepak "EndGame_NEW.avi"`
- `ffmpeg -i "GameOver.avi" -c:a copy -c:v cinepak "GameOver_NEW.avi"`
- `ffmpeg -i "IntroGame.avi" -c:a copy -c:v cinepak "IntroGame_NEW.avi"`
- `ffmpeg -i "Ubi400X300.avi" -c:a copy -c:v cinepak "Ubi400X300_NEW.avi"`

*Note*: The conversion may take a considerable amount of time! You can slightly speed this up by opening four separate command prompts and run each line in a separate terminal (each CPU core will run one conversion).

5. Delete the "old" files and rename the new ones by removing the suffix "_NEW".
6. Copy the new FMV files back to \<GameDir\>.

## Re-enable the Indeo 5 codec
Follow the instructions [here](https://www.pcgamingwiki.com/wiki/Windows#Missing.2Fconflicting_codecs).  *Warning*: After playing the game, the codec should be disabled again, due to the security issues. 

## Disable FMVs
Simply delete or rename the four AVI files in \<GameDir\>. This will disable FMV playback.