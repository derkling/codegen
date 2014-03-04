
#include <stdint.h>
#include <stdio.h>

extern "C" {
void program_entry();
}

int main(int argc, const char *argv[])
{
	printf("CORE Demo\n");

	program_entry();

	return 0;
}

