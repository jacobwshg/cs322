

#include "ast.h"
	

#include "parser.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <regex>


int
main( int argc, char *argv[] )
{
	if ( argc < 2 )
	{
		std::fprintf( stderr, "Usage: parser_test <test_file_path>\n" );
		return 2;
	}
	std::ifstream test_ifs { argv[ 1 ], std::ios_base::in };
	if ( test_ifs.fail() )
	{
		std::fprintf( stderr, "Error: unable to open test file\n" );
		return 2;
	}

	std::cout << "Concept test\n";
	if constexpr( L1::IsRecNode< L1::pNode > )
	{
		std::cout<<"pNode is RecNode\n\n";
	}

	std::cout << "Regex tests\n";
	std::cout << std::regex_match( "+18342", L1::NNZNode::re ) << "\n";
	std::cout << std::regex_match( "-0", L1::NNZNode::re ) << "\n";
	std::cout << std::regex_match( "_b26", L1::nameNode::re ) << "\n";
	std::cout << std::regex_match( "Z0", L1::nameNode::re ) << "\n";
	std::cout << "\n";
	
	L1::Parser prsr {};
	prsr.lex( test_ifs );
	prsr.print_toks();

	const bool success { prsr.parse() };
	std::cout << "parse() success: " << success << "\n";

}

