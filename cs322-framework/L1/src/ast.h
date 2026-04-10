
#ifndef L1_AST_H
#define L1_AST_H

#include <string_view>
#include <array>
#include <vector>
#include <iostream>
#include <variant>

namespace L1
{
	/*
	 * Reserved words
	 */
	namespace RW
	{
		static constexpr std::string_view
			LPAR { "(" },
			RPAR { ")" },
			AT    { "@" },
			COLON { ":" },

			ADD { "+" },
			SUB { "-" },
			INCR { "++" },
			DECR { "--" },
			ASSIGN  { "<-" },
			ADD_EQ  { "+=" },
			SUB_EQ  { "-=" },
			MUL_EQ  { "*=" },
			BAND_EQ { "&=" },
			LSH_EQ  { "<<=" },
			RSH_EQ  { ">>=" },
			LT      { "<" },
			LEQ     { "<=" },
			EQ      { "=" },

			MEM { "mem" },

			CJUMP  { "cjump" },
			GOTO   { "goto" },
			CALL   { "call" },
			RETURN { "return" },

			PRINT { "print" },
			INPUT { "input" },
			ALLOCATE { "allocate" },
			TUPLE_ERROR { "tuple-error" },
			TENSOR_ERROR { "tensor-error" },

			RAX { "rax" },
			RBX { "rbx" },
			RCX { "rcx" },
			RDX { "rdx" },
			RDI { "rdi" },
			RSI { "rsi" },
			RBP { "rbp" },
			RSP { "rsp" },
			R8  { "r8" },
			R9  { "r9" },
			R10 { "r10" },
			R11 { "r11" },
			R12 { "r12" },
			R13 { "r13" },
			R14 { "r14" },
			R15 { "r15" }
			;

		static constexpr int LIBFUNC_CNT { 5 };
		static constexpr std::array< std::string_view, LIBFUNC_CNT >
			LIBFUNCS 
		{
			RW::PRINT, RW::INPUT, RW::ALLOCATE,
			RW::TUPLE_ERROR, RW::TENSOR_ERROR,
		};

		static constexpr int W_REG_CNT { 9 };
		static constexpr std::array< std::string_view, W_REG_CNT >
			W_REGS
		{
			RW::RAX, RW::RBX, RW::RBP,
			RW::R10, RW::R11, RW::R12, RW::R13, RW::R14, RW::R15,
		};

		static constexpr int A_REG_CNT { 5 };
		static constexpr std::array< std::string_view, A_REG_CNT >
			A_REGS
		{
			RW::RDI, RW::RSI, RW::RDX, RW::R8, RW::R9,
		};
	}

	template< typename ... Handlers >
	struct NodeVisitor : Handlers...
	{
		using Handlers::operator()...;
	};

	struct LParNode {};
	struct RParNode {};
	struct AtNode {};
	struct ColonNode {};

	struct AddNode {};
	struct SubNode {};
	struct IncrNode {};
	struct DecrNode {};
	struct AssignNode {};
	struct AddEqNode {};
	struct SubEqNode {};
	struct MulEqNode {};
	struct BAndEqNode {};
	struct LShEqNode {};
	struct RShEqNode {};
	struct LtNode {};
	struct LEqNode {};
	struct EqNode {};

	struct MemNode {};
	struct CJumpNode {};
	struct GotoNode {};
	struct CallNode {};
	struct ReturnNode {};

	struct PrintNode {};
	struct InputNode {};
	struct AllocateNode {};
	struct TupleErrorNode {};
	struct TensorErrorNode {};

	struct RaxNode {};
	struct RbxNode {};
	struct RcxNode {};
	struct RdxNode {};
	struct RdiNode {};
	struct RsiNode {};
	struct RbpNode {};
	struct RspNode {};	
	struct R8Node  {};
	struct R9Node  {};
	struct R10Node {};
	struct R11Node {};
	struct R12Node {};
	struct R13Node {};
	struct R14Node {};
	struct R15Node {};

	struct _0Node {};
	struct _1Node {};
	struct _2Node {};
	struct _3Node {};
	struct _4Node {};
	struct _8Node {};

	struct NIntNode { std::string_view val; };

	struct nameNode { std::string_view val; };
	struct labelNode { ColonNode colon_n; nameNode name_n; };
	struct lNode { AtNode at_n; nameNode name_n; };

	using NNode = std::variant< _0Node, NIntNode >;
	struct MNode { std::string_view val; };
	using FNode = std::variant< _1Node, _3Node, _4Node >;
	using ENode = std::variant< _1Node, _2Node, _4Node, _8Node >;

	using cmpNode = std::variant< LtNode, LEqNode, EqNode >;
	using sopNode = std::variant< LShEqNode, RShEqNode >;
	using aopNode = std::variant< AddEqNode, SubEqNode, MulEqNode, BAndEqNode >;

	using sxNode = std::variant< RcxNode >;
	using aNode = std::variant< RdiNode, RsiNode, sxNode, RdxNode, R8Node, R9Node >;
	using wNode = std::variant< aNode, RaxNode, RbxNode, R10Node, R11Node, R12Node, R13Node, R14Node, R15Node >;

	using xNode = std::variant< wNode, RspNode >;
	using uNode = std::variant< wNode, lNode >;
	using tNode = std::variant< xNode, NNode >;
	using sNode = std::variant< tNode, labelNode, lNode >;

	// w <- s
	struct iAssignNode 
	{
		wNode w_n; AssignNode assign_n; sNode s_n;
	};
	// w <- mem x M
	struct iLoadNode
	{
		wNode w_n;
		AssignNode assign_n;
		MemNode mem_n; xNode x_n; MNode M_n;
	};
	// mem x M <- s
	struct iStoreNode
	{
		MemNode mem_n; xNode x_n; MNode M_n;
		AssignNode assign_n;
		sNode s_n;
	};
	// w aop t
	struct iAOpNode
	{
		wNode w_n; aopNode aop_n; tNode t_n;
	};
	// w sop sx
	struct iSxNode
	{
		wNode w_n; sopNode sop_n; sxNode sx_n;
	};
	// w sop N
	struct iSOpNode
	{
		wNode w_n; sopNode sop_n; NNode N_n;
	};
	// mem x M += t
	struct iAddStoreNode
	{
		MemNode mem_n;	xNode x_n; MNode M_n;
		AddEqNode addeq_n;
		tNode t_n;
	};
	// mem x M -= t
	struct iSubStoreNode
	{
		MemNode mem_n; xNode x_n; MNode M_n;
		SubEqNode subeq_n;
		tNode t_n;
	};
	// w += mem x M
	struct iLoadAddNode	
	{
		wNode w_n;
		AddEqNode addeq_n;
		MemNode mem_n; xNode x_n; MNode M_n;
	};

	// w -= mem x M
	struct iLoadSubNode
	{
		wNode w_n;
		SubEqNode subeq_n;
		MemNode mem_n; xNode x_n; MNode M_n;
	};
	// w <- t cmp t
	struct iCmpAssignNode
	{
		wNode w_n;
		AssignNode assign_n;
		tNode t1_n; cmpNode cmp_n; tNode t2_n;
	};
	// cjump t cmp t label
	struct iCJumpNode
	{
		CJumpNode cjump_n;
		tNode t1_n; cmpNode cmp_n; tNode t2_n;
		labelNode label_n;
	};
	// label
	struct iLabelNode
	{
		labelNode label_n;
	};
	// goto label
	struct iGotoNode
	{
		GotoNode goto_n; labelNode label_n;
	};
	// return
	struct iReturnNode
	{
		ReturnNode return_n;
	};
	// call u N
	struct iCallUNode
	{
		CallNode call_n; uNode u_n; NNode N_n;
	};
	// call print 1
	struct iCallPrintNode
	{
		CallNode call_n; PrintNode print_n; _1Node _1_n;
	};
	// call input 0
	struct iCallInputNode
	{
		CallNode call_n; InputNode input_n; _0Node _0_n;
	};
	// call allocate 2
	struct iCallAllocateNode
	{
		CallNode call_n; AllocateNode allocate_n; _2Node _2_n;
	};
	// call tuple-error 3
	struct iCallTupleErrorNode
	{
		CallNode call_n; TupleErrorNode tuple_error_n; _3Node _3_n;
	};
	// call tensor-error F
	struct iCallTensorErrorNode
	{
		CallNode call_n; TensorErrorNode tensor_error_n; FNode F_n;
	};
	// w ++
	struct iIncrNode
	{
		wNode w_n; IncrNode incr_n;
	};
	// w --
	struct iDecrNode
	{
		wNode w_n; DecrNode decr_n;
	};
	// w @ w w E
	struct iLEANode
	{
		wNode w1_n;
	 	AtNode at_n; 
		wNode w2_n; wNode w3_n; ENode E_n;
	};

}

#endif

