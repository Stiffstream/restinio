template<
	typename Char,
	typename Traits = std::char_traits<Char> >
class basic_string_view_t final
{
	public:

		using traits_type = Traits;
		using value_type = Char;
		using pointer = Char*;
		using const_pointer = const Char*;
		using reference = Char&;
		using const_reference = const Char&;
		using const_iterator = const Char*;
		using iterator = const const_iterator;
		using const_reverse_iterator = std::reverse_iterator< const_iterator >;
		using size_type = size_t;

		static constexpr size_type npos = size_type(-1);

		// Ctor

		basic_string_view_t() noexcept
		:	m_str{ nullptr }
		,	m_size{ 0 }
		{
		}

		basic_string_view_t( const basic_string_view_t & other ) noexcept = default;

		basic_string_view_t( const Char * s, size_type count )
		:	m_str{ s }
		,	m_size{ count }
		{
		}

		basic_string_view_t( const Char * s )
		:	m_str{ s }
		,	m_size{ Traits::length(s) }
		{
		}

		// Assignment
		basic_string_view_t &
		operator= ( const basic_string_view_t & other ) = default;

		// Iterators

		constexpr const_iterator
		begin() const noexcept
		{
			return m_str;
		}

		constexpr const_iterator
		cbegin() const noexcept
		{
			return m_str;
		}

		constexpr const_iterator
		end() const noexcept
		{
			return m_str + m_size;
		}

		constexpr const_iterator
		cend() const noexcept
		{
			return m_str + m_size;
		}

		constexpr const_reverse_iterator
		rbegin() const noexcept
		{
			return const_reverse_iterator( end() );
		}

		constexpr const_reverse_iterator
		crbegin() const noexcept
		{
			return const_reverse_iterator( end() );
		}

		constexpr const_reverse_iterator
		rend() const noexcept
		{
			return const_reverse_iterator( begin() );
		}

		constexpr const_reverse_iterator
		crend() const noexcept
		{
			return const_reverse_iterator( begin() );
		}


		// Element access

		constexpr const_reference
		operator[] ( size_type pos ) const
		{
			return *(m_str + pos);
		}

		constexpr const_reference
		at(size_type pos) const
		{
			if( pos >= size() )
				throw std::out_of_range("Out of range in basic_string_view_t::at() operation.");

			return *(m_str + pos);
		}

		constexpr const_reference
		front() const
		{
			return *m_str;
		}

		constexpr const_reference
		back() const
		{
			return m_str[m_size-1];
		}

		constexpr const_pointer
		data() const noexcept
		{
			return m_str;
		}

		// Capacity

		constexpr size_type
		size() const noexcept
		{
			return m_size;
		}

		constexpr size_type
		length() const noexcept
		{
			return size();
		}

		constexpr size_type
		max_size() const noexcept
		{
			return std::numeric_limits<size_type>::max();
		}

		constexpr bool
		empty() const noexcept
		{
			return m_size == 0;
		}

		// Modifiers

		constexpr void
		remove_prefix( size_type n )
		{
			m_str += n;
			m_size -= n;
		}

		constexpr void
		remove_suffix(size_type n)
		{
			m_size -= n;
		}

		constexpr void
		swap( basic_string_view_t & v ) noexcept
		{
			std::swap( m_str, v.m_str );
			std::swap( m_size, v.m_size );
		}

		// Operations

		size_type
		copy( Char* dest, size_type count, size_type pos = 0 ) const
		{
			if( pos >= m_size )
				throw std::out_of_range(
					"Out of range in basic_string_view_t::copy() operation.");

			auto rcount = min_rcount( pos, count );
			std::copy( m_str + pos, m_str + pos + rcount, dest );

			return rcount;

		}

		constexpr basic_string_view_t
		substr( size_type pos = 0, size_type count = npos ) const
		{
			if( pos >= m_size )
				throw std::out_of_range(
					"Out of range in basic_string_view_t::substr() operation.");

			auto rcount = min_rcount( pos, count );

			return basic_string_view_t( m_str + pos, rcount );
		}

		constexpr int
		compare( basic_string_view_t v ) const noexcept
		{
			const size_type rlen = std::min( m_size, v.m_size );

			auto res = traits_type::compare( data(), v.data(), rlen );

			if( res != 0 )
				return res;
			else
			{
				if( m_size < v.m_size )
					return -1;
				else if( m_size > v.m_size )
					return 1;
				else
					return 0;
			}
		}

		constexpr int
		compare( size_type pos1, size_type count1, basic_string_view_t v ) const
		{
			return substr(pos1, count1).compare(v);
		}

		constexpr int
		compare( size_type pos1, size_type count1, basic_string_view_t v,
			size_type pos2, size_type count2 ) const
		{
			return substr( pos1, count1 ).compare( v.substr( pos2, count2 ) );
		}

		constexpr int
		compare( const Char* s ) const
		{
			return compare( basic_string_view_t(s) );
		}

		constexpr int
		compare( size_type pos1, size_type count1, const Char* s ) const
		{
			return substr( pos1, count1 ).compare( basic_string_view_t(s) );
		}

		constexpr int
		compare( size_type pos1, size_type count1, const Char* s,
			size_type count2 ) const
		{
			return substr( pos1, count1 ).compare( basic_string_view_t( s, count2 ) );
		}

		constexpr size_type
		find( basic_string_view_t v, size_type pos = 0 ) const noexcept
		{
			if ( pos > size() )
				return npos;

			if ( v.empty() )
				return pos;

			const_iterator iter = std::search(
				this->cbegin() + pos, this->cend(), v.cbegin (), v.cend (), traits_type::eq );

			return iter == this->cend () ? npos : std::distance( this->cbegin(), iter );
		}

		constexpr size_type
		find( Char ch, size_type pos = 0 ) const noexcept
		{
				return find( basic_string_view_t(&ch, 1), pos );
		}

		constexpr size_type
		find( const Char* s, size_type pos, size_type count ) const
		{
			return find( basic_string_view_t(s, count), pos );
		}

		constexpr size_type
		find( const Char* s, size_type pos = 0 ) const
		{
			return find( basic_string_view_t(s), pos );
		}

		constexpr size_type
		rfind(
			basic_string_view_t v, size_type pos = npos ) const noexcept
		{
			if ( size() < v.size() )
				return npos;

			if ( pos > size() - v.size() )
				pos = size() - v.size();

			if ( v.size() == 0u )
				return pos;

			for ( const Char* cur = m_str + pos; ; --cur )
			{
				if ( traits_type::compare( cur, v.m_str, v.size() ) == 0 )
					return cur - m_str;

				if ( cur == m_str )
					return npos;
			}
		}

		constexpr size_type
		rfind(
			Char c, size_type pos = npos ) const noexcept
		{
			return rfind( basic_string_view_t(&c, 1), pos );
		}

		constexpr size_type
		rfind(
			const Char* s, size_type pos, size_type count ) const
		{
			return rfind( basic_string_view_t(s, count), pos );
		}

		constexpr size_type
		rfind(
			const Char* s, size_type pos = npos ) const
		{
			return rfind( basic_string_view_t(s), pos );
		}

		constexpr size_type
		find_first_of( basic_string_view_t v, size_type pos = 0 ) const noexcept
		{
			if ( pos >= size() || v.size() == 0 )
				return npos;

			const_iterator iter = std::find_first_of(
				this->cbegin() + pos, this->cend(), v.cbegin(), v.cend(), traits_type::eq );

			return iter == this->cend() ? npos : std::distance ( this->cbegin(), iter );
		}

		constexpr size_type
		find_first_of( Char c, size_type pos = 0 ) const noexcept
		{
			return find_first_of( basic_string_view_t(&c, 1), pos );
		}

		constexpr size_type
		find_first_of( const Char* s, size_type pos, size_type count ) const
		{
			return find_first_of( basic_string_view_t(s, count), pos );
		}

		constexpr size_type
		find_first_of( const Char* s, size_type pos = 0 ) const
		{
			return find_first_of( basic_string_view_t(s), pos );
		}

		constexpr size_type
		find_last_of( basic_string_view_t v, size_type pos = npos ) const noexcept
		{
			if ( v.size() == 0u )
				return npos;

			if( pos >= size() )
				pos = 0;
			else
				pos = size() - ( pos + 1 );

			const_reverse_iterator iter = std::find_first_of(
				this->crbegin() + pos,
				this->crend(), v.cbegin(),
				v.cend(),
				traits_type::eq );

			return iter == this->crend() ? npos : reverse_distance( this->crbegin (), iter );
		}

		constexpr size_type
		find_last_of( Char c, size_type pos = npos ) const noexcept
		{
			return find_last_of( basic_string_view_t(&c, 1), pos );
		}

		constexpr size_type
		find_last_of( const Char* s, size_type pos, size_type count ) const
		{
			return find_last_of( basic_string_view_t(s, count), pos );
		}

		constexpr size_type
		find_last_of( const Char* s, size_type pos = npos ) const
		{
			return find_last_of( basic_string_view_t(s), pos );
		}

		constexpr size_type
		find_first_not_of( basic_string_view_t v, size_type pos = 0 ) const noexcept
		{
			if (pos >= size() )
				return npos;

			if (v.size()  == 0)
				return pos;

			const_iterator iter = find_not_of ( this->cbegin () + pos, this->cend (), v );
			return iter == this->cend () ? npos : std::distance ( this->cbegin (), iter );
		}

		constexpr size_type
		find_first_not_of( Char c, size_type pos = 0 ) const noexcept
		{
			return find_first_not_of( basic_string_view_t(&c, 1), pos );
		}

		constexpr size_type
		find_first_not_of( const Char* s, size_type pos, size_type count ) const
		{
			return find_first_not_of( basic_string_view_t(s, count), pos );
		}

		constexpr size_type
		find_first_not_of( const Char* s, size_type pos = 0 ) const
		{
			return find_first_not_of( basic_string_view_t(s), pos );
		}

		constexpr size_type
		find_last_not_of( basic_string_view_t v, size_type pos = npos ) const noexcept
		{
			if( pos >= size() )
				pos = size() - 1;

			if( v.size()  == 0u )
				return pos;

			pos = size() - ( pos + 1 );
			const_reverse_iterator iter = find_not_of(
				this->crbegin () + pos, this->crend (), v );

			return iter == this->crend () ? npos : reverse_distance ( this->crbegin (), iter );
		}

		constexpr size_type
		find_last_not_of( Char c, size_type pos = npos ) const noexcept
		{
			return find_last_not_of( basic_string_view_t(&c, 1), pos );
		}

		constexpr size_type
		find_last_not_of( const Char* s, size_type pos, size_type count ) const
		{
			return find_last_not_of( basic_string_view_t(s, count), pos );
		}

		constexpr size_type
		find_last_not_of( const Char* s, size_type pos = npos ) const
		{
			return find_last_not_of( basic_string_view_t(s), pos );
		}

	private:

		template< typename Iter >
		size_type
		reverse_distance( Iter first, Iter last ) const noexcept
		{
			return size() - 1 - std::distance( first, last );
		}

		template< typename Iter >
		Iter find_not_of( Iter first, Iter last, basic_string_view_t s ) const noexcept
		{
			for (; first != last ; ++first)
			{
				if ( 0 == traits_type::find( s.data(), s.size(), *first ) )
					return first;
			}

			return last;
		}

		size_type
		min_rcount( size_type pos, size_type count ) const
		{
			return std::min( size() - pos, count );
		}

		const value_type * m_str;
		size_type m_size;
};

template< class Char, class Traits >
constexpr bool operator==(
	basic_string_view_t <Char,Traits> lhs,
	basic_string_view_t <Char,Traits> rhs ) noexcept
{
	return lhs.compare(rhs) == 0;
}

template< class Char, class Traits >
constexpr bool operator!=(
	basic_string_view_t <Char,Traits> lhs,
	basic_string_view_t <Char,Traits> rhs ) noexcept
{
	return lhs.compare(rhs) != 0;
}

template< class Char, class Traits >
constexpr bool operator<(
	basic_string_view_t <Char,Traits> lhs,
	basic_string_view_t <Char,Traits> rhs ) noexcept
{
	return lhs.compare(rhs) < 0;
}

template< class Char, class Traits >
constexpr bool operator<=(
	basic_string_view_t <Char,Traits> lhs,
	basic_string_view_t <Char,Traits> rhs ) noexcept
{
	return lhs.compare(rhs) <= 0;
}

template< class Char, class Traits >
constexpr bool operator>(
	basic_string_view_t <Char,Traits> lhs,
	basic_string_view_t <Char,Traits> rhs ) noexcept
{
	return lhs.compare(rhs) > 0;
}

template< class Char, class Traits >
constexpr bool operator>=(
	basic_string_view_t <Char,Traits> lhs,
	basic_string_view_t <Char,Traits> rhs ) noexcept
{
	return lhs.compare(rhs) >= 0;
}

template <class Char, class Traits>
inline std::basic_ostream<Char, Traits>&
operator<<(
	std::basic_ostream<Char, Traits>& o,
	basic_string_view_t <Char, Traits> v )
{
	o.write( v.data(), v.size() );
	return o;
}

using string_view_t = basic_string_view_t< char >;

// Extra cmp operators.
inline bool operator == ( string_view_t sv, const char *s ) noexcept
{
	return sv == string_view_t{ s };
}

