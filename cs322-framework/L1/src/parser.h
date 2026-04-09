
#ifndef L1_PARSER_H
#define L1_PARSER_H

#include <string_view>
#include <array>
#include <vector>
#include <iostream>
#include <variant>

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

	public:
		Parser( void );

		void
		lex( std::istream &src_is );

		void
		print_toks( void ) const;
	};
}

#endif

