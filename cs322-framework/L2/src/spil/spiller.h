
#ifndef L2_SPIL_SPILLER_H
#define L2_SPIL_SPILLER_H

#include "../ast.h"

#include <string>
#include <cstdint>
#include <string_view>

namespace L2
{
	namespace Spil
	{
		//
		// unlike the variable visitor in L2::Liv, the one we use for spilling
		// doesn't need to maintain mapping state between variables and IDs;
		// it only extracts the name
		//
		struct VarVisitor
		{
			//
			// we might encounter non-GPR KW nodes, but it's fine as long as the KW
			// doesn't match the variable name we're looking for
			//
			template< typename KWNode > requires L2::IsKWNode< KWNode >
			std::string_view operator()( const KWNode &kw_n ) { return KWNode::kw; }

			std::string_view operator()( const nameNode &name_n ) { return name_n.val; }

			std::string_view operator()( const varNode &var_n ) { return ( *this )( var_n.name_n ); }

			std::string_view operator()( const VariantNode &variant_n ) { return ( *this )( variant_n ) };

			template< typename Node >
			std::string_view operator()( const Node &n ) { return L2::EMPTYTOK; }

		};

		//
		// spills a variable from AST nodes ( primarily iNode alternatives, but can accept fNode as well )
		// creating temporary nodes as needed, then directly printing out the unparsed nodes.
		//
		// there is no effort to add the temporary nodes to a new std::vector< iNode >,
		// as the memory cost of this step is unnecessary given the current objective.
		//
		struct Spiller
		{

			//
			// in spilling mode, let the parser parse fNode ( function code ), 
			// varNode ( var to spill ), and varNode ( alias prefix ),
			// initialize spiller with the latter two's names,
			// and call spiller's overload on the fNode
			//

			std::string spill_var {};
			std::string alias_prefix {};

			//
			// incase we need to spill more than one variables 
			// in the future
			// 
			std::size_t spill_var_id { 0 };

			std::size_t next_alias_id { 0 };

			L2::Spil::VarVisitor var_vis {};

			// 
			// the variable to spill becomes a stack variable,
			// which is used to initialize unparser's stk_var_cnt
			//
			L2::Spil::Unparser unparser {};

			L2::fNode f_spill {};

			Spiller( void );

			Spiller(
				const std::string_view spill_var_,
				const std::string_view alias_prefix_
			);

			std::size_t new_alias_id( void );

			//
			// make a node with the current variable alias
			// that can be substituted into the spilled instruction
			//
			inline L2::nameNode make_alias_nameNode( void )
			{
				return nameNode
				{
					.val = std::string { L2::KW::PERCENT }
						+ std::to_string( this->next_alias_id )
				};
			}
			inline L2::varNode make_alias_varNode( void )
			{
				return varNode { .percent_n = {}, .name_n = this->make_alias_nameNode() };
			}
			inline L2::sxNode make_alias_sxNode( void ) { return sxNode { this->make_alias_varNode() }; }
			inline L2::aNode make_alias_aNode( void ) { return aNode { this->make_alias_sxNode() }; }
			inline L2::wNode make_alias_wNode( void ) { return wNode { this->make_alias_aNode() }; }
			inline L2::uNode make_alias_uNode( void ) { return uNode { this->make_alias_wNode() }; }
			inline L2::xNode make_alias_xNode( void ) { return xNode { this->make_alias_wNode() }; }
			inline L2::tNode make_alias_tNode( void ) { return tNode { this->make_alias_xNode() }; }
			inline L2::sNode make_alias_sNode( void ) { return sNode { this->make_alias_tNode() }; }
	

			L2::iLoadNode make_load_node( void );

			L2::iStoreNode make_store_node( void );

			void operator()( const iAssignNode & );

		};

	}

}

#endif

