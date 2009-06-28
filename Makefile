CC = g++
CFLAGS = -I /usr/local/include/boost-1_39/

OBJECTS = main.o /lib/libboost_system-xgcc40-mt.a /lib/libboost_thread-xgcc40-mt-1_39.a

main : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS)  -o main

%.o : %.cpp
	$(CC) $(CFLAGS) -c $<


clean:
	rm *.o
	rm main