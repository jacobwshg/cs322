
#ifndef L1_PARSER_H
#define L1_PARSER_H

#include "ast.h"
#include <cxxabi.h>
#include <cstdint>
#include <string_view>
#include <array>
#include <vector>
#include <iostream>
#include <variant>
#include <optional>
#include <tuple>
#include <regex>
#include <cstdio>
#include <typeinfo>

namespace L1
{
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
		std::optional< L1::pNode >
		parse( void );

		// dispatcher
		template< typename Node >
		std::optional< Node >
		make_node( void )
		{

			//std::printf( "Token idx %0lu ", this->tok_idx  );

			/*
			if constexpr ( std::is_same_v< Node, L1::pNode > )
			{
				return this->make_p_node();
			}

			if constexpr ( std::is_same_v< Node, L1::fNode > )
			{
				return this->make_f_node();
			}
			*/

			if constexpr ( std::is_same_v< Node, L1::MNode > )
			{
				//std::printf( "making M node\n" );
				return this->make_M_node();
			}

			if constexpr ( ::is_variant_v< Node > )
			{
				//std::printf( "making variant node %s\n", typeid( Node{} ).name() );
				return this->make_variant_node< Node >();
			}

			if constexpr ( L1::IsKWNode< Node > )
			{
				//std::printf( "making kw node %s\n", typeid( Node{} ).name() );
				return this->make_kw_node< Node >();
			}

			if constexpr ( L1::IsIdentNode< Node > )
			{
				//std::printf( "making identifier node %s\n", typeid( Node{} ).name() );
				return this->make_ident_node< Node >();
			}

			if constexpr ( L1::IsRecNode< Node > )
			{
				//std::printf( "making record node %s\n", typeid( Node{} ).name() );
				return this->make_record_node< Node >();
			}

			//std::printf( "node type %s unknown\n", typeid( Node{} ).name() );

			return std::nullopt;
		}

	private:
		// for MNode specifically
		std::optional< MNode >
		make_M_node( void );

		// for fNode
		std::optional< fNode >
		make_f_node( void );

		// for pNode
		std::optional< pNode >
		make_p_node( void );

		// for variant nodes
		template< typename VariantNode >
			requires ::is_variant_v< VariantNode >
		std::optional< VariantNode >
		make_variant_node( void )
		{
			std::optional< VariantNode > res_opt { std::nullopt };

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

			expand( (VariantNode *) { nullptr } );

			if ( res_opt ) // successfully parsed an alternative
			{
				return res_opt;
			}
			// match failed, restore idx
			this->tok_idx = cur_idx;
			return std::nullopt;
		}

		// for terminal kw nodes
		template< typename KWNode >
			requires L1::IsKWNode< KWNode >
		std::optional< KWNode >
		make_kw_node( void )
		{
			const std::size_t cur_idx { this->tok_idx };
			const std::string_view tok { this->gettok() };

			//std::printf( "kw expected: %s, token: %s\n", KWNode::kw.data(), tok.data() );

			if ( tok == KWNode::kw ) // token matches kw 
			{
				//std::printf( "kw `%s` match success\n", tok.data() );
				return KWNode {};
			}
			// match failed, restore idx
			this->tok_idx = cur_idx;
			return std::nullopt;
		};

		// for terminal identifier nodes with regex
		template< typename IdentNode >
			requires L1::IsIdentNode< IdentNode >
		std::optional< IdentNode >
		make_ident_node( void )
		{
			const std::size_t cur_idx { this->tok_idx };
			const std::string_view tok { this->gettok() };
			if ( !std::regex_match( tok.data(), IdentNode::re ) ) // token matches node regex
			{
				// match failed, restore idx
				this->tok_idx = cur_idx;
				return std::nullopt;
			}

			//std::printf( "identifier `%s` match success\n", tok.data() );

			if constexpr ( std::is_same_v< IdentNode, L1::nameNode > )
			{
				// name ( val should be a token )
				return IdentNode { .val = tok };
			}
			if constexpr ( std::is_same_v< IdentNode, L1::NNZNode > )
			{
				// nonzero N ( value should be an integer )
				errno = 0;
				const long long val { std::strtoll( tok.data(), nullptr, 0 ) };
				if ( errno == ERANGE ) { return std::nullopt; }
				return IdentNode { .val = val };
			}
			return std::nullopt;
		};

		// for a vector of nodes of a given type; namely,
		// the `f+` field in p nodes and `i+` in f nodes
		template< typename ElemT, typename RDelimT = L1::RParNode >
		std::optional< std::vector< ElemT > >
		make_node_vector( void )
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
		std::optional< std::tuple< LeftT, RightTs... > >
		make_node_tuple( void )
		{
			const std::size_t cur_idx { this->tok_idx };

			// make leftmost member
			std::optional< LeftT > left_opt { std::nullopt };
			if constexpr ( ::is_vector_v< LeftT > )
			{
				// this block is supposed to be used by f+ and i+
				left_opt = this->make_node_vector< typename LeftT::value_type >();
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
				std::optional< std::tuple< RightTs... > > rtup_opt
				{
					this->make_node_tuple< RightTs... >()
				};
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
		template< typename RecNode >
			requires L1::IsRecNode< RecNode >
		std::optional< RecNode >
		make_record_node( void )
		{

			const std::size_t cur_idx { this->tok_idx };

			// IMPORTANT - expand `fields_t`. else the entire `fields_t` gets mapped to
			// `LeftT` in `make_node_tuple()` and `RightT` maps nothing.
			auto make_fields_tuple
			{
				[ this ]< typename ... FieldTs >( std::tuple< FieldTs... > *_tag )
					-> std::optional< std::tuple< FieldTs... > >
				{
					return this->make_node_tuple< FieldTs... >();
				}
			};

			std::optional< typename RecNode::fields_t > ndtuple_opt
			{
				// without expansion ( doesn't work )
				//this->make_node_tuple< typename RecNode::fields_t >()
				make_fields_tuple( ( typename RecNode::fields_t * ) nullptr )
			};
			if ( ndtuple_opt )
			{
				return std::make_from_tuple< RecNode >( *ndtuple_opt );
			}

			this->tok_idx = cur_idx;
			return std::nullopt;
		};

	};

}

#endif

