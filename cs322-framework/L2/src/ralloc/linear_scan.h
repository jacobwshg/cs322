
#ifndef L2_RALLOC_LINEARSCAN_H
#define L2_RALLOC_LINEARSCAN_H

#include "../liveness.h"

#include <vector>
#include <queue>
#include <cassert>
#include <bitset>

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

		//
		// assignments to GPRs,
		// indexed by named var ID
		//
		// for now, the policy is that GPR - named var mapping can be one-to-many,
		// but named var - GPR mapping is one-to-one,
		// that is, the same GPR may be time-multiplexed between vars with 
		// disjoint hot intervals, but a given var sticks to the same GPR
		// should it be live at any point in the program.
		//
		std::vector< L2::var_id_t > assignments {};

		L2::Liv::VarIdSet spill_vars {};

		//
		// IDs of instrs before which hot intervals start,
		// indexed by ID of the variable in use
		//
		std::vector< L2::instr_id_t > hot_interval_starts {};

		//
		// [ unused ]
		// return the effective var ID used for indexing,
		// which is 0 for the // named var with lowest ID
		// 	( no padding for GPRs, like VarVisitor's id_var_tbl )
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
		// GPRs in use by named vars
		//
		std::bitset< 16 > hot_GPRs { 0x0UL };

		LinearScan(
			const L2::Liv::InstrVisitor &
		);

		inline L2::var_id_t
		find_lowest_free_GPR( void ) const
		{
			for (
				var_id_t gpr_id { L2::Liv::MIN_GPR_ID };
				gpr_id <= L2::Liv::MAX_GPR_ID; ++gpr_id
			)
			{
				if ( !this->hot_GPRs.test( gpr_id ) ) { return gpr_id; }
			}
			return L2::VAR_ID_INVAL;
		}

		inline bool GPR_in_use( const L2::var_id_t gpr_id )
		{
			return this->hot_GPRs.test( gpr_id );
		}
		inline void use_GPR( const L2::var_id_t gpr_id )
		{
			if ( gpr_id >= L2::Liv::MIN_GPR_ID ) { this->hot_GPRs.set( gpr_id ); }
		}
		inline void free_GPR( const L2::var_id_t gpr_id )
		{
			if ( gpr_id >= L2::Liv::MIN_GPR_ID ) { this->hot_GPRs.reset( gpr_id ); }
		}

	};	
}

#endif

