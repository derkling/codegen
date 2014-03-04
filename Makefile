
CFLAGS=-Wall -pthread -std=c++11
LDFLAGS=-shared -Wl,-rpath,.

all: clean demo

libxtil: demo_xtil.cc
	@echo "=== Building XTIL example library [$@.so.1]..."
	gcc -fPIC -c $(CFLAGS) $< -o ${<:.cc=.o}
	gcc $(LDFLAGS) -Wl,-soname,$@.so.1 -o $@.so.1.0.1 ${<:.cc=.o}
	[ -f $@.so.1 ] || ln -s $@.so.1.0.1 $@.so.1
	[ -f $@.so ]   || ln -s $@.so.1.0.1 $@.so

libsteps: libxtil demo_steps.cc
	@echo "=== Building STEPS example library [$@.so.1]..."
	g++ -fPIC -c $(CFLAGS) demo_steps.cc -o demo_steps.o
	g++ $(LDFLAGS) -Wl,-soname,$@.so.1 -o $@.so.1.0.1 demo_steps.o -L. -lxtil
	[ -f $@.so.1 ] || ln -s $@.so.1.0.1 $@.so.1
	[ -f $@.so ]   || ln -s $@.so.1.0.1 $@.so
#	g++ -c -o /dev/null $< -fdump-translation-unit -L./ -lxtil

libprogram: libsteps demo_program.cc
	@echo "=== Building PROG demo..."
	g++ -fPIC -c $(CFLAGS) demo_program.cc -o demo_program.o
	g++ $(LDFLAGS) -Wl,-soname,$@.so.1 -o $@.so.1.0.1 demo_program.o -L. -lsteps
	[ -f $@.so.1 ] || ln -s $@.so.1.0.1 $@.so.1
	[ -f $@.so ]   || ln -s $@.so.1.0.1 $@.so

demo: libprogram demo_core.cc
	@echo "=== Building DEMO CORE..."
	g++ $(CFLAGS) -o demo demo_core.cc -Wl,-rpath,. -L. -lprogram

clean:
	rm -f *.o libxtil* libsteps* libprogram* demo

