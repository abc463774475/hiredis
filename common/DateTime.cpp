#include "DateTime.h"
#include <string.h>
#include <stdio.h>

DateTime::DateTime (time_t t)
{
    _dtime = *localtime(&t);
}

time_t DateTime::getTimeValue ()
{
    return mktime (&_dtime);
}

void DateTime::setTimeValue (time_t t)
{
    _dtime = *localtime (&t);
}

int64_t DateTime::operator - (const DateTime &datetime)
{
    tm t_tm = datetime._dtime;
    return (int64_t)difftime (mktime(&_dtime), mktime(&t_tm));
}

int64_t DateTime::diffSecond (const DateTime &datetime)
{
    tm t_tm = datetime._dtime;
    return (int64_t)difftime (mktime(&_dtime), mktime(&t_tm));
}

int64_t DateTime::diffMinute (const DateTime &datetime)
{
    tm t_tm = datetime._dtime;
    return ((int64_t)difftime (mktime(&_dtime), mktime(&t_tm)))/60;
}

int64_t DateTime::diffHour (const DateTime &datetime)
{
    tm t_tm = datetime._dtime;
    return ((int64_t)difftime (mktime(&_dtime), mktime(&t_tm)))/(60*60);
}

int64_t DateTime::diffDay (const DateTime & datetime)
{
    tm t_tm = datetime._dtime;
    return ((int64_t)difftime (mktime(&_dtime), mktime(&t_tm)))/(60*60*24);
}

int64_t DateTime::diffWeek (const DateTime & datetime)
{
    tm t_tm = datetime._dtime;
    return ((int64_t)difftime(mktime(&_dtime), mktime(&t_tm)))/(60*60*24*7);
}

int32_t DateTime::diffMonth (const DateTime & datetime)
{
    return (_dtime.tm_year - datetime._dtime.tm_year) * 12 + (_dtime.tm_mon - datetime._dtime.tm_mon);
}

int32_t DateTime::diffYear (const DateTime & datetime)
{
    return _dtime.tm_year - datetime._dtime.tm_year;
}

DateTime & DateTime::incYear (int32_t year)
{
    _dtime.tm_year += year;
    _dtime.tm_isdst = -1;
    mktime(&_dtime);
    return *this;
}

DateTime & DateTime::incMonth (int32_t month)
{
    _dtime.tm_mon += month;
    _dtime.tm_isdst = -1;
    mktime(&_dtime);
    return *this;
}

DateTime & DateTime::incDay (int32_t day)
{
    _dtime.tm_mday += day;
    _dtime.tm_isdst = -1;
    mktime(&_dtime);
    return *this;
}

DateTime & DateTime::incHour (int32_t hour)
{
    _dtime.tm_hour += hour;
    _dtime.tm_isdst = -1;
    mktime(&_dtime);
    return *this;
}

DateTime & DateTime::incMinute (int32_t minute)
{
    _dtime.tm_min += minute;
    _dtime.tm_isdst = -1;
    mktime(&_dtime);
    return *this;
}

DateTime & DateTime::incSecond (int32_t second)
{
    _dtime.tm_sec += second;
    _dtime.tm_isdst = -1;
    mktime(&_dtime);
    return *this;
}

DateTime & DateTime::incWeek (int32_t week)
{
    _dtime.tm_mday += 7 * week;
    _dtime.tm_isdst = -1;
    mktime(&_dtime);
    return *this;
}

bool DateTime::setDateTime (uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t minutes, uint32_t seconds)
{
    if (checkDate (year, month, day) && checkTime(hours, minutes, seconds))
    {
        _dtime.tm_year = year - 1900;
        _dtime.tm_mon = month - 1;
        _dtime.tm_mday = day;

        _dtime.tm_hour = hours;
        _dtime.tm_min = minutes;
        _dtime.tm_sec = seconds;
        _dtime.tm_isdst = -1;
        mktime(&_dtime);

        return true;
    }

    return false;
}

uint32_t DateTime::getYear() const
{
    return _dtime.tm_year + 1900;
}

uint32_t DateTime::getMonth() const
{
    return _dtime.tm_mon + 1;
}

uint32_t DateTime::getDay() const
{
    return _dtime.tm_mday;
}

uint32_t DateTime::getHour() const
{
    return _dtime.tm_hour;
}

uint32_t DateTime::getMinute() const
{
    return _dtime.tm_min;
}

uint32_t DateTime::getSecond() const
{
    return _dtime.tm_sec;
}

uint32_t DateTime::getWeek() const
{
    return _dtime.tm_wday;
}

bool DateTime::checkDate(uint32_t year, uint32_t month, uint32_t day)
{
    if(year < 1970 || year >= 2037)
    {
        return false;
    }
    if(month < 1 || month > 12)
    {
        return false;
    }
    if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
    {
        if(day < 1 || day > 31)
        {
            return false;
        }
    }
    else if(month == 4 || month == 6 || month == 9 || month == 11)
    {
        if(day < 1 || day > 30)
        {
            return false;
        }
    }
    else if(month == 2)
    {
        /* »ÚƒÍ */
        if ( year%400==0 || year%100!=0 && year%4==0 )
        {
            if(day < 1 || day > 29)
            {
                return false;
            }
        }
        else
        {
            if(day < 1 || day > 28)
            {
                return false;
            }
        }
    }
    return true;
}

bool DateTime::checkTime(uint32_t hours, uint32_t minutes, uint32_t seconds)
{
    if(hours < 0 || hours >= 24)
    {
        return false;
    }
    if(minutes < 0 || minutes >= 60)
    {
        return false;
    }
    if(seconds < 0 || seconds >= 60)
    {
        return false;
    }
    return true;
}

std::string DateTime::toString ()
{
    char ret [256];
    memset (ret, 0, sizeof (ret));
    sprintf (ret, "%u-%u-%u %u %u %u",
            getMonth (),
            getDay (),
            getYear (),
            getHour (),
            getMinute (),
            getSecond ()
            );

    return ret;
}
