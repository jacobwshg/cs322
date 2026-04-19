
#include "ast.h"
#include <string_view>

namespace L2
{

	//
	// visits nodes carrying labels and extracts them to help
	// determine successor relations.
	//
	// LabelViewer doesn't manage label state within a function, 
	// because label state is closely coupled with instruction ID, 
	// and is thus more conveniently managed by Liv::InstrVisitor. 
	// if we were to manage within LabelViewer, InstrVisitor will have to 
	// pass in a current instr ID and whether the instruction is a jump 
	// or a pure label.
	//
	struct LabelViewer
	{
		std::string_view operator()( const labelNode &label_n )
		{
			return label_n.name_n.val; 
		}
	};

}

