/*
 * File H264_VAAPICodec.cpp, from project https://github.com/ebachard/ffmpeg-cpp
 * Author : Eric Bachard  / Thu Apr 23 18:18:20 CEST 2020
 * This file is under GPL v3 license
 * see : http://www.gnu.org/licenses/gpl-3.0.html
 */


#include "H264_VAAPICodec.h"


// WORK IN PROGRESS, NOT COMPLETED !!   (it won't work ... )


namespace ffmpegcpp
{
    //VAProfileH264Main ?

    // CTor
    H264_VAAPICodec::H264_VAAPICodec()
// no idea for Windows case, but certainly something exists already
#ifdef __linux__
	: VideoCodec("h264_vaapi")
#endif
    {
    }

    void H264_VAAPICodec::SetProfile(const char * profile)
    {
        SetOption("profile", profile);  // vainfo will return lot of things ...
    }

    void H264_VAAPICodec::SetTune(const char * tune)
    {
        SetOption("tune", tune);
    }

    void H264_VAAPICodec::SetPreset(const char * preset)
    {
        SetOption("preset", preset);
    }

    void H264_VAAPICodec::SetCrf(int crf)
    {
        SetOption("crf", crf);
    }
}

