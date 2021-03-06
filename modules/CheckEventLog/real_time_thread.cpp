#include <boost/foreach.hpp>

#include "real_time_thread.hpp"

#include <nscapi/nscapi_protobuf_functions.hpp>
#include <nscapi/nscapi_core_helper.hpp>
#include <nscapi/nscapi_plugin_wrapper.hpp>

#include <nscapi/macros.hpp>

#include <parsers/where/unary_fun.hpp>
#include <parsers/where/list_value.hpp>
#include <parsers/where/binary_op.hpp>
#include <parsers/where/unary_op.hpp>
#include <parsers/where/variable.hpp>

void real_time_thread::process_no_events(const filters::filter_config_object &object) {
	std::string response;
	std::string command = object.alias;
	if (!object.command.empty())
		command = object.command;
	if (!nscapi::core_helper::submit_simple_message(object.target, command, NSCAPI::returnOK, object.ok_msg, object.perf_msg, response)) {
		NSC_LOG_ERROR("Failed to submit evenhtlog result: " + response);
	}
}

void real_time_thread::process_record(const filters::filter_config_object &object, const EventLogRecord &record) {
	std::string response;
	int severity = object.severity;
	std::string command = object.alias;
	if (severity == -1) {
		NSC_LOG_ERROR("Severity not defined for: " + object.alias);
		severity = NSCAPI::returnUNKNOWN;
	}
	if (!object.command.empty())
		command = object.command;
	std::string message = record.render(true, object.syntax, object.date_format, object.dwLang);
	if (!nscapi::core_helper::submit_simple_message(object.target, command, object.severity, message, object.perf_msg, response)) {
		NSC_LOG_ERROR("Failed to submit eventlog result " + object.alias + ": " + response);
	}
}

void real_time_thread::debug_miss(const EventLogRecord &record) {
	std::string message = record.render(true, "%id% %level% %source%: %message%", DATE_FORMAT_S, LANG_NEUTRAL);
	NSC_DEBUG_MSG_STD("No filter matched: " + message);
}

void real_time_thread::thread_proc() {

	std::list<filters::filter_config_object> filters;
	std::list<std::string> logs;
	std::list<std::string> filter_list;

	BOOST_FOREACH(const std::string &s, strEx::s::splitEx(logs_, std::string(","))) {
		logs.push_back(s);
	}

	BOOST_FOREACH(filters::filter_config_object object, filters_.get_object_list()) {
		eventlog_filter::filter_argument fargs = eventlog_filter::factories::create_argument(object.syntax, object.date_format);
		fargs->filter = object.filter;
		fargs->debug = object.debug;
		fargs->alias = object.alias;
		fargs->bShowDescriptions = true;
		if (object.log_ != "any" && object.log_ != "all")
			logs.push_back(object.log_);
		// eventlog_filter::filter_engine 
		object.engine = eventlog_filter::factories::create_engine(fargs);

		if (!object.engine) {
			NSC_LOG_ERROR_WA("Invalid filter: ", object.filter);
			continue;
		}

		if (!object.engine->boot()) {
			NSC_LOG_ERROR_WA("Error booting filter: ", object.filter);
			continue;
		}

		std::string message;
		if (!object.engine->validate(message)) {
			NSC_LOG_ERROR("Error validating filter: " + message);
			continue;
		}
		filters.push_back(object);
		filter_list.push_back(object.alias);
	}
	logs.sort();
	logs.unique();
	NSC_DEBUG_MSG_STD("Scanning logs: ", format::join(logs, ", "));
	NSC_DEBUG_MSG_STD("Scanning filters: ", format::join(filter_list, ", "));

	typedef boost::shared_ptr<eventlog_wrapper> eventlog_type;
	typedef std::vector<eventlog_type> eventlog_list;
	eventlog_list evlog_list;

	BOOST_FOREACH(const std::string &l, logs) {
		eventlog_type el = eventlog_type(new eventlog_wrapper(l));
		if (!el->seek_end()) {
			NSC_LOG_ERROR_WA("Failed to find the end of eventlog: ", l);
		} else {
			evlog_list.push_back(el);
		}
	}

	// TODO: add support for scanning "missed messages" at startup

	HANDLE *handles = new HANDLE[1+evlog_list.size()];
	handles[0] = stop_event_;
	for (int i=0;i<evlog_list.size();i++) {
		evlog_list[i]->notify(handles[i+1]);
	}
	__time64_t ltime;
	_time64(&ltime);

	BOOST_FOREACH(filters::filter_config_object &object, filters) {
		object.touch(ltime);
	}

	unsigned int errors = 0;
	while (true) {

		DWORD minNext = INFINITE;
		BOOST_FOREACH(const filters::filter_config_object &object, filters) {
			NSC_DEBUG_MSG_STD("Getting next from: " + utf8::cvt<std::string>(object.alias) + ": " + strEx::s::xtos(object.next_ok_));
			if (object.next_ok_ > 0 && object.next_ok_ < minNext)
				minNext = object.next_ok_;
		}

		_time64(&ltime);

		if (ltime > minNext) {
			NSC_LOG_ERROR("Strange seems we expect to send ok now?");
			continue;
		}
		DWORD dwWaitTime = (minNext - ltime)*1000;
		if (minNext == INFINITE || dwWaitTime < 0)
			dwWaitTime = INFINITE;
		NSC_DEBUG_MSG("Next miss time is in: " + strEx::s::xtos(dwWaitTime) + "s");

		DWORD dwWaitReason = WaitForMultipleObjects(evlog_list.size()+1, handles, FALSE, dwWaitTime);
		if (dwWaitReason == WAIT_TIMEOUT) {
			// we take care of this below...
		} else if (dwWaitReason == WAIT_OBJECT_0) {
			delete [] handles;
			return;
		} else if (dwWaitReason > WAIT_OBJECT_0 && dwWaitReason <= (WAIT_OBJECT_0 + evlog_list.size())) {

			eventlog_type el = evlog_list[dwWaitReason-WAIT_OBJECT_0-1];
			DWORD status = el->read_record(0, EVENTLOG_SEQUENTIAL_READ|EVENTLOG_FORWARDS_READ);
			if (ERROR_SUCCESS != status && ERROR_HANDLE_EOF != status) {
				delete [] handles;
				return;
			}

			_time64(&ltime);

			EVENTLOGRECORD *pevlr = el->read_record_with_buffer();
			while (pevlr != NULL) {
				EventLogRecord elr(el->get_name(), pevlr, ltime);
				boost::shared_ptr<eventlog_filter::filter_obj> arg = boost::shared_ptr<eventlog_filter::filter_obj>(new eventlog_filter::filter_obj(elr));
				bool matched = false;

				BOOST_FOREACH(filters::filter_config_object &object, filters) {
					if (object.log_ != "any" && object.log_ != "all" && object.log_ != utf8::cvt<std::string>(el->get_name())) {
						NSC_DEBUG_MSG_STD("Skipping filter: " + utf8::cvt<std::string>(object.alias));
						continue;
					}
					if (object.engine->match(arg)) {
						process_record(object, elr);
						object.touch(ltime);
						matched = true;
					}
				}
				if (debug_ && !matched)
					debug_miss(elr);

				pevlr = el->read_record_with_buffer();
			}
		} else {
			NSC_LOG_ERROR_WA("Error failed to wait for eventlog message: ", error::lookup::last_error());
			if (errors++ > 10) {
				NSC_LOG_ERROR("To many errors giving up");
				delete [] handles;
				return;
			}
		}
		_time64(&ltime);
		BOOST_FOREACH(filters::filter_config_object &object, filters) {
			if (object.next_ok_ != 0 && object.next_ok_ <= (ltime+1)) {
				process_no_events(object);
				object.touch(ltime);
			} else {
				NSC_DEBUG_MSG_STD("missing: " + utf8::cvt<std::string>(object.alias) + ": " + strEx::s::xtos(object.next_ok_));
			}
		}
	}
	delete [] handles;
	return;
}


bool real_time_thread::start() {
	if (!enabled_)
		return true;

	stop_event_ = CreateEvent(NULL, TRUE, FALSE, _T("EventLogShutdown"));

	thread_ = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&real_time_thread::thread_proc, this)));
	return true;
}
bool real_time_thread::stop() {
	SetEvent(stop_event_);
	if (thread_)
		thread_->join();
	return true;
}

void real_time_thread::add_realtime_filter(boost::shared_ptr<nscapi::settings_proxy> proxy, std::string key, std::string query) {
	try {
		filters_.add(proxy, filters_path_, key, query, key == "default");
	} catch (const std::exception &e) {
		NSC_LOG_ERROR_EXR("Failed to add command: " + utf8::cvt<std::string>(key), e);
	} catch (...) {
		NSC_LOG_ERROR_EX("Failed to add command: " + utf8::cvt<std::string>(key));
	}
}
