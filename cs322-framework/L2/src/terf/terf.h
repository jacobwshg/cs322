
#ifndef L2_INTERF_H
#define L2_INTERF_H

#include "../liveness.h"

#include <vector>
#include <cstdint>


namespace L2
{
	namespace Terf
	{
		//
		// used to identify constrained arithmetic instrs
		// ( namely w sop sx )
		//
		struct SpecArithFinder;

		struct InterferenceGraph;
	}

	struct Terf::SpecArithFinder
	{
		bool
		operator( const iSxNode &n ) { return true; }

		template< typename iNodeAlt > bool
		operator( const iNodeAlt &alt ) { return false; }
	};

	struct Terf::InterferenceGraph
	{

		static constexpr var_id_t
			MIN_GPR_ID { GPRId::val< RaxNode > },
			MAX_GPR_ID { GPRId::val< R15Node > };

		std::size_t var_cnt {};

		Terf::SpecArithFinder spec_arith_fdr {};

		L2::VarViewer var_view {};

		std::vector< Liv::VarIdSet > graph {};

		InterferenceGraph( void ) =default;

		// 
		// initialize with number of variables
		// 
		InterferenceGraph( const std::size_t );

		add_GPRs( void );

		add_sets( const Liv::FuncVarIdSets & );

		add_spec_arith(
			const std::vector< L2::iNode > &,
			const L2::Liv::VarVisitor &
		);

		add_from_instr_vis( const Liv::InstrVisitor & )

		display( const L2::Liv::VarVisitor & );

	}
}

#endif

