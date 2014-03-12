
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
