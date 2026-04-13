
#include "codegen.h"
#include "ast.h"
#include <iostream>
#include <cassert>
#include <string>
#include <string_view>

namespace L1
{
	enum class CmpTag: int
	{
		LT = 0, LEQ, EQ,
	};

	struct aopVisitor: L1::Visitor
	{
		std::string_view operator()( const L1::OpAddEqNode &op_add_eq_n )   { return L1::Instr::ADDQ };
		std::string_view operator()( const L1::OpSubEqNode &op_sub_eq_n )   { return L1::Instr::SUBQ };
		std::string_view operator()( const L1::OpMulEqNode &op_mul_eq_n )   { return L1::Instr::IMULQ };
		std::string_view operator()( const L1::OpBAndEqNode &op_band_eq_n ) { return L1::Instr::ANDQ };
	};

	struct sopVisitor: L1::Visitor
	{
		std::string_view operator()( const L1::OpLshEqNode &op_lsh_eq_n ) { return L1::Instr::SALQ };
		std::string_view operator()( const L1::OpLshEqNode &op_rsh_eq_n ) { return L1::Instr::SARQ };
	};

	// doesn't emit, only retrieve tag
	struct cmpViewer
	{
		L1::CmpTag operator()( const L1::OpLtNode &op_lt_n )   { return L1::CmpTag::LT; }
		L1::CmpTag operator()( const L1::OpLEqNode &op_leq_n ) { return L1::CmpTag::LEQ; }
		L1::CmpTag operator()( const L1::OpEqNode &op_eq_n )   { return L1::CmpTag::EQ; }
	};

	// doens't emit, only retrieve val
	struct NViewer
	{
		long long operator()( const L1::_0Node &_0_n )    { return 0LL; }
		long long operator()( const L1::NNZNode &N_nz_n ) { return N_nz_n.val; }
	};

	// helper for cmp-assign
	struct cmpVisitor: L1::Visitor
	{
		L1::CmpTag cmptag {}; // passed in by iVisitor overload, which can see cmpNode member of overload arg
		std::string_view w_sv {};
		std::string_view w_low_sv {};

		cmpVisitor( const L1::CmpTag cmptag_, const std::string_view w_sv_, const std::string_view w_low_sv ):
			cmptag { cmptag_ }, w_sv { w_sv_ }, w_low_sv { w_low_sv_ }
		{}

		// w <- N cmp N
		std::string operator()( const L1::NNode &t1_n, const L1::NNode &t2_n )
		{
			// directly evaluate comparison of literals
			const long long
				t1val { std::visit( L1::NViewer, t1_n ) },
				t2val { std::visit( L1::NViewer, t2_n ) };

			static constexpr std::string_view
				CMP_TRUVAL { "$1," }, CMP_FLSVAL { "$0," };

			bool cmp_res {};
			switch ( this->cmptag )
			{
			case L1::CmpTag::LT:
				cmp_res = t1val < t2val;
				break;
			case L1::CmpTag::LEQ:
				cmp_res = t1val <= t2val;
				break;
			case L1::CmpTag::EQ:
				cmp_res = t1val == t2val;
				break;
			default:
				assert( false );
				break;
			}

			const std::string_view cmp_val { cmp_res ? CMP_TRUVAL : CMP_FLSVAL };

			return std::string { L1::Instr::MOVQ } + cmp_val + this->w_sv + "\n";
		}

		// w <- x cmp x
		// w <- x cmp N
		// w <- N cmp x ( flip )
		template< typename T1, typename T2 >
		std::string operator()( const T1 &t1_n, const T2 &t2_n )
		{
			constexpr bool t1_is_N { std::is_same_v{ T1, L1::NNode } };
			constexpr bool t2_is_N { std::is_same_v{ T2, L1::NNode } };
			constexpr bool flip { t1_is_N && !t2_is_N };

			std::string t1_str {}, t2_str {};
			if constexpr ( t1_is_N )
			{
				t1_str = std::visit( L1::NVisitor, t1_n );
			}
			else
			{
				t1_str = std::visit( L1::xVisitor, t1_n )
			};
			if constexpr ( t2_is_N )
			{
				t2_str = std::visit( L1::NVisitor, t2_n );
			}
			else
			{
				t2_str = std::visit( L1::xVisitor, t2_n )
			};

			std::string cmp_line {};
			if constexpr ( flip ) // flip, put N (t1) on the left in generated instr
			{
				cmp_line = std::string { L1::Instr::CMPQ } + t1_str + "," + t2_str + "\n"
			};
			else
			{
				cmp_line = std::string { L1::Instr::CMPQ } + t2_str + "," + t1_str + "\n"
			}

			std::string_view set_instr {};
			switch ( this->cmptag )
			{
			case L1::CmpTag::LT:
				if constexpr ( flip ) { set_instr = L1::Instr::SETG };
				else { set_instr = L1::Instr::SETL; }
				break;
			case L1::CmpTag::LEQ:
				if constexpr ( flip ) { set_instr = L1::Instr::SETGE };
				else { set_instr = L1::Instr::SETLE; }
				break;
			case L1::CmpTag::EQ: // not impacted by direction flip
				set_instr = L1::Instr::SETE;
				break;
			default:
				assert( false );
				break;
			}
			const std::string set_line { std::string { set_instr } + this->w_low_sv + "\n" };

			const std::string mov_line
			{
				std::string { L1::Instr::MOVZBQ + this->w_low_sv + "," + this->w_sv + "\n" }
			};

			return cmp_line + set_line + mov_line;
		}

	};

	// helper for cjump	
	struct cjumpVisitor: L1::Visitor
	{
		L1::CmpTag cmptag {}; // passed in by iVisitor overload, which can see cmpNode member of overload arg
		std::string_view &label_sv;

		cmpVisitor( const L1::CmpTag cmptag_, const std::string_view label_sv_ ):
			cmptag { cmptag_ }, label_sv { label_sv_ }
		{}

		// cjump N cmp N label
		std::string operator()( const L1::NNode &t1_n, const L1::NNode &t2_n )
		{
			// directly evaluate comparison of literals
			const long long
				t1val { std::visit( L1::NViewer, t1_n ) },
				t2val { std::visit( L1::NViewer, t2_n ) };

			bool cmp_res {};
			switch ( this->cmptag )
			{
			case L1::CmpTag::LT:
				cmp_res = t1val < t2val;
				break;
			case L1::CmpTag::LEQ:
				cmp_res = t1val <= t2val;
				break;
			case L1::CmpTag::EQ:
				cmp_res = t1val == t2val;
				break;
			default:
				assert( false );
				break;
			}

			const std::string_view cmp_val { cmp_res ? CMP_TRUVAL : CMP_FLSVAL };
			if ( cmp_res )
			{
				// jump
				return std::string { L1::Instr::JMP } + this->label_sv + "\n";
			}
			// don't jump
			return std::string {};
		}

		// cjump x cmp x
		// cjump x cmp N
		// cjump N cmp x ( flip )
		template< typename T1, typename T2 >
		std::string operator()( const T1 &t1_n, const T2 &t2_n )
		{
			constexpr bool t1_is_N { std::is_same_v{ T1, L1::NNode } };
			constexpr bool t2_is_N { std::is_same_v{ T2, L1::NNode } };
			constexpr bool flip { t1_is_N && !t2_is_N };

			std::string t1_str {}, t2_str {};
			if constexpr ( t1_is_N )
			{
				t1_str = std::visit( L1::NVisitor, t1_n );
			}
			else
			{
				t1_str = std::visit( L1::xVisitor, t1_n )
			};
			if constexpr ( t2_is_N )
			{
				t2_str = std::visit( L1::NVisitor, t2_n );
			}
			else
			{
				t2_str = std::visit( L1::xVisitor, t2_n )
			};

			std::string cmp_line {};
			if constexpr ( flip ) // flip, put N (t1) on the left in generated instr
			{
				cmp_line = std::string { L1::Instr::CMPQ } + t1_str + "," + t2_str + "\n"
			};
			else
			{
				cmp_line = std::string { L1::Instr::CMPQ } + t2_str + "," + t1_str + "\n"
			}

			std::string_view j_instr {};
			switch ( this->cmptag )
			{
			case L1::CmpTag::LT:
				if constexpr ( flip ) { j_instr = L1::Instr::JG };
				else { j_instr = L1::Instr::JL; }
				break;
			case L1::CmpTag::LEQ:
				if constexpr ( flip ) { j_instr = L1::Instr::JGE };
				else { j_instr = L1::Instr::JLE; }
				break;
			case L1::CmpTag::EQ: // not impacted by direction flip
				j_instr = L1::Instr::JE;
				break;
			default:
				assert( false );
				break;
			}
			const std::string j_line { std::string { j_instr } + this->label_sv } + "\n";

			return cmp_line + j_line;
		}

	};


}

std::string
L1::
iVisitor::operator()( const L1::iCmpAssignNode &i_cmp_assign_n )
{
	constexpr bool use_low { true };
	// partially traverse
	const L1::CmpTag cmptag { std::visit( L1::cmpViewer, i_cmp_assign_n.cmp_n ) };
	const L1::wNode &w_n { i_cmp_assign_n.w_n };
	const std::string
		w_str     { std::visit( L1::wVisitor{}, w_n ) },
		w_low_str { std::visit( L1::wVisitor{ use_low }, w_n ) };

	// call helper visitor
	return std::visit(
		L1::cmpVisitor{ cmptag, w_str, w_low_str },
		i_cmp_assign_n.t1_n,
		i_cmp_assign_n.t2_n
	);
}

std::string
L1::
iVisitor::operator()( const L1::iCJumpNode &i_cjump_n )
{
	// partially traverse
	const std::string label_str { L1::labelVisitor{}( i_cjump_n.label_n ) };
	const L1::CmpTag cmptag { std::visit( L1::cmpViewer, i_cjump_n.cmp_n ) };

	// call helper visitor
	return std::visit(
		L1::cjumpVisitor{ cmptag, label_str },
		i_cmp_assign_n.t1_n,
		i_cmp_assign_n.t2_n
	);
}

std::string
L1::
iVisitor::operator()( const L1::iAssignNode &i_assign_n )
{
	// w <- s
	// rax <- 1
	//
	// movq $1,%rax
	// movq s,w
	//

	constexpr bool ismem { true };

	const std::string s_str { std::visit( L1::sVisitor{ ismem }, i_assign_n.s_n ) };	
	const std::string w_str { std::visit( L1::wVisitor{}, i_assign_n.w_n ) };

	return std::string { L1::Instr::MOVQ } + s_str + "," + w_str + "\n";

}

std::string
L1::
iVisitor::operator()( const L1::iLoadNode &i_load_n )
{
	// w   <- mem x M
	// rdi <- mem rsp 8
	//
	// movq 8(%rsp),%rdi
	// movq M(x),w

	// MNode is not variant
	const std::string M_str { L1::MVisitor{}( i_load_n.M_n ) };
	const std::string x_str { std::visit( L1::xVisitor{}, i_load_n.x_n ) };
	const std::string w_str { std::visit( L1::wVisitor{}, i_load_n.w_n ) };

	return std::string { L1::Instr::MOVQ } + M_str + "(" + x_str + ")," + w_str + "\n";

}

std::string
L1::
iVisitor::operator()( const L1::iStoreNode &i_store_n )
{
	// mem x   M <- s
	// mem rsp 0 <- rdi
	//
	// movq %rdi,0(%rsp)
	// movq s,M(x)

	constexpr bool ismem { true };

	const std::string s_str { std::visit( L1::sVisitor{ ismem }, i_store_n.s_n ) };
	const std::string M_str { L1::MVisitor{}( i_load_n.M_n ) };
	const std::string x_str { std::visit( L1::xVisitor, i_load_n.x_n ) };

	return std::string { L1::Instr::MOVQ } + s_str + ",",  M_str + "(" + x_str + ")\n";
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

	const std::string
		aop_str { std::visit( L1::aopVisitor{}, i_aop_n.aop_n ) },
		t_str { std::visit( L1::tVisitor{}, i_aop_n.t_n ) },
		w_str{ std::visit( L1::wVisitor{}, i_aop_n.w_n ) };

	return aop_str + t_str + "," + w_str + "\n";
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

	const std::string
		sop_str { std::visit( L1::sopVisitor{}, i_sx_n.sop_n ) },
		sx_str  { std::visit( L1::sxVisitor{ use_low }, i_sx_n.sx_n ) },
		w_str   { std::visit( L1::wVisitor{}, i_sx_n.w_n ) };

	return sop_str + sx_str + "," + w_str + "\n";

}

std::string
L1::
iVisitor::operator()( const L1::iSOpNode &i_sop_n )
{
	// w sop N
	// rdi >>= 3
	//
	// sarq $3,%rdi
	// sop N,w
	//

	const std::string
		sop_str { std::visit( L1::sopVisitor{}, i_sop_n.sop_n ) },
		N_str   { std::visit( L1::NVisitor{}, i_sop_n.N_n ) },
		w_str   { std::visit( L1::wVisitor{}, i_sop_n.w_n ) };

	return sop_str + N_str + "," + w_str + "\n";

}

std::string
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

	const std::string
		t_str { std::visit( L1::tVisitor{}, i_add_store_n.t_n ) },
		M_str { std::visit( L1::MVisitor{}, i_add_store_n.M_n ) },
		x_str { std::visit( L1::xVisitor{}, i_add_store_n.x_n ) };	

	return std::string { L1::Instr::ADDQ } + t_str + "," + M_str + "(" + x_str + ")\n";
}

std::string
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

	const std::string
		t_str { std::visit( L1::tVisitor{}, i_sub_store_n.t_n ) },
		M_str { std::visit( L1::MVisitor{}, i_sub_store_n.M_n ) },	
		x_str { std::visit( L1::xVisitor{}, i_sub_store_n.x_n ) };

	return std::string { L1::Instr::SUBQ } + t_str + "," + M_str + "(" + x_str + ")\n";
}

std::string
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

	const std::string
		M_str { std::visit( L1::MVisitor{}, i_load_sub_n.M_n ) },
		x_str { std::visit( L1::xVisitor{}, i_load_sub_n.x_n ) },	
		w_str { std::visit( L1::wVisitor{}, i_load_sub_n.w_n ) };

	return std::string { L1::Instr::SUBQ } + M_str + "(" + x_str + ")," + w_str + "\n";
}

std::string
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


	const std::string
		M_str { std::visit( L1::MVisitor{}, i_load_add_n.M_n ) },
		x_str { std::visit( L1::xVisitor{}, i_load_add_n.x_n ) },	
		w_str { std::visit( L1::wVisitor{}  i_load_add_n.w_n ) };

	return std::string{ L1::Instr::ADDQ } + M_str + "(" + x_str + ")," + w_str + "\n";

}

std::string
L1::
iVisitor::operator()( const L1::iIncrNode &i_incr_n )
{
	const std::string w_str { std::visit( L1::wVisitor{}, i_incr_n.w_n ) };

	return std::string{ L1::Instr::ADDQ } + "$1," + w_str + "\n";
}

std::string
L1::
iVisitor::operator()( const L1::iDecrNode &i_decr_n )
{
	const std::string w_str { std::visit( L1::wVisitor{}, i_decr_n.w_n ) };

	return std::string{ L1::Instr::SUBQ } + "$1," + w_str + "\n";
}

std::string
L1::
iVisitor::operator()( const L1::iLabelNode &i_label_n )
{
	const std::string label_str { L1::labelVisitor{}( i_label_n.label_n ) };

	return label_str + ":\n";
}

std::string
L1::
iVisitor::operator()( const L1::iGotoNode &i_goto_n )
{
	const std::string label_str { L1::labelVisitor{}( i_goto_n.label_n ) };

	return std::string{ L1::Instr::JMP } + label_str + "\n";
}

std::string
L1::
iVisitor::operator()( const L1::iReturnNode &i_return_n )
{
	return std::string { L1::Instr::RETQ } + "\n";
}

std::string
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

	const std::string
		w1_str { std::visit( L1::wVisitor{}, i_lea_n.w1_n ) },
		w2_str { std::visit( L1::wVisitor{}, i_lea_n.w2_n ) },
		w3_str { std::visit( L1::wVisitor{}, i_lea_n.w3_n ) };
		E_str { L1::EVisitor{}( i_lea_n.E_n ) };

	return std::string{ L1::Instr::LEA } + "(" 
		+ w2_str + "," + w3_str + "," + E_str + ")," + w1_str + "\n";

}

/*

		std::string operator()( const L1::iCJumpNode & );
		std::string operator()( const L1::iCallUNode & );
		std::string operator()( const L1::iCallPrintNode & );
		std::string operator()( const L1::iCallInputNode & );
		std::string operator()( const L1::iCallAllocateNode & );
		std::string operator()( const L1::iCallTupleErrorNode & );
		std::string operator()( const L1::iCallTensorErrorNode & );
*/

