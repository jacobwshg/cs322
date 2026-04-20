
#include "spiller.h"
#include "unparser.h"

#include <cstdint>

L2::Spill::
Spiller::Spiller(
	const L2::fNode &f_n,
	const L2::varNode &var_spill_n,
	const L2::varNode &var_alias_prefix_n
)
{
	this->f_spill_n.l_n = f_n.l_n;
	this->f_spill_n.N_n = f_n.N_n;
	this->f_spill_n.i_ns.reserve( 16 );

	this->spill_var_name = this->var_view( var_spill_n );
	this->alias_prefix   = this->var_view( var_alias_prefix_n );
}

std::size_t
L2::Spill::
Spiller::new_alias_id( void )
{
	const std::size_t id { this->next_alias_id };
	++this->next_alias_id;
	return id;

}

void
L2::Spill::
Spiller::spill( const L2::fNode &f_n )
{
	( *this )( f_n );
}

void
L2::Spill::
Spiller::unparse_and_display( void )
{
	std::printf(
		"%s\n",
		this->unparser( this->f_spill_n ).data()
	);
}

//
// 
//
L2::iLoadNode
L2::Spill::
Spiller::make_alias_iLoadNode( void )
{
	const long long stk_ofs
	{
		static_cast< long long >( this->spill_var_id ) * 8
	};

	return iLoadNode
	{
		// use the spill var name to force an aliased node
		.w_n = this->make_alias_node< wNode >(),
		.op_assign_n = {},
		.mem_n = MemNode {},
		.x_n = xNode { RspNode {} },
		.M_n = MNode { .val = stk_ofs },
	};

}

L2::iStoreNode
L2::Spill::
Spiller::make_alias_iStoreNode( void )
{
	const long long stk_ofs
	{
		static_cast< long long >( this->spill_var_id ) * 8
	};

	return iStoreNode
	{
		.mem_n = MemNode {},
		.x_n = xNode { RspNode {} },
		.M_n = MNode { .val = stk_ofs },
		.op_assign_n = {},
		.s_n = this->make_alias_node< wNode >(),
	};

}

void
L2::Spill::
Spiller::operator()( const iAssignNode &n )
{
	// 
	// w <- s
	//
	// read s
	// write w
	//

	const std::string_view
		w_name { std::visit( this->var_view, n.w_n ) },
		s_name { std::visit( this->var_view, n.s_n ) };

	const bool
		spill_w { this->is_spill_var_name( w_name ) },
		spill_s { this->is_spill_var_name( s_name ) };

	this->try_add_alias_iLoadNode( spill_s );

	this->add_iNode(

		iAssignNode
		{
			.w_n = this->try_make_alias_node< wNode >( n.w_n, spill_w ),
			.op_assign_n = {},
			.s_n = this->try_make_alias_node< sNode >( n.s_n, spill_s ),
		}

	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_s );

}

void
L2::Spill::
Spiller::operator()( const iLoadNode &n ) 
{
	// 
	// w <- mem x M
	//
	// read x
	// write w
	//

	const std::string_view
		w_name { std::visit( this->var_view, n.w_n ) },
		x_name { std::visit( this->var_view, n.x_n ) };
	const bool
		spill_w { this->is_spill_var_name( w_name ) },
		spill_x { this->is_spill_var_name( w_name ) };

	this->try_add_alias_iLoadNode( spill_x );

	this->add_iNode(
		iLoadNode
		{
			.w_n = this->try_make_alias_node< wNode >( n.w_n, spill_w ),
			.op_assign_n = {},
			.mem_n = {},
			.x_n = this->try_make_alias_node< xNode >( n.x_n, spill_x ),
			.M_n = n.M_n,
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_x );

}

void
L2::Spill::
Spiller::operator()( const iStoreNode &n ) 
{
	// 
	// mem x M <- s
	//
	// read x, s
	//

	const std::string_view
		x_name { std::visit( this->var_view, n.x_n ) },
		s_name { std::visit( this->var_view, n.s_n ) };
	const bool
		spill_x { this->is_spill_var_name( x_name ) },
		spill_s { this->is_spill_var_name( s_name ) };

	//
	// x is read from, not written to,
	// to compute the mem location to ultimately write to
	//
	this->try_add_alias_iLoadNode( spill_x || spill_s );

	this->add_iNode(
		iStoreNode
		{
			.mem_n = {},
			.x_n = this->try_make_alias_node< xNode >( n.x_n, spill_x ),
			.M_n = n.M_n,
			.op_assign_n = {},
			.s_n = this->try_make_alias_node< sNode >( n.s_n, spill_s ),

		}
	);

	// x is not written to

	this->try_advance_alias_id( spill_x || spill_s );

}

void
L2::Spill::
Spiller::operator()( const iStackArgNode &n )
{
	// 
	// w <- stack-arg M
	//
	// write w
	//

	const std::string_view
		w_name { std::visit( this->var_view, n.w_n ) };
	const bool
		spill_w { this->is_spill_var_name( w_name ) };

	this->add_iNode(
		iStackArgNode
		{
			.w_n = this->try_make_alias_node< wNode >( n.w_n, spill_w ),
			.op_assign_n = {},
			.stack_arg_n = {},
			.M_n = n.M_n,
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w );

}

void
L2::Spill::
Spiller::operator()( const iAOpNode &n )
{
	//
	// w aop t
	//
	// read w, t
	// write w
	//

	const std::string_view
		w_name { std::visit( this->var_view, n.w_n ) },
		t_name { std::visit( this->var_view, n.t_n ) };
	const bool
		spill_w { this->is_spill_var_name( w_name ) },
		spill_t { this->is_spill_var_name( t_name ) };

	//
	// w is both read from and written to
	//
	this->try_add_alias_iLoadNode( spill_w || spill_t );

	this->add_iNode(
		iAOpNode
		{
			.w_n = this->try_make_alias_node< wNode >( n.w_n, spill_w ),
			.aop_n = n.aop_n,
			.t_n = this->try_make_alias_node< tNode >( n.t_n, spill_t ),
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_t );

}

void
L2::Spill::
Spiller::operator()( const iSxNode &n )
{
	//
	// w sop sx
	//
	// read w, sx
	// write w
	//

	const std::string_view
		w_name  { std::visit( this->var_view, n.w_n ) },
		sx_name { std::visit( this->var_view, n.sx_n ) };
	const bool
		spill_w  { this->is_spill_var_name( w_name ) },
		spill_sx { this->is_spill_var_name( sx_name ) };

	this->try_add_alias_iLoadNode( spill_w || spill_sx );

	this->add_iNode(
		iSxNode
		{
			.w_n = this->try_make_alias_node< wNode >( n.w_n, spill_w ),
			.sop_n = n.sop_n,
			.sx_n = this->try_make_alias_node< sxNode >( n.sx_n, spill_sx ),
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_sx );

}

void
L2::Spill::
Spiller::operator()( const iSOpNode &n )
{
	//
	// w sop N
	//
	// read w
	// write w
	//

	const std::string_view
		w_name  { std::visit( this->var_view, n.w_n ) };
	const bool
		spill_w  { this->is_spill_var_name( w_name ) };

	this->try_add_alias_iLoadNode( spill_w );

	this->add_iNode(
		iSOpNode
		{
			.w_n = this->try_make_alias_node< wNode >( n.w_n, spill_w ),
			.sop_n = n.sop_n,
			.N_n = n.N_n,
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w );

}

void
L2::Spill::
Spiller::operator()( const iAddStoreNode &n )
{
	// 
	// mem x M += t
	//
	// read x, t
	//

	const std::string_view
		x_name  { std::visit( this->var_view, n.x_n ) },
		t_name  { std::visit( this->var_view, n.t_n ) };
	const bool
		spill_x { this->is_spill_var_name( x_name ) },
		spill_t { this->is_spill_var_name( t_name ) };

	this->try_add_alias_iLoadNode( spill_x || spill_t );

	this->add_iNode(
		iAddStoreNode
		{
			.mem_n = {},
			.x_n = this->try_make_alias_node< xNode >( n.x_n, spill_x ),
			.M_n = n.M_n,
			.op_add_eq_n = {},
			.t_n = this->try_make_alias_node< tNode >( n.t_n, spill_t ),
		}
	);

	// x is not written to

	this->try_advance_alias_id( spill_x || spill_t );

}

void
L2::Spill::
Spiller::operator()( const iSubStoreNode &n )
{
	// 
	// mem x M -= t
	//
	// read x, t
	//

	const std::string_view
		x_name  { std::visit( this->var_view, n.x_n ) },
		t_name  { std::visit( this->var_view, n.t_n ) };
	const bool
		spill_x { this->is_spill_var_name( x_name ) },
		spill_t { this->is_spill_var_name( t_name ) };

	this->try_add_alias_iLoadNode( spill_x || spill_t );

	this->add_iNode(
		iSubStoreNode
		{
			.mem_n = {},
			.x_n = this->try_make_alias_node< xNode >( n.x_n, spill_x ),
			.M_n = n.M_n,
			.op_sub_eq_n = {},
			.t_n = this->try_make_alias_node< tNode >( n.t_n, spill_t ),
		}
	);

	// x is not written to

	this->try_advance_alias_id( spill_x || spill_t );

}

void
L2::Spill::
Spiller::operator()( const iLoadAddNode &n )
{
	//
	// w += mem x M
	//
	// read w, x
	// write w
	//

	const std::string_view
		w_name  { std::visit( this->var_view, n.w_n ) },
		x_name  { std::visit( this->var_view, n.x_n ) };
	const bool
		spill_w { this->is_spill_var_name( w_name ) },
		spill_x { this->is_spill_var_name( x_name ) };

	this->try_add_alias_iLoadNode( spill_w || spill_x );

	this->add_iNode(
		iLoadAddNode
		{
			.w_n = this->try_make_alias_node< wNode >( n.w_n, spill_w ),
			.op_add_eq_n = {},
			.mem_n = {},
			.x_n = this->try_make_alias_node< xNode >( n.x_n, spill_x ),
			.M_n = n.M_n,
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_x );

}

void
L2::Spill::
Spiller::operator()( const iLoadSubNode &n )
{
	// 
	// w -= mem x M
	//
	// read w, x
	// write w
	//

	const std::string_view
		w_name  { std::visit( this->var_view, n.w_n ) },
		x_name  { std::visit( this->var_view, n.x_n ) };
	const bool
		spill_w { this->is_spill_var_name( w_name ) },
		spill_x { this->is_spill_var_name( x_name ) };

	this->try_add_alias_iLoadNode( spill_w || spill_x );

	this->add_iNode(
		iLoadSubNode
		{
			.w_n = this->try_make_alias_node< wNode >( n.w_n, spill_w ),
			.op_sub_eq_n = {},
			.mem_n = {},
			.x_n = this->try_make_alias_node< xNode >( n.x_n, spill_x ),
			.M_n = n.M_n,
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_x );

}

void
L2::Spill::
Spiller::operator()( const iCmpAssignNode &n )
{
	// 
	// w <- t cmp t
	//
	// read t1, t2
	// write w
	//

	const std::string_view
		w_name  { std::visit( this->var_view, n.w_n ) },
		t1_name { std::visit( this->var_view, n.t1_n ) },
		t2_name { std::visit( this->var_view, n.t2_n ) };
	const bool
		spill_w  { this->is_spill_var_name( w_name ) },
		spill_t1 { this->is_spill_var_name( t1_name ) },
		spill_t2 { this->is_spill_var_name( t2_name ) };

	this->try_add_alias_iLoadNode( spill_t1 || spill_t2 );

	this->add_iNode(
		iCmpAssignNode
		{
			.w_n = this->try_make_alias_node< wNode >( n.w_n, spill_w ),
			.op_assign_n = {},
			.t1_n = this->try_make_alias_node< tNode >( n.t1_n, spill_t1 ),
			.cmp_n = n.cmp_n,
			.t2_n = this->try_make_alias_node< tNode >( n.t2_n, spill_t2 ),
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_t1 || spill_t2 );

}

void
L2::Spill::
Spiller::operator()( const iCJumpNode &n )
{
	// 
	// cjump t cmp t label
	//
	// read t1, t2
	//

	const std::string_view
		t1_name { std::visit( this->var_view, n.t1_n ) },
		t2_name { std::visit( this->var_view, n.t2_n ) };
	const bool
		spill_t1 { this->is_spill_var_name( t1_name ) },
		spill_t2 { this->is_spill_var_name( t2_name ) };

	this->try_add_alias_iLoadNode( spill_t1 || spill_t2 );

	this->add_iNode(
		iCJumpNode
		{
			.cjump_n = {},
			.t1_n = this->try_make_alias_node< tNode >( n.t1_n, spill_t1 ),
			.cmp_n = n.cmp_n,
			.t2_n = this->try_make_alias_node< tNode >( n.t2_n, spill_t2 ),
			.label_n = n.label_n
		}
	);

	this->try_advance_alias_id( spill_t1 || spill_t2 );

}

void
L2::Spill::
Spiller::operator()( const iLabelNode &n )
{
	// label

	this->add_iNode( n );

}

void
L2::Spill::
Spiller::operator()( const iGotoNode &n )
{
	// goto label

	this->add_iNode( n );

}

void
L2::Spill::
Spiller::operator()( const iReturnNode &n )
{
	// return

	this->add_iNode( n );

}

void
L2::Spill::
Spiller::operator()( const iCallUNode &n )
{
	// 
	// call u N
	//
	// read u
	//

	const std::string_view
		u_name { std::visit( this->var_view, n.u_n ) };
	const bool
		spill_u { this->is_spill_var_name( u_name ) };

	this->try_add_alias_iLoadNode( spill_u );

	this->add_iNode(
		iCallUNode
		{
			.call_n = {},
			.u_n = this->try_make_alias_node< uNode >( n.u_n, spill_u ),
			.N_n = n.N_n,
		}
	);

	this->try_advance_alias_id( spill_u );

}

void
L2::Spill::
Spiller::operator()( const iCallPrintNode &n )
{
	// call print 1

	this->add_iNode( n );

}

void
L2::Spill::
Spiller::operator()( const iCallInputNode &n )
{
	// call input 0

	this->add_iNode( n );

}

void
L2::Spill::
Spiller::operator()( const iCallAllocateNode &n )
{
	// call allocate 2

	this->add_iNode( n );

}

void
L2::Spill::
Spiller::operator()( const iCallTupleErrorNode &n )
{
	// call tuple-error 3

	this->add_iNode( n );

}

void
L2::Spill::
Spiller::operator()( const iCallTensorErrorNode &n )
{
	// call tensor-error F

	this->add_iNode( n );

}

void
L2::Spill::
Spiller::operator()( const iIncrNode &n )
{
	//
	// w ++
	//
	// read w
	// write w
	//

	const std::string_view
		w_name { std::visit( this->var_view, n.w_n ) };
	const bool
		spill_w { this->is_spill_var_name( w_name ) };

	this->try_add_alias_iLoadNode( spill_w );

	this->add_iNode(
		iIncrNode
		{
			.w_n = this->try_make_alias_node< wNode >( n.w_n, spill_w ),
			.op_incr_n = {},
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w );

}

void
L2::Spill::
Spiller::operator()( const iDecrNode &n )
{
	// 
	// w --
	//
	// read w
	// write w
	//

	const std::string_view
		w_name { std::visit( this->var_view, n.w_n ) };
	const bool
		spill_w { this->is_spill_var_name( w_name ) };

	this->try_add_alias_iLoadNode( spill_w );

	this->add_iNode(
		iDecrNode
		{
			.w_n = this->try_make_alias_node< wNode >( n.w_n, spill_w ),
			.op_decr_n = {},
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w );

}

void
L2::Spill::
Spiller::operator()( const iLEANode &n )
{
	//
	// w @ w w E
	//
	// read w2, w3
	// write w1
	//

	const std::string_view
		w1_name { std::visit( this->var_view, n.w1_n ) },
		w2_name { std::visit( this->var_view, n.w2_n ) },
		w3_name { std::visit( this->var_view, n.w3_n ) };
	const bool
		spill_w1 { this->is_spill_var_name( w1_name ) },
		spill_w2 { this->is_spill_var_name( w2_name ) },
		spill_w3 { this->is_spill_var_name( w3_name ) };

	//
	// w2 and w3 are read from
	//
	this->try_add_alias_iLoadNode( spill_w2 || spill_w3 );

	this->add_iNode(
		iLEANode
		{
			.w1_n = this->try_make_alias_node< wNode >( n.w1_n, spill_w1 ),
			.at_n = {},
			.w2_n = this->try_make_alias_node< wNode >( n.w2_n, spill_w2 ),
			.w3_n = this->try_make_alias_node< wNode >( n.w3_n, spill_w3 ),
			.E_n = n.E_n,
		}
	);

	//
	// w1 is written to
	//
	this->try_add_alias_iStoreNode( spill_w1 );

	this->try_advance_alias_id( spill_w1 || spill_w2 || spill_w3 );

}

void
L2::Spill::
Spiller::operator()( const fNode &f_n )
{
	for ( const iNode &i_n: f_n.i_ns )
	{
		std::visit( *this, i_n );
	}
}


