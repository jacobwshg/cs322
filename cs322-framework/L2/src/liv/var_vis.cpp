
#include "var_vis.h"

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

