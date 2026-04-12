

#include "ast.h"
	

#include "parser.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <regex>
#include <string>
#include <string_view>

int
main( int argc, char *argv[] )
{
	if ( argc < 2 )
	{
		std::fprintf( stderr, "Usage: parser_test <test_file_path>\n" );
		return 2;
	}

	const std::string_view a1v { argv[ 1 ] };
	const bool test_f { a1v == "-f" };
	const bool test_i { a1v == "-i" };

	std::ifstream test_ifs { argv[ argc-1 ], std::ios_base::in };
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

	bool success {};
	if ( test_f )
	{
		// test function
		success = prsr.make_node< L1::fNode >()? true: false;
	}
	else if ( test_i )
	{
		// test instruction
		success = prsr.make_node< L1::iNode >()? true :false;
	}
	else
	{
		// test program
		success = prsr.make_node< L1::pNode >()? true: false;
	}

	std::cout << "parse() success: " << success << "\n";

}

