
#include "unparse.h"

#include <string>

std::string
L2::Spil::
Unparser::operator()( const L2::iAssignNode &n )
{
	// w <- s

	return ( *this )(
		n.w_n, SP,
		n.op_assign_n, SP,
		n.s_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iLoadNode &n )
{
	// w <- mem x M

	return ( *this )(
		n.w_n, SP,
		n.op_assign_n, SP,
		n.mem_n, SP,
		n.x_n, SP,
		n.M_n, LF
	);

}

std::string 
L2::Spil::
Unparser::operator()( const L2::iStoreNode &n )
{
	// mem x M <- s

	return ( *this )(
		n.mem_n, SP,
		n.x_n, SP,
		n.M_n, SP,
		n.op_assign_n, SP,
		n.s_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iStackArgNode &n )
{
	// w <- stack-arg M

	return ( *this )(
		n.w_n, SP,
		n.op_assign_n, SP,
		n.stack_arg_n, SP,
		n.M_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iAOpNode &n )
{
	// w aop t

	return ( *this )(
		n.w_n, SP,
		n.aop_n, SP,
		n.t_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iSxNode &n )
{
	// w sop sx

	return ( *this )(
		n.w_n, SP,
		n.sop_n, SP,
		n.sx_n, LF
	);

}


std::string
L2::Spil::
Unparser::operator()( const L2::iSOpNode &n )
{
	// w sop N

	return ( *this )(
		n.w_n, SP,
		n.sop_n, SP,
		n.N_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iAddStoreNode &n )
{
	// mem x M += t

	return ( *this )(
		n.mem_n, SP,
		n.x_n, SP,
		n.M_n, SP,
		n.op_add_eq_n, SP,
		n.t_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iSubStoreNode &n )
{
	// mem x M -= t

	return ( *this )(
		n.mem_n, SP,
		n.x_n, SP,
		n.M_n, SP,
		n.op_sub_eq_n, SP,
		n.t_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iLoadAddNode &n )
{
	// w += mem x M

	return ( *this )(
		n.w_n, SP,
		n.op_add_eq_n, SP,
		n.mem_n, SP,
		n.x_n, SP,
		n.M_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iLoadSubNode &n )
{
	// w -= mem x M

	return ( *this )(
		n.w_n, SP,
		n.op_sub_eq_n, SP,
		n.mem_n, SP,
		n.x_n, SP,
		n.M_n, LF
	);
	
}

std::string
L2::Spil::
Unparser::operator()( const L2::iCmpAssignNode &n )
{
	// w <- t cmp t

	return ( *this )(
		n.w_n, SP,
		n.op_assign_n, SP,
		n.t1_n, SP,
		n.cmp_n, SP,
		n.t2_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iCJumpNode &n )
{
	// cjump t cmp t label

	return ( *this )(
		n.cjump_n, SP,
		n.t1_n, SP,
		n.cmp_n, SP,
		n.t2_n, SP,
		n.label_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iLabelNode &n )
{
	// label

	return ( *this )( n.label_n, LF );

}

std::string
L2::Spil::
Unparser::operator()( const L2::iGotoNode &n )
{
	// goto label

	return ( *this )(
		n.goto_n, SP,
		n.label_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iReturnNode &n )
{
	// return

	return ( *this )( n.return_n, LF );

}

std::string
L2::Spil::
Unparser::operator()( const L2::iCallUNode &n )
{
	// call u N

	return ( *this )(
		n.call_n, SP,
		n.u_n, SP,
		n.N_n, LF
	);
}

std::string
L2::Spil::
Unparser::operator()( const L2::iCallPrintNode &n )
{
	// call print 1

	return ( *this )(
		n.call_n, SP,
		n.print_n, SP,
		n._1_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iCallInputNode &n )
{
	// call input 0

	return ( *this )(
		n.call_n, SP,
		n.input_n, SP,
		n._0_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iCallAllocateNode &n )
{
	// call allocate 2

	return ( *this )(
		n.call_n, SP,
		n.allocate_n, SP,
		n._2_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iCallTupleErrorNode &n )
{
	// call tuple-error 3

	return ( *this )(
		n.call_n, SP,
		n.tuple_error_n, SP,
		n._3_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iCallTensorErrorNode &n )
{
	// call tensor-error f

	return ( *this )(
		n.call_n, SP,
		n.tensor_error_n, SP,
		n.F_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iIncrNode &n )
{
	// w ++

	return ( *this )(
		n.w_n, SP,
		n.op_incr_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iDecrNode &n )
{
	// w --

	return ( *this )(
		n.w_n, SP,
		n.op_decr_n, LF
	);

}

std::string
L2::Spil::
Unparser::operator()( const L2::iLEANode &n )
{
	// w @ w w E

	return ( *this )(
		n.w1_n, SP,
		n.at_n, SP,
		n.w2_n, SP,
		n.w3_n, SP,
		n.E_n, LF
	);

}

