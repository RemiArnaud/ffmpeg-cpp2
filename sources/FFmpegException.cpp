#include "FFmpegException.h"

using namespace std;

namespace ffmpegcpp
{
    FFmpegException::FFmpegException(const char * error) : exception ()
	{
        size_t sz = strnlen(error, AV_ERROR_MAX_STRING_SIZE-1);
        strncpy(m_error, error, sz);
        m_error[sz] = '\0';
	}

    FFmpegException::FFmpegException(const char * error, int returnValue)
		: exception()
	{
        string s = string(string(error) + ": " + av_make_error_string(m_error, AV_ERROR_MAX_STRING_SIZE, returnValue));
        size_t sz = strnlen(s.c_str(), AV_ERROR_MAX_STRING_SIZE-1);
        strncpy(m_error, s.c_str(), sz);
        m_error[sz] = '\0';
    }
}
