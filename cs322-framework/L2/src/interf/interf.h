
#ifndef L2_INTERF_H
#define L2_INTERF_H

#include "../liveness.h"
#include "../var_view.h"
#include "../ast.h"

#include <vector>
#include <cstdint>
#include <string_view>


namespace L2
{
	namespace Interf
	{
		//
		// used to identify constrained arithmetic instrs
		// ( namely w sop sx )
		//
		struct SpecArithFinder;

		struct InterferenceGraph;
	}

	struct Interf::SpecArithFinder
	{
		std::string_view
		operator()( const L2::iSxNode &n )
		{
			return std::visit( L2::VarViewer{}, n.sx_n );
		}

		template< typename iNodeAlt > std::string_view
		operator()( const iNodeAlt &alt ) { return L2::EMPTYTOK; }
	};

	using L2::Liv::GPRId;

	struct Interf::InterferenceGraph
	{

		static constexpr var_id_t
			MIN_GPR_ID { GPRId::val< RaxNode > },
			MAX_GPR_ID { GPRId::val< R15Node > };

		std::size_t var_cnt {};

		Interf::SpecArithFinder spec_arith_fdr {};

		L2::VarViewer var_view {};

		//
		// use a variable ID set to store the graph
		// as an adjacency list
		//
		std::vector< Liv::VarIdSet > graph {};

		InterferenceGraph( void ) =default;

		// 
		// initialize with number of variables
		// 
		InterferenceGraph( const std::size_t );

		//
		// add edges between GPRs, which can be done at
		// initialization time
		//
		void
		add_GPRs( void );

		//
		// add edges from each instruction's sets.
		//
		// called after liveness analysis
		//
		void
		add_sets( const Liv::FnVarIdSets & );

		//
		// given parsed instruction in a function, identify constrained
		// arithmetic instructions, and add edges between
		// constrained variables to GPRs using var-to-ID mappings
		// stored in a VarVisitor
		//
		// called after liveness analysis
		//
		void
		add_spec_arith(
			const std::vector< L2::iNode > &,
			const L2::Liv::VarVisitor &
		);
		void
		add_spec_arith(
			const L2::fNode &,
			const L2::Liv::VarVisitor &
		);

		//
		// print
		//
		void
		display( const L2::Liv::VarVisitor & );

	};
}

#endif

