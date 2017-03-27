/**
 * \brief 日期时间处理相关
 *
 * \author Isoft
 */

#ifndef _DATE_TIME_H_
#define _DATE_TIME_H_

#include <stdint.h>
#include <time.h>
#include <string>

class DateTime;

/**
* \brief 获取代表当前日期时间
*        专为使用者提供的全局函数
*/
DateTime now ();

/**
* @brief 日期时间类,代表1970年午夜到2037年之间的某一个时间
*
*/
class DateTime
{
public:
    DateTime (time_t t = 0);

    /**
     * \brief 获取/设置本类所代表的日期时间的time_t值
     *
     */
    time_t getTimeValue ();
    void setTimeValue (time_t t);

    /**
     * \brief 获取与datetime所代表的日期时间的差值
     *
     */
    int64_t operator - (const DateTime &datetime);
    int64_t diffSecond (const DateTime &datetime);
    int64_t diffMinute (const DateTime &datetime);
    int64_t diffHour (const DateTime &datetime);
    int64_t diffDay (const DateTime &datetime);
    int64_t diffWeek (const DateTime &datetime);
    int32_t diffMonth (const DateTime &datetime);
    int32_t diffYear (const DateTime &datetime);

    /**
     * \brief 增加本类所代表的日期时间
     *
     */
    DateTime &incYear (int32_t year);
    DateTime &incMonth (int32_t month);
    DateTime &incDay (int32_t day);
    DateTime &incHour (int32_t hour);
    DateTime &incMinute (int32_t minute);
    DateTime &incSecond (int32_t second);
    DateTime &incWeek (int32_t week);

    /**
     * \brief 设置本类所代表的日期时间
     *
     */
    bool setDateTime (uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t minutes, uint32_t seconds);
    
    /**
     * \brief 获取本类所代表的年份、月份等
     *
     */
    uint32_t getYear () const;
    uint32_t getMonth () const;
    uint32_t getDay () const;
    uint32_t getHour () const;
    uint32_t getMinute () const;
    uint32_t getSecond () const;
    uint32_t getWeek () const;

    /**
     * format : m-d-y h m s
     *
     */
    std::string toString ();

private:
    bool checkDate (uint32_t year, uint32_t month, uint32_t day);
    bool checkTime (uint32_t hours, uint32_t minutes, uint32_t seconds);

private:
    /**
     * 本类 所代表的日期时间
     *
     */
    tm _dtime;
};


inline DateTime now ()
{
    return time (NULL);
}

#endif // _DATE_TIME_H_
