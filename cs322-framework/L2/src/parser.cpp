
#include "parser.h"
#include "ast.h"

#include <cstdint>
#include <cctype>
#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <cstdlib>

namespace L2
{
 	//
 	// returns whether a character can be used in an identifier (name)
	//
	inline bool
	isident( const char c )
	{
		return std::isalnum( c ) || c=='_';
	}

	//
 	// returns whether a character forms its own token
	//
	inline bool
	is_singleton( const char c )
	{
		switch ( c )
		{
		case L2::KW::LPAR   [ 0 ]:
		case L2::KW::RPAR   [ 0 ]:
		case L2::KW::AT     [ 0 ]:
		case L2::KW::COLON  [ 0 ]:
		case L2::KW::PERCENT[ 0 ]:
			return true;
			break;
		default:
			break;
		}
		return false;
	}

}


L2::
Parser::Parser( void )
{
	this->srcbuf.reserve( 1024 );
	this->tok_base_idxs.reserve( 256 );
	this->tok_idx = 0;
}

void
L2::
Parser::lex( std::istream &src_is )
{
	enum class State
	{
		IN_SPACE, IN_COMMENT, IN_TOK,
	};

	static constexpr char NUL { '\0' };
	int charidx { 0 };
	char
		pprv { NUL },
		prv { NUL },
		cur {};
	State state { State::IN_SPACE };
	bool cur_isspace { false };
	bool in_comment { false };

	for ( cur=src_is.get(); src_is; cur=src_is.get() )
	{
		//std::printf( "%c\t\t", cur );

		// non-space token break condition
		bool tokbrk { false };
		if ( isident( prv ) != isident( cur ) )
		{
			// one of the most recent 2 tokens can appear in an identifier; the other cannot.
			// generally, we have a token break
			//
			tokbrk = true;
			// but don't break up a sign and the following number ( non-zero M )
			//
			if ( ( prv=='+' || prv=='-' ) && std::isdigit( cur ) )
			{
				tokbrk = false;
			}
			// an arrow is its own token; for cases such as `<-6`, this rule
			// overrides the above
			if ( pprv=='<' && prv=='-' )
			{
				tokbrk = true;
			}
		}
		else
		{
			// both chars are identifier or non-identifier
			// generally, there's no token break
			//
			tokbrk = false;
			//
			// force breaks on both sides of a single-char token
			//
			if ( is_singleton( prv ) || is_singleton( cur ) )
			{
				tokbrk = true;
			}

		}

		cur_isspace = std::isspace( cur );
		switch ( state )
		{

		case State::IN_SPACE:
			if ( !cur_isspace )
			{
				// token begin, transition from space
				state = State::IN_TOK;
				this->tok_base_idxs.emplace_back( charidx );
				this->srcbuf.push_back( cur );
				++charidx;
			}
			else
			{
				// if current token is still space, ignore
				//std::printf( "space\n" );
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
				//std::printf( "token end, transition to space\n" );
				state = State::IN_SPACE;
				this->srcbuf.push_back( NUL );
				++charidx;
			}
			else if ( tokbrk )
			{
				// token break without space
				this->srcbuf.push_back( NUL );
				++charidx;
				// register base of new token
				this->tok_base_idxs.emplace_back( charidx );
				this->srcbuf.push_back( cur );
				++charidx;

				if ( std::isalpha( pprv ) && ( prv=='-' ) && std::isalpha( cur ) )
				{
					// relink kebab-case identifiers
					// advanced charidx: ... A \0 - \0 A _
					//                                   ^
					this->srcbuf.pop_back(); // cur
					this->srcbuf.pop_back(); // \0 after prv
					this->srcbuf.pop_back(); // prv
					this->srcbuf.pop_back(); // \0 after pprv
					this->tok_base_idxs.pop_back(); // cur
					this->tok_base_idxs.pop_back(); // prv
					this->srcbuf.push_back( prv );
					this->srcbuf.push_back( cur );
					charidx -= 2;
				}

			}
			else
			{
				// same token
				//std::printf( "same token \n" );
				this->srcbuf.push_back( cur );
				++charidx;

				if ( cur=='/' && prv=='/' )
				{
					// we realized that we are inside a comment. both `/`s have been appended
					// to `srcbuf`, and `charidx` has advanced past the second `/`. we need to 
					// throw them out and ignore all chars until EOL.
					state = State::IN_COMMENT;
					charidx -= 2;
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
		pprv = prv;
		prv = cur;
	}
	// register final token before EOF
	this->srcbuf.push_back( NUL );
	++charidx; // only for consistency
}

void
L2::
Parser::printtoks( void ) const
{
	std::printf( "Tokens\n" );
	for ( const int tokbase : this->tok_base_idxs )
	{
		std::printf( "%0d\t%s\n", tokbase, &this->srcbuf.data()[ tokbase ] );
	}
}

std::string_view
L2::
Parser::gettok( void )
{
	if ( this->tok_idx >= this->tok_base_idxs.size() )
	{
		return L2::EMPTYTOK;
	}
	else
	{
		const std::string_view tok { &this->srcbuf.data()[ this->tok_base_idxs[ this->tok_idx ] ] };
		++this->tok_idx;
		return tok;
	}
}

std::optional< L2::MNode >
L2::
Parser::make_M_node( void )
{
	const std::size_t cur_idx { this->tok_idx };

	const std::string_view tok { this->gettok() };
	const long long val { std::strtoll( tok.data(), nullptr, 0 ) }; 

	if ( 0LL == ( val & ( 0x8LL-1 ) ) ) // multiple of 8
	{
		return L2::MNode { .val = val };
	}
	this->tok_idx = cur_idx;
	return std::nullopt;
}

std::optional< L2::pNode >
L2::
Parser::parse( void )
{
	return this->make_node< pNode >();
}



