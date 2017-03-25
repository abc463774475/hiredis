#pragma once
#pragma execution_character_set ("utf_8")

/*****************************************
	2017-1-12 自己手动创建 redis集群方面的东西
			  1 有哪些方面需要考虑？  是否支持中心节点？  还是去中心节点  gossip   
			  2 客户端怎样访问？ 是否需要总控 节点？  即客户端登录统一的节点?统一节点会造成什么问题 ？？数据过渡集中，，造成性能上的过渡浪费？网络拥塞。。等等
			  3 c++11 么？？如果用这个特性会与 原生c标准的redis会造成冲突么？
			  4 是否加入lua管理相关的东西lua配置相当好
			  5 协议相关的呢 ？   大小端？  是否用原生的底层一套？ 这是个相当大的问题
			  6 服务器不在一个良好的环境中运行的情况？这个咋搞？

	2017-1-22 think about that :slave shutdown   and   master shutdown  we need do what
			  1 slave shutdown
				a delete slave msg in master   ok  do like this
			  2 master shutdown
				a check slaveIs ins 
				b vote a master in slave 
				c slaveof no one in voted master
				d check the info in voted master the opt is end
				e slave the left slaves
				f resetflush the slaves again
	2017-1-25
				u must finish the failover module in the new year and when back work... i must test the module ok?
	2017-2-4	
				the timer must delay ..much situations are very complex
				test the faliover  ok ?
	2017-2-6
				failover finish
				next , i ll finish self net lib and net protocol 
					1 use redis's network 
					2 create myself's network
				test publish scrible
					1 i must know how to work
						yes i know  publish subscribe  psubscribe  
				test timer
					create a timerFun limit times 
					this can not do    because the server can not suport
	2017-2-7
				finish publish scribe module
				finish timer
				the next is network and clusters
	2017-2-8
				learn sharding of cluster
				note  the crc16  algth
	2017-2-21
				add commands in codes 
				lack of network
				add slots cache in cli
				change the cli output format  is very complex
	2017-2-22
				simulate set h0-10  iiiiiii   in cli module  ...and change the slots correctly
	2017-3-3
				add lua in mycluster
	2017-3-8
				centerliztion ? why it?  and how to do it  ?
	2017-3-10
				select  recvMsg and send test
****************************************************/