
#include "liveness.h"
#include "ast.h"

#include <cstdio>
#include <variant>

int
main()
{
	L2::LivenessVisitor lv {};
	std::printf(
		"%d %d\n",
		lv( L2::RdxNode{} ),
		//lv( L2::RspNode{} ) // should fail to compile
		lv( L2::R13Node() )
	);
}

