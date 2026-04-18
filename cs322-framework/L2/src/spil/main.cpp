
#include "spiller.h"
#include "variant_gen.h"
#include "../parser.h"

#include <cstdio>
#include <iostream>
#include <fstream>

int
main( const int argc, const char *argv[] )
{
	if ( argc < 2 )
	{
		std::fprintf( stderr, "usage: test <infile>\n" );
		return 2;
	}

	std::ifstream ifs { argv[ 1 ] };
	if ( !ifs )
	{
		std::fprintf( stderr, "error: unable to open infile %s\n", argv[ 1 ] );
		return 3;
	}

	L2::Parser parser {};
	parser.lex( ifs );

	std::optional< L2::fNode > f_opt { parser.make_node< L2::fNode >() };
	if ( !f_opt )
	{
		std::fprintf( stderr, "error: unable to parse function \n" );
		return 2;
	}

	std::optional< L2::varNode >
		var_spl_opt { parser.make_node< L2::varNode >() };
	if ( !var_spl_opt )
	{
		std::fprintf( stderr, "error: unable to parse spill var \n" );
		return 2;
	}

	std::optional< L2::varNode >
		var_prefix_opt { parser.make_node< L2::varNode >() };
	if ( !var_prefix_opt )
	{
		std::fprintf( stderr, "error: unable to parse alias prefix \n" );
		return 2;
	}

	L2::Spil::Spiller splr { *f_opt, *var_spl_opt, *var_prefix_opt };
	splr.spill( *f_opt );
	splr.unparse_and_display();

}

