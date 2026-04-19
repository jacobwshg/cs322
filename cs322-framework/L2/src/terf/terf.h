
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
		std::string_view
		operator()( const L2::iSxNode &n )
		{
			return std::visit( L2::VarViewer{}, n.sx_n );
		}

		template< typename iNodeAlt > std::string_view
		operator()( const iNodeAlt &alt ) { return L2::EMPTYTOK; }
	};

	using L2::Liv::GPRId;

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

		void
		add_GPRs( void );

		void
		add_sets( const Liv::FnVarIdSets & );

		void
		add_spec_arith(
			const std::vector< L2::iNode > &,
			const L2::Liv::VarVisitor &
		);

		void
		add_from_instr_vis( const Liv::InstrVisitor & );

		void
		display( const L2::Liv::VarVisitor & );

	};
}

#endif

