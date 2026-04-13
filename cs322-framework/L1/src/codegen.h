
#ifndef L1_CODEGEN_H
#define L1_CODEGEN_H

#include "ast.h"
#include <string_view>
#include <iostream>
#include <memory>
#include <variant>
#include <string>

namespace L1
{

	namespace Instr
	{
		static constexpr std::string_view
			CALLQ { "\tcallq " }, RETQ { "\tretq " },
			PUSHQ { "\tpushq " }, POPQ { "\tpopq " },
			MOVQ { "\tmovq " }, MOVZBQ { "\tmovzbq " },
			ADDQ { "\taddq " }, SUBQ { "\tsubq " }, IMULQ { "\timulq " }, ANDQ { "\tandq " },
			SALQ { "\tsalq " }, SARQ { "\tsarq " },
			CMPQ { "\tcmpq " },
			SETE { "\tsete " }, SETL { "\tsetl " }, SETG { "\tsetg " }, SETLE { "\tsetle " }, SETGE { "\tsetge " },
			JMP { "\tjmp " },
			JE { "\tje " }, JL { "\tjl " }, JG { "\tjg " }, JLE { "\tjle " }, JGE { "\tjge " },
			LEA { "\tlea " }
			;
	}

	// when brace-initialized with a list of function objects,
	// `Handlers...` will be deduced to the function types 
	//
	template< typename ... Handlers >
	struct NodeVisitor : Handlers...
	{
		using Handlers::operator()...;
	};

	struct Visitor
	{
	};

	// if the target node type is a struct, then handle it directly
	struct nameVisitor: Visitor
	{
		std::string operator()( const L1::nameNode &name_n ) { return name_n.val; }
	};

	struct labelVisitor: Visitor
	{
		bool ismem { false };

		labelVisitor( std::ostream &os_, const bool ismem_ ): os { os_ }, ismem { ismem_ } {}

		std::string operator()( const L1::labelNode &label_n )
		{
			std::string sbuf {}; sbuf.reserve( 16 ); 
			if { this->ismem } { sbuf += "$" };
			sbuf += "_";
			sbuf += nameVisitor{}( label_n.name_n );
			return std::move( sbuf );
		}
	};

	// function name
	struct lVisitor: Visitor
	{
		std::string operator()( const L1::lNode &l_n )
		{
			std::string sbuf {}; sbuf.reserve( 16 );
			sbuf += "_";
			sbuf += nameVisitor{}( l_n.name_n );
			return std::move( sbuf );
		}
	};

	// if the target node type is a variant, then overload for alternatives
	struct NVisitor: Visitor
	{
		std::string operator()( const L1::_0Node &_0_n )    { return "$0"; }
		std::string operator()( const L1::NNZNode &N_nz_n ) { return std::string { "$" } + N_nz_n.val; }
	};

	struct MVisitor: Visitor
	{
		std::string operator()( const L1::MNode &M_n ) { return std::to_string( M_n.val ); }
	};

	struct FVisitor: Visitor
	{
		template< typename F > requires L1::IsKwNode< F >
		std::string_view operator()( const F &F_n ) { return F::kw; }
	}

	struct EVisitor: Visitor
	{
		template< typename E > requires L1::IsKwNode< E >
		std::string_view operator()( const E &E_n ) { return E::kw; }
	}

	struct RegVisitor: Visitor
	{
		template< typename RegNode > requires L1::IsKWNode< RegNode >
		std::string operator()( const RegNode &reg_n ) { return std::string { "%%" } << RegNode::kw; }
	};

	struct LowRegVisitor: Visitor
	{
		template< typename T > static inline constexpr std::string_view low;
		// "an explicit template specialization can only declare a single entity"
		template<> inline constexpr std::string_view low< L1::RaxNode > { "al" };
		template<> inline constexpr std::string_view low< L1::RbxNode > { "bl" };
		template<> inline constexpr std::string_view low< L1::RcxNode > { "cl" };
		template<> inline constexpr std::string_view low< L1::RdxNode > { "dl" };
		template<> inline constexpr std::string_view low< L1::RdiNode > { "dil" };
		template<> inline constexpr std::string_view low< L1::RsiNode > { "sil" };
		template<> inline constexpr std::string_view low< L1::RbpNode > { "bpl" };
		template<> inline constexpr std::string_view low< L1::RspNode > { "spl" };
		template<> inline constexpr std::string_view low< L1::R8Node >  { "r8b" };
		template<> inline constexpr std::string_view low< L1::R9Node >  { "r9b" };
		template<> inline constexpr std::string_view low< L1::R10Node > { "r10b" };
		template<> inline constexpr std::string_view low< L1::R11Node > { "r11b" };
		template<> inline constexpr std::string_view low< L1::R12Node > { "r12b" };
		template<> inline constexpr std::string_view low< L1::R13Node > { "r13b" };
		template<> inline constexpr std::string_view low< L1::R14Node > { "r14b" };
		template<> inline constexpr std::string_view low< L1::R15Node > { "r15b" };	

		template< typename RegNode > requires L1::IsKWNode< RegNode >
		std::string operator()( const RegNode &reg_n ) { return std::string { "%%" } + LowRegVisitor::low< RegNode >; }
	};

	struct sxVisitor: Visitor
	{
		bool use_low;

		sxVisitor( std::ostream &os_, bool use_low_ ): os { os }, use_low { use_low_ } {}

		std::string operator()( const L1::RcxNode &rcx_n )
		{
			if ( this->use_low ) { return RegVisitor{}( rcx_n ); }
			else { return LowRegVisitor{}( rcx_n ) };
		}
	};

	struct aVisitor: Visitor
	{
		bool use_low;

		aVisitor( bool use_low_ ): use_low { use_low_ } {}

		// if argument is still a variant, it must be properly dispatched to the downstream visitor
		std::string operator()( const L1::sxNode &sx_n ) { return std::visit( sxVisitor{ use_low }, sx_n ); }

		template< typename RegNode > requires L1::IsKWNode< RegNode >
		std::string operator()( const RegNode &reg_n )
		{
			if ( this->use_low ) { return LowRegVisitor{}( reg_n ) };
			else { return RegVisitor{}( reg_n ); }
		}
	};

	// only w regs and those lower in the AST are possibly used in low form
	// by cmp and shift
	struct wVisitor: Visitor
	{
		bool use_low { false };

		wVisitor( bool use_low_ ): use_low { use_low_ } {}

		std::string operator()( const L1::aNode &a_n ) { return std::visit( aVisitor{ this->use_low }, a_n ); }

		template< typename RegNode > requires L1::IsKWNode< RegNode >
		std::string operator()( const RegNode &reg_n )
		{
			if ( this->use_low ) { return LowRegVisitor{}( reg_n ) };
			else { return RegVisitor{}( reg_n ); }
		}
	};

	struct xVisitor: Visitor
	{
		std::string operator()( const L1::wNode &w_n ) { return std::visit( wVisitor{}, w_n ); }
		std::string operator()( const L1::RspNode &rsp_n ) { return RegVisitor{}( rsp_n ); }
	};

	struct uVisitor: Visitor
	{
		std::string operator()( const L1::wNode &w_n ) { return std::visit( wVisitor{}, w_n ); }
		std::string operator()( const L1::lNode &l_n ) { return lVisitor{}( l_n ); }
	};

	struct tVisitor: Visitor
	{
		std::string operator()( const L1::xNode &x_n ) { return std::visit( xVisitor{}, x_n ); }
		std::string operator()( const L1::NNode &N_n ) { return std::visit( NVisitor{}, N_n ); }
	};

	struct sVisitor: Visitor
	{
		bool ismem { false }; // only needed for label name ( labelNode )

		sVisitor( const bool ismem_ ): ismem { ismem_ } {}

		std::string operator()( const L1::tNode &t_n ) { return std::visit( tVisitor{}, t_n ); }
		std::string operator()( const L1::labelNode &label_n )
		{
			return std::visit( labelVisitor{ this->ismem }, label_n );
		}
		std::string operator()( const L1::lNode &l_n ) { return std::visit( lVisitor{}, l_n ); }
	};

	struct iVisitor: Visitor
	{	
		std::string operator()( const L1::iCmpAssignNode & );
		std::string operator()( const L1::iCJumpNode & );

		std::string operator()( const L1::iAssignNode & );
		std::string operator()( const L1::iLoadNode & );
		std::string operator()( const L1::iStoreNode & );
		std::string operator()( const L1::iAOpNode & );
		std::string operator()( const L1::iSxNode & );
		std::string operator()( const L1::iSOpNode & );
		std::string operator()( const L1::iAddStoreNode & );
		std::string operator()( const L1::iSubStoreNode & );
		std::string operator()( const L1::iLoadAddNode & );
		std::string operator()( const L1::iLoadSubNode & );

		std::string operator()( const L1::iIncrNode & );
		std::string operator()( const L1::iDecrNode & );

		std::string operator()( const L1::iLabelNode & );
		std::string operator()( const L1::iGotoNode & );
		std::string operator()( const L1::iReturnNode & );

		std::string operator()( const L1::iCallUNode & );
		std::string operator()( const L1::iCallPrintNode & );
		std::string operator()( const L1::iCallInputNode & );
		std::string operator()( const L1::iCallAllocateNode & );
		std::string operator()( const L1::iCallTupleErrorNode & );
		std::string operator()( const L1::iCallTensorErrorNode & );

		std::string operator()( const L1::iLEANode & );
	};


}

#endif

