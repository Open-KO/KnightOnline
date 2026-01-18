#include "pch.h"
#include "DateTime.h"

DateTime::DateTime() : _tm {}, _unixTimestamp(0)
{
}

DateTime::DateTime(time_t timestamp) : DateTime()
{
	Set(timestamp);
}

DateTime::DateTime(int year, int month, int day, int hour /*= 0*/, int minute /*= 0*/,
	int second /*= 0*/) : DateTime()
{
	Set(year, month, day, hour, minute, second);
}

DateTime::DateTime(tm* _tm) : DateTime()
{
	Set(_tm);
}

DateTime::DateTime(const DateTime& other) : DateTime()
{
	Set(other);
}

void DateTime::Set(time_t timestamp)
{
#ifdef _MSC_VER
	localtime_s(&_tm, &timestamp);
#else
	_tm = *localtime(&timestamp);
#endif
	_unixTimestamp = timestamp;
}

void DateTime::Set(tm* tm_)
{
	_tm            = *tm_;
	_unixTimestamp = mktime(tm_);
}

void DateTime::Set(const DateTime& other)
{
	_tm            = other._tm;
	_unixTimestamp = other._unixTimestamp;
}

void DateTime::Set(
	int year, int month, int day, int hour /*= 0*/, int minute /*= 0*/, int second /*= 0*/)
{
	memset(&_tm, 0, sizeof(_tm));

	// Now update it with the data specified
	_tm.tm_year = year - TM_YEAR_BASE;
	_tm.tm_mon  = month - 1;
	_tm.tm_mday = day;
	_tm.tm_hour = hour;
	_tm.tm_min  = minute;
	_tm.tm_sec  = second;

	// Finally reconstruct it, so the other data is updated.
	Update();
}

void DateTime::AddYears(int years)
{
	_tm.tm_year += years;
	Update();
}

void DateTime::SetYear(int year)
{
	_tm.tm_year = year - TM_YEAR_BASE;
	Update();
}

void DateTime::AddMonths(int months)
{
	_tm.tm_mon += months;
	Update();
}

void DateTime::SetMonth(int month)
{
	_tm.tm_mon = month - 1;
	Update();
}

void DateTime::AddWeeks(int weeks)
{
	AddDays(weeks * DAYS_IN_WEEK);
}

void DateTime::AddDays(int days)
{
	_tm.tm_mday += days;
	Update();
}

void DateTime::SetDay(int day)
{
	_tm.tm_mday = day;
	Update();
}

void DateTime::AddHours(int hours)
{
	_tm.tm_hour += hours;
	Update();
}

void DateTime::SetHour(int hour)
{
	_tm.tm_hour = hour;
	Update();
}

void DateTime::AddMinutes(int minutes)
{
	_tm.tm_min += minutes;
	Update();
}

void DateTime::SetMinute(int minute)
{
	_tm.tm_min = minute;
	Update();
}

void DateTime::AddSeconds(int seconds)
{
	_tm.tm_sec += seconds;
	Update();
}

void DateTime::SetSecond(int second)
{
	_tm.tm_sec = second;
	Update();
}

DateTime DateTime::GetNow()
{
	return DateTime(time(nullptr));
}

void DateTime::Update()
{
	_unixTimestamp = mktime(&_tm);
}
