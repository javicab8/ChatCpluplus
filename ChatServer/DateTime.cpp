#include "DateTime.h"

std::string DateTime::getCurrentDateTimeString()
{
	const auto now = std::chrono::system_clock::now();
	time_t t = std::chrono::system_clock::to_time_t(now);
	struct tm tm;
	localtime_s(&tm, &t);
	char str[26];
	strftime(str, sizeof str, "%D %T", &tm);
	return str;
}
