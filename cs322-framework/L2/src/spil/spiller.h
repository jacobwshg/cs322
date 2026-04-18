
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
		struct VarViewer
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

			std::string spill_var_name {};
			std::string alias_prefix {}; // ex. "S"

			//
			// incase we need to spill more than one variables 
			// in the future
			// 
			std::size_t spill_var_id { 0UL };

			//
			// incrementd for each instruction to spill
			// 0 -> spilled instruction(s) will use %{prefix}0
			// 1 -> spilled instruction(s) will use %{prefix}1
			// ...
			//
			std::size_t next_alias_id { 0UL };

			L2::Spil::VarViewer var_view {};

			// 
			// the variable to spill becomes a stack variable,
			// which is used to initialize unparser's stk_var_cnt
			//
			L2::Spil::Unparser unparser {};

			L2::fNode f_spill {};

			Spiller( void );

			Spiller(
				const std::string_view spill_var_name_,
				const std::string_view alias_prefix_
			);

			std::size_t new_alias_id( void );

			std::string new_alias_name( void );

			//
			// make a node with the current variable alias
			// that can be substituted into spilled instructions
			//

			using sv_t = std::string_view;

			//
			// compare a variable name with the name of the variable to spill
			//
			inline bool is_spill_var_name( const std::string_view var_name )
			{
				return this->spill_var_name == var_name;
			}

			//
			// check if var_name is the same as the variable to spill.
			// if so, make a node with the latest alias name.
			// else, make with the original var_name.
			//
			template< typename Node > Node
			try_make_alias_node( const sv_t var_name ) { return Node {}; }

			template<> nameNode
			try_make_alias_node< nameNode >( const sv_t var_name )
			{
				if ( this->is_spill_var_name( var_name ) )
				{
					return nameNode
					{
						.val = std::string { L2::KW::PERCENT } + std::to_string( this->next_alias_id )
					};
				}
				else
				{
					return nameNode { .val = var_name };
				}
			}
			template<> varNode
			try_make_alias_node< varNode >( const sv_t var_name )
			{
				return varNode { .percent_n = {}, .name_n = this->try_make_alias_node< nameNode >( var_name ) };
			}
			template<> sxNode
			try_make_alias_node< sxNode >( const sv_t var_name )
			{
				return sxNode { this->try_make_alias_node< varNode >( var_name ) };
			0}
			template<> aNode
			try_make_alias_node< aNode >( const sv_t var_name )
			{
				return aNode { this->try_make_alias_node< sxNode >( var_name ) };
			}
			template<> wNode
			try_make_alias_node< wNode >( void )
			{
				return wNode { this->try_make_alias_node< aNode >( var_name ) };
			}
			template<> uNode
			try_make_alias_node< uNode >( void )
			{
				return uNode { this->try_make_alias_node< wNode >( var_name ) };
			}
			template<> xNode
			try_make_alias_node< xNode >( void )
			{
				return xNode { this->try_make_alias_node< wNode >( var_name ) };
			}
			template<> tNode
			try_make_alias_node< tNode >( void )
			{
				return tNode { this->try_make_alias_node< xNode >( var_name ) };
			}
			template<> sNode
			try_make_alias_node< sNode >( void )
			{
				return sNode { this->try_make_alias_node< tNode >( var_name ) };
			}
		
			/*
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
			*/

			//
			// make a load/store node where non-memory location is DEFINITELY replaced with 
			// the latest alias ( no comparison with spill var name )
			//
			L2::iLoadNode make_alias_iLoadNode( void );
			L2::iStoreNode make_alias_iStoreNode( void );

			// 
			// add an instr node to the spilled function node's instr vector
			//
			void add_iNode( const iNode &&i_n ) { this->f_spill.i_ns.emplace_back( i_n ); }
			template< typename iNodeAlt > void
			add_iNode( const iNodeAlt &&i_n_alt ) { this->add_iNode( iNode { i_n_alt } ); }

			//
			// if spill is true ( = the non-memory location is the spill variable )
			// then add an aliased load/store instr to the function node member;
			// else, do nothing
			//
			void try_add_alias_iLoadNode( const bool spill )
			{
				if ( spill ) { this->add_iNode( this->make_alias_iLoadNode() ); }
			}
			void try_add_alias_iStoreNode( const bool spill )
			{
				if ( spill ) { this->add_iNode( this->make_alias_iStoreNode() ); }
			}

			//
			// any_spill is the OR of all spill flags for a given instruction.
			// if the instr spills at least one operand, we increment the 
			// alias ID for later instructions to use.
			//
			// this must be done at the end of each instr handler
			// because making nodes in the body requires the current alias ID.
			//
			void try_advance_alias_id( const bool any_spill )
			{
				if ( any_spill ) { ++this->next_alias_id; }
			}

			void operator()( const iAssignNode & );

		};

	}

}

#endif

