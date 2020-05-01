#include "FFmpegException.h"

using namespace std;

namespace ffmpegcpp
{
	FFmpegException::FFmpegException(const char * p_error) : exception ()
	{
                std::string(p_error).c_str();
	}

	FFmpegException::FFmpegException(const char * p_error, int returnValue)
		: exception()
	{
			std::string(std::string(p_error) + ": " + av_make_error_string(this->error, AV_ERROR_MAX_STRING_SIZE, returnValue)).c_str();
	}
}

/*
namespace ffmpegcpp
{
	FFmpegException::FFmpegException(const char * error) : exception ()
	{
	}

	FFmpegException::FFmpegException(const char * error, int returnValue)
		: exception(
			std::string(std::string(error) + ": " + av_make_error_string(this->error, AV_ERROR_MAX_STRING_SIZE, returnValue)).c_str(), returnValue )
	{
	}
}
*/