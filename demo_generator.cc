
#include <vector>
#include <iomanip>
#include <iostream>

#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

#include "demo_generator.h"

namespace rx = rapidxml;

bool ProgramGenerator::Parse() {
	rx::file<> inFile(xml_source.c_str());
	rx::xml_document<> doc;
	rx::xml_node<> *node;

	// Parse XML document
	doc.parse<0>(inFile.data());
	std::cout << "Loaded (" << inFile.size() << " Bytes) XML file:\n" << std::endl;

	// Opening steps output file
	stepsOut.open(stp_source.c_str(),
			std::ios_base::out | std::ios_base::trunc);

	// Parse Loops
	node = doc.first_node("doxygen")->first_node("loopdef");
	for ( ; node; node = node->next_sibling("loopdef")) {
		loop_count++;
		step_count = 0;
		ParseLoop(node);
	}

	stepsOut.close();

	return true;
}

bool ProgramGenerator::ParseLoop(rx::xml_node<> *node) {
	std::cout << "Parsing loop: "
		<< node->first_attribute("id")->value()
		<< std::endl;

	node = node->first_node("section")->first_node("member");
	for ( ; node; node = node->next_sibling("member")) {
		if (std::string(node->first_attribute("kind")->value()) != "step")
			continue;
		ParseStep(node->first_node("name"));
	}

	return true;
}

bool ProgramGenerator::ParseStep(rx::xml_node<> *node) {

	std::cout << " step# " << std::setw(3) << std::setfill('0')
		<< step_count++ << ": "
		<< node->value() << "..." << std::endl;

	stepsOut << "\n\n// STEP " << step_count << std::endl;
	stepsOut << "Step(" << node->value();

	node = node->next_sibling("param");
	for ( ; node; node = node->next_sibling("param"))
		ParsePrms(node->first_node("type"));
	stepsOut << ");" << std::endl;

	return true;
}

bool ProgramGenerator::ParsePrms(rx::xml_node<> *node) {
	char *type  = node->value();
	node = node->next_sibling();
	// char *name  = node->value();
	node = node->next_sibling();
	char *value = node->value();
	node = node->next_sibling();

	stepsOut << ",\n\tstatic_cast<"
		<< type << ">(" << value << ")";

	return true;
}

bool ProgramGenerator::Build() {
	std::cout << "\nBuilding libprogram...\n" << std::endl;
	std::system("make libprogram");
	std::cout << "\nDONE!" << std::endl;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Product Generator
////////////////////////////////////////////////////////////////////////////////

#define PRODUCT_HEADERS						\
"\n#include <precompiled.h>"

#define PRODUCT							\
"\nextern \"C\" {"						\
"\n"								\
"\nint product_run(int program_id) {"				\
"\n"								\
"\n\tswitch(program_id) {"

#define PRODUCT_DEFAULT						\
"\n\tdefault:"							\
"\n\t\treturn -1;\n"						\
"\n\t} // switch program_id\n"					\
"\n\treturn 0;\n"						\
"\n} // product_run\n\n"					\
"} // extern \"C\""

bool ProductGenerator::Parse() {
	rx::file<> inFile(xml_source.c_str());
	rx::xml_document<> doc;

	// Parse XML document
	doc.parse<0>(inFile.data());
	std::cout << "Loading (" << inFile.size() << " Bytes) XML file...";

	product_cfile.open(product_source, std::ios_base::out | std::ios_base::trunc);
	product_cfile << PRODUCT_HEADERS;

	// Parse PCBs
	rx::xml_node<> *pcb = doc.first_node("doxygen")->first_node("pcb");
	for ( ; pcb; pcb = pcb->next_sibling("pcb")) {
		ParsePCB(pcb);
	}

	// Parse Programs
	rx::xml_node<> *program = doc.first_node("doxygen")->first_node("program");
	for ( ; program; program = program->next_sibling("program")) {
		ParseProgram(program);
	}

	// Add entry to main PRODUCT files
	product_cfile << PRODUCT;
	program = doc.first_node("doxygen")->first_node("program");
	for ( ; program; program = program->next_sibling("program")) {
		int program_id = atoi(program->first_attribute("id")->value());
		std::string program_name(program->first_attribute("name")->value());
		product_cfile << "\n\tcase " << program_id << ":\n"
			<< "\t\tprogram_" << program_name << "();\n"
			<< "\t\tbreak;\n";
	}

	product_cfile << PRODUCT_DEFAULT << std::endl;

	std::cout << "\n\nGenerate product source file [./demo_product.cc]..." << std::endl;
	product_cfile.close();

	return true;
}

#define PRODUCT_PCB(NAME)					\
"\n"								\
"\nclass " << NAME << " : public ProgramControlBlock {"		\
"\npublic:"							\
"\n\t" << NAME << "(const char *name = \"" << NAME << "\") :"	\
"\n\t\tProgramControlBlock(name) {};"				\
"\nprivate:"							\
"\n\tint8_t loops = 5;"						\
"\n\tuint8_t Setup();"						\
"\n\tuint8_t Loop();"						\
"\n\tuint8_t Cleanup();"					\
"\n};"								\
"\n"

#define PRODUCT_PCB_SETUP(NAME)					\
"\nuint8_t " << NAME << "::Setup() {"

#define PRODUCT_PCB_CLEANUP(NAME)				\
"\n\treturn 0;"							\
"\n}"								\
"\nuint8_t " << NAME << "::Cleanup() {"

#define PRODUCT_PCB_LOOP(NAME)					\
"\n\treturn 0;"							\
"\n}"								\
"\nuint8_t " << NAME << "::Loop() {"				\
"\n\tvoid *step[] = {"

#define PRODUCT_PCB_LOOP_CODE(NAME)				\
"\n"								\
"\n\tprintf(\"[%10s] Loops remaining %d\\n\", name, loops--);"	\
"\n\tif (loops <= 0)"						\
"\n\t\treturn 1;"						\
"\n"								\
"\n\tDebugCheck();"						\
"\n"								\
"\nloop_start:"							\
"\n\tif (step_out < 0 || step_out >= steps_count)"		\
"\n\t\tstep_out = steps_count - 1;"				\
"\n\tif (step_into >= 0)"					\
"\n\t\tgoto *step[step_into];"					\
"\n"

bool ProductGenerator::ParsePCB(rapidxml::xml_node<>* pcb) {
	rx::xml_node<> *node, *inb, *outb, *lb, *step;

	// Get PCB name
	std::string name(pcb->first_attribute("name")->value());
	std::cout << "\n\nParse PCB: " << name << "..." << std::endl;

	// Generate PCB Header
	product_cfile << PRODUCT_PCB(name);

	// Generate PCB Setup
	product_cfile << PRODUCT_PCB_SETUP(name);
	// TODO add here PCB Setup generation

	// Generate PCB Cleanup
	product_cfile << PRODUCT_PCB_CLEANUP(name);
	// TODO add here PCB Cleanup generation

	// Generate PCB Loop (Setup)
	product_cfile << PRODUCT_PCB_LOOP(name);
	node = pcb->first_node("steps");
	step = (!node) ? nullptr : node->first_node("step");
	for (int i = 0 ; step; step = step->next_sibling("step")) {
		if (!(i % 10))
			product_cfile << "\n\t\t";
		product_cfile << "&&step_" << std::setfill('0') << std::setw(4)
			<< i++ << ", ";
	}
	product_cfile << "\n\t\t&&loop_start};";
	product_cfile << "\n\tuint16_t steps_count = (sizeof(step) / sizeof(void*));";
	product_cfile << "\n\tpcb_steps = step;";

	// Generate PCB Loop (Code)
	product_cfile << PRODUCT_PCB_LOOP_CODE(name);

	product_cfile << "\n\n\t" << std::string(80, '/');
	product_cfile << "\n\t// Consumer Buffers PULL" << std::endl;

	// Parse input barriers
	product_cfile << "\n\t// Input buffers";
	node = pcb->first_node("pads");
	inb = (!node) ? nullptr : node->first_node("in_barrier");
	for (int i = 0 ; inb; inb = inb->next_sibling("in_barrier")) {
		product_cfile
			<< "\n\t" << inb->first_attribute("type")->value()
			<< " " << inb->first_attribute("name")->value()
			<< " = in_barriers[" << i++
			<< "]->pull_buffer(name, tid);"
			<< std::endl;
	}

	product_cfile << "\n\n\t" << std::string(80, '/');
	product_cfile << "\n\t// Output and Local buffers initialization" << std::endl;

	// Parse output barriers
	product_cfile << "\n\t// Output buffers";
	node = pcb->first_node("pads");
	outb = (!node) ? nullptr : node->first_node("out_barrier");
	for (int i = 0 ; outb; outb = outb->next_sibling("out_barrier")) {
		product_cfile
			<< "\n\t" << outb->first_attribute("type")->value()
			<< " " << outb->first_attribute("name")->value()
			<< " = out_barriers[" << i++
			<< "]->producerBuffer();"
			<< std::endl;
	}

	// Parse local buffers
	product_cfile << "\n\t// Local buffers";
	product_cfile << "\n\t// NOTE: These should be recovered via BufferManager";
	product_cfile << "\n\t//       for the time being this example handles just integer buffers";
	node = pcb->first_node("buffers");
	lb = (!node) ? nullptr : node->first_node("buffer");
	for ( ; lb; lb = lb->next_sibling("buffer")) {
		product_cfile
			<< "\n\t"
			<< "int" //lb->first_attribute("type")->value()
			<< " " << lb->first_attribute("name")->value()
			<< "[] = {0};" << std::endl;
	}


	product_cfile << "\n\n\t" << std::string(80, '/');
	product_cfile << "\n\t// Processing STEPS" << std::endl;

	// Parse Steps
	node = pcb->first_node("steps");
	step = (!node) ? nullptr : node->first_node("step");
	for (int i = 0 ; step; ++i, step = step->next_sibling("step")) {
		product_cfile << "\n\t// STEP " << i;
		product_cfile << "\nstep_" << std::setfill('0') << std::setw(4) << i << ":";
		product_cfile << "\n\tStep<" << i << ">(" << step->first_attribute("id")->value();

		rx::xml_node<> *param = step->first_node("param");
		for ( ; param; param = param->next_sibling("param")) {
			product_cfile << ",\n\t\tstatic_cast<"
				<< param->first_attribute("type")->value()
				<< ">(" << param->first_attribute("value")->value() << ")";
		}
		product_cfile << ");" << std::endl;
	}

	product_cfile << "\n\n\t" << std::string(80, '/');
	product_cfile << "\n\t// Producer Buffers PUSH" << std::endl;

	// Parse output barriers
	node = pcb->first_node("pads");
	outb = (!node) ? nullptr : node->first_node("out_barrier");
	for (int i = 0 ; outb; outb = node->next_sibling("out_barrier")) {
		product_cfile <<
			"\tout_barriers[" << i++ << "]->push_buffer(name);"
			<< std::endl;
	}

	product_cfile << "\n\treturn 0;\n}\n";

	return true;
}

#define PRODUCT_PROGRAM						\
"\n"								\
"\nextern \"C\" {"						\
"\n"


bool ProductGenerator::ParseProgram(rapidxml::xml_node<>* program) {
	std::string program_name(program->first_attribute("name")->value());
	int sb_count = 0;

	std::cout << "\n\nParse Program: " << program_name << "..." << std::endl;

	// Generate PCB Header
	product_cfile << PRODUCT_PROGRAM;
	product_cfile << "\nvoid program_" << program_name << "() {\n";

	// Count required sync buffers
	rx::xml_node<> *sb = program->first_node("link");
	for ( ; sb; sb = sb->next_sibling("link"))
		++sb_count;

	if (!sb_count)
		goto do_pcbs;

	// Pre-allocated buffers (to be replaced by BufferManager calls)
	product_cfile << "\n\n\t" << std::string(80, '/');
	product_cfile << "\n\t// Pre-Allocated Int (double) buffers\n" << std::endl;

	product_cfile << "\tint buffers[" << sb_count << "][2] = {\n\t\t";
	while (sb_count--) {
		product_cfile << "{0,0}, ";
	}
	product_cfile << "};" << std::endl;

	// Synchronization BARRIERS
	product_cfile << "\n\n\t" << std::string(80, '/');
	product_cfile << "\n\t// Synchronization BARRIERS\n" << std::endl;

	sb = program->first_node("link");
	for (int i = 0 ; sb; ++i, sb = sb->next_sibling("link")) {
		const char *name   = sb->first_attribute("name")->value();
		const char *type   = sb->first_attribute("type")->value();
		const char *ccount = sb->first_attribute("ccount")->value();
		product_cfile << "\tp" << type << "_t " << name
			<< " = p" << type << "_t(new " << type
			<< "_t(buffers[" << i << "], buffers[" << i << "]+1, "
			<< "\"" << name << "\", " << ccount << "));\n";
	}

do_pcbs:

	// Program Control Blocks
	product_cfile << "\n\n\t" << std::string(80, '/');
	product_cfile << "\n\t// ProgramControlBlocks and Connection\n" << std::endl;

	rx::xml_node<> *pcb = program->first_node("pcb");
	for (int i = 1 ; pcb; ++i, pcb = pcb->next_sibling("pcb")) {
		const char *name = pcb->first_attribute("name")->value();
		const char *type = pcb->first_attribute("type")->value();
		product_cfile << "\t// PCB #" << std::setfill('0') << std::setw(3)
			<< i << ": " << name << "\n\t"
			<< "pcb_t pcb" << name << " = pcb_t(new "
			<< type << "(\"" << name << "\"));\n";

		// Input barriers
		// e.g. pcbNAME->AddInBarrier(BARRIER_NAME);
		rx::xml_node<> *ib = pcb->first_node("input");
		for ( ; ib; ib = ib->next_sibling("input")) {
			const char *ibname = ib->first_attribute("name")->value();
			product_cfile << "\tpcb" << name << "->AddInBarrier("
				<< ibname << ");\n";
		}

		// Output barriers
		// e.g. pcbNAME->AddOutBarrier(BARRIER_NAME);
		rx::xml_node<> *ob = pcb->first_node("output");
		for ( ; ob; ob = ob->next_sibling("output")) {
			const char *obname = ob->first_attribute("name")->value();
			product_cfile << "\tpcb" << name << "->AddOutBarrier("
				<< obname << ");\n";
		}

	}

	// Network Startup
	product_cfile << "\n\n\t" << std::string(80, '/');
	product_cfile << "\n\t// Program START\n" << std::endl;

	pcb = program->first_node("pcb");
	for ( ; pcb; pcb = pcb->next_sibling("pcb")) {
		const char *name = pcb->first_attribute("name")->value();
		product_cfile << "\tpcb" << name << "->Run();\n";
	}

	// Wait Completion
	product_cfile << "\n\n\t" << std::string(80, '/');
	product_cfile << "\n\t// Completion WAIT\n" << std::endl;

	pcb = program->first_node("pcb");
	for ( ; pcb; pcb = pcb->next_sibling("pcb")) {
		const char *name = pcb->first_attribute("name")->value();
		product_cfile << "\tpcb" << name << "->WaitCompletion();\n";
	}

	product_cfile << "\n} // program\n} // extern \"C\"" << std::endl;
	return true;
}

bool ProductGenerator::Build() {
	std::cout << "\nBuilding libproduct...\n" << std::endl;
	std::system("make libproduct");
	std::cout << "\nDONE!" << std::endl;

	return true;
}

