
#ifndef L2_RALLOC_LINEARSCAN_H
#define L2_RALLOC_LINEARSCAN_H

#include "../liveness.h"

#include <vector>
#include <queue>
#include <cassert>

namespace L2
{
	struct LiveInterval
	{
		L2::var_id_t var_id { L2::VAR_ID_INVAL };
		L2::instr_id_t start { -1 };
		L2::instr_id_t end   { -1 };
	};

	template< typename Intvl >
	struct starts_later
	{
		bool operator()( const Intvl &i1, const Intvl &i2 )
		{ return i1.start > i2.start; }
	};
	template< typename Intvl >
	struct ends_later
	{
		bool operator()( const Intvl &i1, const Intvl &i2 )
		{ return i1.end > i2.end; }
	};

	struct LinearScan
	{
		//
		// pop interval that starts soonest when iterating
		//
		std::priority_queue<
			LiveInterval,
			std::vector< LiveInterval >,	
			starts_later< LiveInterval >
		> intervals {};

		std::priority_queue<
			LiveInterval,
			std::vector< LiveInterval >,
			ends_later< LiveInterval >
		> hot_intervals {};

		L2::Liv::VarIdSet hot_var_ids {};

		//
		// IDs of instrs before which hot intervals start,
		// indexed by ID of the variable in use
		//
		std::vector< L2::instr_id_t > hot_interval_starts {};

		//
		// return the effective var ID used for indexing
		// hot_nterval_starts, which is 0 for the 
		// named var with lowest ID ( no padding for GPRs,
		// like VarVisitor's id_var_tbl )
		//
		inline L2::var_id_t
		ef_var_id ( const L2::var_id_t var_id )
		{
			//
			// sanity check: is named variable
			//
			assert( var_id >= L2::Liv::VarVisitor::BASE_VAR_ID );
			return var_id - L2::Liv::VarVisitor::BASE_VAR_ID;
		}

		//
		// 1111 1111 1111 1110
		// ( register 0 is dummy )
		//
		std::bitset< 16 > available_colors { 0xfeUL };

		LinearScan(
			const L2::Liv::InstrVisitor &
		);

		//scan( void );

	};	
}

#endif

