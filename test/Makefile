#match flags from 3ds and switch makefiles. This may not all be necessary
CFLAGS := -Wall -g -c -O2 \
		  -fno-rtti -fexceptions -std=gnu++17


all: runtests


testrunner.a: testroot.o fontdatatests.o graphicstests.o audiotests.o cartpatchertests.o
	g++ -I$(CURDIR)/include -L$(CURDIR)/lib \
	testroot.o \
	fontdata.o fontdatatests.o \
	graphics.o graphicstests.o stringToDataHelpers.o \
	Audio.o synth.o audiotests.o \
	cartPatcher.o cartpatchertests.o \
	-llua -ldl \
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

cartpatchertests.o: liblua.a cartpatchertests.cpp ../source/cartPatcher.cpp ../source/cartPatcher.h
	g++ $(CFLAGS) -I$(CURDIR)/include -L$(CURDIR)/lib \
	cartpatchertests.cpp ../source/cartPatcher.cpp \
	-llua -ldl
	

liblua.a: 
	@echo 'starting lua build\n' &&\
	cd ../libs/lua-5.3.2 && make linux && \
	cd ../../test && \
	mkdir -p lib && \
	mkdir -p include && \
	cp ../libs/lua-5.3.2/src/liblua.a lib/liblua.a && \
	cp ../libs/lua-5.3.2/src/lua.h include/lua.h && \
	cp ../libs/lua-5.3.2/src/luaconf.h include/luaconf.h && \
	cp ../libs/lua-5.3.2/src/lualib.h include/lualib.h && \
	cp ../libs/lua-5.3.2/src/lauxlib.h include/lauxlib.h 
#make lua (cd ../libs/lua-5.3.2/src make linux, cp liblua.a, lua.h luaconf.h lualib.h lauxlib.h


runtests: testrunner.a
	./testrunner.a

clean:
	-rm -f testrunner.a *.o

echo:
	@echo "CURDIR= $(CURDIR)"
	@echo "CC= $(CC)"
	@echo "CFLAGS= $(CFLAGS)"
	@echo "LDFLAGS= $(SYSLDFLAGS)"
	@echo "LIBS= $(LIBS)"

.PHONY: all buildtests runtests clean