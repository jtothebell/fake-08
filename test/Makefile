CFLAGS := -Wall -g -c


all: runtests


testrunner.a: testroot.o fontdatatests.o
	g++ testroot.o \
	fontdata.o fontdatatests.o \
	-o testrunner.a

testroot.o: testroot.cpp
	g++ $(CFLAGS) testroot.cpp 

fontdatatests.o: fontdatatests.cpp ../source/fontdata.cpp
	g++ $(CFLAGS) fontdatatests.cpp ../source/fontdata.cpp


runtests: testrunner.a
	./testrunner.a

clean:
	-rm -f testrunner.a *.o

.PHONY: all buildtests runtests clean