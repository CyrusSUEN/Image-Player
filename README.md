# Image-Player

To use:


1. Download and extract openframeworks 0.9 (not compatible with 0.8.4 or older)
http://forum.openframeworks.cc/t/0-9-0-release-candidate-1/20434

2. Download and extract [ofxVideoRecorder](https://github.com/timscaffidi/ofxVideoRecorder) to ```of_v0.9.0RC1_osx_release/addons```

3. ```$ cd of_v0.9.0RC1_osx_release/apps/myApps/``` or any custom directory within openFrameworks library with the same depth

4. ```$ git clone https://github.com/CyrusSUEN/Image-Player.git```

5. install ffmpeg for video recording dependency
```$ brew install ffmpeg```

6. install WavTap to capture sound from OS X for audio recording dependency.
If using 10.10 Yosemitee, before install WavTap need to bypass the security measure by  
```$ sudo nvram boot-args=kext-dev-mode=1```
then download and install from the [installer](https://github.com/pje/WavTap/releases/download/0.3.0/WavTap.0.3.0.pkg).
Make sure that WavTap is running in the background (use Option + Click the speaker icon to check that WavTap is the selected Output Device)

7. Use Xcode to open the project file ```imagePlayer.xcodeproj``` in the Git project directory 
