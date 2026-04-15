
#include "liveness.h"
#include "ast.h"

#include <cstdio>
#include <variant>
#include <string>
#include <string_view>
#include <cstdint>
#include <algorithm>

//
// compute var ID set union in place
//
L2::Liv::VarIdSet &
L2::Liv::
VarIdSet::operator|=( const VarIdSet &that )
{
	std::size_t
		this_sz { this->data.size() };
	const std::size_t 
		that_sz { that.data.size() };

	//
	// extend `this` to be as long as `that`, so `this` can
	// collect `that`'s extra set bits
	//
	if ( this_sz < that_sz )
	{
		this->data.resize( this_sz, 0x0ULL );
		this_sz = that_sz;
	}

	std::size_t blk_id { 0 };
	for ( std::uint64_t &blk : this->data )
	{
		if ( blk_id >= that_sz ) { break; }
		blk |= that.data[ blk_id ];

		++blk_id;
	}

	return *this;
}

//
// compute var ID set intersection in place
//
L2::Liv::VarIdSet &
L2::Liv::
VarIdSet::operator&=( const VarIdSet &that )
{
	const std::size_t
		this_sz { this->data.size() },
		that_sz { that.data.size() };

	std::size_t blk_id { 0 };
	for ( std::uint64_t &blk : this->data )
	{
		if ( blk_id < that_sz ) { blk &= that.data[ blk_id ]; }
		//
		// if `this` has more blocks than `that`, the intersection
		// must be cleared of elements ( set bits ) exclusive to `this`
		//
		else { blk = 0x0ULL; }

		++blk_id;
	}

	return *this;
}

//
// compute var ID set subtraction in place
//
L2::Liv::VarIdSet &
L2::Liv::
VarIdSet::operator-=( const VarIdSet &that )
{
	const std::size_t
		this_sz { this->data.size() },
		that_sz { that.data.size() };

	std::size_t blk_id { 0 };
	for ( std::uint64_t &blk: this->data )
	{
		// 
		// if `this` has more blocks than `that`, the blocks missing from `that`
		// have no impact on blocks exclusive to `this`
		//
		if ( blk_id >= that_sz ) { break; }

		//
		// suppose `this` has block `1111`,`that` has block `0101`
		// `that` block flipped is `1010`
		// `this` block becomes `1010` after AND with flipped `that` block
		// shared elements are cleared
		//
		blk &= ~( that.data[ blk_id ] );

		++blk_id;
	}

	return *this;
}

L2::Liv::VarIdSet
L2::Liv::
operator|( const VarIdSet &lhs, const L2::Liv::VarIdSet &rhs )
{
	L2::Liv::VarIdSet lhs_new { lhs }; // copy
	lhs_new |= rhs;
	return std::move( lhs );
}


L2::Liv::VarIdSet
L2::Liv::
operator&( const L2::Liv::VarIdSet &lhs, const L2::Liv::VarIdSet &rhs )
{
	L2::Liv::VarIdSet lhs_new { lhs }; 
	lhs_new &= rhs;
	return std::move( lhs );
}

L2::Liv::VarIdSet
L2::Liv::
operator-( const L2::Liv::VarIdSet &lhs, const L2::Liv::VarIdSet &rhs )
{
	L2::Liv::VarIdSet lhs_new { lhs };
	lhs_new -= rhs;
	return std::move( lhs );
}

L2::Liv::
FuncVarIdSets::FuncVarIdSets( const std::size_t instr_cnt )
{
	this->gen_sets  .resize( instr_cnt, {} );
	this->kill_sets .resize( instr_cnt, {} );
	this->in_sets   .resize( instr_cnt, {} );
	this->out_sets  .resize( instr_cnt, {} );
}

L2::Liv::
VarVisitor::VarVisitor( void )
{
	this->id_var_tbl.reserve( 8 );
}

L2::var_id_t
L2::Liv::
VarVisitor::new_var_id( void )
{
	const var_id_t var_id { this->next_var_id };
	++this->next_var_id;
	return var_id;
}

// given ID, retrieve variable name 
std::string_view
L2::Liv::
VarVisitor::var_by_id( const L2::var_id_t var_id ) const
{

	/*
	using GPRId = L2::LivenessGPRId;

	switch ( var_id )
	{
	case GPRId::val< L2::RaxNode >:
		return  L2::RaxNode::kw;
		break;
	case GPRId::val< L2::RbxNode >:
		return  L2::RbxNode::kw;
		break;
	case GPRId::val< L2::RcxNode >:
		return  L2::RcxNode::kw;
		break;
	case GPRId::val< L2::RdxNode >:
		return  L2::RdxNode::kw;
		break;

	case GPRId::val< L2::RdiNode >:
		return  L2::RdiNode::kw;
		break;
	case GPRId::val< L2::RsiNode >:
		return  L2::RsiNode::kw;
		break;
	case GPRId::val< L2::RbpNode >:
		return  L2::RbpNode::kw;
		break;

	case GPRId::val< L2::R8Node >:
		return  L2::R8Node::kw;
		break;
	case GPRId::val< L2::R9Node >:
		return  L2::R9Node::kw;
		break;
	case GPRId::val< L2::R10Node >:
		return  L2::R10Node::kw;
		break;
	case GPRId::val< L2::R11Node >:
		return  L2::R11Node::kw;
		break;
	case GPRId::val< L2::R12Node >:
		return  L2::R12Node::kw;
		break;
	case GPRId::val< L2::R13Node >:
		return  L2::R13Node::kw;
		break;
	case GPRId::val< L2::R14Node >:
		return  L2::R14Node::kw;
		break;
	case GPRId::val< L2::R15Node >:
		return  L2::R15Node::kw;
		break;

	default:
		if ( var_id >= L2::Liv::VarVisitor::BASE_VAR_ID )
		{
			// to index id_var_tbl, convert logical id to physical id
			return std::string_view { this->id_var_tbl[ var_id - BASE_VAR_ID ] };
		}
		else { return L2::EMPTYTOK; }
		break;
	}
	*/

	if ( var_id > 0 && var_id < VarVisitor::BASE_VAR_ID )
	{
		// var ID is that of a GPR
		return LivenessGPRId::ID_GPR_TBL[ var_id ];
	}
	else if (
		var_id >= VarVisitor::BASE_VAR_ID
		&& var_id < this->next_var_id
	)
	{
		// var ID is that of a named variable
		// to index id_var_tbl, convert logical id to physical id
		return std::string_view { this->id_var_tbl[ var_id - BASE_VAR_ID ] };
	}

	return L2::EMPTYTOK;
}


//
// return the var ID of a named variable. 
// if the variable is previously unknown, assign the next available ID to it.
//
L2::var_id_t
L2::Liv::
VarVisitor::operator()( const L2::nameNode &name_n )
{
	const auto tbl_it { this->var_id_tbl.find( name_n.val ) };
	if ( tbl_it != this->var_id_tbl.end() )
	{
		// if named variable is known, return previously assigned ID
		return tbl_it->second;
	}
	else
	{
		// if named variable is new, assign an ID, register it, and return the new ID
		const var_id_t var_id { this->new_var_id() };

		// var_id_tbl uses logical id: [ var1 ] = 16, [ var2 ] = 17, ...
		this->var_id_tbl.insert( { std::string { name_n.val }, var_id } );

		// id_var_tbl has no padding for GPRs to save space, 
		// uses physical id: [ 0 ] = var1, [ 1 ] = var2, ...
		this->id_var_tbl.emplace_back( std::string { name_n.val } );
		return var_id;
	}
}


L2::var_id_t
L2::Liv::
VarVisitor::operator()( const L2::varNode &var_n )
{
	// recurse on variants that are alternatives of another variant
	// ( sx for a, a for w, ... )
	return ( *this )( var_n.name_n );
}

//
// extract the name in a labelNode
//
std::string_view
L2::Liv::
LabelVisitor::operator()( const L2::labelNode &label_n )
{
	return label_n.name_n.val; 
}

L2::Liv::
InstrVisitor::InstrVisitor( const std::size_t instr_cnt )
{
	//
	// append sets for a dummy instr, which will stay empty
	// if the final instr is not return, accessing its nonexistent "serial successor"
	// will not cause an indexing error
	//
	this->var_id_sets = FuncVarIdSets { instr_cnt + 1 };
	this->succs_tbl.resize( instr_cnt + 1, {} );
}

L2::instr_id_t
L2::Liv::
InstrVisitor::new_instr_id( void )
{
	const instr_id_t instr_id { this->next_instr_id };
	++this->next_instr_id;
	return instr_id;
}


void
L2::Liv::
InstrVisitor::add_serial_succ( const instr_id_t instr_id )
{
	this->succs_tbl[ instr_id ].emplace_back( instr_id + 1 );
}

void
L2::Liv::
InstrVisitor::try_add_label_succ (
	const std::string_view succ_label,
	const instr_id_t jmp_instr_id
)
{
	const auto tbl_it { this->label_id_tbl.find( succ_label ) };
	if ( tbl_it != this->label_id_tbl.end() )
	{
		// label is known, add to own successor instr IDs
		this->succs_tbl[ jmp_instr_id ].emplace_back( tbl_it->second );
	}
	else
	{
		// label is unknown, add request to resolve
		this->requests_tbl[ std::string { succ_label } ].emplace_back( jmp_instr_id );
	}

}

void
L2::Liv::
InstrVisitor::resolve_label_succ (
	const instr_id_t label_instr_id,
	const std::string_view label_sv
)
{
	//
	// add mapping from label to instr ID, regardless of whether it has been requested above
	//
	this->label_id_tbl.insert(
		{
			std::string { label_sv },
			label_instr_id,
		}
	);
	const auto tbl_it { this->requests_tbl.find( label_sv ) };
	if ( tbl_it != this->requests_tbl.end() )
	{
		//
		// label had been requested as succ; resolve requests
		//
		for ( const instr_id_t jmp_instr_id: tbl_it->second )
		{
			this->succs_tbl[ jmp_instr_id ].emplace_back( label_instr_id );
		}
	}

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iAssignNode &i_assign_n )
{
	// w <- s

	const instr_id_t instr_id = this->new_instr_id();
	const var_id_t
		w_var_id { std::visit( this->var_vis, i_assign_n.w_n ) },
		s_var_id { std::visit( this->var_vis, i_assign_n.s_n ) };

	// w is written to, add to KILL set
	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;
	// s is read from, add to GEN set
	this->var_id_sets.gen_sets[ instr_id ] += s_var_id;

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iLoadNode &i_load_n )
{
	// w <- mem x M

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t	
		w_var_id { std::visit( this->var_vis, i_load_n.w_n ) },
		x_var_id { std::visit( this->var_vis, i_load_n.x_n ) };

	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += x_var_id;

	this->add_serial_succ( instr_id );

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iStoreNode &i_store_n )
{
	// mem x M <- s

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t	
		x_var_id { std::visit( this->var_vis, i_store_n.x_n ) },
		s_var_id { std::visit( this->var_vis, i_store_n.s_n ) };
	
	this->var_id_sets.kill_sets[ instr_id ] += x_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += s_var_id;

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iStackArgNode &i_stack_arg_n )
{
	// w <- stack_arg M

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t
		w_var_id { std::visit( this->var_vis, i_stack_arg_n.w_n ) };

	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;

	this->add_serial_succ( instr_id );

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iAOpNode &i_aop_n )
{
	// w aop t

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t	
		w_var_id { std::visit( this->var_vis, i_aop_n.w_n ) },
		t_var_id { std::visit( this->var_vis, i_aop_n.t_n ) };

	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += t_var_id;

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iSxNode &i_sx_n )
{
	// w sop sx

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t	
		w_var_id  { std::visit( this->var_vis, i_sx_n.w_n ) },
		sx_var_id { std::visit( this->var_vis, i_sx_n.sx_n ) };

	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += sx_var_id;

	this->add_serial_succ( instr_id );

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iSOpNode &i_sop_n )
{
	// w sop N

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t
		w_var_id { std::visit( this->var_vis, i_sop_n.w_n ) };

	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;
	
	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iAddStoreNode &i_add_store_n )
{
	// mem x M += t

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t	
		x_var_id { std::visit( this->var_vis, i_add_store_n.x_n ) },
		t_var_id { std::visit( this->var_vis, i_add_store_n.t_n ) };

	this->var_id_sets.kill_sets[ instr_id ] += x_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += t_var_id;

	this->add_serial_succ( instr_id );

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iSubStoreNode &i_sub_store_n )
{
	// mem x M -= t
	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t	
		x_var_id { std::visit( this->var_vis, i_sub_store_n.x_n ) },
		t_var_id { std::visit( this->var_vis, i_sub_store_n.t_n ) };

	this->var_id_sets.kill_sets[ instr_id ] += x_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += t_var_id;

	this->add_serial_succ( instr_id );

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iLoadAddNode &i_load_add_n )
{
	// w += mem x M

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t	
		w_var_id { std::visit( this->var_vis, i_load_add_n.w_n ) },
		x_var_id { std::visit( this->var_vis, i_load_add_n.x_n ) };

	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += x_var_id;

	this->add_serial_succ( instr_id );

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iLoadSubNode &i_load_sub_n )
{
	// w -= mem x M

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t	
		w_var_id { std::visit( this->var_vis, i_load_sub_n.w_n ) },
		x_var_id { std::visit( this->var_vis, i_load_sub_n.x_n ) };

	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += x_var_id;

	this->add_serial_succ( instr_id );

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iCmpAssignNode &i_cmp_assign_n )
{
	// w <- t cmp t

	const instr_id_t instr_id { this->new_instr_id() };

	const var_id_t	
		w_var_id  { std::visit( this->var_vis, i_cmp_assign_n.w_n ) },
		t1_var_id { std::visit( this->var_vis, i_cmp_assign_n.t1_n ) },
		t2_var_id { std::visit( this->var_vis, i_cmp_assign_n.t2_n ) };

	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += t1_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += t2_var_id;

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iCJumpNode &i_cjump_n )
{
	// cjump t cmp t label

	const instr_id_t instr_id { this->new_instr_id() };

	const var_id_t	
		t1_var_id { std::visit( this->var_vis, i_cjump_n.t1_n ) },
		t2_var_id { std::visit( this->var_vis, i_cjump_n.t2_n ) };

	this->var_id_sets.gen_sets[ instr_id ]  += t1_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += t2_var_id;
	
	// one succ is next in sequence ( false br )
	this->add_serial_succ( instr_id );
	// one is label ( true br )
	const std::string_view label_sv { LabelVisitor{}( i_cjump_n.label_n ) };
	this->try_add_label_succ( label_sv, instr_id );

}



void
L2::Liv::
InstrVisitor::operator()( const L2::iLabelNode &i_label_n )
{
	// label

	const instr_id_t instr_id { this->new_instr_id() };

	// resolve any requests from cjump or goto above
	const std::string_view label_sv { LabelVisitor{}( i_label_n.label_n ) };
	this->resolve_label_succ( instr_id, label_sv );

	this->add_serial_succ( instr_id );

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iGotoNode &i_goto_n )
{
	// goto label

	const instr_id_t instr_id { this->new_instr_id() };

	// only succ is label
	const std::string_view label_sv { LabelVisitor{}( i_goto_n.label_n ) };
	this->try_add_label_succ( label_sv, instr_id );

}

/*

		void L2::InstrVisitor::operator()( const L2::iReturnNode & );

		void L2::InstrVisitor::operator()( const L2::iCallUNode & );
		void L2::InstrVisitor::operator()( const L2::iCallPrintNode & );
		void L2::InstrVisitor::operator()( const L2::iCallInputNode & );
		void L2::InstrVisitor::operator()( const L2::iCallAllocateNode & );
		void L2::InstrVisitor::operator()( const L2::iCallTupleErrorNode & );
		void L2::InstrVisitor::operator()( const L2::iCallTensorErrorNode & );

		void L2::InstrVisitor::operator()( const L2::iIncrNode & );
		void L2::InstrVisitor::operator()( const L2::iDecrNode & );

		void L2::InstrVisitor::operator()( const L2::iLEANode & );

*/

int
main()
{
	L2::Liv::VarVisitor lv {};
	std::printf(
		"%d %d\n",
		lv( L2::RdxNode{} ),
		//lv( L2::RspNode{} ) // should fail to compile
		lv( L2::R13Node() )
	);
}

