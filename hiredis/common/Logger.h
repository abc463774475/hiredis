/**
 * \brief 日志，主要实现console与file
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
 * \brief "声明" 全局使用的日志类，所有工程都使用该变量
 *        每个工程在使用NLog前，均需先 “定义”
 *
 */
extern Logger *NLog;

/**
 * \brief 单条日志最大长度
 */
#define MSGBUF_MAX (2048)

/**
 * \brief 单个日志文件最大日志条数
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
		eWriteType_OnlyErro,			// 这里等于关闭日志 只写 erro日志
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
		 * \brief 得到Logger的名字
		 *
		 */
		const std::string &getName ()
		{
			return _strLoggerName;
		}

		/**
		 * \brief 设置Logger的名字，它出现在每条日志信息中
		 *
		 */
		void setName (const std::string &name)
		{
			_strLoggerName = name;
		}

		/**
		 * \brief 设置等级
		 *
		 */
		void setLevel (const int level);

	public:
        /**
		 * \brief 写debug程序日志
		 * \param format 输出格式
		 */
		bool debug (const char *format, ...);

		/**
		 * \brief 写error程序日志
		 * \param  format 输出格式
		 */
		bool error (const char *format, ...);

		/**
		 * \brief 写info程序日志
		 * \param format 输出格式
		 */
		bool info (const char *format, ...);

		/**
		 * \brief 写fatal程序日志
		 * \param format 输出格式
		 */
		bool fatal (const char *format, ...);

		/**
		 * \brief 写warn程序日志
		 * \param format 输出格式
		 */
		bool warning (const char *format, ...);

        /**
		 * \brief 写trace游戏日志
		 * \param format 输出格式
		 */
		bool trace (const char *format, ...);

        /**
         * \brief 只写入文本
         *
         */
        bool onlyFile (const char *format, ...);

        /**
         * \breif 设置控制台颜色
         *
         */
        void color (uint16_t c);

		/**
		 * \ 初始化
		 *
		 */
		bool Startup ();

		/**
		 * \brief 线程函数
		 *
		 */
		int Run ();
		
		/**
		 *	\同步函数
		 *
		 */
		void DoNow (const string &str);

		/**
		 *	\插入日志
		 *
		 */
		void push_back (LevelMessage &lm);

		/**
		 *  \改变是否写入的值
		 * 
		 */
		void ChangeIsWriteAtOnce ();

	private:
		bool log (const struct LevelMessage &lm);
		
		/*
		 * \ 是否理解写入
		 *	是否立即刷磁盘
		 */
		bool _isWriteAtOnce ;

		/*
		 * \ 日志写类型
		 *	  
		 */
		volatile int _writeType ;
        /**
		 * \brief 互斥锁
		 *
		 */
		mutex _lock;

		/**
		 * \brief 此日志库的名称
		 *
		 */
		std::string _strLoggerName;

		/**
		 * \brief 当前的日志等级
		 *
		 */
		//ELogLevel _logLevel;

		/**
		 * \brief 当前正在写的文件名称
		 *
		 */
		std::string _logFileName;

        /**
         * \brief console handler
         *
         */
        //HANDLE _stdoutHandle;

		/**
		 * \brief 日志等级与数值的映射关系
         *
		 */
		std::map<ELogLevel, std::string> _level2String;

		/**
		 * \brief 日志目录
         *
		 */
		std::string _filePath;

        /**
         * \brief 日志文件句柄
         *
         */
        FILE* _file;

        /*
         * \brief 当前日志文件的日志数量
         * 
         */
        uint32_t _logNum;

		/**
		 * \brief 需要打印的日志
		 *
		 */
		std::deque <LevelMessage> _logs;
		
		string	_fileMappingName ;					//使用文件映射的名字

public:
	void		SetFileMappingName (const string &str) { _fileMappingName = str ; }
	string		GetFileMappingName (){ return _fileMappingName ;}

	void		SetWriteType (int _type) ;
	void		checkFilePath();
	string		getFilePath();
};

void	log_thread();
#endif // _LOGGER_H_
