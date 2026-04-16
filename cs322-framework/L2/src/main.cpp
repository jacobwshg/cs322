
#include "ast.h"
#include "parser.h"
//#include "liv/var_id_set.h"
//#include "liv/var_vis.h"
//#include "liv/instr_vis.h"
#include "liveness.h"

#include <cstdio>
#include <fstream>
#include <string_view>
#include <optional>
#include <array>

int main(
	int argc,
	char *argv[]
)
{

	std::printf( "basic var visitor test\n" );

	//{
		L2::Liv::VarVisitor var_vis {};

		constexpr int CNT { 5 };

		static constexpr std::array<
			std::string_view,
			CNT
		> names
		{
			"plagueis", "sidious", "maul", "tyranus", "vader",
		};

		std::vector< L2::sxNode > sx_ns {};
		sx_ns.reserve( CNT );

		for ( const std::string_view n : names )
		{
			sx_ns.emplace_back(
				L2::sxNode
				{
					L2::varNode
					{
						.percent_n = L2::PercentNode{},
						.name_n = L2::nameNode { .val = n, },
					}
				}
			);
		}

		for ( const L2::sxNode &sx_n : sx_ns )
		{
			std::visit( var_vis, sx_n );
		}

		std::printf( "vars:\n" );
		var_vis.display();

	//}

	std::printf( "file test\n" );

	if ( argc < 2 )
	{
		std::fprintf( stderr, "usage: test <infile>\n" );
		return 2;
	}

	const std::string_view ifname { argv[ 1 ] };

	std::ifstream ifs { ifname.data() };
	if ( !ifs )
	{
		std::fprintf( stderr, "error: failed to open infile %s\n", ifname.data() );
		return 2;
	}


	L2::Parser prsr {};
	prsr.lex( ifs );

	prsr.printtoks();

	std::optional< L2::pNode > ast {  prsr.parse() };

	const bool success { ast.operator bool() };
	std::printf( "parse success: %d\n", success );
	if ( !success )
	{
		return 2;
	}

	for ( const L2::fNode &f_n: ( *ast ).f_ns )
	{
		const std::size_t instr_cnt { f_n.i_ns.size() };
		L2::Liv::InstrVisitor fn_instr_vis { instr_cnt };

		for ( const L2::iNode &i_n : f_n.i_ns )
		{
			std::visit( fn_instr_vis, i_n );
		}

		fn_instr_vis.display();

		fn_instr_vis.display_all_instr_var_sets();

		//
		// run liveness alg
		//
		fn_instr_vis.run_liveness();

	}

}

	

