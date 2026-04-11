
#ifndef L1_PARSER_H
#define L1_PARSER_H

#include "ast.h"
#include <cstdint>
#include <string_view>
#include <array>
#include <vector>
#include <iostream>
#include <variant>
#include <optional>

namespace L1
{
 	/* 
 	 * @brief
 	 *   Returns whether a character can be used in an identifier (name).
	 */
	bool
	isident( const char c );

 	/* 
 	 * @brief
 	 *   Returns whether a character is a parenthesis.
	 */
	bool
	isparen( const char c );

	template< typename T > struct node_tag {};

	class Parser
	{
	private:
		// In-memory buffer for L1 source code file being lexed;
		// null-terminators will be inserted after tokens
		std::string srcbuf {};
		// Token base indices in buffer with null-terminated tokens
		std::vector< int > tok_base_idxs {};
		std::size_t tok_idx {};
	
		std::optional< L1::pNode > ast {};

	public:
		Parser( void );

		void
		lex( std::istream &src_is );

		void
		print_toks( void ) const;

		std::string_view
		gettok( void );

		// for variant nodes
		template< typename VariantNodeT >
			requires ::is_variant_v< VariantNodeT >
		std::optional< VariantNodeT > make_variant_node( void )
		{
			std::optional< VariantNodeT > result { std::nullopt };

			auto try_parse
			{
				[ & ]< typename NodeT >() -> bool
				{
					// descend into alternatives.
					// if make_node< NodeT > fails, it is assumed to 
					// restore token idx before returning
					if ( auto node = this->make_node< NodeT >() )
					{
						result = std::move( *node );
						return true;
					}
					return false;
				}
			};
			auto expand
			{
				[ & ]< typename ... Ts >( std::variant< Ts ... > * )
				{
					( try_parse.template operator()< Ts >() || ... );
				}
			};

			const std::size_t cur_idx { this->tok_idx };

			expand( ( VariantNodeT * ) nullptr );

			if ( result )
			{
				return std::move( result );
			}
			else
			{
				this->tok_idx = cur_idx;
				return std::nullopt;
			}
		}

		// for terminal kw nodes
		template< typename KWNodeT >
			requires L1::IsKWNode< KWNodeT >
		std::optional< KWNodeT > make_kw_node( void )
		{
			const std::size_t cur_idx { this->tok_idx };
			const std::string_view tok { this->gettok() };
			if ( tok == KWNodeT::kw ) // token matches kw 
			{
				return KWNodeT {};
			}
			else // match failed, restore idx
			{
				this->tok_idx = cur_idx;
				return std::nullopt;
			}
		};

		// for terminal identifier nodes with regex
		template< typename IdentNodeT >
			requires L1::IsIdentNode< IdentNodeT >
		std::optional< IdentNodeT > make_ident_node( void )
		{
			const std::size_t cur_idx { this->tok_idx };
			const std::string_view tok { this->gettok() };
			if ( std::regex_match( tok, IdentNodeT::re ) ) // token matches node regex
			{
				return IdentNodeT { .val = tok };
			}
			else // match failed, restore idx
			{
				this->tok_idx = cur_idx;
				return std::nullopt;
			}
		};

		// for most record (struct) nodes
		template< typename RecNodeT >
			requires L1::IsRecNode< RecNodeT >
		std::optional< RecNodeT > make_record_node( void )
		{
			// TODO
			const std::size_t cur_idx { this->tok_idx };

			//RecNodeT::fields_t

			this->tok_idx = cur_idx;
			return std::nullopt;
		};

		// dispatcher
		template< typename NodeT >
		std::optional< NodeT > make_node( void )
		{
			if constexpr ( ::is_variant_v< NodeT > )
			{
				return this->make_variant_node< NodeT >();
			}
			if constexpr ( L1::IsKWNode< NodeT > )
			{
				return this->make_kw_node< NodeT >();
			}
			if constexpr ( L1::IsIdentNode< NodeT > )
			{
				return this->make_ident_node< NodeT >();
			}
			/*
			if constexpr ( std::is_same_v< NodeT, L1::fNode > )
			{
				return this->make_f_node();
			}
			if constexpr ( std::is_same_v< NodeT, L1::pNode > )
			{
				return this->make_p_node();
			}
			*/
			if constexpr ( L1::IsRecNode< NodeT > )
			{
				return this->make_record_node< NodeT >();
			}
			return std::nullopt;
		}
	};

}

#endif

