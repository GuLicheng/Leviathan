#include <iostream>
#include <string>

#include <lv_cpp/io/console.hpp>
// #include <format>
#include <memory>
#include <iostream>

#define LV_DEBUG

#ifdef LV_DEBUG
#define INFO(x) std::cout << #x << '\n'
#else
#define INFO(x)
#endif

namespace leviathan
{

	template <typename CharT, typename CharTrait = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	class basic_string
	{
		constexpr static auto MAX_SIZE = 15 / sizeof(CharT);
		enum class State { Long = 0, Short = 1 };

		struct _LongLayout
		{
			CharT* _ptr;
			CharT* _size;
			CharT* _res;
		};

		struct _ShortLayout
		{
			CharT _buf[MAX_SIZE];
			size_t _end;
		};


	public:
		using traits_type = CharTrait;
		using value_type = CharT;
		using allocate_type = Alloc;
		using size_type = typename Alloc::size_type;
		using difference_type = typename Alloc::difference_type;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = typename std::allocator_traits<Alloc>::pointer;
		using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;;


		size_t size() const noexcept
		{
			if (is_long_layout())
				return _heap._size - _heap._ptr;
			return _stack._end;
		}

		size_t length() const noexcept
		{
			return size();
		}

		size_t capacity() const noexcept
		{
			if (is_long_layout())
				return _heap._res - _heap._ptr;
			return MAX_SIZE;
		}


		basic_string()
			: allocator{ }, state{ State::Short }
		{
			_stack._end = 0;
		}

		basic_string(const CharT* ptr)
			: allocator{ }
		{
			const auto len = CharTrait::length(ptr);
			if (len > MAX_SIZE)
			{
				state = State::Long;
				_heap._ptr = allocator.allocate(len);
				// assert(_heap._ptr != nullptr);
				CharTrait::copy(_heap._ptr, ptr, len);
				_heap._size = _heap._ptr + len;
				_heap._res = _heap._size;
			}
			else
			{
				state = State::Short;
				CharTrait::copy(_stack._buf, ptr, len);
				_stack._end = len;
			}
		}

		basic_string(basic_string&& rhs) noexcept
			: allocator{ rhs.allocator }, state{ rhs.state }
		{
			if (rhs.is_long_layout())
			{
				INFO(basic_string && with long);
				_heap._ptr = rhs._heap._ptr;
				_heap._size = rhs._heap._size;
				_heap._res = rhs._heap._res;

				rhs.reset();
			}
			else
			{
				INFO(basic_string && with short);
				const auto _size = rhs.size();
				CharTrait::copy(_stack._buf, rhs._stack._buf, _size);
				_stack._end = _size;
			}
		}


		basic_string(const basic_string& rhs)
			: allocator{ rhs.allocator }, state{ rhs.state }
		{

			const auto _size = rhs.size();
			if (rhs.is_long_layout())
			{
				INFO(basic_string const& with long);
				_heap._ptr = allocator.allocate(_size);
				CharTrait::copy(_heap._ptr, rhs._heap._ptr, _size);
				_heap._size = _heap._ptr + _size;
				_heap._res = _heap._ptr + rhs.capacity();
			}
			else
			{
				INFO(basic_string const& with short);
				CharTrait::copy(_stack._buf, rhs._stack._buf, _size);
				_stack._end = _size;
			}
		}

		basic_string& operator=(const basic_string& rhs)
		{
			this->set(rhs);
			return *this;
		}

		basic_string& operator=(basic_string&& rhs) noexcept
		{
			this->reset(true);
			state = rhs.state;
			if (rhs.is_long_layout())
			{
				INFO(basic_string && with long);
				_heap._ptr = rhs._heap._ptr;
				_heap._size = rhs._heap._size;
				_heap._res = rhs._heap._res;
				rhs.reset();
			}
			else
			{
				INFO(basic_string && with short);
				const auto _size = rhs.size();
				CharTrait::copy(_stack._buf, rhs._stack._buf, _size);
				_stack._end = _size;
			}
			return *this;
		}

		template <typename _CharT, typename _CharTraits>
		friend std::basic_ostream<_CharT, _CharTraits>& operator<<(std::basic_ostream<_CharT, _CharTraits>& os, const basic_string& string)
		{
			return os.write(string.c_str(), string.size());
		}

		bool is_long_layout() const noexcept
		{
			return state == State::Long;
		}

		CharT* date() noexcept
		{
			return static_cast<const basic_string&>(*this).data();
		}

		const CharT* data() const noexcept
		{
			if (is_long_layout())
				return _heap._ptr;
			return _stack._buf;
		}

		const CharT* c_str() const noexcept
		{
			return data();
		}

		void push_back(CharT ch)
		{
			// FIXME...
			const auto _size = size();
			const auto _capacity = capacity();

			if (!is_long_layout() && _size + 1 <= _capacity)
			{
				// just assign
				CharTrait::assign(_stack._buf[_stack._end++], ch);
			}
			else if (!is_long_layout())
			{
				// convert to long, assert (_capacity << 1 < max of size_t)
				auto new_ptr = allocator.allocate(_capacity << 1);
				CharTrait::copy(new_ptr, _stack._buf, _size);
				CharTrait::assign(new_ptr[_size], ch);
				state = State::Long;
				_heap._ptr = new_ptr;
				_heap._size = new_ptr + _size + 1;
				_heap._res = new_ptr + (MAX_SIZE << 1);
			}
			else if (_size + 1 <= _capacity)
			{
				// long layout
				CharTrait::assign(*_heap._size, ch);
				_heap._size++;
			}
			else
			{
				// long and expend
				auto new_ptr = allocator.allocate(_capacity << 1);
				CharTrait::copy(new_ptr, _heap._ptr, _size);
				allocator.deallocate(_heap._ptr, _size);
				_heap._ptr = new_ptr;
				_heap._size = new_ptr + _size + 1;
				_heap._res = new_ptr + (_capacity << 1);
			}

		}

		void pop_back()
		{
			if (is_long_layout())
			{
				_heap._size--;
			}
			else
			{
				_stack._end--;
			}
		}


	private:

		void reset(bool isfree = false) noexcept
		{
			if (isfree && is_long_layout())
				allocator.deallocate(_heap._ptr, size());
			state = State::Short;
			_stack._end = 0;
		}

		void set(const basic_string& rhs)
		{
			const auto _size = rhs.size();
			if (is_long_layout() && rhs.is_long_layout())
			{
				allocator.deallocate(_heap._ptr, size());
				_heap._ptr = allocator.allocate(_size);
				CharTrait::copy(_heap._ptr, rhs._heap._ptr, _size);
				_heap._size = _heap._res = _heap._ptr + _size;
			}
			else if (is_long_layout())
			{
				// rhs is short layout
				allocator.deallocate(_heap._ptr, size());
				_stack._end = _size;
				CharTrait::copy(_stack._buf, rhs._heap._ptr, _size);
			}
			else if (rhs.is_long_layout())
			{
				// rhs is long out
				_heap._ptr = allocator.allocate(_size);
				CharTrait::copy(_heap._ptr, rhs._heap._ptr, _size);
				_heap._size = _heap._res = _heap._ptr + _size;
			}
			else
			{
				CharTrait::copy(_stack._buf, rhs._stack._buf, _size);
				_stack._end = _size;
			}
			state = rhs.state;
		}

		State state;
		union
		{
			_LongLayout _heap;
			_ShortLayout _stack;
		};

		[[no_unique_address]] Alloc allocator;
	};

} // end of namespace



#if 0

void test_init()
{
	leviathan::basic_string<char> s1{ "hello world 123" };
	std::cout << s1.size() << std::endl;
	leviathan::basic_string<char> s2 = { "hello world 1234" };
	std::cout << s2.size() << std::endl;
	leviathan::basic_string<char> s3{ std::move(s1) };
	leviathan::basic_string<char> s4{ std::move(s2) };
	std::cout << s1 << '|' << s2 << std::endl;

	std::cout << s3 << '|' << s4 << std::endl;

	s3 = "1234 hello world-----";
	s4 = "12345";
	std::cout << s3 << '|' << s4 << std::endl;

	std::cout << s3.capacity() << std::endl;
	std::cout << s4.capacity() << std::endl;
}

void test_add()
{
	auto info = [](auto& s)
	{
		// std::cout << std::format("the size is {}, capacity is {},"
		// 	"is long layout ? {} and value is ", 
		// 	s.size(), s.capacity(), s.is_long_layout());
		// std::cout << s << std::endl;
		console::write_line("the size is {0}, capacity is {1}, is long layout ? {2} and value is {3}",
		s.size(), s.capacity(), s.is_long_layout(), s);
	};
	leviathan::basic_string<char> string;
	for (int i = 0; i < 20; ++i)
	{
		info(string);
		string.push_back(i + 'a');
	}

	for (int i = 0; i < 10; ++i)
	{
		string.pop_back();
		info(string);
	}

}




int main()
{
	//test_init();
	test_add();
}

#endif