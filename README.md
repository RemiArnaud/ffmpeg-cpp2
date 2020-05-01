Another study of ffmpeg in C++  FORKED FROM [Raveler/ffmpeg-cpp](https://github.com/Raveler/ffmpeg-cpp)

**BE CAREFULL : ALL CHANGES ARE NOW UNDER GPL V3** (file an issue if you need something) [@ebachard](https://github.com/ebachard) 2020/04/22


BELOW, THE ORIGINAL TEXT


This is the Linux port. Maybe the Windows version works, but I'm unsure, and I'd suggest you to go to the Raveler page ( [Raveler/ffmpeg-cpp](https://github.com/Raveler/ffmpeg-cpp) )


# ffmpeg-cpp
A clean C++ wrapper around the ffmpeg libraries which can be used in any C++ project or C# project (with DllImport or CLR). Makes the most commonly used functionality of FFmpeg easily available for any C++ projects with an easy-to-use interface. The full power of FFmpeg compacted in 10 lines of C++ code: if this sounds useful to you, read on!

# Installation

## Windows

Here is the Linux port. Maybe the Windows version works, but I'm unsure ...

If you want to use the Windows version, I'd suggest you to go to the Raveler page instead:

 **( [Raveler/ffmpeg-cpp](https://github.com/Raveler/ffmpeg-cpp) )**


## Linux

I created a CMakefile from scratch, and the lib is now built straight away. I'll probably add some Makefiles in the demo directory, to test every case (muxer, audio extraction, and so on).

## What works on Linux :
- [x] create the makefile (the usual way)
- [x] make is ok. TODO : fix a bunch of warnings, caused by a prehistoric version of ffmpeg at design time.
- [x] libffmpeg-cpp.a is created
- [x] one can link it
- [x] FFMPEG filters seems to work well, excepted the hwaccels one (WIP)

## Demos

Below, the example tested working on Linux (Intel x86_64, kernel4.15.x) LinuxMint, but whatever the distribution, this should work anywhere

- [x] decode_audio: **works OK**. The audio part is extracted as a data blob.
- [x] decode_video: **works OK** ! big_buck_bunny.mp4 results in 1440 grey images (who can be assembled to retrieve the video)
- [x] encode_audio: **works OK** //**don't forget to install libmp3lame AND to add --enable-libmp3lame at configure time with FFMPEG!**
- [x] encode_video: **works OK**. output.mpg created and works (no sound though, as expected)
- [x] remuxing // **works OK** **don't forget to add --enable-libvpx add configure time with FFMPEG**, and it will work too !!
- [x] print_info  //  **works OK**
- [x] difference //  **works too !**
- [ ] simple interface (WIP, not really Linux since it seems to be Windows specific)
- [x] simple interface demo // **works OK too : video rotated in the counter clock sense**  e.g. : watching a smartphone video
- [x] demo: **works OK**. Testing multimix with other sources, seems to be OK !
- [x] **improved the remuxing demo**
- [x] **improved the decode_video demo**  (MJPEG : not working when using hardware decoder, but some progress done)

## TODO (Linux)

- [x] DONE fix undefined behaviour with delete [done]
- [x] fix all FFMPEG warnings (codec use is deprecated since .... )
- [x] fix all other warnings : unused variables and more
- [ ] create a list of what is currently doable
- [ ] create some patterns, and a list of commonly used video codecs (vp9 / h264 / hevc / mpeg4 / mjpeg and some other)
- [ ] create some patterns, and a list of commonly used audio codecs
- [ ] make hwdecoders / encoders work [vaapi, drm in priority]
- [ ] (more to come)
# Usage 

## Linux

- [ ] (very soon, WIP)
- [ ] (more)


##  Linux build


FIRST : install all dependencies : 

g++ (7.x or superior)
cmake
ffmpeg : version 4.1+
libavutils-dev libavformat-dev libavcodec-dev libavfilter-dev libswresample-dev libswscale-dev (and dependencies)
libmp3lame-dev libmp3lame0 libv4l-dev v4l-utils libx264-dev libvpx-dev

optional : 

- libva (and dependencies)
- v4l2loopback-source  v4l2loopback-utils)

To build :

git clone https://framagit.org/ericb/ffmpeg-cpp2.git
cd ffmpeg-cpp2
mkdir build
cd build
cmake ..
make
=> if nothing wrong occurs, the libffmpeg-cpp.a (static) should be built

To test :

cd ../demos
choose one folder. e.g. : Remuxing  => cd Remuxing

./compile_me.sh

=> ./remuxing 

Have Fun !





## C++

To give you an idea, this code will load a video stream from a container, filter it, and write it back out to another container:

```C++
// Create a muxer that will output the video as MP4.
Muxer* muxer = new Muxer("filtered_video.mp4");

// Create a MPEG2 codec that will encode the raw data.
VideoCodec* codec = new VideoCodec(AV_CODEC_ID_MPEG2VIDEO);

// Create an encoder that will encode the raw audio data using the codec specified above.
// Tie it to the muxer so it will be written to file.
VideoEncoder* encoder = new VideoEncoder(codec, muxer);

// Create a video filter and do some funny stuff with the video data.
Filter* filter = new Filter("scale=640:150,transpose=cclock,vignette", encoder);

// Load a container. Pick the best video stream container in the container
// And send it to the filter.
Demuxer* demuxer = new Demuxer("big_buck_bunny.mp4");
demuxer->DecodeBestVideoStream(filter);

// Prepare the output pipeline.
// This will decode a small amount of frames so the pipeline can configure itself.
demuxer->PreparePipeline();

// Push all the remaining frames through.
while (!demuxer->IsDone())
{
	demuxer->Step();
}
		
// Save everything to disk by closing the muxer.
muxer->Close();
```

If you use the included simple-interface library, which only supports a subset of the full library, using ffmpeg-cpp becomes even easier:

```
#include "SimpleInterface.h"

int main()
{
	void* handle = ffmpegCppCreate("out.mp4");
	ffmpegCppAddVideoStream(handle, "samples/big_buck_bunny.mp4");
	ffmpegCppAddVideoFilter(handle, "transpose=cclock[middle];[middle]vignette");
	ffmpegCppAddAudioStream(handle, "samples/big_buck_bunny.mp4");
	ffmpegCppGenerate(handle);
	ffmpegCppClose(handle);
}
```

## C\#

See   **( [Raveler/ffmpeg-cpp](https://github.com/Raveler/ffmpeg-cpp) )**



# License

This fork will be under GPL V3 for a while. Later probably, it will be back to the LGPL.

If you want the LGPL version, please go to the Raveler github repository (see above).

Please note though that FFmpeg, which you will need to build this library, is not. Depending on how you build it, it is either LGPL or GPL. So if you use the GPL-version of FFmpeg in your project, this library will be GPL too.
