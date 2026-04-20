
#ifndef L2_SPILL_ALIAS_VARIANT_GEN_H
#define L2_SPILL_ALIAS_VARIANT_GEN_H

#include "../ast.h"

#include <string_view>
#include <cstdint>

namespace L2
{
	namespace Spill
	{

		struct sxGenerator
		{
			std::string_view spill_var_name {};
			std::string_view alias_prefix {};
			std::size_t alias_id {};

			sxNode
			operator()( const varNode &var_n )
			{
				if ( var_n.name_n.val == spill_var_name )
				{
					return sxNode
					{
						varNode
						{
							.percent_n = {},
							.name_n = nameNode
							{
								.val = std::string { alias_prefix }
									+ std::to_string( alias_id )
							}
						}
					};
				}
				else { return var_n; }
			}

			// rcx
			template< typename Alt > sxNode
			operator()( const Alt &alt_n ) { return sxNode { alt_n }; }

		};

		struct aGenerator
		{
			std::string_view spill_var_name {};
			std::string_view alias_prefix {};
			std::size_t alias_id {};

			// sx
			aNode
			operator()( const sxNode &sx_n )
			{
				return 
				aNode
				{	
					std::visit(
						sxGenerator { spill_var_name, alias_prefix, alias_id, },
						sx_n
					)
				};
			}

			// rdi, rsi, rdx, r8, r9
			template< typename Alt > aNode
			operator()( const Alt &alt_n ) { return aNode { alt_n }; }

		};

		struct wGenerator
		{
			std::string_view spill_var_name {};
			std::string_view alias_prefix {};
			std::size_t alias_id {};

			// a
			wNode
			operator()( const aNode &a_n )
			{
				return 
				wNode
				{	
					std::visit(
						aGenerator { spill_var_name, alias_prefix, alias_id, },
						a_n
					)
				};
			}

			// rax
			template< typename Alt > wNode
			operator()( const Alt &alt_n ) { return wNode { alt_n }; }

		};

		struct uGenerator
		{
			std::string_view spill_var_name {};
			std::string_view alias_prefix {};
			std::size_t alias_id {};

			// w
			uNode
			operator()( const wNode &w_n )
			{
				return 
				uNode
				{	
					std::visit(
						wGenerator { spill_var_name, alias_prefix, alias_id, },
						w_n
					)
				};
			}

			// l
			template< typename Alt > uNode
			operator()( const Alt &alt_n ) { return uNode { alt_n }; }

		};

		struct xGenerator
		{
			std::string_view spill_var_name {};
			std::string_view alias_prefix {};
			std::size_t alias_id {};

			// w
			xNode
			operator()( const wNode &w_n )
			{
				return 
				xNode
				{	
					std::visit(
						wGenerator { spill_var_name, alias_prefix, alias_id, },
						w_n
					)
				};
			}

			// rsp
			template< typename Alt > xNode
			operator()( const Alt &alt_n ) { return xNode { alt_n }; }

		};

		struct tGenerator
		{
			std::string_view spill_var_name {};
			std::string_view alias_prefix {};
			std::size_t alias_id {};

			// x
			tNode
			operator()( const xNode &x_n )
			{
				return 
				xNode
				{	
					std::visit(
						xGenerator { spill_var_name, alias_prefix, alias_id, },
						x_n
					)
				};
			}

			// N
			template< typename Alt > tNode
			operator()( const Alt &alt_n ) { return tNode { alt_n }; }

		};

		struct sGenerator
		{
			std::string_view spill_var_name {};
			std::string_view alias_prefix {};
			std::size_t alias_id {};

			// t
			tNode
			operator()( const tNode &t_n )
			{
				return 
				tNode
				{	
					std::visit(
						tGenerator { spill_var_name, alias_prefix, alias_id, },
						t_n
					)
				};
			}

			// label, l
			template< typename Alt > sNode
			operator()( const Alt &alt_n ) { return sNode { alt_n }; }

		};

	}
}

#endif

