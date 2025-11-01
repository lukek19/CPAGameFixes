# FMV Troubleshooting Guide

## The FMVs are too small and stretched

The stretching comes from the fact that we run the game at a different aspect ratio.
The fact that the FMVs are too small is because the original FMVs are 400x300, but the game internally runs at a resolution of 640x480, 800x600 or 1024x768.
To fix this, you can convert the FMVs to the desired resolution and add pillarboxes to fix the stretching.
The following instructions show you how you convert the FMVs using the Cinepak codec. This should also fix any issues with Indeo 5 (the original codec) playback, because Microsoft disabled Indeo 5 playback on newer Windows versions due to security converns.

## Convert the FMVs

1. Download [ffmpeg](https://ffmpeg.org/download.html) (the "essentials" build should suffice).
2. Move the four AVI files from \<GameDir\> to the bin subfolder of ffmpeg.
3. Open a command prompt and navigate to the bin subfolder of ffmpeg.
4. The following commands assume you are using the ubi.ini that comes with the fix (which sets an internal resolution of 1024x768) and are playing the game at a 16:9 aspect ratio. For other aspect ratios, see the next section.

- `ffmpeg -i EndGame.avi -vf "scale=768:768,pad=1024:768:128:0:black,format=yuv420p" -c:v cinepak -an EndGame_NEW.avi`
- `ffmpeg -i GameOver.avi -vf "scale=768:768,pad=1024:768:128:0:black,format=yuv420p" -c:v cinepak -an GameOver_NEW.avi`
- `ffmpeg -i IntroGame.avi -vf "scale=768:768,pad=1024:768:128:0:black,format=yuv420p" -c:v cinepak -an IntroGame_NEW.avi`
- `ffmpeg -i Ubi400X300.avi -vf "scale=768:768,pad=1024:768:128:0:black,format=yuv420p" -c:v cinepak -an Ubi400X300_NEW.avi`

*Note*: Conversion to Cinepak is EXTREMELY slow. Luckily, this only needs to be done once. To speed this up slightly, you may open four separate command prompts and run each line in a separate terminal (each CPU core will run one conversion).

5. Delete the "old" files and rename the new ones by removing the suffix "_NEW".
6. Move the new FMV files back to \<GameDir\>.

## Other aspect ratios

**No Pillarboxing** (Use this for 4:3 aspect ratio or if you want the videos stretched)
- `ffmpeg -i XXX.avi -vf "scale=1024:768,format=yuv420p" -c:v cinepak -an XXX_new.avi`

**16:10**
- `ffmpeg -i XXX.avi -vf "scale=682:768,pad=1024:768:171:0:black,format=yuv420p" -c:v cinepak -an XXX_NEW.avi`

**21:9**
- `ffmpeg -i XXX.avi -vf "scale=584:768,pad=1024:768:220:0:black,format=yuv420p" -c:v cinepak -an XXX_NEW.avi`
