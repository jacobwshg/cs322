
#include "linear_scan.h"
#include "../liveness.h"

#include <cstdio>

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
		static_cast< instr_id_t >( var_id_sets.IN.size() )
	};
	L2::instr_id_t instr_id { -1 };

	//
	// all variables are cold ( inactive ) by default
	//
	this->hot_interval_starts.resize( var_vis.next_var_id, -1 );

	const std::vector< L2::Liv::VarIdSet >
		&in_sets  { var_id_sets.IN };
	for ( const L2::Liv::VarIdSet &in : in_sets )
	{
		++instr_id;
		//
		// loop through named variables in scope
		//
		for (
			L2::var_id_t var_id { var_vis.BASE_VAR_ID };
			var_id < var_vis.next_var_id; ++var_id
		)
		{
			if (
				in.has( var_id ) &&
				this->hot_interval_starts[ var_id ] < 0
			)
			{
				std::printf(
					"var #%0d `%%%s` hot interval starting before instr #%0d \n",
					var_id, var_vis.var_name_by_id( var_id ).data(), instr_id
				);
				this->hot_interval_starts[ var_id ] = instr_id;
			}
			else if (
				!in.has( var_id ) &&
				this->hot_interval_starts[ var_id ] > -1
			)
			{
				std::printf(
					"var #%0d `%%%s` hot interval ending before instr #%0d \n",
					var_id, var_vis.var_name_by_id( var_id ).data(), instr_id
				);
				this->hot_interval_starts[ var_id ] = -1;
			}
		}
	}

}

