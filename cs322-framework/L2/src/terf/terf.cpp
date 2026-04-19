
#include "terf.h"
#include "../liveness.h"
#include "../var_view.h"
#include <cassert>
#include <cstdint>
#include <vector>
#include <string_view>

L2::Terf::
InterferenceGraph::InterferenceGraph( const std::size_t var_cnt_ ) :
	var_cnt { std::max( var_cnt_, MAX_GPR_ID+1 ) }
{
	this->graph.resize( this->var_cnt );
	this->add_GPRs();
}

L2::Terf::
InterferenceGraph::add_GPRs( void )
{
	using L2::Liv::GPRId;
	using L2::var_id;

	for (
		var_id_t gpr_id { this->MIN_GPR_ID };
		gpr_id <= this->MAX_GPR_ID; ++gpr_id
	)
	{
		for (
			var_id_t gpr2_id { this->MIN_GPR_ID };
			gpr2_id <= this->MAX_GPR_ID; ++gpr2_id
		)
		{
			if ( gpr_id != gpr2_id )
			{
				this->graph[ gpr_id ]  += gpr2_id;
				this->graph[ gpr2_id ] += gpr_id;
			}
		}
	}
}

L2::Terf::
InterferenceGraph::add_sets( 
	const L2::Liv::FuncVarIdSets &var_id_sets
)
{
	instr_id_t instr_id { -1 };
	//
	// iterate over all IN sets, including that of the dummy guard,
	// which will be empty
	// 
	for ( const var_id_set &in_set : var_id_sets.IN )
	{
		++instr_id;

		//
		// obtain OUT and KILL sets of same instruction
		//
		const var_id_set &out_set  { var_id_sets.OUT [ instr_id ] };
		const var_id_set &kill_set { var_id_sets.KILL[ instr_id ] };

		//
		// register IN[i]-IN[i], OUT[i]-OUT[i], KILL[i]-OUT[i] edges 
		//
		for (
			var_id_t id { MIN_GPR_ID };
			id < this->var_cnt; ++id
		)
		{
			for (
				var_id_t id2 { MIN_GPR_ID };
				id2 < this->var_cnt; ++id2
			)
			{
				if ( id2 == id ) { continue; }

				const bool
					in_in    { in_set  .has( id ) && in_set .has( id2 ) },	
					out_out  { out_set .has( id ) && out_set.has( id2 ) },
					kill_out { kill_set.has( id ) && out_set.has( id2 ) };

				if (  in_in || out_out || kill_out )
				{
					this->graph[ id2 ] += id;
					this->graph[ id ]  += id2;
				}
			}
		}


	}

}

L2::Terf::
InterferenceGraph::add_spec_arith(
	const std::vector< L2::iNode > &i_ns,
	const L2::Liv::VarVisitor &var_vis
)
{
	using L2::Liv::GPRId;

	static constexpr var_id_t RCX_ID { GPRId::val< RcxNode > };

	for ( const iNode &i_n : i_ns )
	{
		if ( !std::visit( this->spec_arith_fdr, i_n ) )
		{
			// 
			// skip non-sx instructions
			// 
			continue;
		}

		const std::string_view sx_name
		{
			std::visit( this->var_view, i_n.sx_n );
		};

		const var_id_t sx_var_id { var_vis.var_id_by_name( sx_name ) };

		//
		// skip rcx itself
		//
		if ( sx_var_id == RCX_ID ) { continue; }
		// 
		// var shouldn't be invalid or a non-rcx GPR
		//
		assert( sx_var_id > this->MAX_GPR_ID ); 

		for (
			var_id_t gpr_id { this->MIN_GPR_ID };
			gpr_id <= this->MAX_GPR_ID; ++gpr_id
		)
		{
			if ( gpr_id != GPRId::val< RcxNode > )
			{
				this->graph[ gpr_id ] += sx_var_id;
				this->graph[ sx_var_id ] += gpr_id;
			}
		}
	}
}

L2::Terf::
InterferenceGraph::display( const L2::Liv::VarVisitor &var_vis )
{
	std::string sbuf {}; sbuf.reserve( 512 );

	for (
		var_id_t id { this->MIN_GPR_ID };
		id < var_vis.next_var_id; ++id
	)
	{
		const std::string_view name { var_vis.var_name_by_id( var_id ) };
		if ( name == L2::EMPTYTOK ) { continue; }

		sbuf += "( ";

		for (
			var_id_t id2 { this->MIN_GPR_ID };
			id2 < var_vis.next_var_id; ++id2
		)
		{
			if ( !st.has[ id2 ] ) { continue; }

			const std::string_view name2 { var_vis.var_name_by_id( id2 ) };
			if ( name2 == L2::EMPTYTOK ) { continue; }

			sbuf += name2;
			sbuf += " ";

		}

		sbuf += ")\n";

	}

	std::printf( "%s", sbuf.data() );

}

