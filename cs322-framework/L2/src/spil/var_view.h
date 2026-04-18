
#ifndef L2_SPIL_VARVIEW_H
#define L2_SPIL_VARVIEW_H

#include "../ast.h"

#include <string_view>

namespace L2
{
	namespace Spil
	{

		//
		// descend into a possible variable node and extract the variable's name.
		//
		struct VarViewer
		{

			std::string_view operator()( const nameNode &name_n ) { return name_n.val; }

			std::string_view operator()( const varNode &var_n ) { return ( *this )( var_n.name_n ); }

			//
			// for reg nodes
			//
			template< typename KWNode > requires L2::IsKWNode< KWNode >
			std::string_view operator()( const KWNode &kw_n ) { return KWNode::kw; }

			//
			// for t, x, w, a, sx - recurse
			//
			template< typename VariantNode > requires ::is_variant_v< VariantNode >
			std::string_view operator()( const VariantNode &variant_n ) { return std::visit( *this, variant_n ); }

			//
			// for other nodes encountered along the way that are
			// irrelevant, such as N
			//
			template< typename Node >
			std::string_view operator()( const Node &n  ) { return L2::EMPTYTOK; }

		};

	}

}

#endif

