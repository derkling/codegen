
CFLAGS=-Wall -pthread -std=c++11 -Iinclude -I.
LDFLAGS=-shared -Wl,-rpath,.

all: libprogram demo

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

PCBs=$(wildcard gen/pcb*.cc)
PRGs=$(wildcard gen/program*.cc)

PCBo=$(PCBs:.cc=.o)
PRGo=$(PRGs:.cc=.o)

PHONY: gen_info
gen_info:
	@echo "=== Product components:"
	@echo "Program Control Blocks: $(PCBs)"
	@echo "Programs: $(PRGs)"

PHONY: pcbs
pcbs: $(PCBo)
pcb_%.o: pcb_%.cc
	@echo "=== Building PCB [$@]..."
	g++ -fPIC -c $(CFLAGS) $< -o $@

gen/pcbs.h: $(PCBs)
	@echo "=== Building PCBs decriptor [$@]"
	@awk '/^class/{P=1}; //{if(P)print $0};  /^};/{P=0; print ""}' \
		gen/pcb_*.cc > gen/pcbs.h

PHONY: programs
programs: $(PRGo)
program_%.o: program_%.cc gen/pcbs.h
	@echo "=== Building Program [$@]..."
	g++ -fPIC -c $(CFLAGS) $< -o $@

libproduct: $(PCBo) $(PRGo) gen/demo_product.cc
	@echo "=== Building PRODUCT demo..."
	g++ -fPIC -c $(CFLAGS) gen/demo_product.cc -o gen/demo_product.o
	g++ $(LDFLAGS) -Wl,-soname,$@.so.1 -o $@.so.1.0.1 \
		$(PCBo) $(PRGo) gen/demo_product.o -L. -lsteps
	[ -f $@.so.1 ] || ln -s $@.so.1.0.1 $@.so.1
	[ -f $@.so ]   || ln -s $@.so.1.0.1 $@.so

demo: demo_core.cc demo_generator.cc
	@echo "=== Building DEMO CORE..."
	[ -d gen ] || mkdir gen
	g++ $(CFLAGS) -c -o demo_core.o demo_core.cc
	g++ $(CFLAGS) -c -o demo_generator.o demo_generator.cc
	g++ $(CFLAGS) -o demo demo_core.o demo_generator.o -ldl

clean:
	rm -f *.o libxtil* libsteps* libprogram* demo
	rm -f gen/*.o gen/*.in gen/demo_product.* gen/pcb_*.cc gen/program_*.cc

