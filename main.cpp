
#include <string>
#include <cstdio>

int
main()
{
	const std::string_view pushs
	{
		"pushq %rbx\n" "pushq %rbp\n"
	};
	std::printf( "%s", pushs.data() );
}

