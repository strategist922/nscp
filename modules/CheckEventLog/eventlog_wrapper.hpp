#pragma once
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

struct eventlog_wrapper {
	HANDLE hLog;
	LPBYTE pBuffer;
	DWORD bufferSize;
	DWORD lastReadSize;
	DWORD nextBufferPosition;
	std::string name;
	eventlog_wrapper(const std::string &name);
	~eventlog_wrapper();

	void open(const std::string &name);
	void close();
	bool isOpen() {
		return hLog != NULL;
	}
	std::string get_name() { return name; }

	bool get_last_Record_number(DWORD* pdwRecordNumber);
	bool notify(HANDLE &handle);
	bool seek_end();
	static std::string find_eventlog_name(std::string name);
	DWORD read_record(DWORD dwRecordNumber, DWORD dwFlags);
	EVENTLOGRECORD* read_record_with_buffer();
	operator HANDLE () {
		return hLog;
	}

	DWORD get_last_buffer_size() { 
		return lastReadSize; 
	}
	const LPBYTE get_last_buffer() {
		return pBuffer;
	}
	void resize_buffer(int size);
};

struct event_source {
	HANDLE hLog;
	event_source(const std::wstring &source);
	~event_source();

	void open(const std::wstring &server, const std::wstring &name);
	void close();
	bool isOpen() {
		return hLog != NULL;
	}

	operator HANDLE () {
		return hLog;
	}

};
