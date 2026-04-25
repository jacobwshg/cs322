
#include "linear_scan.h"
#include "../liveness.h"

#include <cstdio>
#include <cassert>


L2::var_id_t
L2::
LinearScan::find_lowest_free_GPR( const L2::Liv::VarIdSet &instr_IN ) const
{
	for (
		var_id_t gpr_id { this->MIN_GPR_ID };
		gpr_id <= this->MAX_GPR_ID; ++gpr_id
	)
	{
		//
		// don't use GPRs occupied by other live vars
		// or already in the IN set ( args/saved )
		//
		if ( this->hot_GPRs.test( gpr_id ) || instr_IN.has( gpr_id ) ) { continue; }
		return gpr_id;
	}
	return L2::VAR_ID_INVAL;
}


L2::
LinearScan::LinearScan(
	const L2::Liv::InstrVisitor &instr_vis
)
{

	std::printf( "LINEAR SCAN\n" );

	const L2::Liv::FnVarIdSets &var_id_sets
	{ instr_vis.var_id_sets };
	const L2::Liv::VarVisitor &var_vis
	{ instr_vis.var_vis };

	const L2::instr_id_t instr_cnt
	{
		static_cast< instr_id_t >( var_id_sets.INs.size() )
	};
	L2::instr_id_t instr_id { -1 };

	//
	// all variables are cold ( inactive ) by default
	//
	this->hot_interval_starts.resize( var_vis.next_var_id, -1 );
	this->assignments.resize( var_vis.next_var_id, L2::VAR_ID_INVAL );

	const std::vector< L2::Liv::VarIdSet >
		&INs  { var_id_sets.INs };

	for ( const L2::Liv::VarIdSet &instr_IN : INs )
	{
		++instr_id;

		std::printf( "INSTR ID %0d \n", instr_id );

		//
		// housekeep for any ending hot intervals. do this in a separate loop
		// to maximally free up GPRs for incoming hot intervals.
		//
		for (
			L2::var_id_t var_id { var_vis.BASE_VAR_ID };
			var_id < var_vis.next_var_id; ++var_id
		)
		{
			//
			// skip vars that are still alive as of next instr
			//
			if ( instr_IN.has( var_id ) ) { continue; }

			//
			// reference to the current variable's hot interval start instr id
			// ( if it's cold, then -1 )
			//
			L2::instr_id_t & var_hot_intvl_start { this->hot_interval_starts[ var_id ] };

			// 
			// skip vars that died before the prev instr 
			// ( hot interval not having ended recently )
			//
			if ( var_hot_intvl_start < 0 ) { continue; }

			//{
			//
			// var is not in the next instr's IN set while it is holding a hot interval,
			// meaning its hot interval ends right before the next instr
			//
			std::printf(
				"var #%0d `%%%s` hot interval ending before instr #%0d \n",
				var_id, var_vis.var_name_by_id( var_id ).data(), instr_id
			);

			//
			// free up GPR for use by vars that may become live
			//
			const L2::var_id_t gpr_id { this->assignments[ var_id ] };
			this->free_GPR( gpr_id );

			var_hot_intvl_start = -1;
			//}
		}
		//
		// begin any new hot intervals, assigning GPRs or spilling as necessary.
		//
		for (
			L2::var_id_t var_id { var_vis.BASE_VAR_ID };
			var_id < var_vis.next_var_id; ++var_id
		)
		{
			//
			// skip vars that are not alive as of next instr
			//
			if ( !instr_IN.has( var_id ) ) { continue; }

			//
			// reference to the current variable's hot interval start instr id
			// ( if it's cold, then -1 )
			//
			L2::instr_id_t & var_hot_intvl_start { this->hot_interval_starts[ var_id ] };

			// 
			// skip vars that are in the middle of a hot interval
			// ( hot interval not starting immediately )
			//
			if ( var_hot_intvl_start >= 0 ) { continue; }


			//{
			//
			// var is in the next instr's IN set while it isn't holding a hot interval,
			// meaning its hot interval starts right before the next instr
			//
			std::printf(
				"var #%0d `%%%s` hot interval starting before instr #%0d \n ",
				var_id, var_vis.var_name_by_id( var_id ).data(), instr_id
			);
			var_hot_intvl_start = instr_id;

			L2::var_id_t gpr_id { this->assignments[ var_id ] };

			std::printf(
				"var #%0d `%%%s` GPR id according to assignments: %0d\n ", 
				var_id, var_vis.var_name_by_id( var_id ).data(), gpr_id
			);

			if ( gpr_id >= this->MIN_GPR_ID )
			{
				//
				// var has a valid GPR assignment from a previous hot interval
				//
				if ( GPR_in_use( gpr_id, instr_IN ) )
				{
					//
					// conflict; spill current variable and give up 
					// existing assignment to GPR
					//
					std::printf(
						"var #%0d `%%%s` evicted from GPR %s\n ", 
						var_id, var_vis.var_name_by_id( var_id ).data(), 
						//L2::Liv::ID_GPR_TBL[ gpr_id ].data()
						std::to_string( gpr_id ).data() // testing vregs
					);

					this->assignments[ var_id ] = L2::VAR_ID_INVAL;
					this->spill_vars += var_id;
				}
				else
				{
					//
					// retake control of previously assigned GPR
					//
					std::printf(
						"var #%0d `%%%s` repeating assignment to GPR %s\n ", 
						var_id, var_vis.var_name_by_id( var_id ).data(),
						//L2::Liv::ID_GPR_TBL[ gpr_id ].data()
						std::to_string( gpr_id ).data() // testing vregs
					);
					this->use_GPR( gpr_id );
				}
			}
			else if ( !this->spill_vars.has( gpr_id ) )
			{
				//
				// no previous GPR assignment, and hasn't been spilled
				// try assigning a GPR
				//
				gpr_id = this->find_lowest_free_GPR( instr_IN );
				if ( gpr_id < 0 )
				{
					//
					// all GPRs in use; spill current variable
					//
					std::printf(
						"var #%0d `%%%s`  spilled \n ", 
						var_id, var_vis.var_name_by_id( var_id ).data()
					);
					this->spill_vars += var_id;
				}
				else
				{
					// 
					// GPR available; make assignment
					//
					std::printf(
						"var #%0d `%%%s` assigned GPR %s\n ", 
						var_id, var_vis.var_name_by_id( var_id ).data(),
						//L2::Liv::ID_GPR_TBL[ gpr_id ].data()
						std::to_string( gpr_id ).data() // testing vregs
					);
					this->use_GPR( gpr_id );
					this->assignments[ var_id ] = gpr_id;
					this->spill_vars -= var_id;
				}
			}
			//}

		}
	}

	std::printf( "\n\n" );

	for (
		L2::var_id_t var_id { var_vis.BASE_VAR_ID };
		var_id < var_vis.next_var_id; ++var_id
	)
	{
		std::printf(
			"var #%0d `%%%s`:\t", 
			var_id, var_vis.var_name_by_id( var_id ).data()
		);
		const L2::var_id_t gpr_id { this->assignments[ var_id ] };
		if ( gpr_id < 0 && this->spill_vars.has( var_id ) )
		{
			std::printf( "<spill>" );
		}
		else if ( gpr_id >= this->MIN_GPR_ID && !this->spill_vars.has( var_id ) )
		{
			std::printf(
				"%s",
				//L2::Liv::ID_GPR_TBL[ gpr_id ].data()
				std::to_string( gpr_id ).data() // testing vregs
			);
		}
		else
		{
			std::fprintf(
				stdout,
				"error: var %0d `%%%s` assigned GPR ID: %0d, spill status: %0d \n",
				var_id, var_vis.var_name_by_id( var_id ).data(), gpr_id, this->spill_vars.has( var_id )
			);
			//assert( false );
		}
		
		std::printf( "\n" );
	}

}

