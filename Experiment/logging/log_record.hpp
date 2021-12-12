#pragma once

#include <chrono>
#include <string>
#include <thread>
#include <iostream>

namespace leviathan::logging
{
	enum class level : unsigned
	{
		CRITICAL,
		FATAL,
		ERROR,
		WARN,
		INFO,
		DEBUG,
		NOTSET
	};

    constexpr const char* LevelName[] = {
        "CRITICAL",
        "FATAL",
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG",
        "NOTEST"
    };

	using clock = std::chrono::system_clock;
	const inline clock::time_point StartTime = clock::now();

	constexpr const char* get_level_name(level lv)
	{
		return LevelName[static_cast<int>(lv)];
	}

	struct log_record
	{
		clock::time_point m_tp = clock::now();
		std::string m_message;
		std::thread::id m_id;
		level m_lv;

		friend std::ostream& operator<<(std::ostream& os, const log_record& record)
		{
			return os << record.m_message << " " << record.m_id << " " << get_level_name(record.m_lv);
		}
	};

}