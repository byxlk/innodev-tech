CC = gcc

XX = g++
CFLAGS = -Wall -O

 
TARGET = starphone-server 

%.o: %.c
	 $(CC) $(CFLAGS) -c $< -o $@

%.o:%.cpp
	 $(XX) $(CFLAGS) -c $< -o $@

SOURCES = \
	pcm.c \
	gpio-spi.c \
	si3050_api.c \
	network_server.c \
	main.c 
	
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))


$(TARGET) : $(OBJS)
	 $(CC) $(SOURCES) -o $(TARGET) $(LDFLAGS) -DMAIN -DNO_MODEM
	 chmod a+x $(TARGET)

clean:
	rm *.o starphone-server
	
