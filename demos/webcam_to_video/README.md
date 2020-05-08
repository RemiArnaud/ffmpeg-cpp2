Eric Bachard, 2020/05/08


This is experimental. Please do not use: you can damage your machine, and maybe lose your data.


Current status :

- works with mjpeg webcams like Logitech webcams (C920, C922, Brio stream models, other are untested)
- all produce vidéos, that you can read with mpv , ffplay or any video reader

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

