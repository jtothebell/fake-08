#match flags from 3ds and switch makefiles. This may not all be necessary
CFLAGS := -Wall -g -c -O2 \
		  -fno-rtti -fexceptions -std=gnu++17


all: runtests


testrunner.a: testroot.o \
	fontdatatests.o nibblehelperstests.o graphicstests.o audiotests.o cartpatchertests.o \
	carttests.o endtoendtests.o vmtests.o
	g++ -I$(CURDIR)/include -L$(CURDIR)/lib \
	testroot.o \
	fontdata.o fontdatatests.o \
	graphics.o graphicstests.o stringToDataHelpers.o \
	Audio.o synth.o audiotests.o \
	cartPatcher.o cartpatchertests.o \
	cart.o carttests.o nooplogger.o lodepng.o emojiconversion.o filehelpers.o utf8-util.o \
	vm.o endtoendtests.o picoluaapi.o Input.o \
	vmtests.o stubhost.o \
	nibblehelpers.o nibblehelperstests.o \
	-llua -ldl \
	-o testrunner.a

testroot.o: testroot.cpp doctest.h
	g++ $(CFLAGS) testroot.cpp 

fontdatatests.o: fontdatatests.cpp ../source/fontdata.cpp ../source/fontdata.h
	g++ $(CFLAGS) fontdatatests.cpp ../source/fontdata.cpp

nibblehelperstests.o: nibblehelperstests.cpp ../source/nibblehelpers.cpp ../source/nibblehelpers.h
	g++ $(CFLAGS) nibblehelperstests.cpp ../source/nibblehelpers.cpp

graphicstests.o: liblua.a fontdatatests.o graphicstests.cpp ../source/graphics.cpp ../source/graphics.h \
	../source/fontdata.cpp ../source/fontdata.h \
	../source/PicoRam.h ../source/hostVmShared.h \
	../source/stringToDataHelpers.h ../source/stringToDataHelpers.cpp \
	../source/nibblehelpers.h ../source/nibblehelpers.cpp
	g++ $(CFLAGS) -I$(CURDIR)/include -L$(CURDIR)/lib \
	graphicstests.cpp ../source/graphics.cpp \
	../source/stringToDataHelpers.cpp ../source/nibblehelpers.cpp

audiotests.o: audiotests.cpp ../source/Audio.cpp ../source/Audio.h \
	../source/synth.cpp ../source/synth.h \
	../source/PicoRam.h ../source/hostVmShared.h
	g++ $(CFLAGS) audiotests.cpp ../source/Audio.cpp ../source/synth.cpp

cartpatchertests.o: liblua.a cartpatchertests.cpp ../source/cartPatcher.cpp ../source/cartPatcher.h
	g++ $(CFLAGS) -I$(CURDIR)/include -L$(CURDIR)/lib \
	cartpatchertests.cpp ../source/cartPatcher.cpp
	
liblua.a: 
	@echo 'starting lua build\n' &&\
	cd ../libs/z8lua && make a && \
	cd ../../test && \
	mkdir -p lib && \
	mkdir -p include && \
	cp ../libs/z8lua/liblua.a lib/liblua.a && \
	cp ../libs/z8lua/lua.h include/lua.h && \
	cp ../libs/z8lua/luaconf.h include/luaconf.h && \
	cp ../libs/z8lua/lualib.h include/lualib.h && \
	cp ../libs/z8lua/lauxlib.h include/lauxlib.h && \
	cp ../libs/z8lua/fix32.h include/fix32.h 
#make lua (cd ../libs/z8lua make linux, cp liblua.a, lua.h luaconf.h lualib.h lauxlib.h

carttests.o: carttests.cpp ../source/cart.cpp ../source/cart.h \
	../libs/lodepng/lodepng.cpp ../libs/lodepng/lodepng.h \
	../source/logger.h ../source/emojiconversion.h ../source/emojiconversion.cpp \
	../source/filehelpers.h ../source/filehelpers.cpp
	cp ../libs/lodepng/lodepng.h include/lodepng.h && \
	cp ../source/logger.h include/logger.h && \
	cp ../libs/utf8-util/utf8-util.h include/utf8-util.h && \
	g++ $(CFLAGS) -I$(CURDIR)/include carttests.cpp ../source/cart.cpp ../libs/lodepng/lodepng.cpp \
	nooplogger.cpp ../source/emojiconversion.cpp ../source/filehelpers.cpp \
	../libs/utf8-util/utf8-util.cpp

endtoendtests.o: cartpatchertests.o carttests.o graphicstests.o audiotests.o \
	endtoendtests.cpp \
	../source/vm.cpp ../source/vm.h \
	stubhost.cpp stubhost.h ../source/host.h \
	../source/Input.cpp ../source/Input.h \
	../source/picoluaapi.cpp ../source/picoluaapi.h
	g++ $(CFLAGS) -I$(CURDIR)/include -L$(CURDIR)/lib \
	endtoendtests.cpp stubhost.cpp ../source/vm.cpp  ../source/picoluaapi.cpp ../source/Input.cpp

vmtests.o: endtoendtests.o \
	vmtests.cpp stubhost.cpp
	g++ $(CFLAGS) -I$(CURDIR)/include -L$(CURDIR)/lib \
	vmtests.cpp stubhost.cpp ../source/vm.cpp  ../source/picoluaapi.cpp ../source/Input.cpp
	


runtests: testrunner.a
	./testrunner.a

clean:
	-rm -f testrunner.a *.o && \
	rm -rf lib && \
	rm -rf include && \
	cd ../libs/z8lua && make clean

echo:
	@echo "CURDIR= $(CURDIR)"
	@echo "CC= $(CC)"
	@echo "CFLAGS= $(CFLAGS)"
	@echo "LDFLAGS= $(SYSLDFLAGS)"
	@echo "LIBS= $(LIBS)"

.PHONY: all buildtests runtests clean