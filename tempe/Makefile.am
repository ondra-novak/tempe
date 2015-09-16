ACLOCAL_AMFLAGS = -I build/m4 

SUBDIRS = .
LIGHTSPEED_H = $(abs_top_srcdir)/../lightspeed/src
LIGHTSPEED_LIB= $(abs_top_srcdir)/../lightspeed


noinst_LIBRARIES = libaexpress.a
noinst_PROGRAMS = exprtest


.PHONY: libaexpress

libaexpress.a: FORCE
	$(MAKE) -C build --file makefile all "GPP=g++ -O3  -c -fno-strict-aliasing -Wall -fPIC -I$(LIGHTSPEED_H)"
	cp build/libtempe.a ./
                
exprtest: test.cpp test.h 
	g++ -O3 -o tempetest -fno-strict-aliasing -Wall -I$(LIGHTSPEED_H) -Isrc test.cpp -L"$(LIGHTSPEED_H)/.." -L. -L"$(LIGHTSPEED_LIB)" -ltempe -llightspeed -lpthread
	
clean-local:
	$(MAKE) -k -C build --file makefile clean
	rm -f ./libtempe.a
	rm exprtest

distclean-local:
	$(MAKE) -k -C build --file makefile clean
	rm -f ./libtempe.a
	rm exprtest

FORCE:
