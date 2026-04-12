
#ifndef L1_CODEGEN_H
#define L1_CODEGEN_H

#include "ast.h"
#include <string_view>
#include <iostream>
#include <memory>
#include <variant>

namespace L1
{

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
		std::ostream &os;

		Visitor( std::ostream &os_ ): os { os_ } {}
		Visitor( std::shared_ptr< std::ostream > osp ): os { *osp } {}
	};

	// if the target node type is a struct, then handle it directly
	struct nameVisitor: Visitor
	{
		void operator()( const L1::nameNode &name_n ) { this->os << name_n.val; }
	};

	struct labelVisitor: Visitor
	{
		void operator()( const L1::labelNode &label_n )
		{
			this->os << "_";
			nameVisitor{ this->os }( label_n.name_n );
		}
	};

	struct lVisitor: Visitor
	{
		void operator()( const L1::lNode &l_n )
		{
			this->os << "_";
			nameVisitor{ this->os }( l_n.name_n );
		}
	};

	// if the target node type is a variant, then overload for alternatives
	struct NVisitor: Visitor
	{
		void operator()( const L1::_0Node &_0_n )    { this->os << "0"; }
		void operator()( const L1::NNZNode &N_nz_n ) { this->os << N_nz_n.val; }
	};

	struct RegVisitor: Visitor
	{
		template< typename RegNode > requires L1::IsKWNode< RegNode >
		void operator()( const RegNode &reg_n ) { this->os << "%" << RegNode::kw; }
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
		void operator()( const RegNode &reg_n ) { this->os << "%" << LowRegVisitor::low< RegNode >; }
	};

	struct sxVisitor: Visitor
	{
		void operator()( const L1::RcxNode &rcx_n ) { RegVisitor{ this->os }( rcx_n ); }
	};

	struct aVisitor: Visitor
	{
		// if argument is still a variant, it must be properly dispatched to the downstream visitor
		void operator()( const L1::sxNode &sx_n ) { std::visit( sxVisitor{ this->os }, sx_n ); }
		template< typename RegNode > requires L1::IsKWNode< RegNode >
		void operator()( const RegNode &reg_n ) { RegVisitor{ this->os }( reg_n ); }
	};

	struct wVisitor: Visitor
	{
		void operator()( const L1::aNode &a_n ) { std::visit( aVisitor{ this->os }, a_n ); }
		template< typename RegNode > requires L1::IsKWNode< RegNode >
		void operator()( const RegNode &reg_n ) { RegVisitor{ this->os }( reg_n ); }
	};

	struct xVisitor: Visitor
	{
		void operator()( const L1::wNode &w_n ) { std::visit( wVisitor{ this->os }, w_n ); }
		void operator()( const L1::RspNode &rsp_n ) { RegVisitor{ this->os }( rsp_n ); }
	};

	struct uVisitor: Visitor
	{
		void operator()( const L1::wNode &w_n ) { std::visit( wVisitor{ this->os }, w_n ); }
		void operator()( const L1::lNode &l_n ) { lVisitor{ this->os }( l_n ); }
	};

	struct tVisitor: Visitor
	{
		void operator()( const L1::xNode &x_n ) { std::visit( xVisitor{ this->os }, x_n ); }
		void operator()( const L1::NNode &N_n ) { std::visit( NVisitor{ this->os }, N_n ); }
	};

	struct iVisitor: Visitor
	{	
		void operator()( const L1::iCmpAssignNode & );
		void operator()( const L1::iAssignNode & );

		void operator()( const L1::iLoadNode & );
		void operator()( const L1::iStoreNode & );
		void operator()( const L1::iAOpNode & );
		void operator()( const L1::iSxNode & );
		void operator()( const L1::iSOpNode & );
		void operator()( const L1::iAddStoreNode & );
		void operator()( const L1::iSubStoreNode & );
		void operator()( const L1::iLoadAddNode & );
		void operator()( const L1::iLoadSubNode & );

		void operator()( const L1::iCJumpNode & );
		void operator()( const L1::iLabelNode & );
		void operator()( const L1::iGotoNode & );
		void operator()( const L1::iReturnNode & );
		void operator()( const L1::iCallUNode & );
		void operator()( const L1::iCallPrintNode & );
		void operator()( const L1::iCallInputNode & );
		void operator()( const L1::iCallAllocateNode & );
		void operator()( const L1::iCallTupleErrorNode & );
		void operator()( const L1::iCallTensorErrorNode & );

		void operator()( const L1::iIncrNode & );
		void operator()( const L1::iDecrNode & );
		void operator()( const L1::iLEANode & );
	};


}

#endif

