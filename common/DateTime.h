/**
 * \brief ����ʱ�䴦�����
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
* \brief ��ȡ����ǰ����ʱ��
*        רΪʹ�����ṩ��ȫ�ֺ���
*/
DateTime now ();

/**
* @brief ����ʱ����,����1970����ҹ��2037��֮���ĳһ��ʱ��
*
*/
class DateTime
{
public:
    DateTime (time_t t = 0);

    /**
     * \brief ��ȡ/���ñ��������������ʱ���time_tֵ
     *
     */
    time_t getTimeValue ();
    void setTimeValue (time_t t);

    /**
     * \brief ��ȡ��datetime�����������ʱ��Ĳ�ֵ
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
     * \brief ���ӱ��������������ʱ��
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
     * \brief ���ñ��������������ʱ��
     *
     */
    bool setDateTime (uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t minutes, uint32_t seconds);
    
    /**
     * \brief ��ȡ�������������ݡ��·ݵ�
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
     * ���� �����������ʱ��
     *
     */
    tm _dtime;
};


inline DateTime now ()
{
    return time (NULL);
}

#endif // _DATE_TIME_H_
