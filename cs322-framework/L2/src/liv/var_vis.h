
#ifndef L2_LIV_VARVIS_H
#define L2_LIV_VARVIS_H

#include "ints.h"
#include "../svutil.h"
#include "../ast.h"
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace L2
{

	namespace Liv
	{

		struct GPRId
		{
			//
			// use these compile-time expanded values in lieu of a hashtable
			// for faster runtime access
			//
			template < typename Node > static inline constexpr var_id_t val { VAR_ID_INVAL };
			template<> inline constexpr var_id_t val< RaxNode > { 1 };
			template<> inline constexpr var_id_t val< RbxNode > { 2 };
			template<> inline constexpr var_id_t val< RcxNode > { 3 };
			template<> inline constexpr var_id_t val< RdxNode > { 4 };
			template<> inline constexpr var_id_t val< RdiNode > { 5 };
			template<> inline constexpr var_id_t val< RsiNode > { 6 };
			template<> inline constexpr var_id_t val< RbpNode > { 7 };
			// rsp does not participate in liveness
			template<> inline constexpr var_id_t val< R8Node >  { 8 };	
			template<> inline constexpr var_id_t val< R9Node >  { 9 };	
			template<> inline constexpr var_id_t val< R10Node > { 10 };	
			template<> inline constexpr var_id_t val< R11Node > { 11 };	
			template<> inline constexpr var_id_t val< R12Node > { 12 };	
			template<> inline constexpr var_id_t val< R13Node > { 13 };	
			template<> inline constexpr var_id_t val< R14Node > { 14 };	
			template<> inline constexpr var_id_t val< R15Node > { 15 };	

			/*
			static inline const std::unordered_map< std::string, var_id_t >
				GPR_ID_TBL
			{
				{ std::string { KW::RAX }, 1, },
				{ std::string { KW::RBX }, 2, },
				{ std::string { KW::RCX }, 3, },
				{ std::string { KW::RDX }, 4, },

				{ std::string { KW::RDI }, 5, },
				{ std::string { KW::RSI }, 6, },
				{ std::string { KW::RBP }, 7, },

				{ std::string { KW::R8  }, 8,  },
				{ std::string { KW::R9  }, 9,  },
				{ std::string { KW::R10 }, 10, },
				{ std::string { KW::R11 }, 11, },
				{ std::string { KW::R12 }, 12, },
				{ std::string { KW::R13 }, 13, },
				{ std::string { KW::R14 }, 14, },
				{ std::string { KW::R15 }, 15, },
			};
			*/

			static inline constexpr std::array< std::string_view, 16 >
				ID_GPR_TBL
			{
				L2::EMPTYTOK,
				KW::RAX, KW::RBX, KW::RCX, KW::RDX,
				KW::RDI, KW::RSI, KW::RBP,
				KW::R8,  KW::R9,  KW::R10, KW::R11,
				KW::R12, KW::R13, KW::R14, KW::R15,
			};
		};

		template< typename Node >
		concept IsGPR = (
			( requires { typename Node::is_reg; } )
			&& !std::is_same_v< Node, RspNode >
		);

		//
		// visits nodes representing variables ( GPRs or named variables ) in a function
		// and maintains IDs for them in the order of appearance
		//
		struct VarVisitor
		{

			//
			// smallest id for named, non-GPR variables
			//
			static constexpr var_id_t BASE_VAR_ID { 16 };
			var_id_t next_var_id { BASE_VAR_ID };

			//
			// we don't initialize these tables with default mappings between GPRs and their IDs
			// because for the analysis of each new function, the visitor will:
			// - be cleared, in which case the default mappings will be lost;
			// - or be reinstantiated, in which case the overhead of instantiating default mappings
			//   is incurred again.
			// 
			std::vector< std::string > id_var_tbl {};
			std::unordered_map<
				std::string, var_id_t,
				// enable indexing by std::string_view
				SVUtil::TransparentHash, SVUtil::TransparentEq
			> var_id_tbl {};

			VarVisitor( void );

			var_id_t new_var_id( void );

			void display( void ) const;

			//
			// given ID, retrieve variable name.
			// since this->id_var_tbl only stores named variables,
			// if the ID is that of a named var rather than GPR,
			// this function performs translation from logical ID
			// to physical ID.
			//
			// bypassing this function and directly indexing 
			// this->id_var_tbl will likely read garbage.
			//
			std::string_view var_name_by_id( const var_id_t var_id ) const;

			//
			// retrive variable name by ID. if the variable is unknown,
			// return L2::VAR_ID_INVAL.
			//
			var_id_t var_id_by_name( const std::string_view var_name ) const;

			//
			// retrieve variable ID of GPR
			//
			template< typename GPRNode > requires IsGPR< GPRNode >
			var_id_t operator()( const GPRNode &gpr_n )
			{
				//std::printf( "VarVisitor: visiting GPR node\n" );
				return GPRId::val< GPRNode >;
			}

			//
			// retrieve variable ID of named variable, creating one if necessary
			//
			var_id_t operator()( const nameNode &name_n );

			//
			// retrieve variable ID of the name wrapped in a varNode,
			// creating one if necessary
			//
			var_id_t operator()( const varNode &var_n );

			//
			// recurse on variants ( such as aNode encountered when vising wNode )
			//
			template< typename VariantNode > requires ::is_variant_v< VariantNode >
			var_id_t operator()( const VariantNode &variant_n )
			{
				return std::visit( *this, variant_n );
			}

			//
			// default, for nodes that don't express variables
			// ( for example, NNode may be encountered when descending into tNode )
			// return invalid ID and don't register the tables
			//

			template< typename Node >
			var_id_t operator()( const Node &n )
			{
				//std::printf( "VarVisitor: visiting non-variable node\n" );
				return VAR_ID_INVAL;
			}

		};

	}
}

#endif

