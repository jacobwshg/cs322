
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

		// default
		template< typename NodeType >
		std::optional< NodeType > node_handler( void )
		{
			return std::nullopt;
		}

		template< L1::IsKWNode KWNodeType >
		std::optional< KWNodeType > node_handler( void )
		{
			const std::size_t cur_idx { this->tok_idx };
			const std::string_view tok { this->gettok() };
			return ( tok == KWNodeType::kw )
				? KWNodeType {}
				: std::nullopt;
		};

		std::optional< NIntNode > node_handler( void );

	};

}

#endif

