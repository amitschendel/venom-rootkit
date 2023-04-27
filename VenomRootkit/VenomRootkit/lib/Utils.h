#pragma once

namespace std {

	template <typename T>
	struct remove_reference { typedef T type; };

	template <typename T>
	struct remove_reference<T&> { typedef T type; };

	template <typename T>
	struct remove_reference<T&&> { typedef T type; };

	template <typename T>
	typename remove_reference<T>::type&& move(T&& arg)
	{
		return static_cast<typename remove_reference<T>::type&&>(arg);
	}

	template <typename T>
	T&& forward(typename std::remove_reference<T>::type& arg) noexcept {
		return static_cast<T&&>(arg);
	}

	template <typename T>
	T&& forward(typename std::remove_reference<T>::type&& arg) noexcept {
		return static_cast<T&&>(arg);
	}
	//template<class _Ty> inline
	//	constexpr _Ty&& forward(
	//		typename remove_reference<_Ty>::type& _Arg)
	//{	// forward an lvalue as either an lvalue or an rvalue
	//	return (static_cast<_Ty&&>(_Arg));
	//}

	//template<class _Ty> inline
	//	constexpr _Ty&& forward(
	//		typename remove_reference<_Ty>::type&& _Arg)
	//{	// forward an rvalue as an rvalue
	//	return (static_cast<_Ty&&>(_Arg));
	//}
	template <typename T> void swap(T& a, T& b)
	{
		T c(move(a)); a = move(b); b = move(c);
	}
	template <typename T, size_t N> void swap(T(&a)[N], T(&b)[N])
	{
		for (size_t i = 0; i < N; ++i) swap(a[i], b[i]);
	}

	template <typename T, typename U = T>
	T exchange(T& obj, U&& new_val)
	{
		T old_val = move(obj);
		obj = std::forward<U>(new_val);
		return old_val;
	}
}