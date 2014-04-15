
CFLAGS=-Wall -pthread -std=c++11 -I. -Iinclude
LDFLAGS=-shared -Wl,-rpath,.

all: libprogram demo

libxtil: libxtil.so.1.0.1
libxtil.so.1.0.1: demo_xtil.cc
	@echo "=== Building XTIL example library [libxtil.so.1]..."
	g++ -fPIC -c $(CFLAGS) demo_xtil.cc -o demo_xtil.o
	g++ $(LDFLAGS) -Wl,-soname,libxtil.so.1 -o libxtil.so.1.0.1 demo_xtil.o
	[ -f libxtil.so.1 ] || ln -s libxtil.so.1.0.1 libxtil.so.1
	[ -f libxtil.so ]   || ln -s libxtil.so.1.0.1 libxtil.so

libsteps: libsteps.so.1.0.1
libsteps.so.1.0.1: libxtil.so.1.0.1 demo_steps.cc
	@echo "=== Building STEPS example library [libsteps.so.1]..."
	g++ -fPIC -c $(CFLAGS) demo_steps.cc -o demo_steps.o
	g++ $(LDFLAGS) -Wl,-soname,libsteps.so.1 -o libsteps.so.1.0.1 \
		demo_steps.o -L. -lxtil
	[ -f libsteps.so.1 ] || ln -s libsteps.so.1.0.1 libsteps.so.1
	[ -f libsteps.so ]   || ln -s libsteps.so.1.0.1 libsteps.so

libproduct: libproduct.so.1.0.1
libproduct.so.1.0.1: demo_product.cc
	@echo "=== Building PRODUCT demo..."
	g++ -fPIC -c $(CFLAGS) demo_product.cc -o demo_product.o
	g++ $(LDFLAGS) -Wl,-soname,libproduct.so.1 -o libproduct.so.1.0.1 \
		sync_barrier.o program_control_block.o demo_product.o -L. -lsteps
	[ -f libproduct.so.1 ] || ln -s libproduct.so.1.0.1 libproduct.so.1
	[ -f libproduct.so ]   || ln -s libproduct.so.1.0.1 libproduct.so

precompile: precompiled.h.gch sync_barrier.o program_control_block.o
precompiled.h.gch: precompiled.h
	@echo "=== Building PreCompiled headers..."
	g++ -fPIC $(CFLAGS) precompiled.h
sync_barrier.o: sync_barrier.cc
	@echo "=== Building PreCompiled Barriers..."
	g++ -fPIC -c $(CFLAGS) -o $@ $<
program_control_block.o: program_control_block.cc
	@echo "=== Building PreCompiled PCB..."
	g++ -fPIC -c $(CFLAGS) -o $@ $<

demo: demo_core.cc demo_generator.cc libsteps.so.1.0.1 precompile
	@echo "=== Building DEMO CORE..."
	g++ $(CFLAGS) -c -o demo_core.o demo_core.cc
	g++ $(CFLAGS) -c -o demo_generator.o demo_generator.cc
	g++ $(CFLAGS) -o demo demo_core.o demo_generator.o -ldl

PHONY: test100 test300 test1000
test100:
	@echo "=== Test [100 steps] build time"
	@for i in `seq 10`; do \
		./demo build_test_100.xml 2>&1 | \
		grep '>>> Product build'; \
	done
test300:
	@echo "=== Test [300 steps] build time"
	@for i in `seq 10`; do \
		./demo build_test_300.xml 2>&1 | \
		grep '>>> Product build'; \
	done
test1000:
	@echo "=== Test [1000 steps] build time"
	@for i in `seq 10`; do \
		./demo build_test_1000.xml 2>&1 | \
		grep '>>> Product build'; \
	done

clean:
	rm -f *.o *.gch libxtil* libsteps* libprogram* libproduct* demo

