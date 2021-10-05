// https://www.codeproject.com/Articles/2728/C-integer-Class

#include <iostream>
#include <assert.h>
#include <vector>     // std::vector stores number
#include <exception>  // std::exception for class bad_arithmetic
#include <string>     // std::string for to_string()
#include <string_view>
#include <compare>    // for std::strong_ordering, if your comlipier not support C++20, just disable it


namespace leviathan::math
{


	class bad_arithmetic : public std::exception
	{
	public:

		bad_arithmetic(const char* msg = "") : msg{ msg }
		{
		}

		const char* what() const noexcept override
		{
			return msg;
		}

	private:
		const char* msg;
	};

	template <size_t BitWidth = 70>
	class integer
	{

		// maximum length of the big integer in uint(4 bytes)
		// change this to suit the required level of precision
		// total bits equal to MaxLength * 4 * 8

		using uint = unsigned int;
		using ulong = unsigned long long;
		using llong = long long;

		std::vector<uint> m_data;  // stores bytes from the big integer
		int m_length;  // number of actual chars used

	public:
		constexpr static int MaxLength = BitWidth;

		/**
		 *  Constructor (Default value is 0)
		 */
		integer()
			: m_length{ 1 }
		{
			this->m_data.resize(MaxLength);
		}

		// not support
		explicit integer(std::vector<uint> inData)
			: m_data{ std::move(inData) }
		{
			m_data.resize(MaxLength);
			m_length = (int)m_data.size();
			while (m_length > 1 && m_data[m_length - 1] == 0)
				m_length--;
		}

		// explicit avoid implicit convert when debugging 
		explicit integer(llong value)
			: m_length{ 0 }
		{
			this->m_data.resize(MaxLength);
			llong tempVal = value;

			// copy bytes from long to integer without any assumption of
			// the length of the long datatype

			while (value != 0 && m_length < MaxLength)
			{
				m_data[m_length] = (uint)(value & 0xFFFFFFFF);
				value >>= 32;
				m_length++;
			}

			if (tempVal > 0)         // overflow check for +ve value
			{
				if (value != 0 || (m_data[MaxLength - 1] & 0x80000000) != 0)
					throw bad_arithmetic{ "Positive overflow in constructor." };
			}
			else if (tempVal < 0)    // underflow check for -ve value
			{
				if (value != -1 || (m_data[m_length - 1] & 0x80000000) == 0)
					throw bad_arithmetic{ "Negative underflow in constructor." };
			}

			if (m_length == 0)
				m_length = 1;
		}

		explicit integer(std::string value, int radix = 10)
		{

			integer multiplier{ 1 };
			integer result;
			// value = (value.ToUpper()).Trim();
			// value = value | trim | transform(::toupper) | to<string>()
			// trim = drop_while(::isblank) | reverse | drop_while(::isblank)

			int limit = 0;

			if (value[0] == '-')
				limit = 1;

			for (int i = (int)value.size() - 1; i >= limit; i--)
			{
				int posVal = (int)value[i];

				if (posVal >= '0' && posVal <= '9')
					posVal -= '0';
				else if (posVal >= 'A' && posVal <= 'Z')
					posVal = (posVal - 'A') + 10;
				else
					posVal = 9999999;       // arbitrary large

				if (posVal >= radix)
					throw bad_arithmetic{ "Invalid string in constructor." };
				else
				{
					if (value[0] == '-')
						posVal = -posVal;

					result = result + (multiplier * static_cast<integer>(posVal));

					if ((i - 1) >= limit)
						multiplier = multiplier * static_cast<integer>(radix);
				}
			}

			if (value[0] == '-')     // negative values
			{
				if ((result.m_data[MaxLength - 1] & 0x80000000) == 0)
					throw bad_arithmetic{ "Negative underflow in constructor." };
			}
			else    // positive values
			{
				if ((result.m_data[MaxLength - 1] & 0x80000000) != 0)
					throw bad_arithmetic{ "Positive overflow in constructor." };
			}

			//m_data = new uint[MaxLength];
			m_data = std::vector<uint>(MaxLength);
			for (int i = 0; i < result.m_length; i++)
				m_data[i] = result.m_data[i];

			m_length = result.m_length;
		}

		std::string to_string(int radix = 10) const
		{
			if (radix < 2 || radix > 36)
				throw bad_arithmetic{ "Radix must be >= 2 and <= 36" };

			static const char charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
			std::string result;

			integer a = *this;

			bool negative = false;
			if ((m_data[MaxLength - 1] & 0x80000000) != 0)
			{
				negative = true;
				try
				{
					a = -a;
				}
				catch (...)
				{
				}
			}

			integer quotient;
			integer remainder;
			integer biRadix = integer{ radix };

			if (a.m_length == 1 && a.m_data[0] == 0)
				result = "0";
			else
			{
				while (a.m_length > 1 || (a.m_length == 1 && a.m_data[0] != 0))
				{
					single_byte_divide(std::move(a), biRadix, quotient, remainder);
					if (remainder.m_data[0] < 10)
						// result = remainder.m_data[0] + result;
						result += remainder.m_data[0] + '0';
					else
						// result = charSet[(int)remainder.m_data[0] - 10] + result;
						result += charSet[(int)remainder.m_data[0] - 10];

					a = quotient;
				}
				if (negative)
					// result = "-" + result;
					result += '-';
			}
			return { result.rbegin(), result.rend() };
		}



		integer operator-() const
		{
			// handle neg of zero separately since it'll cause an overflow
			// if we proceed.
			if (this->m_length == 1 && this->m_data[0] == 0)
				return integer{};

			integer result = integer(*this);

			// Total process: -x = ~x + 1
			// 1's complement
			for (int i = 0; i < MaxLength; i++)
				result.m_data[i] = ~(this->m_data[i]);

			// add one to result of 1's complement
			llong val, carry = 1;
			int index = 0;

			while (carry != 0 && index < MaxLength)
			{
				val = (llong)(result.m_data[index]);
				val++;

				result.m_data[index] = (uint)(val & 0xFFFFFFFF);
				carry = val >> 32;

				index++;
			}

			if ((m_data[MaxLength - 1] & 0x80000000) == (result.m_data[MaxLength - 1] & 0x80000000))
				throw bad_arithmetic{ "Overflow in negation.\n" };

			result.m_length = MaxLength;

			while (result.m_length > 1 && result.m_data[result.m_length - 1] == 0)
				result.m_length--;
			return result;
		}

		integer operator+(const integer& rhs) const
		{
			integer result;

			result.m_length = (this->m_length > rhs.m_length) ? this->m_length : rhs.m_length;

			llong carry = 0;
			for (int i = 0; i < result.m_length; i++)
			{
				llong sum = (llong)this->m_data[i] + (llong)rhs.m_data[i] + carry;
				carry = sum >> 32;
				result.m_data[i] = (uint)(sum & 0xFFFFFFFF);
			}

			if (carry != 0 && result.m_length < MaxLength)
			{
				result.m_data[result.m_length] = (uint)(carry);
				result.m_length++;
			}

			while (result.m_length > 1 && result.m_data[result.m_length - 1] == 0)
				result.m_length--;


			// overflow check
			int lastPos = MaxLength - 1;
			if ((this->m_data[lastPos] & 0x80000000) == (rhs.m_data[lastPos] & 0x80000000) &&
				(result.m_data[lastPos] & 0x80000000) != (this->m_data[lastPos] & 0x80000000))
			{
				throw bad_arithmetic{};
			}

			return result;
		}
		integer operator-(const integer& rhs) const
		{
			integer result;

			result.m_length = (this->m_length > rhs.m_length) ? this->m_length : rhs.m_length;

			llong carryIn = 0;
			for (int i = 0; i < result.m_length; i++)
			{
				llong diff;

				diff = (llong)this->m_data[i] - (llong)rhs.m_data[i] - carryIn;
				result.m_data[i] = (uint)(diff & 0xFFFFFFFF);

				if (diff < 0)
					carryIn = 1;
				else
					carryIn = 0;
			}

			// roll over to negative
			if (carryIn != 0)
			{
				for (int i = result.m_length; i < MaxLength; i++)
					result.m_data[i] = 0xFFFFFFFF;
				result.m_length = MaxLength;
			}

			// fixed in v1.03 to give correct datalength for a - (-b)
			while (result.m_length > 1 && result.m_data[result.m_length - 1] == 0)
				result.m_length--;

			// overflow check

			int lastPos = MaxLength - 1;
			if ((this->m_data[lastPos] & 0x80000000) != (rhs.m_data[lastPos] & 0x80000000) &&
				(result.m_data[lastPos] & 0x80000000) != (this->m_data[lastPos] & 0x80000000))
			{
				throw bad_arithmetic{};
			}

			return result;
		}
		integer operator*(const integer& rhs) const
		{
			int lastPos = MaxLength - 1;
			bool bi1Neg = false, bi2Neg = false;
			integer b1 = *this;
			integer b2 = rhs;
			// take the absolute value of the inputs
			try
			{
				if ((b1.m_data[lastPos] & 0x80000000) != 0)     // b1 negative
				{
					bi1Neg = true; b1 = -b1;
				}
				if ((b2.m_data[lastPos] & 0x80000000) != 0)     // b2 negative
				{
					bi2Neg = true; b2 = -b2;
				}
			}
			catch (const std::exception&) {}

			integer result;

			// multiply the absolute values
			try
			{
				for (int i = 0; i < b1.m_length; i++)
				{
					if (b1.m_data[i] == 0)    continue;

					ulong mcarry = 0;
					for (int j = 0, k = i; j < b2.m_length; j++, k++)
					{
						// k = i + j
						ulong val = ((ulong)b1.m_data[i] * (ulong)b2.m_data[j]) +
							(ulong)result.m_data[k] + mcarry;

						result.m_data[k] = (uint)(val & 0xFFFFFFFF);
						mcarry = (val >> 32);
					}

					if (mcarry != 0)
						result.m_data[i + b2.m_length] = (uint)mcarry;
				}
			}
			catch (const std::exception&)
			{
				throw bad_arithmetic{ "Multiplication overflow." };
			}


			result.m_length = b1.m_length + b2.m_length;
			if (result.m_length > MaxLength)
				result.m_length = MaxLength;

			while (result.m_length > 1 && result.m_data[result.m_length - 1] == 0)
				result.m_length--;

			// overflow check (result is -ve)
			if ((result.m_data[lastPos] & 0x80000000) != 0)
			{
				if (bi1Neg != bi2Neg && result.m_data[lastPos] == 0x80000000)    // different sign
				{
					// handle the special case where multiplication produces
					// a max negative number in 2's complement.

					if (result.m_length == 1)
						return result;
					else
					{
						bool isMaxNeg = true;
						for (int i = 0; i < result.m_length - 1 && isMaxNeg; i++)
						{
							if (result.m_data[i] != 0)
								isMaxNeg = false;
						}

						if (isMaxNeg)
							return result;
					}
				}

				throw bad_arithmetic{ "Multiplication overflow." };
			}

			// if input has different signs, then result is -ve
			if (bi1Neg != bi2Neg)
				return -result;

			return result;
		}
		integer operator/(const integer& rhs) const
		{
			integer quotient;
			integer remainder;

			integer bi1 = *this;
			integer bi2 = rhs;
			int lastPos = MaxLength - 1;
			bool divisorNeg = false, dividendNeg = false;

			if ((bi1.m_data[lastPos] & 0x80000000) != 0)     // bi1 negative
			{
				bi1 = -bi1;
				dividendNeg = true;
			}
			if ((bi2.m_data[lastPos] & 0x80000000) != 0)     // bi2 negative
			{
				bi2 = -bi2;
				divisorNeg = true;
			}

			if (bi1 < bi2)
			{
				return quotient;
			}

			else
			{
				if (bi2.m_length == 1)
					single_byte_divide(std::move(bi1), std::move(bi2), quotient, remainder);
				else
					multi_byte_divide(std::move(bi1), std::move(bi2), quotient, remainder);

				if (dividendNeg != divisorNeg)
					return -quotient;

				return quotient;
			}
		}
		integer operator%(const integer& rhs) const
		{
			integer bi1 = *this;
			integer bi2 = rhs;
			integer quotient;
			integer remainder{ bi1 };

			int lastPos = MaxLength - 1;
			bool dividendNeg = false;

			if ((bi1.m_data[lastPos] & 0x80000000) != 0)     // bi1 negative
			{
				bi1 = -bi1;
				dividendNeg = true;
			}
			if ((bi2.m_data[lastPos] & 0x80000000) != 0)     // bi2 negative
				bi2 = -bi2;

			if (bi1 < bi2)
			{
				return remainder;
			}

			else
			{
				if (bi2.m_length == 1)
					single_byte_divide(std::move(bi1), std::move(bi2), quotient, remainder);
				else
					multi_byte_divide(std::move(bi1), std::move(bi2), quotient, remainder);

				if (dividendNeg)
					return -remainder;

				return remainder;
			}
		}

		// Bit operation 
		integer operator&(const integer& rhs) const
		{
			integer result;
			int len = (this->m_length > rhs.m_length) ? this->m_length : rhs.m_length;
			for (int i = 0; i < len; i++)
			{
				uint sum = this->m_data[i] & rhs.m_data[i];
				result.m_data[i] = sum;
			}
			result.m_length = MaxLength;
			while (result.m_length > 1 && result.m_data[result.m_length - 1] == 0)
				result.m_length--;
			return result;
		}
		integer operator^(const integer& rhs) const
		{
			integer result;
			int len = (this->m_length > rhs.m_length) ? this->m_length : rhs.m_length;
			for (int i = 0; i < len; i++)
			{
				uint sum = this->m_data[i] ^ rhs.m_data[i];
				result.m_data[i] = sum;
			}
			result.m_length = MaxLength;
			while (result.m_length > 1 && result.m_data[result.m_length - 1] == 0)
				result.m_length--;
			return result;
		}
		integer operator|(const integer& rhs) const
		{
			integer result;
			int len = (this->m_length > rhs.m_length) ? this->m_length : rhs.m_length;
			for (int i = 0; i < len; i++)
			{
				uint sum = this->m_data[i] | rhs.m_data[i];
				result.m_data[i] = sum;
			}
			result.m_length = MaxLength;
			while (result.m_length > 1 && result.m_data[result.m_length - 1] == 0)
				result.m_length--;
			return result;
		}
		integer operator~() const
		{
			integer result = *this;;

			for (int i = 0; i < MaxLength; i++)
				result.m_data[i] = ~(this->m_data[i]);

			result.m_length = MaxLength;

			while (result.m_length > 1 && result.m_data[result.m_length - 1] == 0)
				result.m_length--;

			return result;
		}

		// shift operation
		integer operator<<(int value) const
		{
			integer result = *this;
			result.m_length = shift_left(result.m_data, value);
			return result;
		}
		integer operator>>(int value) const
		{
			integer result = *this;
			result.m_length = shift_right(result.m_data, value);
			return result;
		}

		// op=
		integer& operator+=(const integer& rhs)
		{
			*this = this->operator+(rhs);
			return *this;
		}
		integer& operator-=(const integer& rhs)
		{
			*this = this->operator-(rhs);
			return *this;
		}
		integer& operator*=(const integer& rhs)
		{
			*this = this->operator*(rhs);
			return *this;
		}
		integer& operator/=(const integer& rhs)
		{
			*this = this->operator/(rhs);
			return *this;
		}
		integer& operator%=(const integer& rhs)
		{
			*this = this->operator%(rhs);
			return *this;
		}
		integer& operator&=(const integer& rhs)
		{
			*this = this->operator&(rhs);
			return *this;
		}
		integer& operator|=(const integer& rhs)
		{
			*this = this->operator|(rhs);
			return *this;
		}
		integer& operator^=(const integer& rhs)
		{
			*this = this->operator^(rhs);
			return *this;
		}
		integer& operator>>=(int value)
		{
			*this = this->operator>>(value);
			return *this;
		}
		integer& operator<<=(int value)
		{
			*this = this->operator<<(value);
			return *this;
		}

		integer& operator++()
		{
			integer b1 = *this;
			integer b2{ 1 };
			*this = b1 + b2;
			return *this;
		}
		integer operator++(int)
		{
			integer old = *this;
			++(*this);
			return old;
		}
		integer& operator--()
		{
			integer b1 = *this;
			integer b2{ 1 };
			*this = b1 - b2;
			return *this;
		}
		integer operator--(int)
		{
			integer old = *this;
			--(*this);
			return old;
		}

		// Compare
		bool operator==(const integer& rhs) const
		{
			if (this->m_length != rhs.m_length)
				return false;
			for (int i = 0; i < this->m_length; ++i)
				if (this->m_data[i] != rhs.m_data[i])
					return false;
			return true;
		}
		bool operator!=(const integer& rhs) const
		{
			return this->operator!=(rhs);
		}
		bool operator<(const integer& rhs) const
		{
			int pos = MaxLength - 1;

			// b1 is negative, b2 is positive
			if ((this->m_data[pos] & 0x80000000) != 0 && (rhs.m_data[pos] & 0x80000000) == 0)
				return true;

			// b1 is positive, b2 is negative
			else if ((this->m_data[pos] & 0x80000000) == 0 && (rhs.m_data[pos] & 0x80000000) != 0)
				return false;

			// same sign
			int len = (this->m_length > rhs.m_length) ? this->m_length : rhs.m_length;
			for (pos = len - 1; pos >= 0 && this->m_data[pos] == rhs.m_data[pos]; pos--);

			if (pos >= 0)
			{
				if (this->m_data[pos] < rhs.m_data[pos])
					return true;
				return false;
			}
			return false;
		}
		bool operator<=(const integer& rhs) const
		{
			return !(this->operator>(rhs));
		}
		bool operator>=(const integer& rhs) const
		{
			return !(this->operator<(rhs));
		}
		bool operator>(const integer& rhs) const
		{
			return rhs.operator<(*this);
		}

		// An efficiently way to implement <=> in C++20
		auto operator<=>(const integer& rhs) const
		{
			int pos = MaxLength - 1;

			// b1 is negative, b2 is positive
			if ((this->m_data[pos] & 0x80000000) != 0 && (rhs.m_data[pos] & 0x80000000) == 0)
				return std::strong_ordering::less;

			// b1 is positive, b2 is negative
			else if ((this->m_data[pos] & 0x80000000) == 0 && (rhs.m_data[pos] & 0x80000000) != 0)
				return std::strong_ordering::greater;

			// same sign
			int len = (this->m_length > rhs.m_length) ? this->m_length : rhs.m_length;
			for (pos = len - 1; pos >= 0 && this->m_data[pos] == rhs.m_data[pos]; pos--);

			return pos >= 0 ? this->m_data[pos] <=> rhs.m_data[pos] : std::strong_ordering::equal;
		}


		// Other methods
		integer gcd(const integer& bi) const
		{
			integer x;
			integer y;

			if ((m_data[MaxLength - 1] & 0x80000000) != 0)     // negative
				x = -(*this);
			else
				x = *this;

			if ((bi.m_data[MaxLength - 1] & 0x80000000) != 0)     // negative
				y = -bi;
			else
				y = bi;

			integer g = y;

			while (x.m_length > 1 || (x.m_length == 1 && x.m_data[0] != 0))
			{
				g = x;
				x = y % x;
				y = g;
			}

			return g;
		}

		//***********************************************************************
		// Returns the position of the most significant bit in the integer.
		//
		// Eg.  The result is 0, if the value of integer is 0...0000 0000
		//      The result is 1, if the value of integer is 0...0000 0001
		//      The result is 2, if the value of integer is 0...0000 0010
		//      The result is 2, if the value of integer is 0...0000 0011
		//
		//***********************************************************************
		int bit_count() const
		{
			auto length = m_length;
			while (length > 1 && m_data[length - 1] == 0)
				length--;

			uint value = m_data[length - 1];
			uint mask = 0x80000000;
			int bits = 32;

			while (bits > 0 && (value & mask) == 0)
			{
				bits--;
				mask >>= 1;
			}
			bits += ((length - 1) << 5);

			return bits;
		}

		bool is_zero() const
		{
			return m_length == 1 && m_data[0] == 0;
		}

		// zero is positive
		bool positive() const
		{
			return (m_data.back() & 0x80000000) == 0;
		}

		bool negative() const
		{
			return !positive();
		}

		static integer max_value()
		{
			integer value;
			for (auto& val : value.m_data)
				val = 0xFFFFFFFF;
			value.m_data[MaxLength - 1] &= 0x7FFFFFFF;
			value.m_length = MaxLength;
			return value;
		}

		static integer min_value()
		{
			integer value;
			value.m_data[MaxLength - 1] = 0x80000000;
			value.m_length = MaxLength;
			return value;
		}

		integer mod_pow(const integer& exp, integer n) const
		{
			if ((exp.m_data[MaxLength - 1] & 0x80000000) != 0)
				throw bad_arithmetic{ "Positive exponents only." };

			integer resultNum{ 1 };
			integer tempNum;
			bool thisNegative = false;

			if ((m_data[MaxLength - 1] & 0x80000000) != 0)   // negative this
			{
				tempNum = -(*this) % n;
				thisNegative = true;
			}
			else
				tempNum = (*this) % n;  // ensures (tempNum * tempNum) < b^(2k)

			if ((n.m_data[MaxLength - 1] & 0x80000000) != 0)   // negative n
				n = -n;

			// calculate constant = b^(2k) / m
			integer constant;

			int i = n.m_length << 1;
			constant.m_data[i] = 0x00000001;
			constant.m_length = i + 1;

			constant = constant / n;
			int totalBits = exp.bit_count();
			int count = 0;

			// perform squaring and multiply exponentiation
			for (int pos = 0; pos < exp.m_length; pos++)
			{
				uint mask = 0x01;

				for (int index = 0; index < 32; index++)
				{
					if ((exp.m_data[pos] & mask) != 0)
						resultNum = barrett_reduction(resultNum * tempNum, n, constant);

					mask <<= 1;

					tempNum = barrett_reduction(tempNum * tempNum, n, constant);


					if (tempNum.m_length == 1 && tempNum.m_data[0] == 1)
					{
						if (thisNegative && (exp.m_data[0] & 0x1) != 0)    //odd exp
							return -resultNum;
						return resultNum;
					}
					count++;
					if (count == totalBits)
						break;
				}
			}

			if (thisNegative && (exp.m_data[0] & 0x1) != 0)    //odd exp
				return -resultNum;

			return resultNum;
		}

		integer sqrt() const
		{
			if (is_zero())
				return { };
			uint numBits = (uint)this->bit_count();

			if ((numBits & 0x1) != 0)        // odd number of bits
				numBits = (numBits >> 1) + 1;
			else
				numBits = (numBits >> 1);

			uint bytePos = numBits >> 5;
			unsigned char bitPos = (unsigned char)(numBits & 0x1F);

			uint mask;

			integer result;
			if (bitPos == 0)
				mask = 0x80000000;
			else
			{
				mask = (uint)1 << bitPos;
				bytePos++;
			}
			result.m_length = (int)bytePos;

			for (int i = (int)bytePos - 1; i >= 0; i--)
			{
				while (mask != 0)
				{
					// guess
					result.m_data[i] ^= mask;

					// undo the guess if its square is larger than this
					if ((result * result) > (*this))
						result.m_data[i] ^= mask;

					mask >>= 1;
				}
				mask = 0x80000000;
			}
			return result;
		}

		integer mod_inverse(const integer& modulus) const
		{
			//integer[] p = { 0, 1 };
			//integer[] q = new integer[2];    // quotients
			//integer[] r = { 0, 0 };             // remainders

			integer p[] = { integer{0}, integer{1} };
			integer q[] = { integer{}, integer{} };
			integer r[] = { integer{}, integer{} };
			int step = 0;

			integer a = modulus;
			integer b = *this;

			while (b.m_length > 1 || (b.m_length == 1 && b.m_data[0] != 0))
			{
				integer quotient;
				integer remainder;

				if (step > 1)
				{
					integer pval = (p[0] - (p[1] * q[0])) % modulus;
					p[0] = p[1];
					p[1] = pval;
				}

				if (b.m_length == 1)
					single_byte_divide(a, b, quotient, remainder);
				else
					multi_byte_divide(a, b, quotient, remainder);

				/*
				Console.WriteLine(quotient.m_length);
				Console.WriteLine("{0} = {1}({2}) + {3}  p = {4}", a.to_string(10),
								  b.to_string(10), quotient.to_string(10), remainder.to_string(10),
								  p[1].to_string(10));
				*/

				q[0] = q[1];
				r[0] = r[1];
				q[1] = quotient; r[1] = remainder;

				a = b;
				b = remainder;

				step++;
			}

			if (r[0].m_length > 1 || (r[0].m_length == 1 && r[0].m_data[0] != 1))
				throw (new bad_arithmetic("No inverse!"));

			integer result = ((p[0] - (p[1] * q[0])) % modulus);

			if ((result.m_data[MaxLength - 1] & 0x80000000) != 0)
				result = result + modulus;  // get the least positive modulus

			return result;
		}

		static int jacobi(integer a, integer b)
		{
			// jacobi defined only for odd integers
			if ((b.m_data[0] & 0x1) == 0)
				throw bad_arithmetic{ "jacobi defined only for odd integers." };

			if (a >= b)
				a = a % b;
			if (a.m_length == 1 && a.m_data[0] == 0)      return 0;  // a == 0
			if (a.m_length == 1 && a.m_data[0] == 1)      return 1;  // a == 1

			if (a < static_cast<integer>(0))
			{
				if ((((b - static_cast<integer>(1)).m_data[0]) & 0x2) == 0)       //if( (((b-1) >> 1).m_data[0] & 0x1) == 0)
					return jacobi(-a, b);
				else
					return -jacobi(-a, b);
			}

			int e = 0;
			for (int index = 0; index < a.m_length; index++)
			{
				uint mask = 0x01;

				for (int i = 0; i < 32; i++)
				{
					if ((a.m_data[index] & mask) != 0)
					{
						index = a.m_length;      // to break the outer loop
						break;
					}
					mask <<= 1;
					e++;
				}
			}

			integer a1 = a >> e;

			int s = 1;
			if ((e & 0x1) != 0 && ((b.m_data[0] & 0x7) == 3 || (b.m_data[0] & 0x7) == 5))
				s = -1;

			if ((b.m_data[0] & 0x3) == 3 && (a1.m_data[0] & 0x3) == 3)
				s = -s;

			if (a1.m_length == 1 && a1.m_data[0] == 1)
				return s;
			else
				return (s * jacobi(b % a1, a1));
		}

		template <typename RandomDevice>
		void gen_random_bits(int bits, RandomDevice& rd)
		{
			int dwords = bits >> 5;
			int remBits = bits & 0x1F;

			if (remBits != 0)
				dwords++;

			if (dwords > MaxLength)
				throw bad_arithmetic{ "Number of required bits > MaxLength." };

			for (int i = 0; i < dwords; i++)
				m_data[i] = (uint)(rd() * 0x100000000);

			for (int i = dwords; i < MaxLength; i++)
				m_data[i] = 0;

			if (remBits != 0)
			{
				uint mask = (uint)(0x01 << (remBits - 1));
				m_data[dwords - 1] |= mask;

				mask = (uint)(0xFFFFFFFF >> (32 - remBits));
				m_data[dwords - 1] &= mask;
			}
			else
				m_data[dwords - 1] |= 0x80000000;

			m_length = dwords;

			if (m_length == 0)
				m_length = 1;
		}

		//***********************************************************************
		// Generates a random number with the specified number of bits such
		// that gcd(number, this) = 1
		//***********************************************************************
		template <typename RandomDevice>
		integer gen_coprime(int bits, RandomDevice& rand)
		{
			bool done = false;
			integer result;

			while (!done)
			{
				result.gen_random_bits(bits, rand);
				//Console.WriteLine(result.ToString(16));

			// gcd test
				integer g = result.gcd(*this);
				if (g.m_length == 1 && g.m_data[0] == 1)
					done = true;
			}

			return result;
		}

	private:

		static integer barrett_reduction(const integer& x, const integer& n, const integer& constant)
		{
			int k = n.m_length,
				kPlusOne = k + 1,
				kMinusOne = k - 1;

			integer q1;

			// q1 = x / b^(k-1)
			for (int i = kMinusOne, j = 0; i < x.m_length; i++, j++)
				q1.m_data[j] = x.m_data[i];
			q1.m_length = x.m_length - kMinusOne;
			if (q1.m_length <= 0)
				q1.m_length = 1;


			integer q2 = q1 * constant;
			integer q3;

			// q3 = q2 / b^(k+1)
			for (int i = kPlusOne, j = 0; i < q2.m_length; i++, j++)
				q3.m_data[j] = q2.m_data[i];
			q3.m_length = q2.m_length - kPlusOne;
			if (q3.m_length <= 0)
				q3.m_length = 1;


			// r1 = x mod b^(k+1)
			// i.e. keep the lowest (k+1) words
			integer r1;
			int lengthToCopy = (x.m_length > kPlusOne) ? kPlusOne : x.m_length;
			for (int i = 0; i < lengthToCopy; i++)
				r1.m_data[i] = x.m_data[i];
			r1.m_length = lengthToCopy;


			// r2 = (q3 * n) mod b^(k+1)
			// partial multiplication of q3 and n

			integer r2;
			for (int i = 0; i < q3.m_length; i++)
			{
				if (q3.m_data[i] == 0)     continue;

				ulong mcarry = 0;
				int t = i;
				for (int j = 0; j < n.m_length && t < kPlusOne; j++, t++)
				{
					// t = i + j
					ulong val = ((ulong)q3.m_data[i] * (ulong)n.m_data[j]) +
						(ulong)r2.m_data[t] + mcarry;

					r2.m_data[t] = (uint)(val & 0xFFFFFFFF);
					mcarry = (val >> 32);
				}

				if (t < kPlusOne)
					r2.m_data[t] = (uint)mcarry;
			}
			r2.m_length = kPlusOne;
			while (r2.m_length > 1 && r2.m_data[r2.m_length - 1] == 0)
				r2.m_length--;

			r1 = r1 - r2;
			if ((r1.m_data[MaxLength - 1] & 0x80000000) != 0)        // negative
			{
				integer val;
				val.m_data[kPlusOne] = 0x00000001;
				val.m_length = kPlusOne + 1;
				r1 = r1 + val;
			}

			while (r1 >= n)
				r1 = r1 - n;

			return r1;
		}

		static void single_byte_divide(integer b1, integer b2, integer& outQuotient, integer& outRemainder)
		{
			std::vector<uint> result(MaxLength);
			int resultPos = 0;

			// copy dividend to reminder
			for (int i = 0; i < MaxLength; i++)
				outRemainder.m_data[i] = b1.m_data[i];
			outRemainder.m_length = b1.m_length;

			while (outRemainder.m_length > 1 && outRemainder.m_data[outRemainder.m_length - 1] == 0)
				outRemainder.m_length--;

			ulong divisor = (ulong)b2.m_data[0];
			int pos = outRemainder.m_length - 1;
			ulong dividend = (ulong)outRemainder.m_data[pos];


			if (dividend >= divisor)
			{
				ulong quotient = dividend / divisor;
				result[resultPos++] = (uint)quotient;

				outRemainder.m_data[pos] = (uint)(dividend % divisor);
			}
			pos--;

			while (pos >= 0)
			{
				dividend = ((ulong)outRemainder.m_data[pos + 1] << 32) + (ulong)outRemainder.m_data[pos];
				ulong quotient = dividend / divisor;
				result[resultPos++] = (uint)quotient;

				outRemainder.m_data[pos + 1] = 0;
				outRemainder.m_data[pos--] = (uint)(dividend % divisor);
			}

			outQuotient.m_length = resultPos;
			int j = 0;
			for (int i = outQuotient.m_length - 1; i >= 0; i--, j++)
				outQuotient.m_data[j] = result[i];
			for (; j < MaxLength; j++)
				outQuotient.m_data[j] = 0;

			while (outQuotient.m_length > 1 && outQuotient.m_data[outQuotient.m_length - 1] == 0)
				outQuotient.m_length--;

			if (outQuotient.m_length == 0)
				outQuotient.m_length = 1;

			while (outRemainder.m_length > 1 && outRemainder.m_data[outRemainder.m_length - 1] == 0)
				outRemainder.m_length--;

		}

		static void multi_byte_divide(integer b1, integer b2, integer& outQuotient, integer& outRemainder)
		{
			//uint[] result = new uint[MaxLength];
			std::vector<uint> result(MaxLength);
			int remainderLen = b1.m_length + 1;
			//uint[] remainder = new uint[remainderLen];
			std::vector<uint> remainder(remainderLen);

			uint mask = 0x80000000;
			uint val = b2.m_data[b2.m_length - 1];
			int shift = 0, resultPos = 0;

			while (mask != 0 && (val & mask) == 0)
			{
				shift++; mask >>= 1;
			}


			for (int i = 0; i < b1.m_length; i++)
				remainder[i] = b1.m_data[i];
			shift_left(remainder, shift);
			b2 = b2 << shift;


			int j = remainderLen - b2.m_length;
			int pos = remainderLen - 1;

			ulong firstDivisorByte = b2.m_data[b2.m_length - 1];
			ulong secondDivisorByte = b2.m_data[b2.m_length - 2];

			int divisorLen = b2.m_length + 1;
			//uint[] dividendPart = new uint[divisorLen];
			std::vector<uint> dividendPart(divisorLen);

			while (j > 0)
			{
				ulong dividend = ((ulong)remainder[pos] << 32) + (ulong)remainder[pos - 1];
				//Console.WriteLine("dividend = {0}", dividend);

				ulong q_hat = dividend / firstDivisorByte;
				ulong r_hat = dividend % firstDivisorByte;

				//Console.WriteLine("q_hat = {0:X}, r_hat = {1:X}", q_hat, r_hat);

				bool done = false;
				while (!done)
				{
					done = true;

					if (q_hat == 0x100000000 ||
						(q_hat * secondDivisorByte) > ((r_hat << 32) + remainder[pos - 2]))
					{
						q_hat--;
						r_hat += firstDivisorByte;

						if (r_hat < 0x100000000)
							done = false;
					}
				}

				for (int h = 0; h < divisorLen; h++)
					dividendPart[h] = remainder[pos - h];

				//integer kk = new integer(dividendPart);
				integer kk{ dividendPart }; // move
				integer ss = b2 * static_cast<integer>((llong)q_hat);

				//Console.WriteLine("ss before = " + ss);
				while (ss > kk)
				{
					q_hat--;
					//ss -= b2;
					ss = ss - b2;
				}
				integer yy = kk - ss;

				for (int h = 0; h < divisorLen; h++)
					remainder[pos - h] = yy.m_data[b2.m_length - h];


				result[resultPos++] = (uint)q_hat;

				pos--;
				j--;
			}

			outQuotient.m_length = resultPos;
			int y = 0;
			for (int x = outQuotient.m_length - 1; x >= 0; x--, y++)
				outQuotient.m_data[y] = result[x];
			for (; y < MaxLength; y++)
				outQuotient.m_data[y] = 0;

			while (outQuotient.m_length > 1 && outQuotient.m_data[outQuotient.m_length - 1] == 0)
				outQuotient.m_length--;

			if (outQuotient.m_length == 0)
				outQuotient.m_length = 1;

			outRemainder.m_length = shift_right(remainder, shift);

			for (y = 0; y < outRemainder.m_length; y++)
				outRemainder.m_data[y] = remainder[y];
			for (; y < MaxLength; y++)
				outRemainder.m_data[y] = 0;
		}

		static int shift_left(std::vector<uint>& buffer, int shiftVal)
		{
			int shiftAmount = 32;
			int bufLen = (int)buffer.size();

			while (bufLen > 1 && buffer[bufLen - 1] == 0)
				bufLen--;

			for (int count = shiftVal; count > 0;)
			{
				if (count < shiftAmount)
					shiftAmount = count;


				ulong carry = 0;
				for (int i = 0; i < bufLen; i++)
				{
					ulong val = ((ulong)buffer[i]) << shiftAmount;
					val |= carry;

					buffer[i] = (uint)(val & 0xFFFFFFFF);
					carry = val >> 32;
				}

				if (carry != 0)
				{
					if (bufLen + 1 <= (int)buffer.size())
					{
						buffer[bufLen] = (uint)carry;
						bufLen++;
					}
				}
				count -= shiftAmount;
			}
			return bufLen;
		}

		static int shift_right(std::vector<uint>& buffer, int shiftVal)
		{
			int shiftAmount = 32;
			int invShift = 0;
			int bufLen = (int)buffer.size();

			while (bufLen > 1 && buffer[bufLen - 1] == 0)
				bufLen--;

			for (int count = shiftVal; count > 0;)
			{
				if (count < shiftAmount)
				{
					shiftAmount = count;
					invShift = 32 - shiftAmount;
				}

				ulong carry = 0;
				for (int i = bufLen - 1; i >= 0; i--)
				{
					ulong val = ((ulong)buffer[i]) >> shiftAmount;
					val |= carry;

					carry = ((ulong)buffer[i]) << invShift;
					buffer[i] = (uint)(val);
				}

				count -= shiftAmount;
			}

			while (bufLen > 1 && buffer[bufLen - 1] == 0)
				bufLen--;

			return bufLen;
		}


	};


}