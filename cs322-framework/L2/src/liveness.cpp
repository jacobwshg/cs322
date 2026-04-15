
#include "liveness.h"
#include "ast.h"

#include <cstdio>
#include <variant>
#include <string>
#include <string_view>

L2::
LivenessVisitor::LivenessVisitor( void )
{
	this->id_var_mp.reserve( 8 );
}

L2::var_id_t
L2::
LivenessVisitor::operator()( const nameNode &name_n )
{
	const auto mpit { this->var_id_mp.find( name_n.val ) };
	if ( mpit != this->var_id_mp.end() )
	{
		return mpit->second;
	}
	else
	{
		const var_id_t var_id { this->next_var_id };
		++this->next_var_id;

		// var_id_mp uses logical id: [ var1 ] = 16, [ var2 ] = 17, ...
		this->var_id_mp.insert( { std::string { name_n.val }, var_id } );

		// id_var_mp has no padding for GPRs to save space, 
		// uses physical id: [ 0 ] = var1, [ 1 ] = var2, ...
		this->id_var_mp.emplace_back( std::string { name_n.val } );
		return var_id;
	}
}

L2::var_id_t
L2::
LivenessVisitor::operator()( const varNode &var_n )
{
	return ( *this )( var_n.name_n );
}

std::string_view
L2::
LivenessVisitor::var_by_id( const var_id_t var_id )
{
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
		if ( var_id >= L2::LivenessVisitor::BASE_VAR_ID )
		{
			// to index id_var_mp, convert logical id to physical id
			return std::string_view { this->id_var_mp[ var_id - BASE_VAR_ID ] };
		}
		else { return L2::EMPTYTOK; }
		break;
	}

	return L2::EMPTYTOK;
}

int
main()
{
	L2::LivenessVisitor lv {};
	std::printf(
		"%d %d\n",
		lv( L2::RdxNode{} ),
		//lv( L2::RspNode{} ) // should fail to compile
		lv( L2::R13Node() )
	);
}

