
#include "interf.h"
#include "../liveness.h"
#include "../var_view.h"
#include <cassert>
#include <cstdint>
#include <vector>
#include <string_view>
#include <algorithm>

L2::Interf::
InterferenceGraph::InterferenceGraph( const std::size_t var_cnt_ )
{
	this->var_cnt = std::max(
		static_cast< var_id_t >( var_cnt_ ),
		1 + MAX_GPR_ID
	);

	this->graph.resize( this->var_cnt );
	this->add_GPRs();
}

void
L2::Interf::
InterferenceGraph::add_GPRs( void )
{
	using L2::Liv::GPRId;
	using L2::var_id_t;

	for (
		var_id_t gpr_id { MIN_GPR_ID };
		gpr_id <= MAX_GPR_ID; ++gpr_id
	)
	{
		for (
			var_id_t gpr2_id { MIN_GPR_ID };
			gpr2_id <= MAX_GPR_ID; ++gpr2_id
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

void
L2::Interf::
InterferenceGraph::add_spec_arith(
	const std::vector< L2::iNode > &i_ns,
	const L2::Liv::VarVisitor &var_vis
)
{
	using L2::Liv::GPRId;

	static constexpr var_id_t RCX_ID { GPRId::val< RcxNode > };

	for ( const iNode &i_n : i_ns )
	{
		const std::string_view sx_name
		{
			std::visit( this->spec_arith_fdr, i_n )
		};
		if ( sx_name == L2::EMPTYTOK ) { continue; }

		const var_id_t sx_var_id { var_vis.var_id_by_name( sx_name ) };

		//
		// skip rcx itself
		//
		if ( sx_var_id == RCX_ID ) { continue; }
		// 
		// var shouldn't be invalid or a non-rcx GPR
		//
		assert( sx_var_id > MAX_GPR_ID ); 

		for (
			var_id_t gpr_id { MIN_GPR_ID };
			gpr_id <= MAX_GPR_ID; ++gpr_id
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

void
L2::Interf::
InterferenceGraph::add_spec_arith(
	const L2::fNode &f_n,
	const L2::Liv::VarVisitor &var_vis
)
{
	this->add_spec_arith( f_n.i_ns, var_vis );
}

void
L2::Interf::
InterferenceGraph::add_sets( 
	const L2::Liv::FnVarIdSets &var_id_sets
)
{
	using L2::Liv::VarIdSet;

	instr_id_t instr_id { -1 };
	//
	// iterate over all IN sets, including that of the dummy guard,
	// which will be empty
	// 
	for ( const VarIdSet &in_set : var_id_sets.IN )
	{
		++instr_id;

		//
		// obtain OUT and KILL sets of same instruction
		//
		const VarIdSet &out_set  { var_id_sets.OUT [ instr_id ] };
		const VarIdSet &kill_set { var_id_sets.KILL[ instr_id ] };

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

void
L2::Interf::
InterferenceGraph::display( const L2::Liv::VarVisitor &var_vis )
{
	std::string sbuf {}; sbuf.reserve( 512 );

	for (
		var_id_t id { MIN_GPR_ID };
		id < var_vis.next_var_id; ++id
	)
	{
		const std::string_view name { var_vis.var_name_by_id( id ) };
		if ( name == L2::EMPTYTOK ) { continue; }

		const L2::Liv::VarIdSet neighbors { this->graph[ id ] };

		//sbuf += "( ";
		if ( id > MAX_GPR_ID ) { sbuf += L2::KW::PERCENT; }
		sbuf += name;
		sbuf += " ";
		

		for (
			var_id_t id2 { MIN_GPR_ID };
			id2 < var_vis.next_var_id; ++id2
		)
		{
			if ( !neighbors.has( id2 ) ) { continue; }

			const std::string_view name2 { var_vis.var_name_by_id( id2 ) };
			if ( name2 == L2::EMPTYTOK ) { continue; }

			if ( id2 > MAX_GPR_ID ) { sbuf += L2::KW::PERCENT; }
			sbuf += name2;
			sbuf += " ";

		}

		//sbuf += ")\n";
		sbuf += "\n";

	}

	std::printf( "%s", sbuf.data() );

}


