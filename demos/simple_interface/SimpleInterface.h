// MathLibrary.h - Contains declarations of math functions
#pragma once

#ifdef __linux__
#define DllExport
#else
#define DllExport __declspec(dllexport)
#endif

extern DllExport void * ffmpegCppCreate(const char * outputFileName);

extern DllExport void ffmpegCppAddVideoStream(void * handle, const char * videoFileName);
extern DllExport void ffmpegCppAddAudioStream(void * handle, const char * audioFileName);

extern DllExport void ffmpegCppAddVideoFilter(void * handle, const char * filterString);
extern DllExport void ffmpegCppAddAudioFilter(void * handle, const char * filterString);

extern DllExport void ffmpegCppGenerate(void * handle);

extern DllExport bool ffmpegCppIsError(void * handle);
extern DllExport const char * ffmpegCppGetError(void * handle);

extern DllExport void ffmpegCppClose(void* handle);
