
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

bool ProductGenerator::Parse() {
	rx::file<> inFile(xml_source.c_str());
	rx::xml_document<> doc;

	// Parse XML document
	doc.parse<0>(inFile.data());
	std::cout << "Loading (" << inFile.size() << " Bytes) XML file...";

	// Setup Product main files
	product_hfile.open(product_header, std::ios_base::out | std::ios_base::trunc);
	product_hfile
		<< "\nextern \"C\" {" << std::endl;

	product_cfile.open(product_source, std::ios_base::out | std::ios_base::trunc);
	product_cfile
		<< "\n#include <" << product_header << ">\n"
		<< "\nextern \"C\" {\n"
		<< "\nint product_run(int program_id) {\n"
		<< "\n\tswitch(program_id) {" << std::endl;

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


	// Complete product file
	product_cfile
		<< "\n\tdefault:"
		<< "\n\t\treturn -1;\n"
		<< "\n\t} // switch program_id\n"
		<< "\n\treturn 0;\n"
		<< "\n} // product_run\n\n"
		<< "} // extern \"C\"" << std::endl;
	std::cout << "\n\nGenerate product source file [./demo_product.cc]..." << std::endl;
	product_hfile
		<< "\n} // extern \"C\"" << std::endl;
	std::cout << "Generate product header file [./demo_product.h]..." << std::endl;
	product_hfile.close();

	return true;
}

bool ProductGenerator::ParsePCB(rapidxml::xml_node<>* pcb) {
	rx::xml_node<> *node;
	std::ofstream clean_file;
	std::ofstream setup_file;
	std::ofstream steps_file;
	std::ofstream pcb_file;

	// Get PCB name
	std::string name(pcb->first_attribute("name")->value());
	std::cout << "\n\nParse PCB: " << name << "..." << std::endl;

	std::string cleanFn = std::string("gen/pcb_") + name + "_clean.cc.in";
	std::string setupFn = std::string("gen/pcb_") + name + "_setup.cc.in";
	std::string stepsFn = std::string("gen/pcb_") + name + "_steps.cc.in";
	std::string pcbFn   = std::string("gen/pcb_") + name + ".cc";

	// Setup output files
	clean_file.open(cleanFn.c_str(), std::ios_base::out | std::ios_base::trunc);
	setup_file.open(setupFn.c_str(), std::ios_base::out | std::ios_base::trunc);
	steps_file.open(stepsFn.c_str(), std::ios_base::out | std::ios_base::trunc);
	pcb_file.open(pcbFn.c_str(),     std::ios_base::out | std::ios_base::trunc);

	steps_file << "\n\n" << std::string(80, '/');
	steps_file << "\n// Consumer Buffers PULL" << std::endl;

	// Parse input barriers
	steps_file << "\n// Input buffers" << std::endl;
	node = pcb->first_node("pads");
	rx::xml_node<> *inb = (!node) ? nullptr : node->first_node("in_barrier");
	for (int i = 0 ; inb; inb = inb->next_sibling("in_barrier")) {
		steps_file << inb->first_attribute("type")->value()
			<< " " << inb->first_attribute("name")->value()
			<< " = in_barriers[" << i++
			<< "]->pull_buffer(name, tid);"
			<< std::endl;
	}

	steps_file << "\n\n" << std::string(80, '/');
	steps_file << "\n// Output and Local buffers initialization" << std::endl;

	// Parse output barriers
	steps_file << "\n// Output buffers" << std::endl;
	node = pcb->first_node("pads");
	rx::xml_node<> *outb = (!node) ? nullptr : node->first_node("out_barrier");
	for (int i = 0 ; outb; outb = outb->next_sibling("out_barrier")) {
		steps_file << outb->first_attribute("type")->value()
			<< " " << outb->first_attribute("name")->value()
			<< " = out_barriers[" << i++
			<< "]->producerBuffer();"
			<< std::endl;
	}

	// Parse local buffers
	steps_file << "\n// Local buffers";
	steps_file << "\n// NOTE: These should be recovered via BufferManager";
	steps_file << "\n//       for the time being this example handles just integer buffers" << std::endl;
	node = pcb->first_node("buffers");
	rx::xml_node<> *lb = (!node) ? nullptr : node->first_node("buffer");
	for ( ; lb; lb = lb->next_sibling("buffer")) {
		steps_file << "int" //lb->first_attribute("type")->value()
			<< " " << lb->first_attribute("name")->value()
			<< "[] = {0};" << std::endl;
	}


	steps_file << "\n\n" << std::string(80, '/');
	steps_file << "\n// Processing STEPS" << std::endl;

	// Parse Steps
	node = pcb->first_node("steps");
	rx::xml_node<> *step = (!node) ? nullptr : node->first_node("step");
	for (int i = 0 ; step; step = step->next_sibling("step")) {
		steps_file << "\n// STEP " << ++i << "\n";
		steps_file << "Step(" << step->first_attribute("id")->value();

		rx::xml_node<> *param = step->first_node("param");
		for ( ; param; param = param->next_sibling("param")) {
			steps_file << ",\n\tstatic_cast<"
				<< param->first_attribute("type")->value()
				<< ">(" << param->first_attribute("value")->value() << ")";
		}
		steps_file << ");" << std::endl;
	}

	steps_file << "\n\n" << std::string(80, '/');
	steps_file << "\n// Producer Buffers PUSH" << std::endl;

	// Parse output barriers
	node = pcb->first_node("pads");
	outb = (!node) ? nullptr : node->first_node("out_barrier");
	for (int i = 0 ; outb; outb = node->next_sibling("out_barrier")) {
		steps_file <<
			"out_barriers[" << i++ << "]->push_buffer(name);"
			<< std::endl;
	}

	// Build the PCB main file
	std::string sed_command = std::string("") +
		"sed -e 's/MyProgram/" + name + "/g' pcb_template.cc.in > gen/pcb_" + name + ".cc";
	std::cout << "Generate [" << pcbFn << "]..." << std::endl;
	// std::cout << "\nCommand: " << sed_command << std::endl;
	std::system(sed_command.c_str());

	clean_file.close();
	setup_file.close();
	steps_file.close();
	pcb_file.close();

	return true;
}

bool ProductGenerator::ParseProgram(rapidxml::xml_node<>* program) {
	std::ofstream program_file;
	int program_id = atoi(program->first_attribute("id")->value());
	int sb_count = 0;

	// Get Program name
	std::string program_name(program->first_attribute("name")->value());
	std::cout << "\n\nParse Program: " << program_name << "..." << std::endl;

	std::string programFn = std::string("gen/program_") + program_name + ".cc";

	// Setup output files
	program_file.open(programFn.c_str(), std::ios_base::out | std::ios_base::trunc);

	program_file << "\n#include <demo_core.h>";
	program_file << "\n#include <demo_steps.h>\n";
	program_file << "\n#include <gen/pcbs.h>\n";
	program_file << "\nextern \"C\" {\n";
	program_file << "\nvoid program_" << program_name << "() {\n";

	// Count required sync buffers
	rx::xml_node<> *sb = program->first_node("link");
	for ( ; sb; sb = sb->next_sibling("link"))
		++sb_count;

	if (!sb_count)
		goto do_pcbs;

	// Pre-allocated buffers (to be replaced by BufferManager calls)
	program_file << "\n\n\t" << std::string(80, '/');
	program_file << "\n\t// Pre-Allocated Int (double) buffers\n" << std::endl;

	program_file << "\tint buffers[" << sb_count << "][2] = {\n\t\t";
	while (sb_count--) {
		program_file << "{0,0}, ";
	}
	program_file << "};" << std::endl;

	// Synchronization BARRIERS
	program_file << "\n\n\t" << std::string(80, '/');
	program_file << "\n\t// Synchronization BARRIERS\n" << std::endl;

	sb = program->first_node("link");
	for (int i = 0 ; sb; ++i, sb = sb->next_sibling("link")) {
		const char *name   = sb->first_attribute("name")->value();
		const char *type   = sb->first_attribute("type")->value();
		const char *ccount = sb->first_attribute("ccount")->value();
		program_file << "\tp" << type << "_t " << name
			<< " = p" << type << "_t(new " << type
			<< "_t(buffers[" << i << "], buffers[" << i << "]+1, "
			<< "\"" << name << "\", " << ccount << "));\n";
	}

do_pcbs:

	// Program Control Blocks
	program_file << "\n\n\t" << std::string(80, '/');
	program_file << "\n\t// ProgramControlBlocks and Connection\n" << std::endl;

	rx::xml_node<> *pcb = program->first_node("pcb");
	for (int i = 1 ; pcb; ++i, pcb = pcb->next_sibling("pcb")) {
		const char *name = pcb->first_attribute("name")->value();
		const char *type = pcb->first_attribute("type")->value();
		program_file << "\t// PCB #" << std::setfill('0') << std::setw(3)
			<< i << ": " << name << "\n\t"
			<< "pcb_t pcb" << name << " = pcb_t(new "
			<< type << "(\"" << name << "\"));\n";

		// Input barriers
		// e.g. pcbNAME->AddInBarrier(BARRIER_NAME);
		rx::xml_node<> *ib = pcb->first_node("input");
		for ( ; ib; ib = ib->next_sibling("input")) {
			const char *ibname = ib->first_attribute("name")->value();
			program_file << "\tpcb" << name << "->AddInBarrier("
				<< ibname << ");\n";
		}

		// Output barriers
		// e.g. pcbNAME->AddOutBarrier(BARRIER_NAME);
		rx::xml_node<> *ob = pcb->first_node("output");
		for ( ; ob; ob = ob->next_sibling("output")) {
			const char *obname = ob->first_attribute("name")->value();
			program_file << "\tpcb" << name << "->AddOutBarrier("
				<< obname << ");\n";
		}

	}

	// Network Startup
	program_file << "\n\n\t" << std::string(80, '/');
	program_file << "\n\t// Program START\n" << std::endl;

	pcb = program->first_node("pcb");
	for ( ; pcb; pcb = pcb->next_sibling("pcb")) {
		const char *name = pcb->first_attribute("name")->value();
		program_file << "\tpcb" << name << "->Run();\n";
	}

	// Wait Completion
	program_file << "\n\n\t" << std::string(80, '/');
	program_file << "\n\t// Completion WAIT\n" << std::endl;

	pcb = program->first_node("pcb");
	for ( ; pcb; pcb = pcb->next_sibling("pcb")) {
		const char *name = pcb->first_attribute("name")->value();
		program_file << "\tpcb" << name << "->WaitCompletion();\n";
	}

	program_file << "\n} // program\n} // extern \"C\"" << std::endl;

	std::cout << "Generate [" << programFn << "]..." << std::endl;
	program_file.close();

	if (!product_cfile)
		return true;

	// Add entry to main PRODUCT files
	product_cfile << "\n\tcase " << program_id << ":\n"
		<< "\t\tprogram_" << program_name << "();\n"
		<< "\t\tbreak;\n";

	product_hfile
		<< "\n\t// Program #" << std::setfill('0') << std::setw(2) << program_id
		<< "\n\tvoid program_" << program_name << "();\n";

	return true;
}

bool ProductGenerator::Build() {
	std::cout << "\nBuilding libproduct...\n" << std::endl;
	std::system("make libproduct");
	std::cout << "\nDONE!" << std::endl;

	return true;
}

