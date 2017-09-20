
#==========================================================================
# Name		: Makefile
# Author	: Jianhua Zhang
# Time		: Feb,9th,2014
# Update	: April,7th,2015
# Description	: Dis_Prony algorithm;
# File		: Makefile
#==========================================================================


all:	PronyADMM ADMMServer

ADMMServer: PronyADMM_Server.o rpoly.o Prony_common.o 
	g++ PronyADMM_Server.o rpoly.o Prony_common.o -larmadillo -lpthread -o ADMMServer

PronyADMM_Server.o: PronyADMM_Server.cpp rpoly.cpp Prony_common.cpp
	g++ -g -c -Wall PronyADMM_Server.cpp rpoly.cpp Prony_common.cpp -larmadillo -lpthread


PronyADMM: PronyADMM_Client.o rpoly.o Prony_common.o 
	g++ PronyADMM_Client.o rpoly.o Prony_common.o -larmadillo -lpthread -o PronyADMM

PronyADMM_Client.o: PronyADMM_Client.cpp rpoly.cpp Prony_common.cpp
	g++ -g -c -Wall PronyADMM_Client.cpp rpoly.cpp Prony_common.cpp -larmadillo -lpthread

clean:	
	rm -f *.o PronyADMM ADMMServer
