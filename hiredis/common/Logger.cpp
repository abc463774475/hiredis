#include "Logger.h"
#include "tool.h"
#include <string>
#include <stdarg.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;

Logger *NLog = NULL;
#define getMessage(msg,msglen,pat)	\
do	\
{	\
	va_list ap;	\
	memset(msg, 0, msglen);	\
	va_start(ap, pat);		\
	vsnprintf(msg, msglen - 1, pat, ap);	\
	va_end(ap);	\
}while(0)

Logger::Logger (const char *strLoggerName)
{
	_strLoggerName = strLoggerName;
	_logFileName = strLoggerName;

	//_logLevel		= LOG_ALL;

    //_stdoutHandle   = GetStdHandle (STD_OUTPUT_HANDLE);

    _file = 0;

    _logNum = LOGNUM_MAX;

	_level2String[LOG_DEBUG]    = "DEBUG";
	_level2String[LOG_ERROR]    = "ERROR";
	_level2String[LOG_WARNING]  = "WARN ";
	_level2String[LOG_INFO]     = "INFO ";
	_level2String[LOG_FATAL]    = "FATAL";
    _level2String[LOG_ONLYFILE] = "OFILE";

//   	_isWriteAtOnce = true ;
//   	_writeType = eWriteType_Common;

 	_isWriteAtOnce = true ;
	_writeType = eWriteType_Common ;

 	//_writeType = eWriteType_OnlyErro;
}

Logger::~Logger()
{
}

void Logger::color (uint16_t c)
{
    //SetConsoleTextAttribute (_stdoutHandle, c);
}

bool Logger::Startup()
{
	//CRunnable::Start();
	checkFilePath();
	std::thread	t1(log_thread);
	t1.detach();

	return true;
}

bool Logger::log (const LevelMessage &lm)
{
   	if (_writeType == eWriteType_OnlyErro)
   	{
		if ( lm.level != LOG_ERROR )
		{
			return true ;
		}
   	}
	uint64_t	_startTime = getTime64() ;
    DateTime timenow = now ();

	char strTime[128];
	char strDatyTime[128];
	char strFileTime[128];
	
    if (/*lm.level == LOG_ERROR*/1)
    {
        ++_logNum;

        // set Log file path.
        if (_filePath.empty ())
        {
            _filePath = getFilePath();
        }

        memset (strTime, 0, sizeof (strTime));
        sprintf (strTime, "_%02d-%02d-%d_", timenow.getMonth (), timenow.getDay (), timenow.getYear ()); /* m-d-y */

        memset (strDatyTime, 0, sizeof (strDatyTime));
        sprintf (strDatyTime, "%02d:%02d:%02d", timenow.getHour (), timenow.getMinute (), timenow.getSecond ()); /* h:m:s */

        memset (strFileTime, 0, sizeof (strFileTime));
        sprintf (strFileTime, "%02d%02d%02d", timenow.getHour (), timenow.getMinute (), timenow.getSecond ()); /* hms */

        /* write to file */
        if (_logNum >= LOGNUM_MAX)
        {
            if (_file)
            {
                fflush (_file);
                fclose (_file);
            }
            
            _logFileName = _filePath + string (strTime) + string (strFileTime) + (string)(".log");

            _file = fopen (_logFileName.c_str(), "a+");

			if ( !_file)
			{
				int iErro = errno;
				printf("erro  %d   %s\n", iErro, strerror(iErro));
			}
            _logNum = 0;
        }
        if (_file)
        {
			uint64_t _flushTime = getTime64() - _startTime ;
            if ( _flushTime < 30 )
            {
				fprintf (_file,"%s %s %s %5s:  %s\n",
					strTime,strDatyTime, _strLoggerName.c_str(), _level2String[static_cast<ELogLevel>(lm.level)].c_str(), lm.message);
            }
			else
			{
				fprintf (_file,"%s %s %s %5s: flushTime %lld %s\n",
					strTime,strDatyTime, _strLoggerName.c_str(), _level2String[static_cast<ELogLevel>(lm.level)].c_str(),(long long)_flushTime ,lm.message);
			}
            fflush (_file);
        }
		
        /* write to console */
        char szConsoleMsg[MSGBUF_MAX];
        memset (szConsoleMsg, 0, sizeof (szConsoleMsg));

        switch(lm.level)
        {
        case LOG_FATAL:
        case LOG_ERROR:
            {
                color (CRED);
                
                printf ("%s %s %s %5s:  %s\n", strTime, strDatyTime, _strLoggerName.c_str(), _level2String[static_cast<ELogLevel>(lm.level)].c_str(),
                    lm.message);
                
                color (CNORMAL);
            }
            break;
        case LOG_WARNING:
            {
                color (CYELLOW);
                
				printf ("%s %s %s %5s:  %s\n", strTime, strDatyTime, _strLoggerName.c_str(), _level2String[static_cast<ELogLevel>(lm.level)].c_str(),
					lm.message);
                
                color (CNORMAL);
            }
            break;
        case LOG_DEBUG:
            {
                color (CBLUE);
                
				printf ("%s %s %s %5s:  %s\n", strTime, strDatyTime, _strLoggerName.c_str(), _level2String[static_cast<ELogLevel>(lm.level)].c_str(),
					lm.message);
                
                color (CNORMAL);
            }
            break;
		case LOG_ONLYFILE:
			{

			}
			break;
        default:
            {
				printf ("%s %s %s %5s:  %s\n", strTime, strDatyTime, _strLoggerName.c_str(), _level2String[static_cast<ELogLevel>(lm.level)].c_str(),
					lm.message);
            }
        }
    }
	uint64_t	_endTime = getTime64() - _startTime ;
	if ( _endTime >= 30 )
	{
		color (CRED);
		printf ("%s %s %s ERRO: logWriteTime: %lld\n", strTime, strDatyTime, _strLoggerName.c_str(),
			(long long)_endTime);
		color (CNORMAL);

 		fprintf (_file,"%s %s %s Erro: logWirteTime: %lld  \n",
 			strTime,strDatyTime, _strLoggerName.c_str(), (long long)_endTime);
 		fflush (_file);
	}
	
    return 1;
}

bool Logger::info (const char *format, ...)
{
	LevelMessage lm;
	lm.level = LOG_INFO;
	getMessage(lm.message, MSGBUF_MAX, format);

	{
		push_back(lm);
	}
	
	return 1;
}

bool Logger::error (const char *format, ...)
{
	LevelMessage lm;
	lm.level = LOG_ERROR;
	getMessage(lm.message, MSGBUF_MAX, format);

	{
		push_back(lm) ;
	}

	return 1;
}

bool Logger::warning (const char *format, ...)
{
	LevelMessage lm;
	lm.level = LOG_WARNING;
	getMessage(lm.message, MSGBUF_MAX, format);
	
	{
		push_back(lm);
	}

	return 1;
}

bool Logger::fatal (const char *format, ...)
{
	LevelMessage lm;
	lm.level = LOG_FATAL;
	getMessage (lm.message, MSGBUF_MAX, format);
	
	{
		push_back(lm);
	}

	return 1;
}

bool Logger::debug (const char *format, ...)
{
	LevelMessage lm;
	lm.level = LOG_DEBUG;
	getMessage(lm.message, MSGBUF_MAX, format);

	{
		push_back(lm);
	}

	return 1;
}

bool Logger::trace(const char *format, ...)
{
	LevelMessage lm;
	lm.level = LOG_INFO; 
	getMessage(lm.message, MSGBUF_MAX, format);

	{
		push_back(lm);
	}

	return 1;
}


bool Logger::onlyFile (const char *format, ...)
{
	//CSALocker guard (_lock);
	std::lock_guard<mutex> guard(_lock);
	
	LevelMessage lm;
	lm.level = LOG_ONLYFILE; 
	getMessage(lm.message, MSGBUF_MAX, format);

	{
		push_back(lm);
	}


    return true;
}
void Logger::push_back(LevelMessage &lm)
{
	//CSALocker guard (_lock);
	std::lock_guard<mutex> guard(_lock);
	if ( _isWriteAtOnce )
	{
		log(lm);
	}
	else
	{
		// 使用异步的话  可以考虑使用  内存映射
		_logs.push_back (lm);
// 		if ( gFMappingMgr && (_fileMappingName.size() != 0 ) )
// 		{
// 			FileMappingControl *pFile = gFMappingMgr->GetFileByName(_fileMappingName) ;
// 			if ( pFile)
// 			{
// 				pFile->SetMsgData (lm) ;				
// 			}
// 		}
	}
}

void Logger::ChangeIsWriteAtOnce()
{
	_isWriteAtOnce = !_isWriteAtOnce ;
}

void Logger::SetWriteType(int _type)
{
	_writeType = _type ;
}

int Logger::Run ()
{
	while (1)
	{
 		std::deque<LevelMessage> tmp;
 
 		{
 			std::lock_guard<mutex> guard(_lock);
 			tmp = _logs;
 			_logs.clear();
 		}
 
 		for ( deque<LevelMessage>::iterator it = tmp.begin(); it != tmp.end() ; ++it )
 		{
 			log(*it);
 		}

		usleep(1000);
	}
	
	return 1;
}

void Logger::checkFilePath()
{
// 	char sz_log1[] = "./log";
// 	if ( opendir (sz_log1) == NULL)
// 	{
// 		if (mkdir(sz_log1, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
// 		{
// 			int iErro = errno;
// 			printf("mkdir erro  %d   %s", iErro, strerror(iErro));
// 			return;
// 		}
// 	}
// 	string strPath = getFilePath();
// 	if ( opendir (strPath.c_str()) == NULL)
// 	{
// 		if (mkdir(strPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
// 		{
// 			int iErro = errno;
// 			printf("mkdir erro  %d   %s", iErro, strerror(iErro));
// 			return;
// 		}
// 	}

	string strPath = getFilePath();
	if (opendir(strPath.c_str()) == NULL)
	{
		char sz[1024];
		sprintf(sz, "mkdir -p %s", strPath.c_str());
		if (system(sz) == -1)
		{
			printf("erro %s", strerror(errno));
		}
	}
}

string Logger::getFilePath()
{
	return (string)("./log/") + _strLoggerName + string("/");
}

void log_thread()
{
	NLog->Run();
}