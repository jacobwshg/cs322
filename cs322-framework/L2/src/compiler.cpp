
#include "liveness.h"
#include "interference.h"
#include "spill.h"
#include "ast.h"
#include "parser.h"
#include "var_view.h"
#include "label_view.h"

#include <utils.h>

#include <unistd.h>

#include <algorithm>
#include <assert.h>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <cstdint>
#include <cstdlib>
//#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


void print_help( const std::string_view progname )
{
	std::cerr
		<< "Usage: "
		<< progname << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l] [-i] SOURCE" << "\n";
	return;
}

int main( int argc, char **argv )
{
	bool
		enable_code_generator { true },
		spill_only            { false },
		interference_only     { false },
		liveness_only         { false };
	std::int32_t optLevel { 3 };

	/*
	 * Check the compiler arguments.
	 */
	Utils::verbose = false;
	if ( argc < 2 )
	{
		print_help( argv[0] );
		return 1;
	}

	std::int32_t opt {};
	std::int64_t functionNumber { -1 };

	while (
		( opt = getopt( argc, argv, "vg:O:slif:" ) ) 
		!= -1
	)
	{
		switch ( opt )
		{

		case 'l':
			liveness_only = true;
			break;

		case 'i':
			interference_only = true;
			break;

		case 's':
			spill_only = true;
			break;

		case 'O':
			optLevel = std::strtoul( optarg, NULL, 0 );
			break;

		case 'f':
			functionNumber = std::strtoul( optarg, NULL, 0 );
			break;

		case 'g':
			enable_code_generator = ( std::strtoul ( optarg, NULL, 0 ) == 0 ) 
				? false : true;
			break;

		case 'v':
			Utils::verbose = true;
			break;

		default:
			print_help( argv[ 0 ] );
			return 1;
		}
	}

	const std::string_view inf { argv[ argc-1 ] };
	//std::printf( "infile: %s \n", inf.data() );

	std::ifstream ifs { inf.data() };
	if ( !ifs )
	{
		std::fprintf( stderr, "unable to open \n" );
		return 2;
	}

	L2::Parser parsr {};
	parsr.lex( ifs );

	/*
	 * Parse the input file.
	 */

	std::optional< L2::fNode > 
		f_opt { std::nullopt };

	std::optional< L2::varNode > 
		spl_var_opt { std::nullopt },
		spl_prefix_opt { std::nullopt };

	std::optional< L2::pNode > 
		p_opt { std::nullopt };

	if ( spill_only )
	{
		/*
		 * Parse an L2 function and the spill arguments.
		 */
		// TODO

		f_opt = parsr.make_node< L2::fNode >();
		if ( !f_opt ) { std::fprintf( stderr, "spill: unable to parse function \n" ); return 2; }

		spl_var_opt = parsr.make_node< L2::varNode >();
		if ( !spl_var_opt ) { std::fprintf( stderr, "spill: unable to parse spill var \n" ); return 2; }

		spl_prefix_opt = parsr.make_node< L2::varNode >();
		if ( !spl_prefix_opt ) { std::fprintf( stderr, "spill: unable to parse alias prefix \n" ); return 2; }

	}
	else if ( liveness_only )
	{

		/*
		 * Parse an L2 function.
		 */
		// TODO
	
		f_opt = parsr.make_node< L2::fNode >();
		if ( !f_opt ) { std::fprintf( stderr, "spill: unable to parse function \n" ); return 2; }

	}
	else if ( interference_only )
	{

		/*
		 * Parse an L2 function.
		 */
		// TODO

		f_opt = parsr.make_node< L2::fNode >();
		if ( !f_opt ) { std::fprintf( stderr, "spill: unable to parse function \n" ); return 2; }

	}
	else
	{

		/*
		 * Parse the L2 program.
		 */
		// TODO

		p_opt = parsr.make_node< L2::pNode >();
		if ( !p_opt ) { std::fprintf( stderr, "error: unable to parse program \n" ); return 2; }

		const L2::pNode p_n { *p_opt };

	}

	/*
	 * Special cases.
	 */
	if ( spill_only )
	{

		/*
		 * Spill.
		 */

		const L2::fNode f_n { *f_opt };
		const L2::varNode spl_var_n { *spl_var_opt };
		const L2::varNode spl_prefix_n { *spl_prefix_opt };

		L2::Spill::Unparser unparsr {};

		L2::Spill::Spiller splr { f_n, spl_var_n, spl_prefix_n };
		splr.spill( f_n );

		if ( splr.spilled() )
		{
			//
			// if spilling happened, increment the unparser's count of
			// stack variables so it prints `N 1` rather than `N 0`
			//
			++unparsr.stk_var_cnt;
		}

		std::printf( "%s\n", unparsr( splr.f_spill_n ).data() );

		return 0;
	}

	/*
	 * Liveness test.
	 */
	if ( liveness_only )
	{

		//for ( ;; ){}

		const L2::fNode f_n { *f_opt };
		const std::size_t instr_cnt { f_n.i_ns.size() };

		L2::Liv::InstrVisitor fn_instr_vis { instr_cnt };

		//
		// process instruction nodes in function, storing information
		// about variables and successors
		//
		for ( const L2::iNode i_n: f_n.i_ns )
		{
			std::visit( fn_instr_vis, i_n );
		}

		fn_instr_vis.run_liveness();

		fn_instr_vis.display_test_in_out();

		return 0;
	}

	/*
	 * Interference graph test.
	 */
	if ( interference_only )
	{

		const L2::fNode f_n { *f_opt };
		const std::size_t instr_cnt { f_n.i_ns.size() };

		L2::Liv::InstrVisitor fn_instr_vis { instr_cnt };

		for ( const L2::iNode i_n: f_n.i_ns )
		{
			std::visit( fn_instr_vis, i_n );
		}

		const std::size_t var_cnt { static_cast< std::size_t >( fn_instr_vis.var_vis.next_var_id ) };

		fn_instr_vis.run_liveness();
	
		L2::Interf::InterferenceGraph igraph { var_cnt };

		igraph.add_spec_arith( f_n, fn_instr_vis.var_vis );
		igraph.add_sets( fn_instr_vis.var_id_sets );

		igraph.display( fn_instr_vis.var_vis );

		return 0;
	}

	/*
	 * Print a single L2 function case.
	 */
	if ( functionNumber != -1 )
	{
		// TODO

		return 0;
	}

	/*
	 * Generate the target code normally.
	 */
	if ( enable_code_generator )
	{
		// TODO
	}

	return 0;
}

