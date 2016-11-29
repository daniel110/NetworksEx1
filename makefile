CPP = g++

OBJS_Server = mainServer.o Server.o FDSet.o Common.o Inbox.o MailObj.o Packet.o Socket.o ServerSessionSocket.o User.o 
OBJS_Client = mainClient.o Client.o Common.o Packet.o Socket.o

#The executabel filename
EXEC_Server = mail_server
EXEC_Client = mail_client

CPP_COMP_FLAG = -Wall -g


all: $(EXEC_Server) $(EXEC_Client)

$(EXEC_Server): $(OBJS_Server)
	$(CPP) $(OBJS_Server) -o $@
$(EXEC_Client): $(OBJS_Client)
	$(CPP) $(OBJS_Client) -o $@

Client.o: Client.cpp Client.h Socket.h Packet.h Common.h
	$(CPP) $(CPP_COMP_FLAG) -c $*.cpp

mainServer.o: mainServer.cpp Server.h User.h Packet.h Inbox.h MailObj.h ServerSessionSocket.h Socket.h Common.h FDSet.h
	$(CPP) $(CPP_COMP_FLAG) -c $*.cpp

Server.o: Server.cpp Server.h User.h Packet.h Inbox.h MailObj.h ServerSessionSocket.h Socket.h Common.h FDSet.h
	$(CPP) $(CPP_COMP_FLAG) -c $*.cpp

FDSet.o: FDSet.cpp FDSet.h Socket.h Packet.h
	$(CPP) $(CPP_COMP_FLAG) -c $*.cpp

Common.o: Common.cpp Common.h
	$(CPP) $(CPP_COMP_FLAG) -c $*.cpp

Inbox.o: Inbox.cpp Inbox.h MailObj.h Packet.h User.h
	$(CPP) $(CPP_COMP_FLAG) -c $*.cpp

MailObj.o: MailObj.cpp MailObj.h Packet.h
	$(CPP) $(CPP_COMP_FLAG) -c $*.cpp

Packet.o: Packet.cpp Packet.h
	$(CPP) $(CPP_COMP_FLAG) -c $*.cpp

Socket.o: Socket.cpp Socket.h Packet.h
	$(CPP) $(CPP_COMP_FLAG) -c $*.cpp

ServerSessionSocket.o: ServerSessionSocket.cpp ServerSessionSocket.h Socket.h Packet.h Inbox.h MailObj.h User.h Common.h
	$(CPP) $(CPP_COMP_FLAG) -c $*.cpp

User.o: User.cpp User.h Packet.h
	$(CPP) $(CPP_COMP_FLAG) -c $*.cpp

clean:
	rm -f $(OBJS_Server) $(EXEC_Server)
	rm -f $(OBJS_Client) $(EXEC_Client)

#use g++ -MM <sourcefile>.cpp to see the dependencies