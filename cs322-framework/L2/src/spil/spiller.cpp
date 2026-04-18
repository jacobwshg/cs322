
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
Spiller::new_alias_id( void );
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
		.w_n = this->make_alias_node< wNode >( this->spill_var_name );

		.op_assign_n = iOpAssignNode {},

		.mem_n = MemNode {},
		.xNode = xNode { RspNode {} },
		.MNode = MNode { .val = stk_ofs };
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
		.xNode = xNode { RspNode {} },
		.MNode = MNode { .val = stk_ofs };

		.op_assign_n = iOpAssignNode {},

		.s_n = this->make_alias_node< wNode >( this->spill_var_name );
	};

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
			.M_n = w_n.M_n,
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
	this->try_add_alias_iLoadNode( spill_x );
	this->try_add_alias_iLoadNode( spill_s );

	this->add_iNode(
		iStoreNode
		{
			.mem_n = {},
			.x_n = this->try_make_alias_node< xNode >( x_name ),
			.M_n = w_n.M_n,
			.op_assign_n = {},
			.s_n = this->try_make_alias_node< sNode >( s_name ),

		}
	);

	this->try_advance_alias_id( spill_x || spill_s );

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
























