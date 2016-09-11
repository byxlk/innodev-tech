CC = gcc

XX = g++
CFLAGS = -Wall -O

 
TARGET = starphone-server 

%.o: %.c
	 $(CC) $(CFLAGS) -c $< -o $@

%.o:%.cpp
	 $(XX) $(CFLAGS) -c $< -o $@

SOURCES = usbModem.c udp_broadcast.c tcp_server.c main.c dtmf.c uLawCodec.c
	
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))


$(TARGET) : $(OBJS)
	 $(CC) $(SOURCES) -o $(TARGET) $(LDFLAGS) -DMAIN -DNO_MODEM
	 chmod a+x $(TARGET)

clean:
	rm *.o starphone-server
	
