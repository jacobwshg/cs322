
#ifndef L1_AST_H
#define L1_AST_H

#include <string_view>
#include <array>
#include <vector>
#include <iostream>
#include <variant>

namespace L1
{

	using sv_t = std::string_view;

	/*
	 * Reserved words
	 */
	namespace KW
	{
		static constexpr std::string_view
			LPAR { "(" },
			RPAR { ")" },
			AT    { "@" },
			COLON { ":" },

			OP_ADD     { "+" },
			OP_SUB     { "-" },
			OP_INCR    { "++" },
			OP_DECR    { "--" },
			OP_ASSIGN  { "<-" },
			OP_ADD_EQ  { "+=" },
			OP_SUB_EQ  { "-=" },
			OP_MUL_EQ  { "*=" },
			OP_BAND_EQ { "&=" },
			OP_LSH_EQ  { "<<=" },
			OP_RSH_EQ  { ">>=" },
			OP_LT      { "<" },
			OP_LEQ     { "<=" },
			OP_EQ      { "=" },

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
			KW::PRINT, KW::INPUT, KW::ALLOCATE,
			KW::TUPLE_ERROR, KW::TENSOR_ERROR,
		};

		static constexpr int W_REG_CNT { 9 };
		static constexpr std::array< std::string_view, W_REG_CNT >
			W_REGS
		{
			KW::RAX, KW::RBX, KW::RBP,
			KW::R10, KW::R11, KW::R12, KW::R13, KW::R14, KW::R15,
		};

		static constexpr int A_REG_CNT { 5 };
		static constexpr std::array< std::string_view, A_REG_CNT >
			A_REGS
		{
			KW::RDI, KW::RSI, KW::RDX, KW::R8, KW::R9,
		};
	}

	template< typename ... Handlers >
	struct NodeVisitor : Handlers...
	{
		using Handlers::operator()...;
	};

	struct LParNode  { static constexpr sv_t kw { KW::LPAR }; };
	struct RParNode  { static constexpr sv_t kw { KW::RPAR }; };
	struct AtNode    { static constexpr sv_t kw { KW::AT }; };
	struct ColonNode { static constexpr sv_t kw { KW::COLON }; };

	struct AddNode    { static constexpr sv_t kw { KW::OP_ADD }; };
	struct SubNode    { static constexpr sv_t kw { KW::OP_SUB }; };
	struct IncrNode   { static constexpr sv_t kw { KW::OP_INCR }; };
	struct DecrNode   { static constexpr sv_t kw { KW::OP_DECR }; };
	struct AssignNode { static constexpr sv_t kw { KW::OP_ASSIGN }; };
	struct AddEqNode  { static constexpr sv_t kw { KW::OP_ADD_EQ }; };
	struct SubEqNode  { static constexpr sv_t kw { KW::OP_SUB_EQ }; };
	struct MulEqNode  { static constexpr sv_t kw { KW::OP_MUL_EQ }; };
	struct BAndEqNode { static constexpr sv_t kw { KW::OP_BAND_EQ }; };
	struct LShEqNode  { static constexpr sv_t kw { KW::OP_LSH_EQ }; };
	struct RShEqNode  { static constexpr sv_t kw { KW::OP_RSH_EQ }; };
	struct LtNode     { static constexpr sv_t kw { KW::OP_LT }; };
	struct LEqNode    { static constexpr sv_t kw { KW::OP_LEQ }; };
	struct EqNode     { static constexpr sv_t kw { KW::OP_EQ }; };

	struct MemNode    { static constexpr sv_t kw { KW::MEM }; };
	struct CJumpNode  { static constexpr sv_t kw { KW::CJUMP }; };
	struct GotoNode   { static constexpr sv_t kw { KW::GOTO }; };
	struct CallNode   { static constexpr sv_t kw { KW::CALL }; };
	struct ReturnNode { static constexpr sv_t kw { KW::RETURN }; };

	struct PrintNode       { static constexpr sv_t kw { KW::PRINT }; };
	struct InputNode       { static constexpr sv_t kw { KW::INPUT }; };
	struct AllocateNode    { static constexpr sv_t kw { KW::ALLOCATE }; };
	struct TupleErrorNode  { static constexpr sv_t kw { KW::TUPLE_ERROR }; };
	struct TensorErrorNode { static constexpr sv_t kw { KW::TENSOR_ERROR }; };

	struct RaxNode { static constexpr sv_t kw { KW::RAX }; };
	struct RbxNode { static constexpr sv_t kw { KW::RBX }; };
	struct RcxNode { static constexpr sv_t kw { KW::RCX }; };
	struct RdxNode { static constexpr sv_t kw { KW::RDX }; };
	struct RdiNode { static constexpr sv_t kw { KW::RDI }; };
	struct RsiNode { static constexpr sv_t kw { KW::RSI }; };
	struct RbpNode { static constexpr sv_t kw { KW::RBP }; };
	struct RspNode { static constexpr sv_t kw { KW::RSP }; };	
	struct R8Node  { static constexpr sv_t kw { KW::R8  }; };
	struct R9Node  { static constexpr sv_t kw { KW::R9  }; };
	struct R10Node { static constexpr sv_t kw { KW::R10 }; };
	struct R11Node { static constexpr sv_t kw { KW::R11 }; };
	struct R12Node { static constexpr sv_t kw { KW::R12 }; };
	struct R13Node { static constexpr sv_t kw { KW::R13 }; };
	struct R14Node { static constexpr sv_t kw { KW::R14 }; };
	struct R15Node { static constexpr sv_t kw { KW::R15 }; };

	struct _0Node { static constexpr sv_t kw { "0" }; };
	struct _1Node { static constexpr sv_t kw { "1" }; };
	struct _2Node { static constexpr sv_t kw { "2" }; };
	struct _3Node { static constexpr sv_t kw { "3" }; };
	struct _4Node { static constexpr sv_t kw { "4" }; };
	struct _8Node { static constexpr sv_t kw { "8" }; };

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
		AddEqNode add_eq_n;
		tNode t_n;
	};
	// mem x M -= t
	struct iSubStoreNode
	{
		MemNode mem_n; xNode x_n; MNode M_n;
		SubEqNode sub_eq_n;
		tNode t_n;
	};
	// w += mem x M
	struct iLoadAddNode	
	{
		wNode w_n;
		AddEqNode add_eq_n;
		MemNode mem_n; xNode x_n; MNode M_n;
	};

	// w -= mem x M
	struct iLoadSubNode
	{
		wNode w_n;
		SubEqNode sub_eq_n;
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

	using iNode = std::variant<
		iAssignNode, iLoadNode, iStoreNode,
		iAOpNode, iSxNode, iSOpNode,
		iAddStoreNode, iSubStoreNode, iLoadAddNode, iLoadSubNode,
		iCmpAssignNode, iCJumpNode, iLabelNode, iGotoNode,
		iReturnNode,
		iCallUNode, iCallPrintNode, iCallAllocateNode, iCallTupleErrorNode, iCallTensorErrorNode,
		iIncrNode, iDecrNode, iLEANode
	>;

	struct fNode
	{
		LParNode lpar_n;

		lNode l_n;
		NNode N1_n; NNode N2_n;
		std::vector< iNode > i_ns;

		RParNode rpar_n;
	};

	struct pNode
	{
		LParNode lpar_n;

		lNode l_n;
		std::vector< fNode > f_ns;

		RParNode rpar_n;		
	};
}

#endif

