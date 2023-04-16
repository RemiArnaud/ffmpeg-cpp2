#pragma once

#include "ffmpeg.h"
#include <stdexcept>
#include <string>

namespace ffmpegcpp
{
	class FFmpegException : public std::exception
	{
	public:
		explicit FFmpegException(const char * error);
		explicit FFmpegException(const char * error, int returnValue);

		virtual const char* what() const noexcept
		{
            return m_error;
		}
	private:
		char m_error[AV_ERROR_MAX_STRING_SIZE];
	};
}

/*
namespace ffmpegcpp
{
	class FFmpegException : public std::exception
	{
	    public:
		FFmpegException(const char * error);

		FFmpegException(const char * error, int returnValue);

		virtual const char* what() const noexcept
		{
			return (const char *)error;
		}
//	    private:
		char error[AV_ERROR_MAX_STRING_SIZE];
	};
}
*/
