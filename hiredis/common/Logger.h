/**
 * \brief ��־����Ҫʵ��console��file
 *
 * \author Isoft
 */
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdint.h>
#include <map>
#include <deque>
#include "DateTime.h"
#include <iostream>
#include <string>
#include <mutex>
using namespace std;

class Logger;

/**
 * \brief "����" ȫ��ʹ�õ���־�࣬���й��̶�ʹ�øñ���
 *        ÿ��������ʹ��NLogǰ�������� �����塱
 *
 */
extern Logger *NLog;

/**
 * \brief ������־��󳤶�
 */
#define MSGBUF_MAX (2048)

/**
 * \brief ������־�ļ������־����
 *
 */
#define LOGNUM_MAX  (8192)

/**
 * \brief color s.
 *
 */
#define CRED    0
#define CYELLOW 1
#define CBLUE   2
#define CNORMAL 3

class Logger
{
public:
    enum ELogLevel
    {
        LOG_DEBUG			= 0x00000001,
        LOG_WARNING			= 0x00000002,
        LOG_ERROR			= 0x00000004,
        LOG_INFO			= 0x00000008,
        LOG_FATAL			= 0x00000010,
        LOG_ONLYFILE        = 0x00000020,

        LOG_ALL				= 0xffffffff,
    };

	enum eWriteType
	{
		eWriteType_Common ,
		eWriteType_OnlyErro,			// ������ڹر���־ ֻд erro��־
	};

    struct LevelMessage
    {
        ELogLevel level;
        char message[MSGBUF_MAX];
    };

	public:
		Logger (const char *name);
		~Logger ();

		/**
		 * \brief �õ�Logger������
		 *
		 */
		const std::string &getName ()
		{
			return _strLoggerName;
		}

		/**
		 * \brief ����Logger�����֣���������ÿ����־��Ϣ��
		 *
		 */
		void setName (const std::string &name)
		{
			_strLoggerName = name;
		}

		/**
		 * \brief ���õȼ�
		 *
		 */
		void setLevel (const int level);

	public:
        /**
		 * \brief дdebug������־
		 * \param format �����ʽ
		 */
		bool debug (const char *format, ...);

		/**
		 * \brief дerror������־
		 * \param  format �����ʽ
		 */
		bool error (const char *format, ...);

		/**
		 * \brief дinfo������־
		 * \param format �����ʽ
		 */
		bool info (const char *format, ...);

		/**
		 * \brief дfatal������־
		 * \param format �����ʽ
		 */
		bool fatal (const char *format, ...);

		/**
		 * \brief дwarn������־
		 * \param format �����ʽ
		 */
		bool warning (const char *format, ...);

        /**
		 * \brief дtrace��Ϸ��־
		 * \param format �����ʽ
		 */
		bool trace (const char *format, ...);

        /**
         * \brief ֻд���ı�
         *
         */
        bool onlyFile (const char *format, ...);

        /**
         * \breif ���ÿ���̨��ɫ
         *
         */
        void color (uint16_t c);

		/**
		 * \ ��ʼ��
		 *
		 */
		bool Startup ();

		/**
		 * \brief �̺߳���
		 *
		 */
		int Run ();
		
		/**
		 *	\ͬ������
		 *
		 */
		void DoNow (const string &str);

		/**
		 *	\������־
		 *
		 */
		void push_back (LevelMessage &lm);

		/**
		 *  \�ı��Ƿ�д���ֵ
		 * 
		 */
		void ChangeIsWriteAtOnce ();

	private:
		bool log (const struct LevelMessage &lm);
		
		/*
		 * \ �Ƿ����д��
		 *	�Ƿ�����ˢ����
		 */
		bool _isWriteAtOnce ;

		/*
		 * \ ��־д����
		 *	  
		 */
		volatile int _writeType ;
        /**
		 * \brief ������
		 *
		 */
		mutex _lock;

		/**
		 * \brief ����־�������
		 *
		 */
		std::string _strLoggerName;

		/**
		 * \brief ��ǰ����־�ȼ�
		 *
		 */
		//ELogLevel _logLevel;

		/**
		 * \brief ��ǰ����д���ļ�����
		 *
		 */
		std::string _logFileName;

        /**
         * \brief console handler
         *
         */
        //HANDLE _stdoutHandle;

		/**
		 * \brief ��־�ȼ�����ֵ��ӳ���ϵ
         *
		 */
		std::map<ELogLevel, std::string> _level2String;

		/**
		 * \brief ��־Ŀ¼
         *
		 */
		std::string _filePath;

        /**
         * \brief ��־�ļ����
         *
         */
        FILE* _file;

        /*
         * \brief ��ǰ��־�ļ�����־����
         * 
         */
        uint32_t _logNum;

		/**
		 * \brief ��Ҫ��ӡ����־
		 *
		 */
		std::deque <LevelMessage> _logs;
		
		string	_fileMappingName ;					//ʹ���ļ�ӳ�������

public:
	void		SetFileMappingName (const string &str) { _fileMappingName = str ; }
	string		GetFileMappingName (){ return _fileMappingName ;}

	void		SetWriteType (int _type) ;
	void		checkFilePath();
	string		getFilePath();
};

void	log_thread();
#endif // _LOGGER_H_
