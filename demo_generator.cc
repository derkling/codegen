
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
