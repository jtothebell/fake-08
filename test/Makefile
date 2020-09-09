#match flags from 3ds and switch makefiles. This may not all be necessary
CFLAGS := -Wall -g -c -O2 \
		  -fno-rtti -fexceptions -std=gnu++17


all: runtests


testrunner.a: testroot.o fontdatatests.o graphicstests.o audiotests.o
	g++ testroot.o \
	fontdata.o fontdatatests.o \
	graphics.o graphicstests.o stringToDataHelpers.o \
	Audio.o synth.o audiotests.o \
	-o testrunner.a

testroot.o: testroot.cpp doctest.h
	g++ $(CFLAGS) testroot.cpp 

fontdatatests.o: fontdatatests.cpp ../source/fontdata.cpp ../source/fontdata.h
	g++ $(CFLAGS) fontdatatests.cpp ../source/fontdata.cpp

graphicstests.o: graphicstests.cpp ../source/graphics.cpp ../source/graphics.h \
	../source/fontdata.cpp ../source/fontdata.h \
	../source/PicoRam.h ../source/hostVmShared.h \
	../source/stringToDataHelpers.h ../source/stringToDataHelpers.cpp
	g++ $(CFLAGS) graphicstests.cpp ../source/graphics.cpp ../source/stringToDataHelpers.cpp

audiotests.o: audiotests.cpp ../source/Audio.cpp ../source/Audio.h \
	../source/synth.cpp ../source/synth.h \
	../source/PicoRam.h ../source/hostVmShared.h
	g++ $(CFLAGS) audiotests.cpp ../source/Audio.cpp ../source/synth.cpp


runtests: testrunner.a
	./testrunner.a

clean:
	-rm -f testrunner.a *.o

.PHONY: all buildtests runtests clean