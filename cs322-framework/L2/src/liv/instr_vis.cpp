
#include "../ast.h"

#include <cstdio>
#include <variant>
#include <string>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include <cassert>


L2::Liv::
FnVarIdSets::FnVarIdSets( const std::size_t instr_cnt )
{
	const std::size_t safe_cnt { instr_cnt + 1 };
	this->gen_sets  .resize( safe_cnt, {} );
	this->kill_sets .resize( safe_cnt, {} );
	this->in_sets   .resize( safe_cnt, {} );
	this->out_sets  .resize( safe_cnt, {} );
}

void
L2::Liv::
FnVarIdSets::display( void ) const
{
	const std::size_t sz { this->gen_sets.size() };

	instr_id_t instr_id { -1 };

	std::printf( "var id set vectors\n" );

	for ( const std::vector< VarIdSet > &gen_st : this->gen_sets )
	{
		++instr_id;
		std::printf( "instruction ID %0d\n", instr_id );

		std::printf( "\tGEN\n" );
		gen_st.display();

		std::printf( "\tKILL\n" );
		this->kill_sets[ instr_id ].display();

		std::printf( "\tIN\n" );
		this->in_sets[ instr_id ].display();

		std::printf( "\tOUT\n" );
		this->out_sets[ instr_id ].display();

	}

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
	this->var_id_sets = FnVarIdSets { instr_cnt + 1 };
	this->succs_tbl.resize( instr_cnt + 1, {} );
}

void
L2::Liv::
InstrVisitor::display( void ) const
{
	instr_id_t instr_id { -1 };

	//
	// print succ ID table
	//
	std::printf( "InstrVisitor" );
	std::printf( "succs table\n" );
	for ( const std::vector< instr_id_t > &succ_id_vec : this->succs_tbl )
	{
		++instr_id;
		std::printf( "\tinstr %0d successors: ", instr_id );

		for ( const instr_id_t succ_instr_id : succ_id_vec )
		{
			std::printf( "%0d ", succ_instr_id );
		}

		std::printf( "\n" );
	}

	//
	// print label to instr ID table
	//
	std::printf( "label ID table\n" );
	for ( const auto &[ label_str, label_instr_id ] : this->label_id_tbl )
	{
		std::printf( "\t'%s': %0d\n", label_str.data(), label_instr_id );
	}

	//
	// print succ request table
	//
	std::printf( "succ request table\n" );
	for ( const auto &[ label_str, pred_id_vec ] : this->requests_tbl )
	{
		std::printf( "\t'%s' requested by ", label_str.data() );
		for ( const pred_instr_id : pred_id_vec )
		{
			std::printf( "%0d " );
		}
		std::printf( "\n" );
	}

	//
	// print var ID sets
	//
	this->var_id_sets.display();
	std::printf( "\n" );

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
		// and add self as succ on behalf of predecessor
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

	// s is read from, add to GEN set
	this->var_id_sets.gen_sets[ instr_id ] += s_var_id;
	// w is written to, add to KILL set
	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;

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

	this->var_id_sets.gen_sets[ instr_id ]  += x_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;

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
	
	this->var_id_sets.gen_sets[ instr_id ]  += s_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += x_var_id;

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

	this->var_id_sets.gen_sets[ instr_id ]  += t_var_id;
	// w is both read from and written to
	this->var_id_sets.gen_sets[ instr_id ]  += w_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;

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

	this->var_id_sets.gen_sets[ instr_id ]  += sx_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += w_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;

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

	this->var_id_sets.gen_sets[ instr_id ]  += w_var_id;
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

	this->var_id_sets.gen_sets[ instr_id ]  += t_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += x_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += x_var_id;

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

	this->var_id_sets.gen_sets[ instr_id ]  += t_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += x_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += x_var_id;

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

	this->var_id_sets.gen_sets[ instr_id ]  += x_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += w_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;

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

	this->var_id_sets.gen_sets[ instr_id ]  += x_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += w_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;

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

	this->var_id_sets.gen_sets[ instr_id ]  += t1_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += t2_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;

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

void
L2::Liv::
InstrVisitor::operator()( const L2::iReturnNode & )
{
	// return

	const instr_id_t instr_id { this->new_instr_id() };

	this->var_id_sets.gen_sets[ instr_id ] += LivenessGPRId::val< RaxNode >;
	// TODO gen add callee-save regs
	//assert( false );

	// no succ

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iCallUNode &i_call_u_n )
{
	// call u N

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t	
		u_var_id { std::visit( this->var_vis, i_call_u_n.u_n ) };

	this->var_id_sets.gen_sets[ instr_id ] += u_var_id;
	// TODO gen add args
	// TODO kill add caller save regs

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iCallPrintNode &i_call_print_n )
{
	// call print 1

	const instr_id_t instr_id { this->new_instr_id() };

	// TODO gen add args
	// TODO kill add caller save regs

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iCallInputNode &i_call_input_n )
{
	// call input 0

	const instr_id_t instr_id { this->new_instr_id() };

	// TODO gen add args
	// TODO kill add caller save regs

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iCallAllocateNode &i_call_allocate_node )
{
	// call allocate 2

	const instr_id_t instr_id { this->new_instr_id() };

	// TODO gen add args
	// TODO kill add caller save regs

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iCallTupleErrorNode &i_call_tuple_error_node )
{
	// call tuple-error 3

	const instr_id_t instr_id { this->new_instr_id() };

	// TODO gen add args
	// TODO kill add caller save regs

	// no succ

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iCallTensorErrorNode &i_call_tensor_error_n )
{
	// call tensor-error F

	const instr_id_t instr_id { this->new_instr_id() };

	// TODO gen add args
	// TODO kill add caller save regs

	// no succ	

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iIncrNode &i_incr_n )
{
	// w ++
	
	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t
		w_var_id { std::visit( this->var_vis, i_incr_n.w_n ) };

	this->var_id_sets.gen_sets[ instr_id ]  += w_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;

	this->add_serial_succ( instr_id );

}

void 
L2::Liv::
InstrVisitor::operator()( const L2::iDecrNode &i_decr_n )
{
	// w --
	
	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t
		w_var_id { std::visit( this->var_vis, i_decr_n.w_n ) };

	this->var_id_sets.gen_sets[ instr_id ]  += w_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += w_var_id;

	this->add_serial_succ( instr_id );

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iLEANode &i_lea_n )
{
	// w @ w w E

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t
		w1_var_id { std::visit( this->var_vis, i_lea_n.w1_n ) },
		w2_var_id { std::visit( this->var_vis, i_lea_n.w2_n ) },
		w3_var_id { std::visit( this->var_vis, i_lea_n.w3_n ) };

	this->var_id_sets.gen_sets[ instr_id ]  += w2_var_id;
	this->var_id_sets.gen_sets[ instr_id ]  += w3_var_id;
	this->var_id_sets.kill_sets[ instr_id ] += w1_var_id;

	this->add_serial_succ( instr_id );

}

