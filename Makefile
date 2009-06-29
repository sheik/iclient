CC = g++
CFLAGS = -I /usr/local/include/boost-1_39/

OBJECTS = iclient.o main.o /lib/libboost_system-xgcc40-mt.a

main : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS)  -o main

%.o : %.cpp
	$(CC) $(CFLAGS) -c $<


clean:
	rm *.o
	rm main
