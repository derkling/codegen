
#include <string>
#include <fstream>
#include <rapidxml.hpp>

class ProgramGenerator {
public:
	bool Parse();
	bool Build();
private:
	std::string xml_source = "program_test.xml";
	std::string stp_source = "demo_program_steps.cc";
	std::ofstream stepsOut;
	int loop_count = 0;
	int step_count = 0;
	bool ParseLoop(rapidxml::xml_node<>* node);
	bool ParseStep(rapidxml::xml_node<>* node);
	bool ParsePrms(rapidxml::xml_node<>* node);
};

class ProductGenerator {
public:
	bool Parse();
	bool Build();
private:
	std::string xml_source = "build_test_100.xml";
	std::string product_header = "gen/demo_product.h";
	std::string product_source = "gen/demo_product.cc";
	std::ofstream product_hfile;
	std::ofstream product_cfile;

	bool ParsePCB(rapidxml::xml_node<>* node);
	bool ParseProgram(rapidxml::xml_node<>* node);

};
