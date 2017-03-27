#ifndef _WorkPacket_H
#define _WorkPacket_H
#include "byteBuffer.h"
#include <stdint.h>
#include <string.h>
#include "Logger.h"


namespace sim{
	class	Sx_Exception
	{
	public:
		Sx_Exception(bool	bRet, uint32_t	_pos, uint32_t	_len, uint32_t	_totalLen, uint32_t _packType)
		{
			if (bRet == false)
			{
				NLog->error("bytebuffer :packType %d _pos %d , _len %d , _totalLen %d", _packType, _pos, _len, _totalLen);
			}
		}

		virtual	~Sx_Exception()
		{
		}
	};

};

class	WorkPacket:public	sim::ByteBuffer<sim::Sx_Exception>
{
public:
	enum eInte_Type
	{
		eInte_Type_packType = 0 ,
		eInte_Type_reqId ,
		eInte_Type_sessionId,
		eInte_Type_accountId ,
		eInte_Type_max,
	};
public:
	WorkPacket( int _type = -1)
	{
		memset(m_value, 0, sizeof (m_value)) ;
		m_value[eInte_Type_packType] = _type;
		_packType = _type;
	}

	WorkPacket(const uint8_t *data , int length)
	{
		memset(m_value, 0, sizeof (m_value)) ;
		SetData(data,length);
	}

	~WorkPacket()
	{
		
	}
	
	void	SetData (const uint8_t *data,int length)
	{
		clear();
		if ( length < eInte_Type_max*sizeof(int) )
		{
			throw int(55);
		}
		for (int i = 0 ; i < eInte_Type_max ; ++i)
		{
			m_value[i] = *((int*)data +i);
		}

		if ( length > eInte_Type_max*sizeof(int) )
		{
			std::vector<uint8_t>	vTemp(data+eInte_Type_max*sizeof(int),data+length);
			_storage = vTemp;
		}
	}

	int	GetType()
	{
		return m_value[eInte_Type_packType] ;
	}
	void	SetType(int _type)
	{
		m_value[eInte_Type_packType] = _type;
		_packType = _type;
	}
	int GetReqId ()
	{
		return m_value[eInte_Type_reqId];
	}
	void SetReqId (int _id)
	{
		m_value[eInte_Type_reqId] = _id ;
	}
	int GetSessionId ()
	{
		return m_value[eInte_Type_sessionId] ;
	}
	void SetSessionId (int _id)
	{
		m_value[eInte_Type_sessionId] = _id;
	}
	int GetAccountId ()
	{
		return m_value[eInte_Type_accountId];
	}
	void SetAccountId (int _id)
	{
		m_value[eInte_Type_accountId] = _id ;
	}
	void	Reset ()
	{
		_rpos = 0 ;
		_wpos = 0 ;
		_packType = -1;
		_storage.clear() ;
		m_value[eInte_Type_packType] = -1;
	}
	
	int		GetNeedLen ()
	{
		return sizeof (m_value) + size ();
	}

public:
	int m_value[eInte_Type_max] ;
};


#endif