
#include "ast.h"
#include "parser.h"
#include "codegen.h"

#include "../../util/utils.h"

#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

void
print_help( char *progName )
{
	std::cerr
		<< "Usage: " << progName
		<< " [-v] [-g 0|1] [-O 0|1|2] SOURCE [INPUT_FILE]" << "\n";
	return;
}

int
main( int argc, char **argv )
{
	bool enable_code_generator { true };
	int32_t optLevel { 3 };

	/*
	 * Check the compiler arguments.
	 */
	Utils::verbose = false;
	if ( argc < 2 )
	{
		print_help( argv[ 0 ] );
		return 1;
	}
	int32_t opt {};
	while ( ( opt = getopt( argc, argv, "vg:O:" ) ) != -1 )
	{
		switch ( opt )
		{
		case 'O':
			optLevel = std::strtoul( optarg, NULL, 0 );
			break;

		case 'g':
			enable_code_generator = 
				( std::strtoul( optarg, NULL, 0 ) == 0 ) 
				? false 
				: true;
			break;

		case 'v':
			Utils::verbose = true;
			break;

		default:
			print_help( argv[ 0 ] );
			return 1;
		}
	}

	/*
	 * Parse the input file.
	 */
	const std::string_view path { argv[ argc - 1 ] };
	std::ifstream ifs { path.data(), std::ios_base::in  };
	if ( !ifs )
	{
		std::fprintf( stderr, "error: unable to open input file %s \n", path.data() );
		return 2;
	}

	L1::Parser parser {};
	parser.lex( ifs );

	const std::optional< L1::pNode > ast_opt { parser.parse() };
	if ( !ast_opt )
	{
		std::fprintf( stderr, "error: failed to parse program \n" );
		return 2;
	}

	/*
	 * Print the source program.
	 */
	if ( Utils::verbose )
	{
		std::ifstream vb_ifs { path.data() };
		while ( vb_ifs )
		{
			vb_ifs.get( *std::cout.rdbuf() );
			std::cout << "\n";
		}
	}

	/*
	 * Generate x86_64 assembly.
	 */
	if ( enable_code_generator )
	{
		L1::CodeGenerator cgr {};
		cgr.emit( std::cout, *ast_opt );
	}

	return 0;
}

