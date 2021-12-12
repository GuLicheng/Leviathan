#pragma once

#include "logging.hpp"

#include <fstream>
#include <syncstream>
#include <string_view>
#include <ios>
#include <memory>

namespace leviathan::logging
{
    
    template <typename FileHandler>
    class file_handler_impl
    {
    public:

        file_handler_impl() = default;
        file_handler_impl(const file_handler_impl&) = delete;
        file_handler_impl(file_handler_impl&&) = default;
        file_handler_impl& operator=(const file_handler_impl&) = delete;
        file_handler_impl& operator=(file_handler_impl&&) = default;

		auto& handler() { return static_cast<FileHandler&>(*this); }
		auto& handler() const { return static_cast<const FileHandler&>(*this); }

		template <typename... Args>
		void log(level lv, std::string_view fmt, const Args&... args)
		{
			// auto record = log_record {
			// 	.m_message = std::format(fmt, args...),
			// 	.m_lv = lv
			// };
			// handler().fmt().format(handler().stream(), record);
		}
    };

    class file_handler : public file_handler_impl<file_handler>
    {
    public:
        explicit file_handler(
            std::string file,
            std::ios::openmode mode = std::ios::binary | std::ios::out | std::ios::app,
            std::string name = "")
            : m_filename{std::move(file)}, m_mode{mode}, m_name{std::move(name)}
        {
            init();
        }

        void init() { m_os.open(m_filename, m_mode); }	

        template <typename T>
        file_handler& operator<<(const T& val)
        {
            m_os << val;
            return *this;
        }

    private:	
        std::string m_filename;
		std::ios::openmode m_mode;
		std::string m_name;
		std::ofstream m_os;
		// std::unique_ptr<log_formatter> m_fmt = std::make_unique<log_formatter>();
    };


    class sync_file_handler : public file_handler_impl<sync_file_handler>
    {
    public:
        explicit sync_file_handler(std::string file, std::ios::openmode mode = std::ios::binary | std::ios::out | std::ios::app,
                                   std::string name = "")
            : m_filename{std::move(file)}, m_mode{mode}, m_name{std::move(name)}, m_sync{ m_os }
        {
            init();
        }

        void init() { m_os.open(m_filename, m_mode); }	

        template <typename T>
        sync_file_handler& operator<<(const T& val)
        {
            m_os << val;
            return *this;
        }

    protected:
        std::string m_filename;
		std::ios::openmode m_mode;
		std::string m_name;
		std::ofstream m_os;
        std::osyncstream m_sync;
		// std::unique_ptr<log_formatter> m_fmt = std::make_unique<log_formatter>();
    };

} // namespace leviathan::logging

