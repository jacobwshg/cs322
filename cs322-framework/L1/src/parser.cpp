
#include "parser.h"
#include "ast.h"

#include <cstdint>
#include <cctype>
#include <iostream>
#include <vector>
#include <string>
#include <cctype>

/*
 * @brief
 *   Returns whether a character can be used in an identifier (name).
 * @param
 *   c: a character
 * @returns
 *   true exactly if the character can be used in an identifier, 
 *   that is, is alphanumeric or underscore.
 */
bool
L1::
isident( const char c )
{
	return std::isalnum( c ) || c=='_';
}


/*
 * @brief
 *   Returns whether a character is a parenthesis.
 * @param
 *   c: a character
 * @returns
 *   true exactly if the character is a parenthesis.
 */
bool
L1::
isparen( const char c )
{
	return c==L1::KW::LPAR[ 0 ] || c==L1::KW::RPAR[ 0 ];
}


L1::
Parser::Parser( void )
{
	this->srcbuf.reserve( 1024 );
	this->tok_base_idxs.reserve( 256 );
	this->tok_idx = 0;
}

void
L1::
Parser::lex( std::istream &src_is )
{
	enum class State
	{
		IN_SPACE, IN_COMMENT, IN_TOK,
	};

	static constexpr char NUL { '\0' };
	int idx { 0 };
	char prv { NUL }, cur {};
	State state { State::IN_SPACE };
	bool cur_isspace { false };
	bool in_comment { false };

	for ( cur=src_is.get(); src_is; cur=src_is.get() )
	{
		std::printf( "%c\t\t", cur );

		cur_isspace = std::isspace( cur );
		switch ( state )
		{

		case State::IN_SPACE:
			// if current token is still space, ignore
			if ( !cur_isspace )
			{
				// token begin, transition from space
				std::printf( "token begin, transition from space, idx %0d\n", idx );
				state = State::IN_TOK;
				this->tok_base_idxs.emplace_back( idx );
				this->srcbuf.push_back( cur );
				++idx;
			}
			else
			{
				std::printf( "space\n" );
			}
			break;

		case State::IN_COMMENT:
			if ( cur=='\r' || cur=='\n' )
			{
				state = State::IN_SPACE;
			}
			break;

		case State::IN_TOK:
			if ( cur_isspace )
			{
				// token end, transition to space
				std::printf( "token end, transition to space\n" );
				state = State::IN_SPACE;
				this->srcbuf.push_back( NUL );
				++idx;
			}
			else if (
				( L1::isident( prv ) ^ L1::isident( cur ) )
				|| ( L1::isparen( prv ) ^ L1::isparen( cur ) )
			)
			{
				std::printf( "token boundary without space, idx %0d\n", idx );
				// token boundary without space
				// ( one is identifier, one is not; alternatively, 
				// both aren't identifiers, but one is a parenthesis
				// and must be its own token )
				this->srcbuf.push_back( NUL );
				++idx;
				// register base of new token
				this->tok_base_idxs.emplace_back( idx );
				this->srcbuf.push_back( cur );
				++idx;
			}
			else
			{
				// same token
				std::printf( "same token \n" );
				this->srcbuf.push_back( cur );
				++idx;

				if ( cur=='/' && prv=='/' )
				{
					// we realized that we are inside a comment. both `/`s have been appended
					// to `srcbuf`, and `idx` has advanced past the second `/`. we need to 
					// throw them out and ignore all chars until EOL
					state = State::IN_COMMENT;
					idx -= 2;
					this->srcbuf.pop_back();
					this->srcbuf.pop_back();
					this->tok_base_idxs.pop_back();
				}

			}
			break;

		default:
			break;

		}

		// shift
		prv = cur;
	}
	// register final token before EOF
	this->srcbuf.push_back( NUL );
	++idx; // only for consistency
}

void
L1::
Parser::print_toks( void ) const
{
	std::printf( "Tokens\n" );
	for ( const int tokbase : this->tok_base_idxs )
	{
		std::printf( "%0d\t%s\n", tokbase, &this->srcbuf.data()[ tokbase ] );
	}
}

std::string_view
L1::
Parser::gettok( void )
{
	static constexpr std::string_view EMPTYTOK { "" };
	if ( this->tok_idx >= this->tok_base_idxs.size() )
	{
		return EMPTYTOK;
	}
	else
	{
		return std::string_view { &this->srcbuf.data()[ this->tok_idx ] };
		++this->tok_idx;
	}
}

