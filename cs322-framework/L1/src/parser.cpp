
#include "parser.h"
#include "ast.h"

#include <cstdint>
#include <cctype>
#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <cstdlib>

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

}

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
	int charidx { 0 };
	char prv { NUL }, cur {};
	State state { State::IN_SPACE };
	bool cur_isspace { false };
	bool in_comment { false };

	for ( cur=src_is.get(); src_is; cur=src_is.get() )
	{
		//std::printf( "%c\t\t", cur );

		// non-space token break condition
		bool tokbrk { false };
		if ( L1::isident( prv ) ^ L1::isident( cur ) )
		{
			// don't break up `tuple-error` and `tensor-error`
			tokbrk = ( prv=='-' ) == ( cur=='-' );
		}
		else
		{
			// parentheses form their own tokens and break from surrounding
			// non-identifier characters
			tokbrk = L1::isparen( prv ) ^ L1::isparen( cur );
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
				// token boundary without space
				// ( one is identifier, one is not; alternatively, 
				// both aren't identifiers, but one is a parenthesis
				// and must be its own token )
				this->srcbuf.push_back( NUL );
				++charidx;
				// register base of new token
				this->tok_base_idxs.emplace_back( charidx );
				this->srcbuf.push_back( cur );
				++charidx;
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
					// throw them out and ignore all chars until EOL
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
		prv = cur;
	}
	// register final token before EOF
	this->srcbuf.push_back( NUL );
	++charidx; // only for consistency
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
	if ( this->tok_idx >= this->tok_base_idxs.size() )
	{
		return L1::EMPTYTOK;
	}
	else
	{
		const std::string_view tok { &this->srcbuf.data()[ this->tok_base_idxs[ this->tok_idx ] ] };
		++this->tok_idx;
		return tok;
	}
}

std::optional< L1::MNode >
L1::
Parser::make_M_node( void )
{
	const std::size_t cur_idx { this->tok_idx };

	const std::string_view tok { this->gettok() };
	const unsigned long val { std::strtoul( tok.data(), nullptr, 0 ) }; 

	if ( 0UL == ( val & ( 0x8UL-1 ) ) ) // multiple of 8
	{
		return L1::MNode { .val = val };
	}
	this->tok_idx = cur_idx;
	return std::nullopt;
}

std::optional< L1::fNode >
L1::
Parser::make_f_node( void )
{
	const std::size_t cur_idx { this->tok_idx };

	std::optional< L1::LParNode > lpar_n_opt {};
	std::optional< L1::lNode > l_n_opt {};
	std::optional< L1::NNode > N1_n_opt {};
	std::optional< L1::NNode > N2_n_opt {};
	std::optional< std::vector< L1::iNode > > i_ns_opt {};
	std::optional< L1::RParNode > rpar_n_opt {};

	if ( !( lpar_n_opt = this->make_node< L1::LParNode >() ) ) { goto fail; }
	if ( !( l_n_opt = this->make_node< L1::lNode >() ) ) { goto fail; }
	if ( !( N1_n_opt = this->make_node< L1::NNode >() ) ) { goto fail; }
	if ( !( N2_n_opt = this->make_node< L1::NNode >() ) ) { goto fail; }
	if ( !( i_ns_opt = this->make_node_vector< L1::iNode >() ) ) { goto fail; }
	if ( !( rpar_n_opt = this->make_node< L1::RParNode >() ) ) { goto fail; }

	return
		L1::fNode
		{
			.lpar_n = *lpar_n_opt,
			.l_n = *l_n_opt,
			.N1_n = *N1_n_opt, .N2_n = *N2_n_opt,
			.i_ns = std::move( *i_ns_opt ),
			.rpar_n = *rpar_n_opt
		};

	fail:
		this->tok_idx = cur_idx;
		return std::nullopt;
}

std::optional< L1::pNode >
L1::
Parser::make_p_node( void )
{
	const std::size_t cur_idx { this->tok_idx };

	std::optional< L1::LParNode > lpar_n_opt {};
	std::optional< L1::lNode >    l_n_opt {};
	std::optional< std::vector< L1::fNode > > f_ns_opt {};
	std::optional< L1::RParNode > rpar_n_opt {};

	if ( !( lpar_n_opt = this->make_node< L1::LParNode >() )  ) { goto fail; }
	if ( !( l_n_opt    = this->make_node< L1::lNode >() ) ) { goto fail; }
	if ( !( f_ns_opt   = this->make_node_vector< L1::fNode >() ) ) { goto fail; }
	if ( !( rpar_n_opt = this->make_node< L1::RParNode >() ) ) { goto fail; }

	return
		L1::pNode
		{
			.lpar_n = *lpar_n_opt,
			.l_n    = *l_n_opt,
			.f_ns   = std::move( *f_ns_opt ),
			.rpar_n = *rpar_n_opt
		};

	fail:
		this->tok_idx = cur_idx;
		return std::nullopt;
}

bool
L1::
Parser::parse( void )
{
	this->ast = this->make_node< pNode >();
	return this->ast ? true: false;
}



