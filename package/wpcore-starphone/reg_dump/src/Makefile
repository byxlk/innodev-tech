CC = gcc

XX = g++
CFLAGS = -Wall -O

TARGET = regdump

%.o: %.c
	 $(CC) $(CFLAGS) -c $< -o $@

%.o:%.cpp
	 $(XX) $(CFLAGS) -c $< -o $@

SOURCES = \
    gpio-spi.c \
    main.c
	
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))


$(TARGET) : $(OBJS)
	 $(CC) $(OBJS) -o $(TARGET) -lm
	 chmod a+x $(TARGET)

clean:
	rm *.o atest
	
