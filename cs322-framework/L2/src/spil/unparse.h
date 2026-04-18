
#ifndef L2_SPIL_UNPARSE
#define L2_SPIL_UNPARSE

#include "../ast.h"

#include <vector>
#include <string>

namespace L2
{
	namespace Spil
	{

		struct Unparser
		{
			template< typename KWNode > requires L2::IsKWNode< KWNode >
			std::string operator()( const KWNode &kw_n )
			{
				return std::string { KWNode::kw };
			}

			std::string operator()( const nameNode name_n )
			{
				return std::string { name_n.val };
			}

			std::string operator()( const MNode &M_n )
			{
				return std::to_string( M_n.val );
			}

			std::string operator()( const NNZNode &n_nz_n )
			{
				return std::to_string( n_nz_n.val );
			}

			template< typename VariantNode > requires ::is_variant_v< VariantNode >
			std::string operator()( const VariantNode &variant_n )
			{
				return std::visit( *this, variant_n );
			}

			template< typename Node >
			void operator()( const Node &n, std::string &sbuf )
			{
				sbuf += ( *this )( n );
			}

			template< typename ...Nodes >
			std::string operator()( const Nodes & ...ns )
			{
				std::string sbuf {};
				sbuf.reserve( 32 );
				( ( *this )( ns, sbuf ) , ... );
				sbuf += "\n";
				return std::move( sbuf );
			}

		};
	}
}

#endif

