#include "StdAfx.h"

#include <map>
#include <list>

#include <boost/bind.hpp>
#include <boost/assign.hpp>


#include <parsers/where.hpp>
#include <parsers/filter/where_filter.hpp>
#include <parsers/filter/where_filter_impl.hpp>

#include <parsers/where/unary_fun.hpp>
#include <parsers/where/list_value.hpp>
#include <parsers/where/binary_op.hpp>
#include <parsers/where/unary_op.hpp>
#include <parsers/where/variable.hpp>

#include <strEx.h>
#include <format.hpp>
#include "filter.hpp"

#define DATE_FORMAT _T("%#c")
using namespace boost::assign;
using namespace parsers::where;

tasksched_filter::filter_obj_handler::filter_obj_handler() {
		insert(types)
			("title", (type_string))
			("account", (type_string))
			("application", (type_string))
			("comment", (type_string))
			("creator", (type_string))
			("parameters", (type_string))
			("working_directory", (type_string))
			("error_retry_count", (type_int))
			("error_retry_interval", (type_int))
//			("idle_wait", (type_int))
			("exit_code", (type_int))
			("flags", (type_int))
			("max_run_time", (type_int))
			("priority", (type_int))
			("status", (type_custom_hresult))
			("most_recent_run_time", (type_date));
	}

bool tasksched_filter::filter_obj_handler::has_variable(std::string key) {
	return types.find(key) != types.end();
}
parsers::where::value_type tasksched_filter::filter_obj_handler::get_type(std::string key) {
	types_type::const_iterator cit = types.find(key);
	if (cit == types.end())
		return parsers::where::type_invalid;
	return cit->second;
}
bool tasksched_filter::filter_obj_handler::can_convert(parsers::where::value_type from, parsers::where::value_type to) {
	if ((from == parsers::where::type_string)&&(to == type_custom_hresult))
		return true;
	if ((from == parsers::where::type_int)&&(to == type_custom_hresult))
		return true;
	return false;
}

tasksched_filter::filter_obj_handler::base_handler::bound_string_type tasksched_filter::filter_obj_handler::bind_simple_string(std::string key) {
	base_handler::bound_string_type ret;
	if (key == "title")
		ret = &object_type::get_title;
	else if (key == "account")
		ret = &object_type::get_account_name;
	else if (key == "application")
		ret = &object_type::get_application_name;
	else if (key == "comment")
		ret = &object_type::get_comment;
	else if (key == "creator")
		ret = &object_type::get_creator;
	else if (key == "parameters")
		ret = &object_type::get_parameters;
	else if (key == "working_directory")
		ret = &object_type::get_working_directory;
 	else
		NSC_LOG_ERROR("Failed to bind (string): " + key);
	return ret;
}


tasksched_filter::filter_obj_handler::base_handler::bound_int_type tasksched_filter::filter_obj_handler::bind_simple_int(std::string key) {
	base_handler::bound_int_type ret;
	if (key == "error_retry_count")
		ret = &object_type::get_error_retry_count;
	else if (key == "error_retry_interval")
		ret = &object_type::get_error_retry_interval;
// 	else if (key == "idle_wait")
// 		ret = &object_type::get_idle_wait;
	else if (key == "exit_code")
		ret = &object_type::get_exit_code;
	else if (key == "flags")
		ret = &object_type::get_flags;
	else if (key == "max_run_time")
		ret = &object_type::get_max_run_time;
	else if (key == "priority")
		ret = &object_type::get_priority;
 	else if (key == "status")
 		ret = &object_type::get_status;
	else if (key == "most_recent_run_time")
		ret = &object_type::get_most_recent_run_time;
 	else
		NSC_LOG_ERROR("Failed to bind (int): " +key);
	return ret;
}

bool tasksched_filter::filter_obj_handler::has_function(parsers::where::value_type to, std::string name, expression_ast_type *subject) {
	if (to == type_custom_hresult)
		return true;
	return false;
}

long tasksched_filter::filter_obj::convert_status(std::string status) {
	if (status == "ready")
		return SCHED_S_TASK_READY;
	if (status == "running")
		return SCHED_S_TASK_RUNNING;
	if (status == "not_scheduled")
		return SCHED_S_TASK_NOT_SCHEDULED;
	if (status == "has_not_run")
		return SCHED_S_TASK_HAS_NOT_RUN;
	if (status == "disabled")
		return SCHED_S_TASK_DISABLED;
	if (status == "no_more_runs")
		return SCHED_S_TASK_NO_MORE_RUNS;
	if (status == "no_valid_triggers")
		return SCHED_S_TASK_NO_VALID_TRIGGERS;
	return 0;
}

std::string tasksched_filter::filter_obj::convert_status(long status) {
	std::wstring ret;
	if (status == SCHED_S_TASK_READY)
		return "ready";
	if (status == SCHED_S_TASK_RUNNING)
		return "running";
	if (status == SCHED_S_TASK_NOT_SCHEDULED)
		return "not_scheduled";
	if (status == SCHED_S_TASK_HAS_NOT_RUN)
		return "has_not_run";
	if (status == SCHED_S_TASK_DISABLED)
		return "disabled";
	if (status == SCHED_S_TASK_NO_MORE_RUNS)
		return "has_more_runs";
	if (status == SCHED_S_TASK_NO_VALID_TRIGGERS)
		return "no_valid_triggers";
	return strEx::s::xtos(status);
}

tasksched_filter::filter_obj_handler::base_handler::bound_function_type tasksched_filter::filter_obj_handler::bind_simple_function(parsers::where::value_type to, std::string name, expression_ast_type *subject) {
	base_handler::bound_function_type ret;
	if (to == type_custom_hresult)
		ret = &object_type::fun_convert_status;
	else
		NSC_LOG_ERROR("Failed to bind (function): " + name);
	return ret;
}





//////////////////////////////////////////////////////////////////////////

#define DEFINE_GET_EX(type, variable, helper, func) type tasksched_filter::filter_obj::get_ ## variable() { return helper.fetch(this, &ITask::func, variable); }

#define DEFINE_GET_STRING(variable, helper, func) DEFINE_GET_EX(std::string, variable, helper, func)
#define DEFINE_GET_DWORD(variable, helper, func) DEFINE_GET_EX(unsigned long, variable, helper, func)
#define DEFINE_GET_WORD(variable, helper, func) DEFINE_GET_EX(unsigned short, variable, helper, func)
#define DEFINE_GET_DATE(variable, helper, func) DEFINE_GET_EX(tasksched_filter::filter_obj::task_sched_date, variable, helper, func)
#define DEFINE_GET_HRESULT(variable, helper, func) DEFINE_GET_EX(long, variable, helper, func)

DEFINE_GET_STRING(account_name, string_fetcher, GetAccountInformation);
DEFINE_GET_STRING(application_name, string_fetcher, GetApplicationName);
DEFINE_GET_STRING(comment, string_fetcher, GetComment);
DEFINE_GET_STRING(creator, string_fetcher, GetCreator);
DEFINE_GET_STRING(parameters, string_fetcher, GetParameters);
DEFINE_GET_STRING(working_directory, string_fetcher, GetWorkingDirectory);

DEFINE_GET_WORD(error_retry_count, word_fetcher, GetErrorRetryCount);
DEFINE_GET_WORD(error_retry_interval, word_fetcher, GetErrorRetryInterval);
DEFINE_GET_DWORD(exit_code, dword_fetcher, GetExitCode);
DEFINE_GET_DWORD(flags, dword_fetcher, GetFlags);
DEFINE_GET_DWORD(max_run_time, dword_fetcher, GetMaxRunTime);
DEFINE_GET_DWORD(priority, dword_fetcher, GetPriority);
//DEFINE_GET_WORD(idle_wait, word_fetcher, GetIdleWait);


DEFINE_GET_HRESULT(status, hresult_fetcher, GetStatus);

DEFINE_GET_DATE(most_recent_run_time, date_fetcher, GetMostRecentRunTime);
// FETCH_TASK_SIMPLE_TIME(nextRunTime,GetNextRunTime);

tasksched_filter::filter_obj::expression_ast_type tasksched_filter::filter_obj::fun_convert_status(parsers::where::value_type target_type, parsers::where::filter_handler handler, expression_ast_type const& subject) {
	return expression_ast_type(parsers::where::int_value(convert_status(subject.get_string(handler))));
}


std::string tasksched_filter::filter_obj::render(std::string format, std::string datesyntax) {
	strEx::replace(format, "%title%", get_title());
	strEx::replace(format, "%account%", get_account_name());
	strEx::replace(format, "%application%", get_application_name());
	strEx::replace(format, "%comment%", get_comment());
	strEx::replace(format, "%creator%", get_creator());
	strEx::replace(format, "%parameters%", get_parameters());
	strEx::replace(format, "%working_directory%", get_working_directory());

	strEx::replace(format, "%exit_code%", strEx::s::xtos(get_exit_code()));
	strEx::replace(format, "%error_retry_count%", strEx::s::xtos(get_error_retry_count()));
	strEx::replace(format, "%error_retry_interval%", strEx::s::xtos(get_error_retry_interval()));
	strEx::replace(format, "%flags%", strEx::s::xtos(get_flags()));
	//strEx::replace(format, "%idle_wait%", strEx::itos(get_idle_wait()));
	strEx::replace(format, "%max_run_time%", strEx::s::xtos(get_max_run_time()));
	strEx::replace(format, "%priority%", strEx::s::xtos(get_priority()));
	strEx::replace(format, "%status%", convert_status(get_status()));

	//	strEx::replace(format, _T("%next_run%"), strEx::format_date(get_next_run()));
	if (get_most_recent_run_time()) {
		task_sched_date date = get_most_recent_run_time();
		unsigned long long t = date;
		if (t == 0 || date.never_) {
			strEx::replace(format, "%most_recent_run_time%", "never");
			strEx::replace(format, "%most_recent_run_time-raw%",  "never");
		} else {
			strEx::replace(format, "%most_recent_run_time%", format::format_date(t, datesyntax));
			strEx::replace(format, "%most_recent_run_time-raw%", strEx::s::xtos(t));
		}
	}

	strEx::replace(format, "\n", "");
	return format;
}

//////////////////////////////////////////////////////////////////////////

tasksched_filter::filter_engine tasksched_filter::factories::create_engine(tasksched_filter::filter_argument arg) {
	return filter_engine(new filter_engine_type(arg));
}
tasksched_filter::filter_argument tasksched_filter::factories::create_argument(std::string syntax, std::string datesyntax) {
	return filter_argument(new tasksched_filter::filter_argument_type(tasksched_filter::filter_argument_type::error_type(new where_filter::nsc_error_handler(GET_CORE())), syntax, datesyntax));
}

tasksched_filter::filter_result tasksched_filter::factories::create_result(tasksched_filter::filter_argument arg) {
	return filter_result(new where_filter::simple_count_result<filter_obj>(arg));
}





