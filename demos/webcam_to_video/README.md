Eric Bachard, 2020/05/08


This is experimental. Please do not use: you can damage your machine, and maybe lose your data.

Current status :

- works with mjpeg webcams like Logitech webcams (C920, C922, Brio stream models, other are untested)
- don't forget /dev/video0 is used (adapt to your devices if you need to select another webcam)
- all binaries should produce vidéos, that can be read by mpv, ffplay or any video reader
- VP9 ha not been implemented yet (WIP)
- the machine load is important, precisely for h264 encoding. Due to hw implementation not ready.

To build : 

chmod ug+x compile_me.sh
./compile_me.sh



If nothing's wrong (e.g. you built ffmpeg libraries with --enable-libmp3-lame --enable-libx264 --enable-gpl,
AND you installed everything, it should work.

If something goes wrong, please file an issue !

Known issues :

- no sound yet, but should work 
- filters are untested
- timings are wrong (probably due to fantaisist use of frameRate that I need to learn)

