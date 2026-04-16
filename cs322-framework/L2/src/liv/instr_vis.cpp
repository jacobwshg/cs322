
#include "instr_vis.h"
#include "../ast.h"
#include "../callconv.h"

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
	this->GEN  .resize( safe_cnt, {} );
	this->KILL .resize( safe_cnt, {} );
	this->IN   .resize( safe_cnt, {} );
	this->OUT  .resize( safe_cnt, {} );
}

void
L2::Liv::
FnVarIdSets::display( void ) const
{
	const std::size_t sz { this->GEN.size() };

	instr_id_t instr_id { -1 };

	std::printf( "function var id sets\n" );

	for ( const VarIdSet &gen_st : this->GEN )
	{
		++instr_id;
		std::printf( "instruction ID %0d\n", instr_id );

		std::printf( "GEN\n" );
		gen_st.display();

		std::printf( "KILL\n" );
		this->KILL[ instr_id ].display();

		std::printf( "IN\n" );
		this->IN[ instr_id ].display();

		std::printf( "OUT\n" );
		this->OUT[ instr_id ].display();

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
	// print variable visitor
	//
	this->var_vis.display();

	//
	// print succ ID table
	//
	std::printf( "InstrVisitor\n" );
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
		for ( const instr_id_t pred_instr_id : pred_id_vec )
		{
			std::printf( "%0d ", pred_instr_id );
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
InstrVisitor::run_liveness( void )
{

	long long int _iter { 0 };

	bool changed { false };
	do
	{
		std::printf( "liveness iter %0lld\n", ++_iter );	

		changed = false;
		//
		// one backward pass through all instructions
		//
		
		for (
			instr_id_t instr_id { this->next_instr_id - 1 };
			instr_id >= 0;
			--instr_id
		)
		{
			VarIdSet new_out_set {};
			//
			// loop over all successors of current instr 
			//
			for ( const instr_id_t succ_id : this->succs_tbl[ instr_id ] )
			{
				//
				// iterative over IDs of all encountered variables;
				// if any is in the current successor's IN set, 
				// add it to our new OUT set
				//
				for (
					var_id_t var_id { 0 };
					var_id < this->var_vis.next_var_id;
					++var_id 
				)
				{

					if ( this->var_id_sets.IN[ succ_id ].has( var_id ) )
					{
						new_out_set += var_id;
					}
				}
			}
			if ( new_out_set != this->var_id_sets.OUT[ instr_id ] )
			{
				changed = true;
			}

			VarIdSet new_in_set { this->var_id_sets.GEN[ instr_id ] };
			new_in_set |= (
				new_out_set - this->var_id_sets.KILL[ instr_id ]
			);
			if ( new_in_set != this->var_id_sets.IN[ instr_id ] )
			{
				changed = true;
			}

			this->var_id_sets.OUT[ instr_id ] = std::move( new_out_set );
			this->var_id_sets.IN [ instr_id ] = std::move( new_in_set );
		}
	}
	while ( changed );
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
	this->var_id_sets.GEN[ instr_id ] += s_var_id;
	// w is written to, add to KILL set
	this->var_id_sets.KILL[ instr_id ] += w_var_id;

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

	this->var_id_sets.GEN[ instr_id ]  += x_var_id;
	this->var_id_sets.KILL[ instr_id ] += w_var_id;

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
	
	this->var_id_sets.GEN[ instr_id ]  += s_var_id;
	this->var_id_sets.KILL[ instr_id ] += x_var_id;

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

	this->var_id_sets.KILL[ instr_id ] += w_var_id;

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

	this->var_id_sets.GEN[ instr_id ]  += t_var_id;
	// w is both read from and written to
	this->var_id_sets.GEN[ instr_id ]  += w_var_id;
	this->var_id_sets.KILL[ instr_id ] += w_var_id;

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

	this->var_id_sets.GEN[ instr_id ]  += sx_var_id;
	this->var_id_sets.GEN[ instr_id ]  += w_var_id;
	this->var_id_sets.KILL[ instr_id ] += w_var_id;

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

	this->var_id_sets.GEN[ instr_id ]  += w_var_id;
	this->var_id_sets.KILL[ instr_id ] += w_var_id;

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

	this->var_id_sets.GEN [ instr_id ] += t_var_id;
	this->var_id_sets.GEN [ instr_id ] += x_var_id;
	this->var_id_sets.KILL[ instr_id ] += x_var_id;

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

	this->var_id_sets.GEN [ instr_id ] += t_var_id;
	this->var_id_sets.GEN [ instr_id ] += x_var_id;
	this->var_id_sets.KILL[ instr_id ] += x_var_id;

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

	this->var_id_sets.GEN[ instr_id ]  += x_var_id;
	this->var_id_sets.GEN[ instr_id ]  += w_var_id;
	this->var_id_sets.KILL[ instr_id ] += w_var_id;

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

	this->var_id_sets.GEN [ instr_id ] += x_var_id;
	this->var_id_sets.GEN [ instr_id ] += w_var_id;
	this->var_id_sets.KILL[ instr_id ] += w_var_id;

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

	this->var_id_sets.GEN [ instr_id ] += t1_var_id;
	this->var_id_sets.GEN [ instr_id ] += t2_var_id;
	this->var_id_sets.KILL[ instr_id ] += w_var_id;

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

	this->var_id_sets.GEN[ instr_id ] += t1_var_id;
	this->var_id_sets.GEN[ instr_id ] += t2_var_id;

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

	// add rax
	this->var_id_sets.GEN[ instr_id ] += CallConv::RETVAL_REG_ID;
	// add callee save regs
	for ( const var_id_t sav_id : CallConv::CALLEE_SAVE_REG_IDS )
	{
		this->var_id_sets.GEN[ instr_id ] += sav_id;
	}

	// no succ

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iCallUNode &i_call_u_n )
{
	// call u N

	struct NVisitor
	{
		long long operator()( const _0Node &_0_n ) { return _0Node::val; }
		long long operator()( const NNZNode &N_nz_n ) { return N_nz_n.val; }
	};

	const instr_id_t instr_id { this->new_instr_id() };
	const var_id_t	
		u_var_id { std::visit( this->var_vis, i_call_u_n.u_n ) };

	const long long N_val { std::visit( NVisitor{}, i_call_u_n.N_n ) };
	assert( N_val >= 0 );

	const std::size_t argcnt { static_cast< std::size_t >( N_val ) };

	// add u
	this->var_id_sets.GEN[ instr_id ] += u_var_id;
	// add args regs ( at most 6 )
	for (
		std::size_t arg_id { 0 };
		arg_id < std::min( argcnt, CallConv::ARG_REG_CNT );
		++arg_id
	)
	{
		this->var_id_sets.GEN[ instr_id ] += CallConv::ARG_REG_IDS[ arg_id ];
	}
	// add caller save regs
	for ( const var_id_t sav_id: CallConv::CALLER_SAVE_REG_IDS )
	{
		this->var_id_sets.KILL[ instr_id ] += sav_id;
	}

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iCallPrintNode &i_call_print_n )
{
	// call print 1

	const instr_id_t instr_id { this->new_instr_id() };

	// add args
	for (
		std::size_t arg_id { 0 };
		arg_id < builtin_argcnt::val< iCallPrintNode >;
		++arg_id
	)
	{
		this->var_id_sets.GEN[ instr_id ] += CallConv::ARG_REG_IDS[ arg_id ];
	}
	// add caller save regs
	for ( const var_id_t sav_id: CallConv::CALLER_SAVE_REG_IDS )
	{
		this->var_id_sets.KILL[ instr_id ] += sav_id;
	}

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iCallInputNode &i_call_input_n )
{
	// call input 0

	const instr_id_t instr_id { this->new_instr_id() };

	// no args to add

	// add caller save regs
	for ( const var_id_t sav_id: CallConv::CALLER_SAVE_REG_IDS )
	{
		this->var_id_sets.KILL[ instr_id ] += sav_id;
	}

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iCallAllocateNode &i_call_allocate_node )
{
	// call allocate 2

	const instr_id_t instr_id { this->new_instr_id() };

	// add args
	for (
		std::size_t arg_id { 0 };
		arg_id < builtin_argcnt::val< iCallAllocateNode >;
		++arg_id
	)
	{
		this->var_id_sets.GEN[ instr_id ] += CallConv::ARG_REG_IDS[ arg_id ];
	}
	// add caller save regs
	for ( const var_id_t sav_id: CallConv::CALLER_SAVE_REG_IDS )
	{
		this->var_id_sets.KILL[ instr_id ] += sav_id;
	}

	this->add_serial_succ( instr_id );

}


void
L2::Liv::
InstrVisitor::operator()( const L2::iCallTupleErrorNode &i_call_tuple_error_n )
{
	// call tuple-error 3

	const instr_id_t instr_id { this->new_instr_id() };

	// add args
	for (
		std::size_t arg_id { 0 };
		arg_id < builtin_argcnt::val< iCallTupleErrorNode >;
		++arg_id
	)
	{
		this->var_id_sets.GEN[ instr_id ] += CallConv::ARG_REG_IDS[ arg_id ];
	}
	// add caller save regs
	for ( const var_id_t sav_id: CallConv::CALLER_SAVE_REG_IDS )
	{
		this->var_id_sets.KILL[ instr_id ] += sav_id;
	}

	// no succ

}

void
L2::Liv::
InstrVisitor::operator()( const L2::iCallTensorErrorNode &i_call_tensor_error_n )
{
	// call tensor-error F

	struct FVisitor
	{
		long long operator()( const _1Node &_1_n ) { return _1Node::val; }
		long long operator()( const _3Node &_3_n ) { return _3Node::val; }
		long long operator()( const _4Node &_4_n ) { return _4Node::val; }
	};

	const instr_id_t instr_id { this->new_instr_id() };

	const long long F_val { std::visit( FVisitor{}, i_call_tensor_error_n.F_n ) };
	const std::size_t argcnt { static_cast< std::size_t >( F_val ) };

	// add args
	for ( std::size_t arg_id { 0 }; arg_id < argcnt; ++arg_id )
	{
		this->var_id_sets.GEN[ instr_id ] += CallConv::ARG_REG_IDS[ arg_id ];
	}
	// add caller save regs
	for ( const var_id_t sav_id: CallConv::CALLER_SAVE_REG_IDS )
	{
		this->var_id_sets.KILL[ instr_id ] += sav_id;
	}


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

	this->var_id_sets.GEN [ instr_id ] += w_var_id;
	this->var_id_sets.KILL[ instr_id ] += w_var_id;

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

	this->var_id_sets.GEN [ instr_id ] += w_var_id;
	this->var_id_sets.KILL[ instr_id ] += w_var_id;

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

	this->var_id_sets.GEN [ instr_id ] += w2_var_id;
	this->var_id_sets.GEN [ instr_id ] += w3_var_id;
	this->var_id_sets.KILL[ instr_id ] += w1_var_id;

	this->add_serial_succ( instr_id );

}

