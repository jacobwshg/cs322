
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
#include <tuple>
#include <regex>

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

		// extract one lexed token and advance position
		std::string_view
		gettok( void );

		// parse tokens into AST
		bool
		parse( void );

		// for MNode specifically
		std::optional< MNode > make_M_node( void );

		// for fNode
		std::optional< fNode > make_f_node( void );

		// for pNode
		std::optional< pNode > make_p_node( void );

		// for variant nodes
		template< typename VariantNodeT >
			requires ::is_variant_v< VariantNodeT >
		std::optional< VariantNodeT > make_variant_node( void )
		{
			std::optional< VariantNodeT > res_opt { std::nullopt };

			// don't use a local struct with a `bool operator()`
			// because local struct methods can't be templates and we can't parameterize
			// operator() with NodeAltT
			auto try_make_alternative
			{
				[ & ]< typename NodeAltT >( void ) -> bool
				{
					// descend into alternative.
					// if make_node< NodeAltT > fails, it is assumed to 
					// restore token idx before returning
					if ( std::optional< NodeAltT > node_opt { this->make_node< NodeAltT >() } )
					{
						res_opt = std::move( *node_opt );
						return true;
					}
					return false;
				}
			};
			auto expand
			{
				[ & ]< typename ... AltTs >( std::variant< AltTs... > *_tag )
				{
					( try_make_alternative.template operator()< AltTs >() || ... );
				}
			};

			const std::size_t cur_idx { this->tok_idx };

			expand( (VariantNodeT *) nullptr );

			if ( res_opt ) // successfully parsed an alternative
			{
				return res_opt;
			}
			// match failed, restore idx
			this->tok_idx = cur_idx;
			return std::nullopt;
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
			// match failed, restore idx
			this->tok_idx = cur_idx;
			return std::nullopt;
		};

		// for terminal identifier nodes with regex
		template< typename IdentNodeT >
			requires L1::IsIdentNode< IdentNodeT >
		std::optional< IdentNodeT > make_ident_node( void )
		{
			const std::size_t cur_idx { this->tok_idx };
			const std::string_view tok { this->gettok() };
			if ( std::regex_match( tok.data(), IdentNodeT::re ) ) // token matches node regex
			{
				return IdentNodeT { .val = tok };
			}
			// match failed, restore idx
			this->tok_idx = cur_idx;
			return std::nullopt;
		};

		// for a vector of nodes of a given type; namely,
		// the `f+` field in p nodes and `i+` in f nodes
		template< typename ElemT, typename RDelimT = L1::RParNode >
		std::optional< std::vector< ElemT > > make_node_vector( void )
		{
			std::vector< ElemT > nodevec {};

			while ( true )
			{
				const std::size_t cur_idx { this->tok_idx };
				if ( const std::optional< RDelimT > rdelim_n { this->make_node< RDelimT >() } )
				{
					// unget right delimiter and break
					this->tok_idx = cur_idx;
					break;
				}

				if ( std::optional< ElemT > node_opt { this->make_node< ElemT >() } )
				{
					// parsed elem node
					nodevec.emplace_back( std::move( *node_opt ) );
				}
				else
				{
					// bad node ( neither elem nor rdelim )
					goto fail;
				}
			}

			if ( !nodevec.empty() )
			{
				return nodevec;
			}

			fail:
				// empty vector ( grammar requires +, not * )
				// or bad node
				return std::nullopt;
		}

		// helper for make_record_node(): make the tuple of members
		template< typename LeftT, typename ... RightTs >
		std::optional< std::tuple< LeftT, RightTs... > > make_node_tuple( void )
		{
			const std::size_t cur_idx { this->tok_idx };

			// make leftmost member
			std::optional< LeftT > left_opt { std::nullopt };
			if constexpr ( ::is_vector_v< LeftT > )
			{
				// this block is supposed to be used by f+ and i+
				left_opt = this->make_node_vector< LeftT::value_type >();
			}
			else
			{
				left_opt = this->make_node< LeftT >();
			}
			if ( !left_opt )
			{
				this->tok_idx = cur_idx;
				return std::nullopt;
			}

			// recurse on remaining members
			if constexpr ( sizeof...( RightTs ) == 0 )
			{
				// no more remaining members
				return std::make_tuple( *left_opt );
			}
			else
			{
				auto rtup_opt { this->make_node_tuple< RightTs... >() };
				if ( !rtup_opt )
				{
					// failed
					this->tok_idx = cur_idx;
					return std::nullopt;
				}
				// succeeded
				return std::tuple_cat(
					std::make_tuple( *left_opt ),
					std::move( *rtup_opt )
				);
			}

			return std::nullopt;
		};

		// for most record (struct) nodes
		template< typename RecNodeT >
			requires L1::IsRecNode< RecNodeT >
		std::optional< RecNodeT > make_record_node( void )
		{
			const std::size_t cur_idx { this->tok_idx };

			std::optional< typename RecNodeT::fields_t > ndtuple_opt
			{
				this->make_node_tuple< typename RecNodeT::fields_t >()
			};
			if ( ndtuple_opt )
			{
				return std::make_from_tuple< RecNodeT >( *ndtuple_opt );
			}

			this->tok_idx = cur_idx;
			return std::nullopt;
		};

		// dispatcher
		template< typename NodeT >
		std::optional< NodeT > make_node( void )
		{
			if constexpr ( std::is_same_v< NodeT, L1::pNode > )
			{
				return this->make_p_node();
			}

			if constexpr ( std::is_same_v< NodeT, L1::fNode > )
			{
				return this->make_f_node();
			}

			if constexpr ( std::is_same_v< NodeT, L1::MNode > )
			{
				return this->make_M_node();
			}

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

			if constexpr ( L1::IsRecNode< NodeT > )
			{
				return this->make_record_node< NodeT >();
			}

			std::cout << "unable to make node of type " << typeid( std::declval<NodeT> ).name() << "\n";
			return std::nullopt;
		}
	};

}

#endif

