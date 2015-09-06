# Image-Player

To use:


1. Download openframeworks 0.9 (not compatible with 0.8.4 or older)
http://forum.openframeworks.cc/t/0-9-0-release-candidate-1/20434

2. ```$ cd of_v0.9.0RC1_osx_release/apps/myApps/``` or any custom directory within openFrameworks library with the same depth
3. ```$ git clone https://github.com/dhowe/Image-Player.git```

4. install ffmpeg for video recording dependency
```$ brew install ffmpeg```

5. install WavTap to capture sound from OS X for audio recording dependency.
If using 10.10 Yosemitee, before install WavTap need to bypass the security measure by  
```$ sudo nvram boot-args=kext-dev-mode=1```
then download and install from the [installer](https://github.com/pje/WavTap/releases/download/0.3.0/WavTap.0.3.0.pkg).
Make sure that WavTap is running in the background (use Option + Click the speaker icon to check that WavTap is the selected Output Device)

6. Use Xcode to open the project file
