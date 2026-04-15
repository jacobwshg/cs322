
#ifndef L1_AST_H
#define L1_AST_H

#include <string_view>
#include <concepts>
#include <array>
#include <vector>
#include <iostream>
#include <variant>
#include <regex>
#include <tuple>

namespace
{

	template< typename T >
	using TypeTag = std::unique_ptr< T >;

	/*
	 * Two ways to check if a type is a std::variant< ... >
	 *
	 */
	template< typename T >
	struct is_variant: std::false_type
	{};
	// specialize `is_variant` for types that are `std::variant<...>`s of some other types
	template< typename ... Ts >
	struct is_variant< std::variant< Ts... > >: std::true_type
	{};
	template< typename T >
	inline constexpr bool is_variant_v = is_variant< T >::value;

	template< typename T >
	concept IsVariant = requires( T t )
	{
		[]< typename ... Ts >
		(
			std::variant< Ts... >
		) {}( t );
	};

	template< typename T >
	struct is_tuple: std::false_type
	{};
	template< typename ... Ts >
	struct is_tuple< std::tuple< Ts... > >: std::true_type
	{};
	template< typename T >
	inline constexpr bool is_tuple_v = is_tuple< T >::value;

	template< typename T >
	struct is_vector: std::false_type
	{};
	template< typename T, typename A >
	struct is_vector< std::vector< T, A > >: std::true_type
	{};
	template< typename T >
	inline constexpr bool is_vector_v = is_vector< T >::value;
}

namespace L1
{

	using sv_t = std::string_view;

	// whether a node represents a keyword
	template< typename NodeT >
	concept IsKWNode = requires
	{
		{ NodeT::kw } -> std::convertible_to< std::string_view >;
	};

	// whether a node represents a numeric constant
	template< typename NodeT >
	concept IsNumConstNode = requires
	{
		{ NodeT::val } -> std::convertible_to< long long >;
	};

	// whether a node represents an identifier token that matches a regular expression
	// ( namely, a `name` or nonzero `N` token )
	template< typename NodeT >
	concept IsIdentNode = requires
	{
		{ NodeT::re } -> std::convertible_to< std::regex >;
	};

	// whether a node is a record (struct with subnodes as members)
	template< typename NodeT >
	concept IsRecNode = requires
	{
		true == ::is_tuple_v< typename NodeT::fields_t >;
	};

	static constexpr std::string_view EMPTYTOK { "" };

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
			PERCENT { "%" },

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

			STACK_ARG { "stack-arg" },
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

	struct LParNode  { static constexpr sv_t kw { KW::LPAR }; };
	struct RParNode  { static constexpr sv_t kw { KW::RPAR }; };
	struct AtNode    { static constexpr sv_t kw { KW::AT }; };
	struct ColonNode { static constexpr sv_t kw { KW::COLON }; };
	struct PercentNode { static constexpr sv_t kw { KW::PERCENT }; };

	struct OpAssignNode { static constexpr sv_t kw { KW::OP_ASSIGN }; };
	struct OpAddNode    { static constexpr sv_t kw { KW::OP_ADD }; };
	struct OpSubNode    { static constexpr sv_t kw { KW::OP_SUB }; };
	struct OpIncrNode   { static constexpr sv_t kw { KW::OP_INCR }; };
	struct OpDecrNode   { static constexpr sv_t kw { KW::OP_DECR }; };
	struct OpAddEqNode  { static constexpr sv_t kw { KW::OP_ADD_EQ }; };
	struct OpSubEqNode  { static constexpr sv_t kw { KW::OP_SUB_EQ }; };
	struct OpMulEqNode  { static constexpr sv_t kw { KW::OP_MUL_EQ }; };
	struct OpBAndEqNode { static constexpr sv_t kw { KW::OP_BAND_EQ }; };
	struct OpLShEqNode  { static constexpr sv_t kw { KW::OP_LSH_EQ }; };
	struct OpRShEqNode  { static constexpr sv_t kw { KW::OP_RSH_EQ }; };
	struct OpLtNode     { static constexpr sv_t kw { KW::OP_LT }; };
	struct OpLEqNode    { static constexpr sv_t kw { KW::OP_LEQ }; };
	struct OpEqNode     { static constexpr sv_t kw { KW::OP_EQ }; };

	struct StackArgNode { static constexpr sv_t kw { KW::STACK_ARG }; };

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

	struct _0Node { static constexpr sv_t kw { "0" }; static constexpr long long val { 0LL }; };
	struct _1Node { static constexpr sv_t kw { "1" }; static constexpr long long val { 1LL }; };
	struct _2Node { static constexpr sv_t kw { "2" }; static constexpr long long val { 2LL }; };
	struct _3Node { static constexpr sv_t kw { "3" }; static constexpr long long val { 3LL }; };
	struct _4Node { static constexpr sv_t kw { "4" }; static constexpr long long val { 4LL }; };
	struct _8Node { static constexpr sv_t kw { "8" }; static constexpr long long val { 8LL }; };

	struct NNZNode
	{
		static inline const std::regex re { "[+-]?[1-9][0-9]*" }; 
		long long val;
	};

	struct nameNode
	{
		static inline const std::regex re { "[a-zA-Z_][a-zA-z_0-9]*" };
		std::string_view val; // outlived by actual token in Parser::srcbuf
	};
	struct varNode
	{
		using fields_t = std::tuple< PercentNode, nameNode >;
		PercentNode percent_n; nameNode name_n;
	}
	struct labelNode
	{
		using fields_t = std::tuple< ColonNode, nameNode >;
		ColonNode colon_n; nameNode name_n;
	};
	struct lNode
	{
		using fields_t = std::tuple< AtNode, nameNode >;
		AtNode at_n; nameNode name_n;
	};

	using NNode = std::variant< _0Node, NNZNode >;
	struct MNode { long long val; }; // multiples of 8 
	using FNode = std::variant< _1Node, _3Node, _4Node >;
	using ENode = std::variant< _1Node, _2Node, _4Node, _8Node >;

	using cmpNode = std::variant< OpLtNode, OpLEqNode, OpEqNode >;
	using sopNode = std::variant< OpLShEqNode, OpRShEqNode >;
	using aopNode = std::variant< OpAddEqNode, OpSubEqNode, OpMulEqNode, OpBAndEqNode >;

	using sxNode = std::variant< RcxNode, varNode >;
	using aNode = std::variant< RdiNode, RsiNode, sxNode, RdxNode, R8Node, R9Node >;
	using wNode = std::variant< aNode, RaxNode >;

	using xNode = std::variant< wNode, RspNode >;
	using uNode = std::variant< wNode, lNode >;
	using tNode = std::variant< xNode, NNode >;
	using sNode = std::variant< tNode, labelNode, lNode >;

	// w <- s
	struct iAssignNode 
	{
		using fields_t = std::tuple< wNode, OpAssignNode, sNode >;
		wNode w_n; OpAssignNode op_assign_n; sNode s_n;
	};
	// w <- mem x M
	struct iLoadNode
	{
		using fields_t = std::tuple< wNode, OpAssignNode, MemNode, xNode, MNode >;
		wNode w_n;
		OpAssignNode op_assign_n;
		MemNode mem_n; xNode x_n; MNode M_n;
	};
	// mem x M <- s
	struct iStoreNode
	{
		using fields_t = std::tuple< MemNode, xNode, MNode, OpAssignNode, sNode >;
		MemNode mem_n; xNode x_n; MNode M_n;
		OpAssignNode op_assign_n;
		sNode s_n;
	};
	// w <- stack-arg M
	struct iStackArgNode
	{
		using fields_t = std::tuple< wNode, OpAssignNode, StackArgNode, MNode >;
		wNode w_n;
		OpAssignNode op_assign_n;
		StackArgNode stack_arg_n; MNode M_n;
	};
	// w aop t
	struct iAOpNode
	{
		using fields_t = std::tuple< wNode, aopNode, tNode >;
		wNode w_n; aopNode aop_n; tNode t_n;
	};
	// w sop sx
	struct iSxNode
	{
		using fields_t = std::tuple< wNode, sopNode, sxNode >;
		wNode w_n; sopNode sop_n; sxNode sx_n;
	};
	// w sop N
	struct iSOpNode
	{
		using fields_t = std::tuple< wNode, sopNode, NNode >;
		wNode w_n; sopNode sop_n; NNode N_n;
	};
	// mem x M += t
	struct iAddStoreNode
	{
		using fields_t = std::tuple< MemNode, xNode, MNode, OpAddEqNode, tNode >;
		MemNode mem_n;	xNode x_n; MNode M_n;
		OpAddEqNode op_add_eq_n;
		tNode t_n;
	};
	// mem x M -= t
	struct iSubStoreNode
	{
		using fields_t = std::tuple< MemNode, xNode, MNode, OpSubEqNode, tNode >;
		MemNode mem_n; xNode x_n; MNode M_n;
		OpSubEqNode op_sub_eq_n;
		tNode t_n;
	};
	// w += mem x M
	struct iLoadAddNode	
	{
		using fields_t = std::tuple< wNode, OpAddEqNode, MemNode, xNode, MNode >;
		wNode w_n;
		OpAddEqNode op_add_eq_n;
		MemNode mem_n; xNode x_n; MNode M_n;
	};

	// w -= mem x M
	struct iLoadSubNode
	{
		using fields_t = std::tuple< wNode, OpSubEqNode, MemNode, xNode, MNode >;
		wNode w_n;
		OpSubEqNode op_sub_eq_n;
		MemNode mem_n; xNode x_n; MNode M_n;
	};
	// w <- t cmp t
	struct iCmpAssignNode
	{
		using fields_t = std::tuple< wNode, OpAssignNode, tNode, cmpNode, tNode >;
		wNode w_n;
		OpAssignNode op_assign_n;
		tNode t1_n; cmpNode cmp_n; tNode t2_n;
	};
	// cjump t cmp t label
	struct iCJumpNode
	{
		using fields_t = std::tuple< CJumpNode, tNode, cmpNode, tNode, labelNode >;
		CJumpNode cjump_n;
		tNode t1_n; cmpNode cmp_n; tNode t2_n;
		labelNode label_n;
	};
	// label
	struct iLabelNode
	{
		using fields_t = std::tuple< labelNode >;
		labelNode label_n;
	};
	// goto label
	struct iGotoNode
	{
		using fields_t = std::tuple< GotoNode, labelNode >;
		GotoNode goto_n; labelNode label_n;
	};
	// return
	struct iReturnNode
	{
		using fields_t = std::tuple< ReturnNode >;
		ReturnNode return_n;
	};
	// call u N
	struct iCallUNode
	{
		using fields_t = std::tuple< CallNode, uNode, NNode >;
		CallNode call_n; uNode u_n; NNode N_n;
	};
	// call print 1
	struct iCallPrintNode
	{
		using fields_t = std::tuple< CallNode, PrintNode, _1Node >;
		CallNode call_n; PrintNode print_n; _1Node _1_n;
	};
	// call input 0
	struct iCallInputNode
	{
		using fields_t = std::tuple< CallNode, InputNode, _0Node >;
		CallNode call_n; InputNode input_n; _0Node _0_n;
	};
	// call allocate 2
	struct iCallAllocateNode
	{
		using fields_t = std::tuple< CallNode, AllocateNode, _2Node >;
		CallNode call_n; AllocateNode allocate_n; _2Node _2_n;
	};
	// call tuple-error 3
	struct iCallTupleErrorNode
	{
		using fields_t = std::tuple< CallNode, TupleErrorNode, _3Node >;
		CallNode call_n; TupleErrorNode tuple_error_n; _3Node _3_n;
	};
	// call tensor-error F
	struct iCallTensorErrorNode
	{
		using fields_t = std::tuple< CallNode, TensorErrorNode, FNode >;
		CallNode call_n; TensorErrorNode tensor_error_n; FNode F_n;
	};
	// w ++
	struct iIncrNode
	{
		using fields_t = std::tuple< wNode, OpIncrNode >;
		wNode w_n; OpIncrNode op_incr_n;
	};
	// w --
	struct iDecrNode
	{
		using fields_t = std::tuple< wNode, OpDecrNode >;
		wNode w_n; OpDecrNode op_decr_n;
	};
	// w @ w w E
	struct iLEANode
	{
		using fields_t = std::tuple< wNode, AtNode, wNode, wNode, ENode >;
		wNode w1_n;
	 	AtNode at_n; 
		wNode w2_n; wNode w3_n; ENode E_n;
	};

	using iNode = std::variant<

		// `w <- s` could be a prefix of `w <- t cmp t`, and since make_variant_node() use short-circuit parsing,
		// we must try matching the longer instruction first to avoid always parsing as the shorter one
		//
		iCmpAssignNode,
		iAssignNode,

		iLoadNode, iStoreNode,

		iStackArgNode,

		iAOpNode, iSxNode, iSOpNode,
		iAddStoreNode, iSubStoreNode, iLoadAddNode, iLoadSubNode,

		iCJumpNode, iLabelNode, iGotoNode,
		iReturnNode,
		iCallUNode, iCallPrintNode, iCallInputNode, iCallAllocateNode, iCallTupleErrorNode, iCallTensorErrorNode,
		iIncrNode, iDecrNode, iLEANode
	>;

	struct fNode
	{
		using fields_t = std::tuple< LParNode, lNode, NNode, NNode, std::vector<iNode>, RParNode >;

		LParNode lpar_n;
		lNode l_n;
		NNode N_n;
		std::vector< iNode > i_ns;
		RParNode rpar_n;
	};

	struct pNode
	{
		using fields_t = std::tuple< LParNode, lNode, std::vector< fNode >, RParNode >;

		LParNode lpar_n;
		lNode l_n;
		std::vector< fNode > f_ns;
		RParNode rpar_n;		
	};
}

#endif

