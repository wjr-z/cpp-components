
#pragma once
#ifndef __WJR_STRING_H
#define __WJR_STRING_H

#include <string_view>
#include <iostream>

#include <wjr/vector.h>
#include <wjr/functional.h>

_WJR_BEGIN

// Please note that string always has a '\0' terminator, so the capacity of string
// should be the capacity of the container - 1, 
// by inheriting vector_sso_data and setting the virtual capacity to the actual capacity - 1
// All operations on string_data are performed only through the virtual capacity, 
// and the virtual capacity is converted to the actual capacity inside string_data
// virtual size is equal to actual size, so EOF needs to be handled manually
// virtual size = actual size <= virtual capacity = actual capacity - 1 < actual capacity

enum string_mode {
	case_sensitive = 0,
	case_insensitive = 1,
};

template<typename Char, typename Traits = std::char_traits<Char>>
class basic_string_view {

	using _Std_view_type = std::basic_string_view<Char, Traits>;

	template<typename T>
	struct _Is_string_view_like : std::conjunction<
		std::is_convertible<const T&, basic_string_view>
	> {};

	template<typename T>
	constexpr static bool _Is_string_view_like_v = _Is_string_view_like<T>::value;

	template<typename T>
	struct _Is_noptr_std_string_view_like : std::conjunction<
		std::is_convertible<const T&, _Std_view_type>,
		std::negation<std::is_convertible<const T&, const Char*>>
	> {};

	template<typename T>
	constexpr static bool _Is_noptr_std_string_view_like_v = _Is_noptr_std_string_view_like<T>::value;

	template<typename T>
	struct _Is_noptr_string_view_like : std::conjunction<
		_Is_string_view_like<T>,
		std::negation<std::is_convertible<const T&, const Char*>>
	> {};

	template<typename T>
	constexpr static bool _Is_noptr_string_view_like_v = _Is_noptr_string_view_like<T>::value;
public:
	static_assert(!std::is_array_v<Char>&& std::is_trivial_v<Char>&& std::is_standard_layout_v<Char>,
		"The character type of basic_string_view must be a non-array trivial standard-layout type. See N4910 "
		"23.1 [strings.general]/1.");

	using traits_type = std::char_traits<Char>;
	using value_type = Char;
	using pointer = Char*;
	using const_pointer = const Char*;
	using reference = Char&;
	using const_reference = const Char&;
	using const_iterator = const Char*;
	using iterator = const_iterator;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using reverse_iterator = const_reverse_iterator;
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	static constexpr auto npos = static_cast<size_type>(-1);
private:

	constexpr static size_type traits_length(const value_type* s) {
		return std::char_traits<value_type>::length(s);
	}
public:

	WJR_INTRINSIC_CONSTEXPR basic_string_view() noexcept : _Mydata(), _Mysize(0) {}

	WJR_INTRINSIC_CONSTEXPR basic_string_view(const basic_string_view&) noexcept = default;
	WJR_INTRINSIC_CONSTEXPR basic_string_view& operator=(const basic_string_view&) noexcept = default;

	WJR_INLINE_CONSTEXPR basic_string_view(const const_pointer ptr)
		: _Mydata(ptr), _Mysize(traits_length(ptr)) {}

	WJR_INTRINSIC_CONSTEXPR basic_string_view(
		const const_pointer ptr, const size_type n)
		: _Mydata(ptr), _Mysize(n) {}

private:
	WJR_INTRINSIC_CONSTEXPR basic_string_view(_Std_view_type s, disable_tag) noexcept
		: _Mydata(s.data()), _Mysize(s.size()) {}
public:

	template<typename StringView, std::enable_if_t<_Is_noptr_std_string_view_like_v<StringView>, int> = 0>
	WJR_INTRINSIC_CONSTEXPR basic_string_view(const StringView& t) noexcept
		: basic_string_view(_Std_view_type(t), disable_tag{}) {}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR operator _Std_view_type() const noexcept {
		return { data(), size() };
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_iterator begin() const noexcept {
		return const_iterator(_Mydata);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_iterator end() const noexcept {
		return const_iterator(_Mydata + _Mysize);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_iterator cbegin() const noexcept {
		return begin();
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_iterator cend() const noexcept {
		return end();
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_reverse_iterator rbegin() const noexcept {
		return const_reverse_iterator{ end() };
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_reverse_iterator rend() const noexcept {
		return const_reverse_iterator{ begin() };
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_reverse_iterator crbegin() const noexcept {
		return rbegin();
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_reverse_iterator crend() const noexcept {
		return rend();
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR size_type size() const noexcept {
		return _Mysize;
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR size_type length() const noexcept {
		return _Mysize;
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR bool empty() const noexcept {
		return _Mysize == 0;
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_pointer data() const noexcept {
		return _Mydata;
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR size_type max_size() const noexcept {
		return std::min(static_cast<size_t>(PTRDIFF_MAX), static_cast<size_t>(-1) / sizeof(Char));
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_reference operator[](const size_type off) const noexcept  {
		return _Mydata[off];
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_reference at(const size_type off) const {
		if(off >= size()){
			throw std::out_of_range("error at basic_string_view::at");
		}
		return _Mydata[off];
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_reference front() const noexcept {
		return _Mydata[0];
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR const_reference back() const noexcept {
		return _Mydata[_Mysize - 1];
	}

	WJR_INTRINSIC_CONSTEXPR void remove_prefix(const size_type n) noexcept  {
		_Mydata += n;
		_Mysize -= n;
	}

	WJR_INTRINSIC_CONSTEXPR void remove_suffix(const size_type n) noexcept {
		_Mysize -= n;
	}

	WJR_INTRINSIC_CONSTEXPR void swap(basic_string_view& other) noexcept {
		const basic_string_view tmp{ other }; 
		other = *this;
		*this = tmp;
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR basic_string_view substr(
		const size_type off = 0, size_type n = npos) const WJR_NOEXCEPT	{
#if defined(_WJR_EXCEPTION)
		if (is_likely(off <= size())) {
#endif // _WJR_EXCEPTION
			return noexcept_substr(off, n);
#if defined(_WJR_EXCEPTION)
		}
		throw std::out_of_range("out of range at basic_string_view::substr");
#endif // _WJR_EXCEPTION
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const StringView& t) const noexcept {
		const auto sv = view(t);
		if constexpr (std::is_same_v<Char, char>) {
			return wjr::compare(
				reinterpret_cast<const uint8_t*>(begin()),
				reinterpret_cast<const uint8_t*>(end()),
				reinterpret_cast<const uint8_t*>(sv.begin()),
				reinterpret_cast<const uint8_t*>(sv.end())
			);
		}
		else {
			return wjr::compare(begin(), end(), sv.begin(), sv.end());
		}
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const size_type off, const size_type n, const StringView& t) 
		const WJR_NOEXCEPT {
		return except_view(*this, off, n).compare(t);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const size_type off1, const size_type n1, const StringView& t,
		const size_type off2, const size_type n2) 
		const WJR_NOEXCEPT {
		return except_view(*this, off1, n1).compare(except_view(t, off2, n2));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const Char* const ptr) const {
		return compare(view(ptr));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const size_type off, const size_type n, const Char* const ptr) const {
		return except_view(*this, off, n).compare(view(ptr));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const size_type off1, const size_type n1,
		const Char* const ptr, const size_type n2) const {
		return except_view(*this, off1, n1).compare(except_view(ptr, n2));
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool starts_with(const StringView& t) const noexcept {
		const auto sv = view(t);
		return prefix(sv.size()).equal(sv);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool starts_with(const Char ch) const noexcept {
		return !empty() && traits_type::eq(front(), ch);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool starts_with(const Char* const other) const {
		return starts_with(view(other));
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool ends_with(const StringView& t) const noexcept {
		const auto sv = view(t);
		return suffix(sv.size()).equal(sv);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool ends_with(const Char ch) const noexcept {
		return !empty() && traits_type::eq(back(), ch);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool ends_with(const Char* const other) const {
		return ends_with(view(other));
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool contains(const StringView& t) const noexcept {
		return find(t) != npos;
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool contains(const Char other) const noexcept {
		return find(other) != npos;
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool contains(const Char* const other) const {
		return find(other) != npos;
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type find(const StringView& t, const size_type off = 0) const noexcept {
		if (is_likely(off <= size())) {
			const auto sv1 = view(*this, off);
			const auto sv2 = view(t);
			auto iter = wjr::search(sv1.begin(), sv1.end(), sv2.begin(), sv2.end());
			return iter == end() ? npos : static_cast<size_type>(iter - begin());
		}
		return npos;
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find(const Char ch, const size_type off = 0) const noexcept {
		if (is_likely(off <= size())) {
			const auto sv = view(*this, off);
			auto iter = wjr::find(sv.begin(), sv.end(), ch);
			return iter == end() ? npos : static_cast<size_type>(iter - begin());
		}
		return npos;
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find(const Char* const ptr, const size_type off,
		const size_type n) const {
		return find(view(ptr, n), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find(const Char* const ptr, const size_type off = 0)
		const {
		return find(view(ptr), off);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type rfind(const StringView& t, const size_type off = npos) const noexcept {
		if (is_likely(size() != 0)) {
			const auto sv1 = view(*this, 0, std::min(off, size() - 1) + 1);
			const auto sv2 = view(t);
			auto iter = wjr::search(sv1.rbegin(), sv1.rend(),
				sv2.rbegin(), sv2.rend());
			return iter == rend() ? npos : static_cast<size_type>(rend() - iter) - sv2.size();
		}
		return npos;
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type rfind(const Char ch, const size_type off = npos) const noexcept {
		if (is_likely(size() != 0)) {
			const auto sv = view(*this, 0, std::min(off, size() - 1) + 1);
			auto iter = wjr::find(sv.rbegin(), sv.rend(), ch);
			return iter == rend() ? npos : static_cast<size_type>(rend() - iter) - 1;
		}
		return npos;
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type rfind(const Char* const ptr, const size_type off,
		const size_type n) const {
		return rfind(view(ptr, n), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type rfind(const Char* const ptr, const size_type off = npos)
		const {
		return rfind(view(ptr), off);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_of(const StringView& t,
		const size_type off = 0) const noexcept { 
		auto sv = _Std_view_type(view(t));
		return _Std_view_type(*this).find_first_of(sv, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_of(const Char ch, const size_type off = 0) const noexcept {
		return find(ch, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_of(const Char* const ptr, const size_type off,
		const size_type n) const {
		return find_first_of(view(ptr, n), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_of(
		const Char* const ptr, const size_type off = 0) const  {
		return find_first_of(view(ptr), off);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_of(const StringView& t,
		const size_type off = npos) const noexcept { 
		const auto sv = _Std_view_type(view(t));
		return _Std_view_type(*this).find_last_of(sv, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_of(const Char ch, const size_type off = npos) const noexcept {
		return rfind(ch, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_of(const Char* const ptr, const size_type off,
		const size_type n) const {
		return find_last_of(view(ptr, n), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_of(
		const Char* const ptr, const size_type off = npos) const {
		return find_last_of(view(ptr), off);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_not_of(const StringView& t,
		const size_type off = 0) const noexcept { 
		const auto sv = _Std_view_type(view(t));
		return _Std_view_type(*this).find_first_not_of(sv, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_not_of(const Char ch, const size_type off = 0) const noexcept {
		if (is_likely(off <= size())) {
			const auto sv = view(*this, off);
			auto iter = wjr::find(sv.begin(), sv.end(), ch, std::not_equal_to<>{});
			return iter == end() ? npos : static_cast<size_type>(iter - begin());
		}
		return npos;
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_not_of(const Char* const ptr, const size_type off,
		const size_type n) const  {
		return find_first_not_of(view(ptr, n), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_not_of(
		const Char* const ptr, const size_type off = 0) const  {
		return find_first_not_of(view(ptr), off);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_not_of(const StringView& t,
		const size_type off = npos) const noexcept { 
		const auto sv = _Std_view_type(view(t));
		return _Std_view_type(*this).find_last_not_of(sv, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_not_of(const Char ch, const size_type off = npos) const noexcept {
		if (is_likely(size() != 0)) {
			const auto sv = view(*this, 0, std::min(off, size() - 1) + 1);
			auto iter = wjr::find(sv.rbegin(), sv.rend(), ch, std::not_equal_to<>{});
			return iter == rend() ? npos : static_cast<size_type>(rend() - iter) - 1;
		}
		return npos;
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_not_of(const Char* const ptr, const size_type off,
		const size_type count) const  {
		return find_last_not_of(view(ptr, count), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_not_of(
		const Char* const ptr, const size_type off = npos) const  {
		return find_last_not_of(view(ptr), off);
	}

	// non-standard extension functions
	
	/*------External extension function------*/

	// no exception
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR basic_string_view 
		noexcept_substr(const size_type off = 0, size_type n = npos) const noexcept {
		n = std::min(n, _Mysize - off);
		return basic_string_view(_Mydata + off, n);
	}

	// no exception view

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR static basic_string_view view(
		const value_type* s, const size_type n) noexcept {
		return basic_string_view(s, n);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR static basic_string_view view(
		const value_type* s) noexcept {
		return basic_string_view(s);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR static basic_string_view view(
		std::initializer_list<value_type> il) noexcept {
		return view(il.begin(), il.size());
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR static basic_string_view view(const StringView& t) noexcept {
		return basic_string_view(t);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR static basic_string_view view(const StringView& t,
		const size_type off, const size_type n = npos) noexcept {
		return view(t).noexcept_substr(off, n);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR basic_string_view view() const noexcept {
		return *this;
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR static basic_string_view except_view(const StringView& t,
		const size_type off, const size_type n = npos) WJR_NOEXCEPT {
		return view(t).substr(off, n);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR basic_string_view prefix(size_type n) const noexcept {
		n = std::min(n, size());
		return basic_string_view(begin(), n);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR basic_string_view suffix(size_type n) const noexcept {
		n = std::min(n, size());
		return basic_string_view(end() - n, n);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const StringView& t) const noexcept {
		const auto sv = view(t);
		return wjr::equal(begin(), end(), sv.begin(), sv.end());
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const size_type off, const size_type n, const StringView& t) 
		const WJR_NOEXCEPT {
		return except_view(*this, off, n).equal(view(t));
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const size_type off1, const size_type n1, const StringView& t,
		const size_type off2, const size_type n2) const WJR_NOEXCEPT {
		return except_view(*this, off1, n1).equal(except_view(t, off2, n2));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const Char* const ptr) const {
		return equal(view(ptr));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const size_type off, const size_type n, const Char* const ptr) const {
		return equal(off, n, view(ptr));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const size_type off1, const size_type n1,
		const Char* const ptr, const size_type n2) const {
		return equal(off1, n1, view(ptr, n2));
	}

	// user-defined eq/lt

private:

	const_pointer _Mydata;
	size_type _Mysize;
};

template<typename Char>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator==(const basic_string_view<Char> lhs, const basic_string_view<Char> rhs) noexcept {
	return lhs.equal(rhs);
}

template<typename Char, int = 1>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator==(
	const type_identity_t<basic_string_view<Char>> lhs, 
	const basic_string_view<Char> rhs) noexcept {
	return lhs.equal(rhs);
}

template<typename Char, int = 2>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator==(
	const basic_string_view<Char> lhs, 
	const type_identity_t<basic_string_view<Char>> rhs) noexcept {
	return lhs.equal(rhs);
}

template<typename Char>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator!=(const basic_string_view<Char> lhs, const basic_string_view<Char> rhs) noexcept {
	return !(lhs == rhs);
}

template<typename Char, int = 1>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator!=(
	const type_identity_t<basic_string_view<Char>> lhs,
	const basic_string_view<Char> rhs) noexcept {
	return !(lhs == rhs);
}

template<typename Char, int = 2>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator!=(
	const basic_string_view<Char> lhs,
	const type_identity_t<basic_string_view<Char>> rhs) noexcept {
	return !(lhs == rhs);
}

template<typename Char>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<(const basic_string_view<Char> lhs, const basic_string_view<Char> rhs) noexcept {
	return lhs.compare(rhs) < 0;
}

template<typename Char, int = 1>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<(
	const type_identity_t<basic_string_view<Char>> lhs,
	const basic_string_view<Char> rhs) noexcept {
	return lhs.compare(rhs) < 0;
}

template<typename Char, int = 2>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<(
	const basic_string_view<Char> lhs,
	const type_identity_t<basic_string_view<Char>> rhs) noexcept {
	return lhs.compare(rhs) < 0;
}


template<typename Char>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>(const basic_string_view<Char> lhs, const basic_string_view<Char> rhs) noexcept {
	return rhs < lhs;
}

template<typename Char, int = 1>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>(
	const type_identity_t<basic_string_view<Char>> lhs,
	const basic_string_view<Char> rhs) noexcept {
	return rhs < lhs;
}

template<typename Char, int = 2>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>(
	const basic_string_view<Char> lhs,
	const type_identity_t<basic_string_view<Char>> rhs) noexcept {
	return rhs < lhs;
}

template<typename Char>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<=(const basic_string_view<Char> lhs, const basic_string_view<Char> rhs) noexcept {
	return !(rhs < lhs);
}

template<typename Char, int = 1>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<=(
	const type_identity_t<basic_string_view<Char>> lhs,
	const basic_string_view<Char> rhs) noexcept {
	return !(rhs < lhs);
}

template<typename Char, int = 2>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<=(
	const basic_string_view<Char> lhs,
	const type_identity_t<basic_string_view<Char>> rhs) noexcept {
	return !(rhs < lhs);
}

template<typename Char>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>=(const basic_string_view<Char> lhs, const basic_string_view<Char> rhs) noexcept {
	return !(lhs < rhs);
}

template<typename Char, int = 1>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>=(
	const type_identity_t<basic_string_view<Char>> lhs,
	const basic_string_view<Char> rhs) noexcept {
	return !(lhs < rhs);
}

template<typename Char, int = 2>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>=(
	const basic_string_view<Char> lhs,
	const type_identity_t<basic_string_view<Char>> rhs) noexcept {
	return !(lhs < rhs);
}

using string_view = basic_string_view<char>;
#if defined(WJR_CHAR8_T)
using u8string_view = basic_string_view<char8_t>;
#endif // WJR_CHAR8_T
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;
using wstring_view = basic_string_view<wchar_t>;

namespace literals {
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR string_view operator""_sv(const char* str, size_t len) noexcept {
		return string_view(str, len);
	}
#if defined(WJR_CHAR8_T)
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR u8string_view operator""_sv(const char8_t* str, size_t len) noexcept {
		return u8string_view(str, len);
	}
#endif // WJR_CHAR8_T
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR u16string_view operator""_sv(const char16_t* str, size_t len) noexcept {
		return u16string_view(str, len);
	}
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR u32string_view operator""_sv(const char32_t* str, size_t len) noexcept {
		return u32string_view(str, len);
	}
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR wstring_view operator""_sv(const wchar_t* str, size_t len) noexcept {
		return wstring_view(str, len);
	}
}

template<typename Char>
std::basic_ostream<Char, std::char_traits<Char>>& operator<<(
	std::basic_ostream<Char, std::char_traits<Char>>& os, wjr::basic_string_view<Char> sv) {
	return (os << (std::basic_string_view<Char, std::char_traits<Char>>)(sv));
}

template<typename Char, typename Alloc = std::allocator<Char>, 
	typename Data = string_sso_data<Char, std::max(size_t(1), size_t(16 / sizeof(Char))), Alloc>>
class basic_string {
public:
	using traits_type = std::char_traits<Char>;
	using allocator_type = Alloc;

	using value_type = Char;
	using reference = value_type&;
	using const_reference = const value_type&;

	using size_type = size_t;
	using difference_type = size_t;

	using pointer = value_type*;
	using const_pointer = const value_type*;

	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	constexpr static size_type npos = static_cast<size_type>(-1);

	using vector_type = wjr::vector<Char, Alloc, Data>;
	using data_type = typename vector_type::data_type;
	using view_type = wjr::basic_string_view<Char>;

	using _Alty = typename vector_type::_Alty;
	using _Alty_traits = typename vector_type::_Alty_traits;

	static_assert(std::is_same_v<iterator, typename vector_type::iterator>, "");
	static_assert(std::is_same_v<const_iterator, typename vector_type::const_iterator>, "");
	static_assert(std::is_same_v<const_iterator, typename view_type::const_iterator>, "");
	//static_assert(!std::is_array_v<Char>&& std::is_trivial_v<Char> && std::is_standard_layout_v<Char>, "");
	static_assert(std::is_trivially_destructible_v<Char>, "");

private:
	template<typename T>
	struct _Is_string_view_like : std::conjunction<
		std::is_convertible<const T&, view_type>
	> {};

	template<typename T>
	constexpr static bool _Is_string_view_like_v = _Is_string_view_like<T>::value;

	template<typename T>
	struct _Is_noptr_string_view_like : std::conjunction<
		_Is_string_view_like<T>,
		std::negation<std::is_convertible<const T&, const Char*>>
	> {};

	template<typename T>
	constexpr static bool _Is_noptr_string_view_like_v = _Is_noptr_string_view_like<T>::value;

public:

	WJR_CONSTEXPR20 basic_string() 
		noexcept(std::is_nothrow_default_constructible_v<vector_type>)
		: m_core() {
		set_end();
	}

	WJR_CONSTEXPR20 explicit basic_string(const allocator_type& al) noexcept
		: m_core(al) {
		set_end();
	}

	WJR_CONSTEXPR20 basic_string(const basic_string& other)
		: m_core(other.m_core) {
		set_end();
	}

	WJR_CONSTEXPR20 basic_string(const basic_string& other, const allocator_type& al)
		: m_core(other.m_core, al) {
		set_end();
	}

	WJR_CONSTEXPR20 basic_string(basic_string&& other)
		: m_core(std::move(other.m_core)) {
		set_end();
		other.set_end();
	}

	WJR_CONSTEXPR20 basic_string(basic_string&& other, const allocator_type& al)
		: m_core(std::move(other.m_core), al) {
		set_end();
		other.set_end();
	}

	template<typename iter, std::enable_if_t<is_iterator_v<iter>, int> = 0>
	WJR_CONSTEXPR20 basic_string(iter first, iter last, const allocator_type& al = allocator_type())
		: m_core(first, last, al) {
		set_end();
	}

	WJR_CONSTEXPR20 basic_string(const basic_string& other, 
		const size_type off, const allocator_type& al = allocator_type())
		: basic_string(except_view(other, off), al) {}

	WJR_CONSTEXPR20 basic_string(const basic_string& other, 
		const size_type off, const size_type n, const allocator_type& al = allocator_type())
		: basic_string(except_view(other, off, n), al) {}

	WJR_CONSTEXPR20 basic_string(const value_type* s, const size_type n, const allocator_type& al = allocator_type())
		: basic_string(view(s, n), al) {}

	WJR_CONSTEXPR20 basic_string(const value_type* s, const allocator_type& al = allocator_type())
		: basic_string(view(s), al) {}

	WJR_CONSTEXPR20 basic_string(std::initializer_list<value_type> il, const allocator_type& al = allocator_type())
		: basic_string(view(il), al) {}

private:
	WJR_CONSTEXPR20 basic_string(view_type sv, const allocator_type& al, disable_tag)
		: basic_string(sv.data(), sv.data() + sv.size(), al) {}

	WJR_CONSTEXPR20 basic_string(
		view_type sv,
		const size_type off, const size_type n,
		const allocator_type& al, disable_tag)
		: basic_string(except_view(sv, off, n), al) {}
public:

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 explicit basic_string(const StringView& t, const allocator_type& al = allocator_type())
		:basic_string(view_type(t), al, disable_tag{}) {}

	template<typename StringView, std::enable_if_t<_Is_string_view_like_v<StringView>, int> = 0>
	explicit basic_string(const StringView& t, 
		const size_type off, const size_type n, 
		const allocator_type& al = allocator_type())
		: basic_string(view_type(t), off, n, al, disable_tag{}) {}

	WJR_CONSTEXPR20 explicit basic_string(const size_type n,
		const value_type c = value_type(), const allocator_type& al = allocator_type())
		: m_core(n, c, al) {
		set_end();
	}

	WJR_CONSTEXPR20 operator view_type() const noexcept {
		return { data(), size()};
	}

	WJR_CONSTEXPR20 operator std::basic_string_view<Char, traits_type>() const noexcept {
		return { data(), size() };
	}

	WJR_CONSTEXPR20 ~basic_string() noexcept = default;

	WJR_CONSTEXPR20 basic_string& operator=(const basic_string& other) {

		if (is_likely(this != std::addressof(other))) {
			m_core = other.m_core;
			set_end();
		}

		return *this;
	}

	WJR_CONSTEXPR20 basic_string& operator=(basic_string&& other) noexcept {

		if (is_likely(this != std::addressof(other))) {
			m_core = std::move(other.m_core);
			set_end();
			other.set_end();
		}
		
		return *this;
	}

	WJR_CONSTEXPR20 basic_string& operator=(const value_type* str) {
		return assign(str);
	}

	WJR_CONSTEXPR20 basic_string& operator=(std::initializer_list<value_type> il) {
		return assign(il);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& operator=(const StringView& t) {
		return assign(t);
	}

	WJR_CONSTEXPR20 basic_string& operator=(const value_type c) {
		resize(1);
		*begin() = c;
		return *this;
	}

	WJR_INTRINSIC_CONSTEXPR20 iterator begin() { return m_core.begin(); }
	WJR_INTRINSIC_CONSTEXPR20 const_iterator begin()const { return m_core.begin(); }
	WJR_INTRINSIC_CONSTEXPR20 const_iterator cbegin()const { return m_core.cbegin(); }

	WJR_INTRINSIC_CONSTEXPR20 iterator end() { return m_core.end(); }
	WJR_INTRINSIC_CONSTEXPR20 const_iterator end()const { return m_core.end(); }
	WJR_INTRINSIC_CONSTEXPR20 const_iterator cend()const { return m_core.end(); }

	WJR_INTRINSIC_CONSTEXPR20 reverse_iterator rbegin() { return reverse_iterator(end()); }
	WJR_INTRINSIC_CONSTEXPR20 const_reverse_iterator rbegin()const { return const_reverse_iterator(end()); }
	WJR_INTRINSIC_CONSTEXPR20 const_reverse_iterator crbegin()const { return const_reverse_iterator(cend()); }

	WJR_INTRINSIC_CONSTEXPR20 reverse_iterator rend() { return reverse_iterator(begin()); }
	WJR_INTRINSIC_CONSTEXPR20 const_reverse_iterator rend()const { return const_reverse_iterator(begin()); }
	WJR_INTRINSIC_CONSTEXPR20 const_reverse_iterator crend()const { return const_reverse_iterator(begin()); }

	WJR_INTRINSIC_CONSTEXPR20 reference front() { return m_core.front(); }
	WJR_INTRINSIC_CONSTEXPR20 const_reference front()const { return m_core.front(); }

	WJR_INTRINSIC_CONSTEXPR20 reference back() {
		return m_core.back();
	}

	WJR_INTRINSIC_CONSTEXPR20 const_reference back()const {
		return m_core.back();
	}

	WJR_CONSTEXPR20 void resize(const size_type n) {
		m_core.resize(n);
		set_end();
	}

	WJR_CONSTEXPR20 void resize(const size_type n, const value_type c) {
		m_core.resize(n, c);
		set_end();
	}

	WJR_CONSTEXPR20 void reserve(const size_type c) {
		m_core.reserve(c);
		set_end();
	}

	WJR_CONSTEXPR20 void shrink_to_fit() {
		m_core.shrink_to_fit();
		set_end();
	}

	WJR_CONSTEXPR20 void clear() {
		resize(0);
	}

	WJR_INTRINSIC_CONSTEXPR20 size_type size() const {
		return m_core.size();
	}

	WJR_INTRINSIC_CONSTEXPR20 size_type length() const {
		return size();
	}

	WJR_INTRINSIC_CONSTEXPR20 size_type max_size() const { return std::numeric_limits<size_type>::max(); }

	WJR_INTRINSIC_CONSTEXPR20 size_type capacity() const {
		return m_core.capacity();
	}

	WJR_INTRINSIC_CONSTEXPR20 reference operator[](const size_type index) {
		return m_core[index];
	}

	WJR_INTRINSIC_CONSTEXPR20 const_reference operator[](const size_type index)const {
		return m_core[index];
	}

	WJR_INTRINSIC_CONSTEXPR20 reference at(const size_type index) {
		return m_core.at(index);
	}

	WJR_INTRINSIC_CONSTEXPR20 const_reference at(const size_type index)const {
		return m_core.at(index);
	}

	WJR_CONSTEXPR20 basic_string& operator+=(const basic_string& other) { return append(other); }
	WJR_CONSTEXPR20 basic_string& operator+=(const value_type* s) { return append(s); }
	WJR_CONSTEXPR20 basic_string& operator+=(std::initializer_list<value_type> il) {
		append(il);
		return *this;
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& operator+=(const StringView& t) {
		return append(t);
	}
	
	WJR_CONSTEXPR20 basic_string& operator+=(const value_type c) { return append(c); }

	WJR_CONSTEXPR20 basic_string& append(const basic_string& other) {
		return append(view(other));
	}

	WJR_CONSTEXPR20 basic_string& append(
		const basic_string& other, const size_type off, const size_type n = npos) {
		return append(except_view(other, off, n));
	}

	template<typename iter, std::enable_if_t<is_iterator_v<iter>, int> = 0>
	WJR_CONSTEXPR20 basic_string& append(iter first, iter last) {
		m_core.append(first, last);
		set_end();
		return *this;
	}

	WJR_CONSTEXPR20 basic_string& append(const value_type* s, size_type n) {
		return append(view(s, n));
	}

	WJR_CONSTEXPR20 basic_string& append(const value_type* s) {
		return append(view(s));
	}

	WJR_CONSTEXPR20 basic_string& append(std::initializer_list<value_type> il) {
		return append(view(il));
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& append(const StringView& t) {
		const auto sv = view(t);
		return append(sv.data(), sv.data() + sv.size());
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& append(const StringView& t,
		const size_type off, const size_type n = npos) {
		return apend(except_view(t, off, n));
	}

	WJR_CONSTEXPR20 basic_string& append(const size_type n, const value_type c) {
		m_core.append(n, c);
		set_end();
		return *this;
	}

	WJR_CONSTEXPR20 basic_string& append(const value_type c) {
		push_back(c);
		return *this;
	}

	WJR_CONSTEXPR20 void push_back(const value_type c) {
		m_core.push_back(c);
		set_end();
	}

	WJR_CONSTEXPR20 void push_front(const value_type c) {
		prepend(c);
	}

	WJR_CONSTEXPR20 void pop_back() {
		m_core.pop_back();
		set_end();
	}

	WJR_CONSTEXPR20 basic_string& assign(const basic_string& other) {
		return (*this) = other;
	}

	WJR_CONSTEXPR20 basic_string& assign(basic_string&& other) noexcept {
		return (*this) = std::move(other);
	}

	WJR_CONSTEXPR20 basic_string& assign(const basic_string& other, const size_type off, const size_type n = npos) {
		return assign(except_view(other, off, n));
	}

	template<typename iter, std::enable_if_t<is_iterator_v<iter>, int> = 0>
	WJR_CONSTEXPR20 basic_string& assign(iter first, iter last) {
		m_core.assign(first, last);
		set_end();
		return *this;
	}

	WJR_CONSTEXPR20 basic_string& assign(const value_type* s, const size_type n) {
		return assign(view(s, n));
	}

	WJR_CONSTEXPR20 basic_string& assign(const value_type* s) {
		return assign(view(s));
	}

	WJR_CONSTEXPR20 basic_string& assign(std::initializer_list<value_type> il) {
		return assign(view(il));
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& assign(const StringView& t) {
		const auto sv = view(t);
		return assign(sv.data(), sv.data() + sv.size());
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& assign(const StringView& t, const size_type off, const size_type n = npos) {
		return assign(except_view(t, off, n));
	}

	WJR_CONSTEXPR20 basic_string& assign(const size_type n, const value_type c){
		m_core.assign(n, c);
		set_end();
		return *this;
	}

	WJR_CONSTEXPR20 basic_string& insert(const size_type off, const basic_string& str) {
		return insert(off, view(str));
	}

	WJR_CONSTEXPR20 basic_string& insert(const size_type off1,
		const basic_string& str, const size_type off2, const size_type n = npos) {
		return insert(off1, except_view(str, off2, n));
	}

	template<typename iter, std::enable_if_t<is_iterator_v<iter>, int> = 0>
	WJR_CONSTEXPR20 iterator insert(const_iterator _Where, iter first, iter last) {
		auto __old_pos = static_cast<size_type>(_Where - cbegin());
		m_core.insert(_Where, first, last);
		set_end();
		return begin() + __old_pos;
	}

	WJR_CONSTEXPR20 basic_string& insert(const size_type off, const value_type* s, const size_type n) {
		return insert(off, view(s, n));
	}

	WJR_CONSTEXPR20 basic_string& insert(const size_type off, const value_type* s) {
		return insert(off, view(s));
	}

	WJR_CONSTEXPR20 iterator insert(const_iterator _Where, const size_type n, const value_type c) {
		auto __old_pos = static_cast<size_type>(_Where - cbegin());
		m_core.insert(_Where, n, c);
		set_end();
		return begin() + __old_pos;
	}

	WJR_CONSTEXPR20 basic_string& insert(const size_type off, const size_type n, const value_type c) {
		WJR_UNUSED const auto __ck = except_view(*this, off);
		insert(cbegin() + off, n, c);
		return *this;
	}

	WJR_CONSTEXPR20 iterator insert(const_iterator _Where, const value_type c) {
		auto __old_pos = static_cast<size_type>(_Where - cbegin());
		m_core.insert(_Where, c);
		set_end();
		return begin() + __old_pos;
	}

	WJR_CONSTEXPR20 iterator insert(const_iterator _Where, std::initializer_list<value_type> il) {
		return insert(_Where, il.begin(), il.end());
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& insert(const size_type off, const StringView& t) {
		WJR_UNUSED const auto __ck = except_view(*this, off);
		const auto sv = view(t);
		insert(cbegin() + off, sv.data(), sv.data() + sv.size());
		return *this;
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& insert(const size_type off1, 
		const StringView& t, const size_type off2, const size_type n = npos) {
		return insert(off1, view(t, off2, n));
	}

	WJR_CONSTEXPR20 iterator erase(const_iterator first, const_iterator last) {
		auto __old_pos = first - cbegin();
		m_core.erase(first, last);
		set_end();
		return begin() + __old_pos;
	}

	WJR_CONSTEXPR20 basic_string& erase(const size_type off = 0, size_type n = npos){
		const auto sv = except_view(*this, off, n);
		erase(sv.begin(), sv.end());
		return *this;
	}

	WJR_CONSTEXPR20 iterator erase(const_iterator _Where) {
		auto __old_pos = _Where - cbegin();
		m_core.erase(_Where);
		set_end();
		return begin() + __old_pos;
	}

	WJR_CONSTEXPR20 void swap(basic_string& other)noexcept { 
		m_core.swap(other.m_core);
		set_end();
		other.set_end();
	}

	WJR_INTRINSIC_CONSTEXPR20 const value_type* c_str() const { return m_core.data(); }
	WJR_INTRINSIC_CONSTEXPR20 value_type* data() { return m_core.data(); }
	WJR_INTRINSIC_CONSTEXPR20 const value_type* data() const { return m_core.data(); }

	WJR_INTRINSIC_CONSTEXPR20 bool empty()const { return m_core.empty(); }
	
	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const basic_string& other) const noexcept{
		return compare(view(other));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const size_type off1, const size_type n1, const basic_string& other) const {
		return compare(off1, n1, view(other));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const size_type off1, const size_type n1, const basic_string& other,
		const size_type off2, const size_type n2 = npos) const {
		return compare(off1, n1, view(other, off2, n2));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const value_type* s) const {
		return compare(view(s));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const size_type off1, const size_type n1, const value_type* s) const {
		return compare(off1, n1, view(s));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const size_type off1, const size_type n1, const value_type* s,
		const size_type n2) const {
		return compare(off1, n1, view(s, n2));
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const StringView& t) const noexcept {
		return view().compare(view(t));
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const size_type off1, const size_type n1, const StringView& t) const {
		return view(*this, off1, n1).compare(view(t));
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 int compare(const size_type off1, const size_type n1, const StringView& t,
		const size_type off2, const size_type n2 = npos) const {
		return compare(off1, n1, view(t, off2, n2));
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool starts_with(const StringView& t) const noexcept {
		return view().starts_with(view(t));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool starts_with(const value_type ch) const noexcept {
		return view().starts_with(ch);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool starts_with(const value_type* const ptr) const {
		return view().starts_with(ptr);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool ends_with(const StringView& t) const noexcept {
		return view().ends_with(view(t));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool ends_with(const value_type ch) const noexcept {
		return view().ends_with(ch);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool ends_with(const value_type* const ptr) const {
		return view().ends_with(ptr);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool contains(const StringView& t) const noexcept {
		return view().contains(view(t));
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool contains(const value_type ch) const noexcept {
		return view().contains(ch);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool contains(const value_type* const ptr) const {
		return view().contains(ptr);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type find(const StringView& t, const size_type off = 0) const noexcept{
		return view().find(view(t), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find(const value_type ch, const size_type off = 0) const noexcept {
		return view().find(ch, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find(const value_type* const ptr, 
		const size_type off, const size_type n) const {
		return view().find(ptr, off, n);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find(const value_type* const ptr, const size_type off = 0) const {
		return view().find(ptr, off);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type rfind(const StringView& t, const size_type off = npos) const noexcept {
		return view().rfind(view(t), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type rfind(const value_type ch, const size_type off = npos) const noexcept {
		return view().rfind(ch, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type rfind(const value_type* const ptr,
		const size_type off, const size_type n) const {
		return view().rfind(ptr, off, n);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type rfind(const value_type* const ptr, const size_type off = npos) const {
		return view().rfind(ptr, off);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_of(
		const StringView& t, const size_type off = 0) const noexcept {
		return view().find_first_of(view(t), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_of(
		const value_type ch, const size_type off = 0) const noexcept {
		return view().find_first_of(ch, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_of(const value_type* const ptr,
		const size_type off, const size_type n) const {
		return view().find_first_of(ptr, off, n);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_of(
		const value_type* const ptr, const size_type off = 0) const {
		return view().find_first_of(ptr, off);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_of(
		const StringView& t, const size_type off = npos) const noexcept {
		return view().find_last_of(view(t), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_of(
		const value_type ch, const size_type off = npos) const noexcept {
		return view().find_last_of(ch, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_of(const value_type* const ptr,
		const size_type off, const size_type n) const {
		return view().find_last_of(ptr, off, n);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_of(
		const value_type* const ptr, const size_type off = npos) const {
		return view().find_last_of(ptr, off);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_not_of(
		const StringView& t, const size_type off = 0) const noexcept {
		return view().find_first_not_of(view(t), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_not_of(
		const value_type ch, const size_type off = 0) const noexcept {
		return view().find_first_not_of(ch, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_not_of(const value_type* const ptr,
		const size_type off, const size_type n) const {
		return view().find_first_not_of(ptr, off, n);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_first_not_of(
		const value_type* const ptr, const size_type off = 0) const {
		return view().find_first_not_of(ptr, off);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_not_of(
		const StringView& t, const size_type off = npos) const noexcept {
		return view().find_last_not_of(view(t), off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_not_of(
		const value_type ch, const size_type off = npos) const noexcept {
		return view().find_last_not_of(ch, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_not_of(const value_type* const ptr,
		const size_type off, const size_type n) const {
		return view().find_last_not_of(ptr, off, n);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 size_type find_last_not_of(
		const value_type* const ptr, const size_type off = npos) const {
		return view().find_last_not_of(ptr, off);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 basic_string substr(
		const size_type off = 0, const size_type n = npos) const {
		return basic_string(except_view(*this, off, n));
	}

	WJR_CONSTEXPR20 basic_string& replace(size_type off, size_type n,
		const basic_string& str) {
		return replace(off, n, view(str));
	}

	WJR_CONSTEXPR20 basic_string& replace(const_iterator first, const_iterator last,
		const basic_string& str) {
		return replace(first, last, view(str));
	}

	WJR_CONSTEXPR20 basic_string& replace(size_type off1, size_type n1,
		const basic_string& str,
		size_type off2, size_type n2 = npos) {
		return replace(off1, n1, view(str, off2, n2));
	}

	template<typename iter>
	WJR_CONSTEXPR20 basic_string& replace(const_iterator first, const_iterator last,
		iter first2, iter last2) {
		m_core.replace(first, last, first2, last2);
		set_end();
		return *this;
	}

	WJR_CONSTEXPR20 basic_string& replace(size_type off, size_type n1,
		const value_type* ptr, size_type n2) {
		const auto sv = view(*this, off, n1);
		return replace(sv.begin(), sv.end(), view(ptr, n2));
	}

	WJR_CONSTEXPR20 basic_string& replace(const_iterator first, const_iterator last,
		const value_type* ptr, size_type n2) {
		return replace(first, last, view(ptr, n2));
	}

	WJR_CONSTEXPR20 basic_string& replace(size_type off, size_type n1,
		const value_type* ptr) {
		return replace(off, n1, view(ptr));
	}

	WJR_CONSTEXPR20 basic_string& replace(const_iterator first, const_iterator last,
		const value_type* ptr) {
		return replace(first, last, view(ptr));
	}

	WJR_CONSTEXPR20 basic_string& replace(size_type off, size_type n1,
		size_type n2, value_type ch) {
		const auto sv = view(*this, off, n1);
		return replace(sv.begin(), sv.end(), n2, ch);
	}

	WJR_CONSTEXPR20 basic_string& replace(const_iterator first, const_iterator last,
		size_type n2, value_type ch) {
		m_core.replace(first, last, n2, ch);
		set_end();
		return *this;
	}

	WJR_CONSTEXPR20 basic_string& replace(const_iterator first, const_iterator last,
		std::initializer_list<value_type> il) {
		return replace(first, last, view(il));
	}

	template <typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& replace(size_type off, size_type n,
		const StringView& t) {
		const auto sv = view(*this, off, n);
		return replace(sv.begin(), sv.end(), view(t));
	}

	template <typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& replace(const_iterator first, const_iterator last,
		const StringView& t) {
		const auto sv = view(t);
		return replace(first, last, sv.begin(), sv.end());
	}

	template <typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& replace(size_type off1, size_type n1, const StringView& t,
		size_type off2, size_type n2 = npos) {
		return replace(off1, n1, view(t, off2, n2));
	}
	
	WJR_CONSTEXPR20 allocator_type& get_allocator() {
		return m_core.get_allocator();
	}

	WJR_CONSTEXPR20 const allocator_type& get_allocator()const {
		return m_core.get_allocator();
	}

	// non-standard extension functions

	/*------Internal management function------*/

	WJR_CONSTEXPR20 static void copyConstruct(_Alty& al, const data_type& _Src, data_type& _Dest)
		noexcept(noexcept(vector_type::copyConstruct(al, _Src, _Dest))) {
		vector_type::copyConstruct(al, _Src, _Dest);
	}

	WJR_CONSTEXPR20 static void moveConstruct(_Alty& al, data_type&& _Src, data_type& _Dest)
		noexcept(noexcept(vector_type::moveConstruct(al, std::move(_Src), _Dest))) {
		vector_type::moveConstruct(al, std::move(_Src), _Dest);
	}

	WJR_CONSTEXPR20 static void Swap(_Alty& al, data_type& _Left, data_type& other)
		noexcept(noexcept(vector_type::Swap(al, _Left, other))) {
		vector_type::Swap(al, _Left, other);
	}

	WJR_CONSTEXPR20 static void Destroy(_Alty& al, data_type& _Data) {
		vector_type::Destroy(al, _Data);
	}

	WJR_CONSTEXPR20 static void Deallocate(_Alty& al, data_type& _Data)
		noexcept(noexcept(data_type::Deallocate(al, _Data))) {
		vector_type::Deallocate(al, _Data);
	}

	WJR_CONSTEXPR20 static void Tidy(_Alty& al, data_type& _Data) {
		vector_type::Tidy(al, _Data);
	}

	WJR_INTRINSIC_CONSTEXPR20 static size_type getGrowthCapacity(
		const size_type _Oldcapacity, const size_type _Newsize) noexcept {
		return vector_type::getGrowthCapacity(_Oldcapacity, _Newsize);
	}

	WJR_CONSTEXPR20 static void shrinkToFit(_Alty& al, data_type& _Data) {
		vector_type::shrinkToFit(al, _Data);
	}

	WJR_INTRINSIC_CONSTEXPR20 _Alty& getAllocator() noexcept {
		return m_core.getAllocator();
	}
	WJR_INTRINSIC_CONSTEXPR20 const _Alty& getAllocator() const noexcept {
		return m_core.getAllocator();
	}

	WJR_INTRINSIC_CONSTEXPR20 data_type& getData() noexcept {
		return m_core.getData();
	}
	WJR_INTRINSIC_CONSTEXPR20 const data_type& getData() const noexcept {
		return m_core.getData();
	}

	WJR_INTRINSIC_CONSTEXPR20 pointer lastPtr() noexcept {
		return getData().lastPtr();
	}
	WJR_INTRINSIC_CONSTEXPR20 const_pointer lastPtr() const noexcept {
		return getData().lastPtr();
	}

	WJR_INTRINSIC_CONSTEXPR20 pointer endPtr() noexcept {
		return getData().endPtr();
	}
	WJR_INTRINSIC_CONSTEXPR20 const_pointer endPtr() const noexcept {
		return getData().endPtr();
	}

	WJR_INTRINSIC_CONSTEXPR20 void set_size(const size_type _Size) noexcept {
		getData().set_size(_Size);
	}

	WJR_INTRINSIC_CONSTEXPR20 void inc_size(const difference_type _Size) noexcept {
		getData().inc_size(_Size);
	}

	WJR_INTRINSIC_CONSTEXPR20 void set_end() {
		wjr::construct_at(getAllocator(), lastPtr());
	}

	/*------External extension function------*/

	WJR_NODISCARD WJR_CONSTEXPR20 basic_string noexcept_substr(
		const size_type off = 0, const size_type n = npos) const {
		return basic_string(view(*this, off, n));
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR20 static view_type view(const basic_string& str) noexcept {
		return view_type::view(str);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR20 static view_type view(const basic_string& str,
		const size_type off, const size_type n = npos) noexcept {
		return view_type::view(str, off, n);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR20 static view_type view(const value_type* s, const size_type n) noexcept {
		return view_type::view(s, n);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR20 static view_type view(const value_type* s) noexcept {
		return view_type::view(s);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR20 static view_type view(std::initializer_list<value_type> il) noexcept {
		return view_type::view(il);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR20 static view_type view(const StringView& t) noexcept {
		return view_type::view(t);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR20 static view_type view(const StringView& t,
		const size_type off, const size_type n = npos) noexcept{
		return view_type::view(t, off, n);
	}

	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR20 view_type view() const noexcept {
		return view(*this);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_INTRINSIC_CONSTEXPR20 static view_type except_view(const StringView& t,
		const size_type off, const size_type n = npos) WJR_NOEXCEPT {
		return view_type::except_view(t, off, n);
	}

	WJR_CONSTEXPR20 basic_string& prepend(const basic_string& other) {
		return prepend(view(other));
	}

	WJR_CONSTEXPR20 basic_string& prepend(const basic_string& other, const size_type off, const size_type n = npos) {
		return prepend(view(other, off, n));
	}

	template<typename iter, std::enable_if_t<is_iterator_v<iter>, int> = 0>
	WJR_CONSTEXPR20 basic_string& prepend(iter first, iter last) {
		insert(begin(), first, last);
		return *this;
	}

	WJR_CONSTEXPR20 basic_string& prepend(const value_type* s, size_type n) {
		return prepend(view(s, n));
	}

	WJR_CONSTEXPR20 basic_string& prepend(const value_type* s) {
		return prepend(view(s));
	}

	WJR_CONSTEXPR20 basic_string& prepend(std::initializer_list<value_type> il) {
		return prepend(view(il));
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& prepend(const StringView& t) {
		const auto sv = view_type(t);
		return prepend(sv.data(), sv.data() + sv.size());
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_CONSTEXPR20 basic_string& prepend(const StringView& t,
		const size_type off, const size_type n = npos) {
		return prepend(view(t, off, n));
	}

	WJR_CONSTEXPR20 basic_string& prepend(const size_type n, const value_type c) {
		insert(begin(), n, c);
		return *this;
	}

	WJR_CONSTEXPR20 basic_string& prepend(const value_type c) {
		m_core.emplace(m_core.begin(), c);
		set_end();
		return *this;
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const StringView& t) const noexcept {
		return view().equal(t);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const size_type off, const size_type n, const StringView& t)
		const WJR_NOEXCEPT {
		return view().equal(off, n, t);
	}

	template<typename StringView, std::enable_if_t<_Is_noptr_string_view_like_v<StringView>, int> = 0>
	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const size_type off1, const size_type n1, const StringView& t,
		const size_type off2, const size_type n2) const WJR_NOEXCEPT {
		return view().equal(off1, n1, t, off2, n2);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const Char* const ptr) const {
		return view().equal(ptr);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const size_type off, const size_type n, const Char* const ptr) const {
		return view().equal(off, n, ptr);
	}

	WJR_NODISCARD WJR_CONSTEXPR20 bool equal(const size_type off1, const size_type n1,
		const Char* const ptr, const size_type n2) const {
		return view().equal(off1, n1, ptr, n2);
	}

private:
	vector_type m_core;
};

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator==(
	const basic_string<Char, Alloc, Data>& lhs, 
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return lhs.equal(rhs);
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator==(
	const basic_string<Char, Alloc, Data>& lhs,
	const Char* rhs) noexcept {
	return lhs.equal(rhs);
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator==(
	const Char* lhs,
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return rhs == lhs;
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator!=(
	const basic_string<Char, Alloc, Data>& lhs,
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return !(lhs == rhs);
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator!=(
	const basic_string<Char, Alloc, Data>& lhs,
	const Char* rhs) noexcept {
	return !(lhs == rhs);
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator!=(
	const Char* lhs,
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return !(lhs == rhs);
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<(
	const basic_string<Char, Alloc, Data>& lhs,
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return lhs.compare(rhs) < 0;
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<(
	const basic_string<Char, Alloc, Data>& lhs,
	const Char* rhs) noexcept {
	return lhs.compare(rhs) < 0;
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<(
	const Char* lhs,
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return rhs.compare(lhs) > 0;
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>(
	const basic_string<Char, Alloc, Data>& lhs,
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return rhs < lhs;
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>(
	const basic_string<Char, Alloc, Data>& lhs,
	const Char* rhs) noexcept {
	return rhs < lhs;
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>(
	const Char* lhs,
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return rhs < lhs;
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<=(
	const basic_string<Char, Alloc, Data>& lhs,
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return !(rhs < lhs);
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<=(
	const basic_string<Char, Alloc, Data>& lhs,
	const Char* rhs) noexcept {
	return !(rhs < lhs);
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator<=(
	const Char* lhs,
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return !(rhs < lhs);
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>=(
	const basic_string<Char, Alloc, Data>& lhs,
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return !(lhs < rhs);
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>=(
	const basic_string<Char, Alloc, Data>& lhs,
	const Char* rhs) noexcept {
	return !(lhs < rhs);
}

template<typename Char, typename Alloc, typename Data>
WJR_NODISCARD WJR_CONSTEXPR20 bool operator>=(
	const Char* lhs,
	const basic_string<Char, Alloc, Data>& rhs) noexcept {
	return !(lhs < rhs);
}

template<typename Char>
std::basic_ostream<Char, std::char_traits<Char>>& operator<<(
	std::basic_ostream<Char, std::char_traits<Char>>& os,
	const basic_string<Char>& str) {
	return os << str.view();
}

template<typename Char>
std::basic_istream<Char, std::char_traits<Char>>& operator>>(
	std::basic_istream<Char, std::char_traits<Char>>& is,
	basic_string<Char>& str) {
	return is;
}

using string = basic_string<char>;
#if defined(WJR_CHAR8_T)
using u8string = basic_string<char8_t>;
#endif // WJR_CHAR8_T
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;
using wstring = basic_string<wchar_t>;

template<typename Char, size_t N, typename Alloc = std::allocator<Char>>
using static_basic_string = basic_string<Char, Alloc, string_static_data<Char, N, Alloc>>;

template<size_t N>
using static_string = static_basic_string<char, N>;
#if defined(WJR_CHAR8_T)
template<size_t N>
using static_u8string = static_basic_string<char8_t, N>;
#endif // WJR_CHAR8_T
template<size_t N>
using static_u16string = static_basic_string<char16_t, N>;
template<size_t N>
using static_u32string = static_basic_string<char32_t, N>;
template<size_t N>
using static_wstring = static_basic_string<wchar_t, N>;

_WJR_END

#endif // __WJR_STRING_H