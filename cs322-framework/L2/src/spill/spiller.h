
#ifndef L2_SPIL_SPILLER_H
#define L2_SPIL_SPILLER_H

#include "../ast.h"
#include "../var_view.h"
#include "unparser.h"

#include <string>
#include <cstdint>
#include <string_view>

namespace L2
{
	namespace Spill
	{

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

			L2::VarViewer var_view {};

			//
			// function node after spilling, with updated
			// instruction vector
			//
			L2::fNode f_spill_n {};

			Spiller( void );

			//
			// initialize from parser outputs
			//
			Spiller(
				const L2::fNode &f_n,
				const L2::varNode &var_spill_n,
				const L2::varNode &var_alias_prefix_n
			);

			std::size_t new_alias_id( void );

			//
			// spill a variable over the given function,
			// storing spilled instructions in this->f_n.i_ns
			//
			void spill( const fNode & );

			//
			// return whether spilling has occurred based on
			// whether next_alias_id has been incremented
			//
			bool spilled( void ) const { return this->next_alias_id > 0; }

			//
			// unparse and display the spilled function
			//
			void unparse_and_display( void );

			//
			// make a node with the current variable alias
			// that can be substituted into spilled instructions
			//

			using sv_t = std::string_view;

			//
			// compare a variable name with the name of the variable to spill
			//
			inline bool is_spill_var_name( const std::string_view var_name ) const
			{
				return this->spill_var_name == var_name;
			}

			//
			// make a varNode ( or its various wrappers ) with the latest alias name.
			//
			template< typename Node > Node
			make_alias_node( void ) const { return Node {}; }
			template<> nameNode
			make_alias_node< nameNode >( void ) const
			{
				return nameNode
				{
					.val = this->alias_prefix + std::to_string( this->next_alias_id )
				};
			}
			template<> varNode
			make_alias_node< varNode >( void ) const
			{
				return varNode
				{	
					.percent_n = {},
					.name_n = this->make_alias_node< nameNode >(),
				};
			}
			template<> sxNode
			make_alias_node< sxNode >( void ) const
			{
				return sxNode { this->make_alias_node< varNode >() };
			}
			template<> aNode
			make_alias_node< aNode >( void ) const
			{
				return aNode { this->make_alias_node< sxNode >() };
			}
			template<> wNode
			make_alias_node< wNode >( void ) const
			{
				return wNode { this->make_alias_node< aNode >() };
			}
			template<> uNode
			make_alias_node< uNode >( void ) const
			{
				return uNode { this->make_alias_node< wNode >() };
			}
			template<> xNode
			make_alias_node< xNode >( void ) const
			{
				return xNode { this->make_alias_node< wNode >() };
			}
			template<> tNode
			make_alias_node< tNode >( void ) const
			{
				return tNode { this->make_alias_node< xNode >() };
			}
			template<> sNode
			make_alias_node< sNode >( void ) const
			{
				return sNode { this->make_alias_node< tNode >() };
			}
	
			//
			// if `spill` is true, make an alias node properly wrapped
			// to the level of Node; else, simply copy the provided node.
			//
			template< typename Node > Node
			try_make_alias_node( const Node &n, const bool spill ) const
			{
				if ( spill ) { return this->make_alias_node< Node >(); }
				else { return n; }
			}
	
			//
			// make a load/store node where non-memory location is DEFINITELY replaced with 
			// the latest alias ( no comparison with spill var name )
			//
			L2::iLoadNode make_alias_iLoadNode( void ) const;
			L2::iStoreNode make_alias_iStoreNode( void ) const;

			// 
			// add an instr node to the spilled function node's instr vector
			//
			void add_iNode( const iNode &&i_n ) { this->f_spill_n.i_ns.emplace_back( i_n ); }
			template< typename iNodeAlt > void
			add_iNode( const iNodeAlt &&i_n_alt ) { this->add_iNode( iNode { i_n_alt } ); }

			//
			// if spill is true ( = one of the non-memory locations is the spill variable )
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
			// if the instr spills at least one operand, we increment the 
			// alias ID for later instructions to use.
			//
			// this must be done at the end of each instr handler
			// because making nodes in the body requires the current alias ID.
			//
			void try_advance_alias_id( const bool spill )
			{
				if ( spill ) { ++this->next_alias_id; }
			}

			void operator()( const iAssignNode & );

			void operator()( const iLoadNode & );
			void operator()( const iStoreNode & );

			void operator()( const iStackArgNode & );

			void operator()( const iAOpNode & );
			void operator()( const iSxNode & );
			void operator()( const iSOpNode & );

			void operator()( const iAddStoreNode & );
			void operator()( const iSubStoreNode & );
			void operator()( const iLoadAddNode & );
			void operator()( const iLoadSubNode & );

			void operator()( const iCmpAssignNode & );
			void operator()( const iCJumpNode & );
			void operator()( const iLabelNode & );
			void operator()( const iGotoNode & );

			void operator()( const iReturnNode & );
	
			void operator()( const iCallUNode & );
			void operator()( const iCallPrintNode & );
			void operator()( const iCallInputNode & );
			void operator()( const iCallAllocateNode & );
			void operator()( const iCallTupleErrorNode & );
			void operator()( const iCallTensorErrorNode & );

			void operator()( const iIncrNode & );
			void operator()( const iDecrNode & );
			void operator()( const iLEANode & );

			void operator()( const fNode & );
			//void operator()( const pNode & );

		};

	}

}

#endif

