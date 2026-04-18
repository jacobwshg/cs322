
#include "spiller.h"
#include "unparser.h"

#include <cstdint>

L2::Spil::
Spiller::Spiller(
	const std::string_view spill_var_name_,
	const std::string_view alias_prefix_
):
	spill_var_name { spill_var_name_ },
	alias_prefix { alias_prefix_ },
	unparser { 1LL }
{
	this->f_spill.i_ns.reserve( 16 );
}

std::size_t
L2::Spil::
Spiller::new_alias_id( void )
{
	const std::size_t id { this->next_alias_id };
	++this->next_alias_id;
	return id;

}

//
// 
//
L2::iLoadNode
L2::Spil::
Spiller::make_alias_iLoadNode( void )
{
	const long long stk_ofs
	{
		static_cast< long long >( this->spill_var_id ) * 8
	};

	return iLoadNode
	{
		// use the spill var name to force an aliased node
		.w_n = this->try_make_alias_node< wNode >( this->spill_var_name ),
		.op_assign_n = {},
		.mem_n = MemNode {},
		.x_n = xNode { RspNode {} },
		.M_n = MNode { .val = stk_ofs },
	};

}


L2::iStoreNode
L2::Spil::
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
		.s_n = this->try_make_alias_node< wNode >( this->spill_var_name ),
	};

}


void
L2::Spil::
Spiller::operator()( const iAssignNode &n )
{
	// w <- s

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
			.w_n = this->try_make_alias_node< wNode >( w_name ),
			.op_assign_n = {},
			.s_n = this->try_make_alias_node< sNode >( s_name ),
		}

	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_s );

}

void
L2::Spil::
Spiller::operator()( const iLoadNode &n ) 
{
	// w <- mem x M

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
			.w_n = this->try_make_alias_node< wNode >( w_name ),
			.op_assign_n = {},
			.mem_n = {},
			.x_n = this->try_make_alias_node< xNode >( x_name ),
			.M_n = n.M_n,
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_x );

}

void
L2::Spil::
Spiller::operator()( const iStoreNode &n ) 
{
	// mem x M <- s

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
			.x_n = this->try_make_alias_node< xNode >( x_name ),
			.M_n = n.M_n,
			.op_assign_n = {},
			.s_n = this->try_make_alias_node< sNode >( s_name ),

		}
	);

	// x is not written to

	this->try_advance_alias_id( spill_x || spill_s );

}

void
L2::Spil::
Spiller::operator()( const iStackArgNode &n )
{
	// w <- stack-arg M

	const std::string_view
		w_name { std::visit( this->var_view, n.w_n ) };
	const bool
		spill_w { this->is_spill_var_name( w_name ) };

	this->add_iNode(
		iStackArgNode
		{
			.w_n = this->try_make_alias_node< wNode >( w_name ),
			.op_assign_n = {},
			.stack_arg_n = {},
			.M_n = n.M_n,
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w );

}

void
L2::Spil::
Spiller::operator()( const iAOpNode &n )
{
	// w aop t

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
			.w_n = this->try_make_alias_node< wNode >( w_name ),
			.aop_n = n.aop_n,
			.t_n = this->try_make_alias_node< wNode >( t_name ),
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_t );

}

void
L2::Spil::
Spiller::operator()( const iSxNode &n )
{
	// w sop sx

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
			.w_n = this->try_make_alias_node< wNode >( w_name ),
			.sop_n = n.sop_n,
			.sx_n = this->try_make_alias_node< sxNode >( sx_name ),
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_sx );

}

void
L2::Spil::
Spiller::operator()( const iSOpNode &n )
{
	// w sop N

	const std::string_view
		w_name  { std::visit( this->var_view, n.w_n ) };
	const bool
		spill_w  { this->is_spill_var_name( w_name ) };

	this->try_add_alias_iLoadNode( spill_w );

	this->add_iNode(
		iSOpNode
		{
			.w_n = this->try_make_alias_node< wNode >( w_name ),
			.sop_n = n.sop_n,
			.N_n = n.N_n,
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w );

}

void
L2::Spil::
Spiller::operator()( const iAddStoreNode &n )
{
	// mem x M += t

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
			.x_n = this->try_make_alias_node< xNode >( x_name ),
			.M_n = n.M_n,
			.op_add_eq_n = {},
			.t_n = this->try_make_alias_node< tNode >( t_name ),
		}
	);

	// x is not written to

	this->try_advance_alias_id( spill_x || spill_t );

}

void
L2::Spil::
Spiller::operator()( const iSubStoreNode &n )
{
	// mem x M -= t

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
			.x_n = this->try_make_alias_node< xNode >( x_name ),
			.M_n = n.M_n,
			.op_sub_eq_n = {},
			.t_n = this->try_make_alias_node< tNode >( t_name ),
		}
	);

	// x is not written to

	this->try_advance_alias_id( spill_x || spill_t );

}

void
L2::Spil::
Spiller::operator()( const iLoadAddNode &n )
{
	// w += mem x M

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
			.w_n = this->try_make_alias_node< wNode >( w_name ),
			.op_add_eq_n = {},
			.mem_n = {},
			.x_n = this->try_make_alias_node< xNode >( x_name ),
			.M_n = n.M_n,
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_x );

}

void
L2::Spil::
Spiller::operator()( const iLoadSubNode &n )
{
	// w -= mem x M

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
			.w_n = this->try_make_alias_node< wNode >( w_name ),
			.op_sub_eq_n = {},
			.mem_n = {},
			.x_n = this->try_make_alias_node< xNode >( x_name ),
			.M_n = n.M_n,
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_x );

}

void
L2::Spil::
Spiller::operator()( const iCmpAssignNode &n )
{
	// w <- t cmp t

	const std::string_view
		w_name  { std::visit( this->var_view, n.w_n ) },
		t1_name { std::visit( this->var_view, n.t1_n ) },
		t2_name { std::visit( this->var_view, n.t2_n ) };
	const bool
		spill_w  { this->is_spill_var_name( w_name ) },
		spill_t1 { this->is_spill_var_name( t1_name ) },
		spill_t2 { this->is_spill_var_name( t2_name ) };

	this->try_add_alias_iLoadNode( spill_w || spill_t1 || spill_t2 );

	this->add_iNode(
		iCmpAssignNode
		{
			.w_n = this->try_make_alias_node< wNode >( w_name ),
			.op_assign_n = {},
			.t1_n = this->try_make_alias_node< tNode >( t1_name ),
			.cmp_n = n.cmp_n,
			.t2_n = this->try_make_alias_node< tNode >( t2_name ),
		}
	);

	this->try_add_alias_iStoreNode( spill_w );

	this->try_advance_alias_id( spill_w || spill_t1 || spill_t2 );

}

























