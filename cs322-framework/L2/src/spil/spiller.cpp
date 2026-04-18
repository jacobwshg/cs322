
#include "spiller.h"
#include "unparser.h"

#include <cstdint>

L2::Spil::
Spiller::Spiller(
	const std::string_view spill_var_,
	const std::string_view alias_prefix_
):
	spill_var { spill_var_ },
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

L2::iLoadNode
L2::Spil::
Spiller::make_load_node( const std::size_t spill_var_id )
{
	const long long stk_ofs
	{
		static_cast< long long >( this->spill_var_id ) * 8
	};

	return iLoadNode
	{
		.w_n = this->make_alias_wNode;

		.op_assign_n = iOpAssignNode {},

		.mem_n = MemNode {},
		.xNode = xNode { RspNode {} },
		.MNode = MNode { .val = stk_ofs };
	};
}


L2::iLoadNode
L2::Spil::
Spiller::make_store_node( const std::size_t spill_var_id )
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

		.s_n = this->make_alias_sNode();
	};
}

void
L2::Spil::
Spiller::operator()( const iAssignNode &n )
{
	// w <- s

	bool spill { false } ;

	const std::string_view
		w_name { std::visit( this->var_vis, n.w_n ) },
		s_name { std::visit( this->var_vis, n.s_n ) };

	if ( this->spill_var == s_name )
	{
		spill = true;
		this.f_spill.i_ns.emplace_back( this->make_load_node() );
	}

	this.f_spill.i_ns.emplace_back(
		iAssignNode
		{
			.w_n = 
		}
	);

	if ( this->spill_var == w_name )
	{
		spill = true;
		this.f_spill.i_ns.emplace_back( this->make_store_node() );
	}

	if ( spill )
	{
		++this->next_alias_id;
	}

}
























