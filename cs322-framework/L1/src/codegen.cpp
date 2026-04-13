
#include "codegen.h"
#include "ast.h"
#include <iostream>

namespace L1
{

	struct aopVisitor: L1::Visitor
	{
		void operator()( const L1::OpAddEqNode &op_add_eq_n )   { this->os << L1::Instr::ADDQ };
		void operator()( const L1::OpSubEqNode &op_sub_eq_n )   { this->os << L1::Instr::SUBQ };
		void operator()( const L1::OpMulEqNode &op_mul_eq_n )   { this->os << L1::Instr::IMULQ };
		void operator()( const L1::OpBAndEqNode &op_band_eq_n ) { this->os << L1::Instr::ANDQ };
	};

	struct sopVisitor: L1::Visitor
	{
		void operator()( const L1::OpLshEqNode &op_lsh_eq_n ) { this->os << L1::Instr::SALQ };
		void operator()( const L1::OpLshEqNode &op_rsh_eq_n ) { this->os << L1::Instr::SARQ };
	};

	struct cmp_N1N2_Visitor: L1::Visitor
	{
		cmpNode &cmp_n;

		cmp_N1N2_Visitor( std::ostream &os_, cmpNode &cmp_n_ ): os { os_ }, cmp_n { cmp_n_ } {}

	};

	struct cmp_t1t2_Visitor : L1::Visitor
	{
		void operator()( const NNode &t1_n, const NNode &t2_n )
		{
			
		}
	};
}

/*
void
L1::
iVisitor::operator()( const L1::iCmpAssignNode &i_cmp_assign_n )
{
	constexpr bool use_low { true };
	const L1::wNode w_n { i_cmp_assign_n.w_n };

	// op aux
	// t1 t2 aux
	
	L1::
	this->os << " ";
	L1::wVisitor{ this->os }( w_n );
	this->os << "\n";

	this->os << "\tmovzbq ";
	L1::wVisitor{ this->os, use_low }( w_n );
	this->os << "," ;
	// re-recurse on w because each visitor outputs directly to stream and can't cache strings
	// but that's OK since it's all going to burn anyway
	L1::wVisitor{ this->os }( w_n );
	this->os << "\n";
}
*/

void
L1::
iVisitor::operator()( const L1::iAssignNode &i_assign_n )
{
	constexpr bool ismem { true };

	this->os << L1::Instr::MOVQ;
	std::visit( L1::sVisitor{ this->os, ismem }, this->s_n );	
	this->os << ",";
	std::visit( L1::wVisitor{ this->os }, this->w_n );
	this->os << "\n";
}

void
L1::
iVisitor::operator()( const L1::iLoadNode &i_load_n )
{
	// w   <- mem x M
	// rdi <- mem rsp 8
	//
	// movq 8(%rsp),%rdi
	// movq M(x),w

	this->os << L1::Instr::MOVQ;

	// MNode is not variant
	L1::MVisitor{ this->os }( i_load_n.M_n );
	this->os << "(";
	std::visit( L1::xVisitor, i_load_n.x_n );
	this->os << "),";
	std::visit( L1::wVisitor{ this->os }, i_load_n.w_n );
	this->os << "\n";

}

void
L1::
iVisitor::operator()( const L1::iStoreNode &i_store_n )
{
	// mem x   M <- s
	// mem rsp 0 <- rdi
	//
	// movq %rdi,0(%rsp)
	// movq s,M(x)

	constexpr bool ismem { true };

	this->os << L1::Instr::MOVQ;

	std::visit( L1::sVisitor{ this->os, ismem }, i_store_n.s_n );
	this->os << ",";
	L1::MVisitor{ this->os }( i_load_n.M_n );
	this->os << "(";
	std::visit( L1::xVisitor, i_load_n.x_n );
	this->os << ")\n";
}

void
L1::
iVisitor::operator()( const L1::iAOpNode &i_aop_n )
{
	// w aop t
	// rdi += rax
	//
	// addq %rax,%rdi
	// aop t,w
	//

	std::visit( L1::aopVisitor{ this->os }, i_aop_n.aop_n );
	this->os << " ";
	std::visit( L1::tVisitor{ this->os }, i_aop_n.t_n );
	this->os << ",";
	std::visit( L1::wVisitor{ this->os }, i_aop_n.w_n );
	this->os << "\n";
}

void
L1::
iVisitor::
operator()( const L1::iSxNode &i_sx_n )
{
	// w sop sx
	// rdi <<= rcx
	//
	// salq %cl,%rdi
	// sop sx,w
	//

	constexpr bool use_low { true };

	std::visit( L1::sopVisitor{ this->os }, i_sx_n.sop_n );
	this->os << " ";
	std::visit( L1::sxVisitor{ this->os, use_low }, i_sx_n.sx_n );
	this->os << ",";
	std::visit( L1::wVisitor{ this->os }, i_sx_n.w_n );
	this->os << "\n";
}

void
L1::
iVisitor::operator()( const L1::iSOpNode &i_sop_n )
{
	// w sop N
	// rdi >>= 3
	//
	// sarq $3,%rdi
	// sop N,w
	//

	std::visit( L1::sopVisitor{ this->os }, i_sop_n.sop_n );
	this->os << " ";
	std::visit( L1::NVisitor{ this->os }, i_sop_n.N_n );
	this->os << ",";
	std::visit( L1::wVisitor{ this->os }, i_sop_n.w_n );
	this->os << "\n";

}

void
L1::
iVisitor::operator()( const L1::iAddStoreNode &i_add_store_n )
{
	//
	// mem x M += t
	// mem rsp 8 += rdi
	//
	// addq %rdi,8(%rsp)
	// addq t,M(x)
	//

	this->os << L1::Instr::ADDQ;
	this->os << " ";
	std::visit( L1::tVisitor( this->os ), i_add_store_n.t_n );
	this->os << ",";
	std::visit( L1::MVisitor( this->os ), i_add_store_n.M_n );	
	this->os << "(";
	std::visit( L1::xVisitor( this->os ), i_add_store_n.x );	
	this->os << ")\n";
	
}

void
L1::
iVisitor::operator()( const L1::iSubStoreNode &i_sub_store_n )
{
	//
	// mem x M -= t
	// mem rsp 8 -= rdi
	//
	// subq %rdi,8(%rsp)
	// subq t,M(x)
	//

	this->os << L1::Instr::SUBQ
	this->os << " ";
	std::visit( L1::tVisitor( this->os ), i_sub_store_n.t_n );
	this->os << ",";
	std::visit( L1::MVisitor( this->os ), i_sub_store_n.M_n );	
	this->os << "(";
	std::visit( L1::xVisitor( this->os ), i_sub_store_n.x );	
	this->os << ")";
	this->os << "\n";
}


void
L1::
iVisitor::operator()( const L1::iLoadSubNode &i_load_sub_n )
{
	//
	// w -= mem x M 
	// rdi -= mem rsp 8
	//
	// subq 8(%rsp),%rdi
	// subq M(x),w
	//

	this->os << L1::Instr::SUBQ;
	this->os << " ";
	std::visit( L1::MVisitor( this->os ), i_load_sub_n.M_n );	
	this->os << "(";
	std::visit( L1::xVisitor( this->os ), i_load_sub_n.x );	
	this->os << "),";
	std::visit( L1::wVisitor( this->os ), i_load_sub_n.w_n );
	this->os << "\n";
}


void
L1::
iVisitor::operator()( const L1::iLoadAddNode &i_load_add_n )
{
	//
	// w += mem x M 
	// rdi += mem rsp 8
	//
	// addq 8(%rsp),%rdi
	// addq M(x),w
	//

	this->os << L1::Instr::ADDQ;
	std::visit( L1::MVisitor( this->os ), i_load_add_n.M_n );
	this->os << "(";
	std::visit( L1::xVisitor( this->os ), i_load_add_n.x );	
	this->os << "),";
	std::visit( L1::wVisitor( this->os ), i_load_add_n.w_n );
	this->os << "\n";

}


void operator()( const L1::iIncrNode &i_incr_n )
{
	this->os << L1::Instr::ADDQ;
	this->os << "$1,";
	std::visit( L1::wVisitor( this->os ), i_incr_n.w_n );
	this->os << "\n";
	
}

void operator()( const L1::iDecrNode &i_decr_n )
{
	this->os << L1::Instr::SUBQ;
	this->os << "$1,";
	std::visit( L1::wVisitor( this->os ), i_decr_n.w_n );
	this->os << "\n";
	
}

void
L1::
iVisitor::operator()( const L1::iLabelNode &i_label_n )
{
	L1::labelVisitor{ this->os }( i_label_n.label_n );
	this->os << ":\n";
}

void
L1::
iVisitor::operator()( const L1::iGotoNode &i_goto_n )
{
	this->os << L1::Instr::JMP;
	L1::labelVisitor{ this->os }( i_goto_n.label_n );
	this->os << "\n";
}


void
L1::
iVisitor::operator()( const L1::iReturnNode &i_return_n )
{
	this->os << L1::Instr::RETQ << "\n";
}

void
L1::
iVisitor::operator()( const L1::iLEANode &i_lea_n )
{
	//
	// w1 @ w2 w3 E
	// rax @ rdi rsi 4
	//
	// lea (%rdi,%rsi,4),%rax
	// lea (w2,w3,E),w1
	//

	this->os << L1::Instr::LEA;
	this->os << "(";
	std::visit( L1::wVisitor{ this->os }, i_lea_n.w2_n );
	this->os << ",";
	std::visit( L1::wVisitor{ this->os }, i_lea_n.w3_n );	
	this->os << ",";
	L1::EVisitor{ this->os }( i_lea_n.E_n );
	this->os << "),";
	std::visit( L1::wVisitor{ this->os }, i_lea_n.w1_n );
	this->os << "\n";
}

/*

		void operator()( const L1::iCJumpNode & );
		void operator()( const L1::iCallUNode & );
		void operator()( const L1::iCallPrintNode & );
		void operator()( const L1::iCallInputNode & );
		void operator()( const L1::iCallAllocateNode & );
		void operator()( const L1::iCallTupleErrorNode & );
		void operator()( const L1::iCallTensorErrorNode & );
*/

