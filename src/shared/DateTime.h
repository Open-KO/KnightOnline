#ifndef SHARED_DATETIME_H
#define SHARED_DATETIME_H

#pragma once

#include <ctime>

class DateTime
{
public:
	static constexpr int TM_YEAR_BASE = 1900;
	static constexpr int DAYS_IN_WEEK = 7;

	// Constructs an empty date/time
	DateTime();

	// Constructs a date/time using the timestamp specified
	DateTime(time_t timestamp);

	// Constructs a date/time using the specified date parts.
	DateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0);

	// Constructs a date/time from the specified time struct
	DateTime(tm* _tm);

	// Constructs a date/time from the specified DateTime
	DateTime(const DateTime& other);

	// Uses the timestamp specified
	void Set(time_t timestamp);

	// Uses the specified time struct
	void Set(tm* tm_);

	// Uses the specified DateTime
	void Set(const DateTime& other);

	// Constructs a date/time using the specified date parts.
	void Set(int year, int month, int day, int hour = 0, int minute = 0, int second = 0);

	int GetYear() const
	{
		return (_tm.tm_year + TM_YEAR_BASE);
	}

	int GetMonth() const
	{
		return (_tm.tm_mon + 1);
	}

	int GetDay() const
	{
		return _tm.tm_mday;
	}

	int GetDayOfWeek() const
	{
		return _tm.tm_wday;
	}

	int GetHour() const
	{
		return _tm.tm_hour;
	}

	int GetMinute() const
	{
		return _tm.tm_min;
	}

	int GetSecond() const
	{
		return _tm.tm_sec;
	}

	time_t GetUnixTimestamp() const
	{
		return _unixTimestamp;
	}

	// NOTE: If any of these overflow, they'll be handled by mktime() accordingly.
	// This makes our life *much* easier; date/time logic is not pretty.

	void AddYears(int years);
	void SetYear(int year);
	void AddMonths(int months);
	void SetMonth(int month);
	void AddWeeks(int weeks);
	void AddDays(int days);
	void SetDay(int day);
	void AddHours(int hours);
	void SetHour(int hour);
	void AddMinutes(int minutes);
	void SetMinute(int minute);
	void AddSeconds(int seconds);
	void SetSecond(int second);

	static DateTime GetNow();

private:
	void Update();

private:
	tm _tm;
	time_t _unixTimestamp;
};

#endif // SHARED_DATETIME_H
