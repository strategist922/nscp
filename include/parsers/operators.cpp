#include <parsers/operators.hpp>
#include <parsers/helpers.hpp>
#include <parsers/where/unary_fun.hpp>
#include <parsers/where/list_value.hpp>
#include <parsers/where/binary_op.hpp>
#include <parsers/where/unary_op.hpp>
#include <parsers/where/variable.hpp>

#include <boost/regex.hpp>

namespace parsers {
	namespace where {
		static bool debug_enabled = false;
		static int debug_level = 15;

		namespace operator_impl {

			struct simple_bool_binary_operator_impl : public binary_operator_impl {
				expression_ast evaluate(filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					value_type ltype = left.get_type();
					value_type rtype = right.get_type();

					if ( (ltype != rtype) && (rtype != type_tbd) ) {
						handler->error("Invalid types (not same) for binary operator");
						return expression_ast(int_value(FALSE));
					}
					value_type type = left.get_type();
					if (type_is_int(type))
						return eval_int(type, handler, left, right)?expression_ast(int_value(TRUE)):expression_ast(int_value(FALSE));
					if (type == type_string)
						return eval_string(type, handler, left, right)?expression_ast(int_value(TRUE)):expression_ast(int_value(FALSE));
					handler->error("missing impl for simple bool binary operator");
					return expression_ast(int_value(FALSE));
				}
				virtual bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const = 0;
				virtual bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const = 0;
			};

			struct simple_int_binary_operator_impl : public binary_operator_impl {
				expression_ast evaluate(filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					value_type ltype = left.get_type();
					value_type rtype = right.get_type();

					if ( (ltype != rtype) && (rtype != type_tbd) ) {
						handler->error("Invalid types (not same) for binary operator");
						return expression_ast(int_value(FALSE));
					}
					value_type type = left.get_type();
					if (type_is_int(type))
						return expression_ast(int_value(eval_int(type, handler, left, right)));
					if (type == type_string)
						return expression_ast(int_value(eval_string(type, handler, left, right)));
					handler->error("missing impl for simple bool binary operator");
					return expression_ast(int_value(FALSE));
				}
				virtual long long eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const = 0;
				virtual long long eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const = 0;
			};

			struct operator_and : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					return left.get_int(handler) && right.get_int(handler);
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					left;
					right;
					handler->error("missing impl for and binary operator");
					// TODO convert strings
					return false;
				};
			};
			struct operator_or : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					return left.get_int(handler) || right.get_int(handler);
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					left;
					right;
					handler->error("missing impl for or binary operator");
					// TODO convert strings
					return false;
				};
			};
			struct operator_eq : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					return left.get_int(handler) == right.get_int(handler);
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					if (debug_enabled && debug_level > 10) {
						std::string lhs = left.get_string(handler);
						std::string rhs = right.get_string(handler);
						std::cout << "(op_gt) " << lhs << " > " << rhs << std::endl;
					}
					return left.get_string(handler) == right.get_string(handler);
				};
			};
			struct operator_ne : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					return left.get_int(handler) != right.get_int(handler);
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const { 
					type;
					return left.get_string(handler) != right.get_string(handler);
				};
			};
			struct operator_gt : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					if (debug_enabled && debug_level > 10) {
						long long lhs = left.get_int(handler);
						long long rhs = right.get_int(handler);
						std::cout << "(op_gt) " << lhs << " > " << rhs << std::endl;
					}
					return left.get_int(handler) > right.get_int(handler);
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const { 
					type;
					return left.get_string(handler) > right.get_string(handler);
				};
			};
			struct operator_lt : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					if (debug_enabled && debug_level > 10) {
						long long lhs = left.get_int(handler);
						long long rhs = right.get_int(handler);
						std::cout << "(op_lt) " << lhs << " < " << rhs << std::endl;
					}
					return left.get_int(handler) < right.get_int(handler);
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const { 
					type;
					return left.get_string(handler) < right.get_string(handler);
				};
			};
			struct operator_le : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					return left.get_int(handler) <= right.get_int(handler);
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const { 
					type;
					return left.get_string(handler) <= right.get_string(handler);
				};
			};
			struct operator_ge : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					return left.get_int(handler) >= right.get_int(handler);
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const { 
					type;
					return left.get_string(handler) >= right.get_string(handler);
				};
			};


			struct operator_bin_and : public simple_int_binary_operator_impl {
				long long eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					return left.get_int(handler) & right.get_int(handler);
				}
				long long eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const { 
					type;
					handler;
					left;
					right;
					return 0;
				};
			};
			struct operator_bin_or : public simple_int_binary_operator_impl {
				long long eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					return left.get_int(handler) | right.get_int(handler);
				}
				long long eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const { 
					type;
					handler;
					left;
					right;
					return 0;
				};
			};

			struct operator_like : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					left;
					right;
					handler->error("Like not supported on numbers...");
					return false;
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const { 
					type;
					std::string s1 = left.get_string(handler);
					std::string s2 = right.get_string(handler);
					if (s1.size() == 0 && s2.size() == 0)
						return true;
					if (s1.size() == 0 || s2.size() == 0)
						return false;
					if (s1.size() > s2.size() && s2.size() > 0)
						return s1.find(s2) != std::string::npos;
					return s2.find(s1) != std::string::npos;
				};
			};
			struct operator_regexp : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					left;
					right;
					handler->error("Regular expression not supported on numbers...");
					return false;
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const { 
					type;
					std::string str = left.get_string(handler);
					std::string regexp = right.get_string(handler);
					if (debug_enabled)
						std::cout << "(op_regexp) " << str << " regexp " << regexp << std::endl;
					try {
						boost::regex re(regexp);
						return boost::regex_match(str, re);
					} catch (const boost::bad_expression e) {
						handler->error("Invalid syntax in regular expression:" + regexp);
						return false;
					} catch (...) {
						handler->error("Invalid syntax in regular expression:" + regexp);
						return false;
					}
				};
			};
			struct operator_not_regexp : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					left;
					right;
					handler->error("Regular expression not supported on numbers...");
					return false;
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const { 
					type;
					std::string str = left.get_string(handler);
					std::string regexp = right.get_string(handler);
					if (debug_enabled)
						std::cout << "(op_regexp) " << str << " regexp " << regexp << std::endl;
					try {
						boost::regex re(regexp);
						return !boost::regex_match(str, re);
					} catch (const boost::bad_expression e) {
						handler->error("Invalid syntax in regular expression:" + regexp);
						return false;
					} catch (...) {
						handler->error("Invalid syntax in regular expression:" + regexp);
						return false;
					}
				};
			};
			struct operator_not_like : public simple_bool_binary_operator_impl {
				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					left;
					right;
					handler->error("Not like not supported on numbers...");
					return false;
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const { 
					type;
					std::string s1 = left.get_string(handler);
					std::string s2 = right.get_string(handler);
					if (s1.size() > s2.size() && s2.size() > 0)
						return s1.find(s2) == std::string::npos;
					return s2.find(s1) == std::string::npos;
				};
			};
			struct operator_not_in : public simple_bool_binary_operator_impl {

				typedef expression_ast::list_type list_type;
				typedef expression_ast list_item_type;
				expression_ast::list_type list;
				operator_not_in(const expression_ast &subject) : list(subject.get_list()) {}

				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					right;
					long long val = left.get_int(handler);
					BOOST_FOREACH(list_item_type itm, list) {
						if (itm.get_int(handler) == val)
							return false;
					}
					return true;
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					right;
					std::string val = left.get_string(handler);
					BOOST_FOREACH(list_item_type itm, list) {
						if (itm.get_string(handler) == val)
							return false;
					}
					return true;
				};
			};
			struct operator_in : public simple_bool_binary_operator_impl {

				typedef expression_ast::list_type list_type;
				typedef expression_ast list_item_type;
				expression_ast::list_type list;
				operator_in(const expression_ast &subject) : list(subject.get_list()) {}

				bool eval_int(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					right;
					long long val = left.get_int(handler);
					BOOST_FOREACH(list_item_type itm, list) {
						if (itm.get_int(handler) == val)
							return true;
					}
					return false;
				}
				bool eval_string(value_type type, filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					type;
					right;
					std::string val = left.get_string(handler);
					BOOST_FOREACH(list_item_type itm, list) {
						if (itm.get_string(handler) == val)
							return true;
					}
					return false;
				};
			};
			struct operator_false : public binary_operator_impl, unary_operator_impl, binary_function_impl {
				expression_ast evaluate(filter_handler handler, const expression_ast &left, const expression_ast & right) const {
					left;
					right;
					handler->error("missing impl for FALSE");
					return expression_ast(int_value(FALSE));
				}
				expression_ast evaluate(filter_handler handler, const expression_ast &subject) const {
					subject;
					handler->error("missing impl for FALSE");
					return expression_ast(int_value(FALSE));
				}
				expression_ast evaluate(parsers::where::value_type type, filter_handler handler, const expression_ast &subject) const {
					type;
					subject;
					handler->error("missing impl for FALSE");
					return expression_ast(int_value(FALSE));
				}
			};

			struct function_convert : public binary_function_impl {
				typedef expression_ast::list_type list_type;
				expression_ast list_entry;
				expression_ast::list_type list;
				bool single_item;
				function_convert(const expression_ast &subject) : list(subject.get_list()), single_item(list.size()==1) {}
				expression_ast evaluate(value_type type, filter_handler handler, const expression_ast &subject) const {
					if (single_item) {
						if (type_is_int(type)) {
							return expression_ast(int_value(list.front().get_int(handler)));
						}
						if (type == type_string) {
							return expression_ast(string_value(list.front().get_string(handler)));
						}
						handler->error("1:Failed to handle type: " + to_string(type));
						return expression_ast(int_value(FALSE));
					}
					if (list.size()==2) {
						list_type::const_iterator item = list.begin();
						list_type::const_iterator unit = item;
						std::advance(unit, 1);
						if (type == type_date) {
							return expression_ast(int_value(parse_time((*item).get_int(handler), (*unit).get_string(handler))));
						}
						if (type == type_size) {
							return expression_ast(int_value(parse_size((*item).get_int(handler), (*unit).get_string(handler))));
						}
						handler->error("m:Failed to handle type: " + to_string(type) + " " + (*item).to_string() + ", " + (*unit).to_string());
						return expression_ast(int_value(FALSE));
					}
					std::cout << "----------------------------------------------\n";
					std::cout << list.size() << _T("\n");
					std::cout << subject.to_string() << _T("\n");
					std::cout << "----------------------------------------------\n";
					handler->error("Missing implementation for convert function");
					return expression_ast(int_value(FALSE));
				}

				inline long long parse_time(long long value, std::string unit) const {
					long long now = constants::get_now();
					if (unit.empty())
						return now + value;
					else if ( (unit == "s") || (unit == "S") )
						return now + (value);
					else if ( (unit == "m") || (unit == "M") )
						return now + (value * 60);
					else if ( (unit == "h") || (unit == "H") )
						return now + (value * 60 * 60);
					else if ( (unit == "d") || (unit == "D") )
						return now + (value * 24 * 60 * 60);
					else if ( (unit == "w") || (unit == "W") )
						return now + (value * 7 * 24 * 60 * 60);
					return now + value;
				}

				inline long long parse_size(long long value, std::string unit) const {
					if (unit.empty())
						return value;
					else if ( (unit == "b") || (unit == "B") )
						return value;
					else if ( (unit == "k") || (unit == "k") )
						return value * 1024;
					else if ( (unit == "m") || (unit == "M") )
						return value * 1024 * 1024;
					else if ( (unit == "g") || (unit == "G") )
						return value * 1024 * 1024 * 1024;
					else if ( (unit == "t") || (unit == "T") )
						return value * 1024 * 1024 * 1024 * 1024;
					return value;
				}
				
			};


			struct simple_bool_unary_operator_impl : public unary_operator_impl {
				expression_ast evaluate(filter_handler handler, const expression_ast &subject) const {
					value_type type = subject.get_type();
					if (type_is_int(type))
						return eval_int(type, handler, subject)?expression_ast(int_value(TRUE)):expression_ast(int_value(FALSE));
					if (type == type_string)
						return eval_string(type, handler, subject)?expression_ast(int_value(TRUE)):expression_ast(int_value(FALSE));
					handler->error("missing impl for bool unary operator");
					return expression_ast(int_value(FALSE));
				}
				virtual bool eval_int(value_type type, filter_handler handler, const expression_ast &subject) const = 0;
				virtual bool eval_string(value_type type, filter_handler handler, const expression_ast &subject) const = 0;
			};

			struct operator_not : public unary_operator_impl, binary_function_impl {
				operator_not(const expression_ast &subject) {subject;}
				operator_not() {}
				expression_ast evaluate(filter_handler handler, const expression_ast &subject) const {
					return evaluate(subject.get_type(), handler, subject);
				}
				expression_ast evaluate(value_type type, filter_handler handler, const expression_ast &subject) const {
					if (type == type_bool)
						return subject.get_int(handler)?expression_ast(int_value(TRUE)):expression_ast(int_value(FALSE));
					if (type == type_int)
						return  expression_ast(int_value(-subject.get_int(handler)));
					if (type == type_date) {
						long long now = constants::get_now();
						long long val = now - (subject.get_int(handler) - now);
						return expression_ast(int_value(val));
					}
					handler->error("missing impl for NOT operator");
					return expression_ast(int_value(FALSE));
				}
			};
		}

		factory::bin_op_type factory::get_binary_operator(operators op, const expression_ast &left, const expression_ast &right) {
			left;
			// op_in, op_nin
			if (op == op_eq)
				return bin_op_type(new operator_impl::operator_eq());
			if (op == op_gt)
				return bin_op_type(new operator_impl::operator_gt());
			if (op == op_lt)
				return bin_op_type(new operator_impl::operator_lt());
			if (op == op_le)
				return bin_op_type(new operator_impl::operator_le());
			if (op == op_ge)
				return bin_op_type(new operator_impl::operator_ge());
			if (op == op_ne)
				return bin_op_type(new operator_impl::operator_ne());
			if (op == op_like)
				return bin_op_type(new operator_impl::operator_like());
			if (op == op_not_like)
				return bin_op_type(new operator_impl::operator_not_like());
			if (op == op_regexp)
				return bin_op_type(new operator_impl::operator_regexp());
			if (op == op_regexp)
				return bin_op_type(new operator_impl::operator_not_regexp());

			

			if (op == op_and)
				return bin_op_type(new operator_impl::operator_and());
			if (op == op_or)
				return bin_op_type(new operator_impl::operator_or());
			if (op == op_in)
				return bin_op_type(new operator_impl::operator_in(right));
			if (op == op_nin)
				return bin_op_type(new operator_impl::operator_not_in(right));

			if (op == op_binand)
				return bin_op_type(new operator_impl::operator_bin_and());
			if (op == op_binor)
				return bin_op_type(new operator_impl::operator_bin_or());

			std::cout << "======== UNHANDLED OPERATOR\n";
			return bin_op_type(new operator_impl::operator_false());
		}

		factory::bin_fun_type factory::get_binary_function(std::string name, const expression_ast &subject) {
			if (name == "convert")
				return bin_fun_type(new operator_impl::function_convert(subject));
			if (name == "auto_convert")
				return bin_fun_type(new operator_impl::function_convert(subject));
			if (name == "neg")
				return bin_fun_type(new operator_impl::operator_not(subject));
			std::cout << "======== UNDEFINED FUNCTION: " << name << std::endl;
			return bin_fun_type(new operator_impl::operator_false());
		}
		factory::un_op_type factory::get_unary_operator(operators op) {
			// op_inv, op_not
			if (op == op_not)
				return un_op_type(new operator_impl::operator_not());
			std::cout << "======== UNHANDLED OPERATOR\n";
			return un_op_type(new operator_impl::operator_false());
		}
	}
}
