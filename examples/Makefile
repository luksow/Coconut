# simple & dirty for better readability

all:
	(cd ../src; make)
	cp ../src/libcoconut.a ./
	cp ../src/coconut_pub.h ./coconut.h
	gcc simple_events.c libcoconut.a -lpthread -o simple_events
	gcc simple_blocks.c libcoconut.a -lpthread -o simple_blocks
	g++ --std=c++11 extended_events.cpp libcoconut.a -lpthread -o extended_events
	g++ --std=c++11 extended_blocks.cpp libcoconut.a -lpthread -o extended_blocks

clean:
	rm -f simple_events
	rm -f simple_blocks
	rm -f extended_events
	rm -f extended_blocks
	rm -f libcoconut.a
	rm -f coconut.h
