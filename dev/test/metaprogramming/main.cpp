/*
	restinio
*/

/*!
	Tests for restinio::utils::metaprogramming.
*/

#include <restinio/utils/metaprogramming.hpp>

#include <tuple>

int main()
{
	using namespace restinio::utils::metaprogramming;

	{
		using T = head_of_t<int, float, double>;
		static_assert(std::is_same<T, int>::value, "!Ok");
	}

	{
		using T = tail_of_t<int, float, double>;
		static_assert(std::is_same<T, type_list<float, double>>::value, "!Ok");
	}

	{
		using T = tail_of_t<int>;
		static_assert(std::is_same<T, type_list<>>::value, "!Ok");
	}

	{
		using T = put_front_t<int, type_list<>>;
		static_assert(std::is_same<T, type_list<int>>::value, "!Ok");
	}

	{
		using T = put_front_t<int, type_list<float, double>>;
		static_assert(std::is_same<T, type_list<int, float, double>>::value, "!Ok");
	}

	{
		using T = rename_t<type_list<int, float, double>, std::tuple>;
		static_assert(std::is_same<T, std::tuple<int, float, double>>::value, "!Ok");
	}

	{
		constexpr bool v = all_of_v<std::is_integral, int, char, long>;
		static_assert(v, "!Ok");
	}

	{
		constexpr bool v = all_of_v<std::is_integral, int, float, long>;
		static_assert(!v, "!Ok");
	}

	return 0;
}

