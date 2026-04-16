
#include "var_vis.h"

#include <cstdio>

int main()
{
	std::printf( "%d\n", L2::Liv::IsGPR< L2::R9Node > );
	std::printf( "%d\n", L2::Liv::IsGPR< L2::RspNode > );

	L2::Liv::VarVisitor varvis {};

	std::printf( "%d\n", 
		varvis( L2::R9Node {} )
	);

	// treat as variant

	std::printf( "%d\n", 
		std::visit( varvis, L2::aNode { L2::R9Node {} } )
	);


	std::printf( "%d\n", 
		varvis( L2::aNode { L2::R9Node {} } )
	);

}

