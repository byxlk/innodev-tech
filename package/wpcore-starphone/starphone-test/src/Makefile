CC = gcc

XX = g++
CFLAGS = -Wall -O

TARGET = wifi-phone

%.o: %.c
	 $(CC) $(CFLAGS) -c $< -o $@

%.o:%.cpp
	 $(XX) $(CFLAGS) -c $< -o $@

SOURCES = \
	gpio-spi.c \
	si3050_api.c \
	test.c \
	pcm.c \
    	main.c
	
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))


$(TARGET) : $(OBJS)
	 $(CC) $(OBJS) -o $(TARGET) -lm
	 chmod a+x $(TARGET)

clean:
	rm *.o wifi-phone
	