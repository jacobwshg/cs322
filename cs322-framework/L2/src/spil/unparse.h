
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
			static inline constexpr std::string_view
				SP  { " " },
				TAB { "\t" },
				LF  { "\n" };

			long long int stk_var_cnt { 0 };

			Unparser( void ) =default;

			Unparser( const long long int stk_var_cnt_ ):
				stk_var_cnt { stk_var_cnt_ }
			{}

			//
			// overload for string literals, such as spaces inserted between tokens
			// 
			std::string operator()( const std::string_view s )
			{
				return std::string { s };
			}

			//
			// overloads for terminal nodes
			//

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

			//
			// overloads for internal nodes
			//

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

				return std::move( sbuf );
			}

			std::string operator()( const iAssignNode & );

			std::string operator()( const iLoadNode & );
			std::string operator()( const iStoreNode & );

			std::string operator()( const iStackArgNode & );

			std::string operator()( const iAOpNode & );
			std::string operator()( const iSxNode & );
			std::string operator()( const iSOpNode & );

			std::string operator()( const iAddStoreNode & );
			std::string operator()( const iSubStoreNode & );
			std::string operator()( const iLoadAddNode & );
			std::string operator()( const iLoadSubNode & );

			std::string operator()( const iCmpAssignNode & );
			std::string operator()( const iCJumpNode & );
			std::string operator()( const iLabelNode & );
			std::string operator()( const iGotoNode & );

			std::string operator()( const iReturnNode & );
	
			std::string operator()( const iCallUNode & );
			std::string operator()( const iCallPrintNode & );
			std::string operator()( const iCallInputNode & );
			std::string operator()( const iCallAllocateNode & );
			std::string operator()( const iCallTupleErrorNode & );
			std::string operator()( const iCallTensorErrorNode & );

			std::string operator()( const iIncrNode & );
			std::string operator()( const iDecrNode & );
			std::string operator()( const iLEANode & );

			std::string operator()( const fNode & );
			std::string operator()( const pNode & );

		};
	}
}

#endif

