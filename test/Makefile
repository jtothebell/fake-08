 .PHONY: all buildtests runtests clean
 
 all: buildtests runtests
 
 buildtests:
	g++ testroot.cpp \
	fontdatatests.cpp ../source/fontdata.cpp \
	-o testexec.a

runtests:
	./testexec.a

clean:
	rm testexec.a