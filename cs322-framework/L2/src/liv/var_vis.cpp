
#include "var_vis.h"
#include "ints.h"

#include <cstdio>
#include <string_view>
#include <string>

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

void
L2::Liv::
VarVisitor::display( void ) const
{
	var_id_t var_id { -1 };

	std::printf( "variable visitor\n" );
	for ( const std::string_view gpr_sv : GPRId::ID_GPR_TBL )
	{
		++var_id;
		std::printf( "%d\t%s\n", var_id, gpr_sv.data() );
	}
	for ( const std::string &var_name: this->id_var_tbl )
	{
		std::printf( "%d\t%s%s\n", this->var_id_tbl.at( var_name ), L2::KW::PERCENT.data(), var_name.data() );
	}
	std::printf( "\n" );
}

// given ID, retrieve variable name 
std::string_view
L2::Liv::
VarVisitor::var_name_by_id( const L2::var_id_t var_id ) const
{

	if ( var_id > 0 && var_id < VarVisitor::BASE_VAR_ID )
	{
		// var ID is that of a GPR
		return GPRId::ID_GPR_TBL[ var_id ];
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
// retrieve var ID associated with a variable name, without creating a new one 
// for an unknown variable. for unknown variables, VAR_ID_INVAL is returned.
//
L2::var_id_t
L2::Liv::
VarVisitor::var_id_by_name( const std::string_view var_name ) const
{
	const auto tbl_it { this->var_id_tbl.find( var_name ) };
	if ( tbl_it == this->var_id_tbl.end() ) { return L2::VAR_ID_INVAL; }
	return tbl_it->second;
}

//
// return the var ID of a node representing a named variable. 
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
	//
	// descend into name member of var node
	//
	return ( *this )( var_n.name_n );
}

