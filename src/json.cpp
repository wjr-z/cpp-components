#include <algorithm>
#include <cmath>
#include <string>
#include "../include/json.h"

namespace wjr {
	// It will be modified later
	inline double power_of_10(int index) {
		struct pw_cache {
			double pw10[310], npw10[310];
			pw_cache() {
				pw10[0] = npw10[0] = 1.0;
				for (int i = 1; i <= 309; ++i) {
					pw10[i] = pw10[i - 1] * 10.0;
					npw10[i] = 1.0 / pw10[i];
				}
			}
		};
		static pw_cache _cache;
		assert(index > -310 && index < 310);
		return (index >= 0) ? _cache.pw10[index] : _cache.npw10[-index];
	}

	double read_double(const char* s, const char* e, const char*& next) {
		bool neg = false;
		auto* ptr = (const uint8_t*)skip_whitespace(s, e);
		switch (*ptr) {
		case '-':neg = true; ++ptr; break;
		case '+':++ptr; break;
		}

		unsigned long long v = 0;
		int num = 0;

		for (; qisdigit(*ptr) && num < 18; ++ptr, ++num) {
			v = v * 10 + (*ptr - '0');
		}

		int pw10 = 0;
		for (; qisdigit(*ptr); ++ptr, ++pw10);

		if (*ptr == '.') {
			++ptr;
			for (; qisdigit(*ptr) && num < 18; ++ptr, ++num, --pw10) {
				v = v * 10 + (*ptr - '0');
			}
			for (; qisdigit(*ptr); ++ptr);
		}

		double val = v * power_of_10(pw10);
		if ((*ptr == 'e') || (*ptr == 'E')) {
			++ptr;
			int pw = String_traits_helper<uint8_t>::unsafe_first_to_int(ptr, (const uint8_t*)e, ptr);
			val *= power_of_10(pw);
		}
		if (neg) val = -val;
		next = (const char*)ptr;
		return val;
	}

	inline namespace json_memory {
		constexpr static size_t string_step[256] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
		};

		const uint8_t* skip_string(const uint8_t* s, const uint8_t* e) {
			while (string_step[*s]) {
				s += string_step[*s];
			}
			return s;
		}

		bool check_num(const uint8_t*& s, const uint8_t* e) {
			if (*s == '+' || *s == '-')
				++s;

			if (s == e || !qisdigit(*s))
				return false;

			if (*s == '0') {
				++s;
			}
			else {
				for (; s != e && qisdigit(*s); ++s);
			}

			if (*s == '.') {
				for (++s; s != e && qisdigit(*s); ++s);
			}

			if (*s == 'e' || *s == 'E') {
				++s;
				if (*s == '+' || *s == '-')
					++s;
				if (s == e || !qisdigit(*s))
					return false;
				for (; s != e && qisdigit(*s); ++s);
			}

			return true;
		}

		bool check_string(const uint8_t*& s, const uint8_t* e) {
			if (*s != '"')return false;
			++s;
			while (s != e && *s != '"') {
				if (*s == '\\') {
					++s;
					if (s == e)return false;
					switch (*s) {
					case '"':
					case '\\':
					case '/':
					case 'b':
					case 'f':
					case 'n':
					case 'r':
					case 't':
					case 'u':
						break;
					default:
						return false;
					}
				}
				else {
					if (*s < 32 || *s == 127)
						return false;
				}
				++s;
			}
			return s != e;
		}
	}

	// default constructor
	json::json()
		: vtype((uint8_t)(value_t::undefined)), _Number(0) {
	}

	json::json(const json& other)
		: vtype(other.vtype), _Number(other._Number) {
		// copy Number firstly
		switch (vtype) {
			case (uint8_t)(value_t::string) : {
				_String = mallocator<String>().allocate(1);
				new (_String) String(*other._String);
				break;
			}
			case (uint8_t)(value_t::object) : {
				_Object = mallocator<Object>().allocate(1);
				new (_Object) Object(*other._Object);
				break;
			}
			case (uint8_t)(value_t::array) : {
				_Array = mallocator<Array>().allocate(1);
				new (_Array) Array(*other._Array);
				break;
			}
		}
	}

	json::json(json&& other)noexcept
		: vtype(other.vtype), _Number(other._Number) {
		other.vtype = (uint8_t)(value_t::undefined);
	}

	json::json(Null)
		: vtype((uint8_t)(value_t::null)), _Number(0) {
	}

	json::json(Boolean f)
		: vtype((uint8_t)(value_t::boolean)), _Boolean(f) {
	}

	json::json(Number v)
		: vtype((uint8_t)(value_t::number)), _Number(v) {
	}

	json::json(int v)
		: vtype((uint8_t)(value_t::number)), _Number(v) {
	}

	json::json(unsigned int v)
		: vtype((uint8_t)(value_t::number)), _Number(v) {
	}

	json::json(long long v)
		: vtype((uint8_t)(value_t::number)), _Number(v) {
	}

	json::json(unsigned long long v)
		: vtype((uint8_t)(value_t::number)), _Number(v) {
	}

	json::json(const Object& v)
		: vtype((uint8_t)(value_t::object)), _Object(mallocator<Object>().allocate(1)) {
		new (_Object) Object(v);
	}

	json::json(Object&& v)noexcept
		: vtype((uint8_t)(value_t::object)), _Object(mallocator<Object>().allocate(1)) {
		new (_Object) Object(std::move(v));
	}

	json::json(const Array& v)
		: vtype((uint8_t)(value_t::array)), _Array(mallocator<Array>().allocate(1)) {
		new (_Array) Array(v);
	}

	json::json(Array&& v)noexcept
		: vtype((uint8_t)(value_t::array)), _Array(mallocator<Array>().allocate(1)) {
		new (_Array) Array(std::move(v));
	}

	json::json(const size_type _Count, const json& _Val)
		: vtype((uint8_t)(value_t::array)), _Array(mallocator<Array>().allocate(1)) {
		new (_Array) Array(_Count, _Val);
	}

	json::json(std::initializer_list<json> il)
		: vtype((uint8_t)(value_t::undefined)), _Number(0) {
		bool is_object = std::all_of(il.begin(), il.end(),
			[](const json& json_ref) {
				return json_ref.is_array() && json_ref.size() == 2 && json_ref[0].is_string();
			}
		);
		if (is_object) {
			set_object();
			for (const json* i = il.begin(); i != il.end(); ++i) {
				json& element = *const_cast<json*>(i);
				insert(std::move(element[0].to_string()), std::move(element[1]));
			}
		}
		else {
			set_array();
			for (const json* i = il.begin(); i != il.end(); ++i) {
				json& element = *const_cast<json*>(i);
				push_back(std::move(element));
			}
		}
	}

	void json::_Tidy() {
		switch (vtype) {
			case (uint8_t)(value_t::string) : {
				_String->~String();
				mallocator<String>().deallocate(_String, 1);
				break;
			}
			case (uint8_t)(value_t::object) : {
				_Object->~Object();
				mallocator<Object>().deallocate(_Object, 1);
				break;
			}
			case (uint8_t)(value_t::array) : {
				_Array->~Array();
				mallocator<Array>().deallocate(_Array, 1);
				break;
			}
			default:
				break;
		}
	}

	void json::reset() {
		_Tidy();
		vtype = (uint8_t)(value_t::undefined);
	}

	void json::clear() {
		reset();
	}

	void json::swap(json& other) noexcept {
		std::swap(_Number, other._Number);
		std::swap(vtype, other.vtype);
	}

	json::~json() {
		_Tidy();
	}

	json& json::operator=(const json& other) {
		if (this == &other) {
			return *this;
		}
		// different type
		if (vtype != other.vtype) {
			_Tidy();
			vtype = other.vtype;
			switch (vtype) {
				case (uint8_t)(value_t::string) : {
					_String = mallocator<String>().allocate(1);
					new (_String) String(*other._String);
					break;
				}
				case (uint8_t)(value_t::object) : {
					_Object = mallocator<Object>().allocate(1);
					new (_Object) Object(*other._Object);
					break;
				}
				case (uint8_t)(value_t::array) : {
					_Array = mallocator<Array>().allocate(1);
					new (_Array) Array(*other._Array);
					break;
				}
				default: {
					_Number = other._Number;
					break;
				}
			}
		}
		else {
			switch (vtype) {
				case (uint8_t)(value_t::string) : {
					*_String = *other._String;
					break;
				}
				case (uint8_t)(value_t::object) : {
					*_Object = *other._Object;
					break;
				}
				case (uint8_t)(value_t::array) : {
					*_Array = *other._Array;
					break;
				}
				default: {
					_Number = other._Number;
					break;
				}
			}
		}
		return *this;
	}

	json& json::operator=(json&& other)noexcept {
		_Tidy();
		_Number = other._Number;
		vtype = other.vtype;
		other.vtype = (uint8_t)(value_t::undefined);
		return *this;
	}

	json& json::operator=(Null) {
		_Tidy();
		vtype = (uint8_t)(value_t::null);
		return *this;
	}

	json& json::operator=(Boolean f) {
		_Tidy();
		_Boolean = f;
		vtype = (uint8_t)(value_t::boolean);
		return *this;
	}

	json& json::operator=(Number v) {
		_Tidy();
		_Number = v;
		vtype = (uint8_t)(value_t::number);
		return *this;
	}

	json& json::operator=(int v) {
		_Tidy();
		_Number = v;
		vtype = (uint8_t)(value_t::number);
		return *this;
	}

	json& json::operator=(unsigned int v) {
		_Tidy();
		_Number = v;
		vtype = (uint8_t)(value_t::number);
		return *this;
	}

	json& json::operator=(long long v) {
		_Tidy();
		_Number = v;
		vtype = (uint8_t)(value_t::number);
		return *this;
	}

	json& json::operator=(unsigned long long v) {
		_Tidy();
		_Number = v;
		vtype = (uint8_t)(value_t::number);
		return *this;
	}

	json& json::operator=(const Object& v) {
		if (vtype != (uint8_t)(value_t::object)) {
			_Tidy();
			_Object = mallocator<Object>().allocate(1);
			new (_Object) Object(v);
			vtype = (uint8_t)(value_t::object);
		}
		else {
			*_Object = v;
		}
		return *this;
	}

	json& json::operator=(Object&& v)noexcept {
		if (vtype != (uint8_t)(value_t::object)) {
			_Tidy();
			_Object = mallocator<Object>().allocate(1);
			new (_Object) Object(std::move(v));
			vtype = (uint8_t)(value_t::object);
		}
		else {
			*_Object = std::move(v);
		}
		return *this;
	}

	json& json::operator=(const Array& v) {
		if (vtype != (uint8_t)(value_t::array)) {
			_Tidy();
			_Array = mallocator<Array>().allocate(1);
			new (_Array) Array(v);
			vtype = (uint8_t)(value_t::array);
		}
		else {
			*_Array = v;
		}
		return *this;
	}

	json& json::operator=(Array&& v)noexcept {
		if (vtype != (uint8_t)(value_t::array)) {
			_Tidy();
			_Array = mallocator<Array>().allocate(1);
			new (_Array) Array(std::move(v));
			vtype = (uint8_t)(value_t::array);
		}
		else {
			*_Array = std::move(v);
		}
		return *this;
	}

	bool json::is_null()const {
		return vtype == (uint8_t)(value_t::null);
	}

	bool json::is_boolean()const {
		return vtype == (uint8_t)(value_t::boolean);
	}

	bool json::is_number()const {
		return vtype == (uint8_t)(value_t::number);
	}

	bool json::is_string()const {
		return vtype == (uint8_t)(value_t::string);
	}

	bool json::is_object()const {
		return vtype == (uint8_t)(value_t::object);
	}

	bool json::is_array()const {
		return vtype == (uint8_t)(value_t::array);
	}

	json::Null json::to_null() {
		assert(is_null());
		return nullptr;
	}

	json::Null json::to_null()const {
		assert(is_null());
		return nullptr;
	}

	json::Boolean& json::to_boolean() {
		assert(is_boolean());
		return _Boolean;
	}

	json::Boolean json::to_boolean()const {
		assert(is_boolean());
		return _Boolean;
	}

	json::Number& json::to_number() {
		assert(is_number());
		return _Number;
	}

	json::Number json::to_number()const {
		assert(is_number());
		return _Number;
	}

	json::String& json::to_string() {
		assert(is_string());
		return *_String;
	}

	const json::String& json::to_string()const {
		assert(is_string());
		return *_String;
	}

	json::Object& json::to_object() {
		assert(is_object() || empty());
		if (empty()) {
			set_object();
		}
		return *_Object;
	}

	const json::Object& json::to_object()const {
		assert(is_object());
		return *_Object;
	}

	json::Array& json::to_array() {
		assert(is_array() || empty());
		if (empty()) {
			set_array();
		}
		return *_Array;
	}

	const json::Array& json::to_array()const {
		assert(is_array());
		return *_Array;
	}

	bool json::get_value(Null& v) const {
		if (vtype != (uint8_t)(value_t::null)) {
			return false;
		}
		v = nullptr;
		return true;
	}

	bool json::get_value(Boolean& f) const {
		if (vtype != (uint8_t)(value_t::boolean)) {
			return false;
		}
		f = _Boolean;
		return true;
	}

	bool json::get_value(Number& v) const {
		if (vtype != (uint8_t)(value_t::number)) {
			return false;
		}
		v = _Number;
		return true;
	}

	bool json::get_value(int& v) const {
		if (vtype != (uint8_t)(value_t::number)) {
			return false;
		}
		v = _Number;
		return true;
	}

	bool json::get_value(unsigned int& v) const {
		if (vtype != (uint8_t)(value_t::number)) {
			return false;
		}
		v = _Number;
		return true;
	}

	bool json::get_value(long long& v) const {
		if (vtype != (uint8_t)(value_t::number)) {
			return false;
		}
		v = _Number;
		return true;
	}

	bool json::get_value(unsigned long long& v) const {
		if (vtype != (uint8_t)(value_t::number)) {
			return false;
		}
		v = _Number;
		return true;
	}

	bool json::get_value(String& v) const {
		if (vtype != (uint8_t)(value_t::string)) {
			return false;
		}
		v = *_String;
		return true;
	}

	bool json::get_value(Object& v) const {
		if (vtype != (uint8_t)(value_t::object)) {
			return false;
		}
		v = *_Object;
		return true;
	}

	bool json::get_value(Array& v) const {
		if (vtype != (uint8_t)(value_t::array)) {
			return false;
		}
		v = *_Array;
		return true;
	}

	json& json::operator[](const size_t index) {
		return to_array()[index];
	}

	const json& json::operator[](const size_t index)const {
		return to_array()[index];
	}

	json& json::at(const size_t index) {
		return to_array().at(index);
	}

	const json& json::at(const size_t index)const {
		return to_array().at(index);
	}

	void json::push_back(const json& data) {
		to_array().push_back(data);
	}

	void json::push_back(json&& data) {
		to_array().push_back(std::move(data));
	}

	void json::pop_back() {
		assert(is_array());
		_Array->pop_back();
	}

	json::Object& json::set_object() {
		if (vtype == (uint8_t)value_t::object) { return *_Object; }
		_Tidy();
		_Object = mallocator<Object>().allocate(1);
		new (_Object) Object();
		vtype = (uint8_t)(value_t::object);
		return *_Object;
	}

	json::Array& json::set_array() {
		if (vtype == (uint8_t)value_t::array) { return *_Array; }
		_Tidy();
		_Array = mallocator<Array>().allocate(1);
		new (_Array) Array();
		vtype = (uint8_t)(value_t::array);
		return *_Array;
	}

	size_t json::size()const {
		assert(is_array() || is_object());
		return is_array() ? _Array->size() : _Object->size();
	}

	bool json::count(const size_t index)const {
		assert(is_array());
		return _Array->size() > index;
	}

	bool json::empty()const {
		return vtype == (uint8_t)(value_t::undefined);
	}

	bool json::has_value()const {
		return vtype != (uint8_t)(value_t::undefined);
	}

	json_string json::stringify(int tab)const {
		return stringify(0, tab);
	}

	json_string json::minify()const {
		json_string str;
		_minify(str);
		return str;
	}

	void json::MergePatch(json& Target, const json& Patch) {
		if (Patch.is_object()) {
			if (!Target.is_object()) {
				Target.set_object();
			}
			auto& lhs_obj = Target.to_object();
			auto& rhs_obj = Patch.to_object();
			for (auto& Value : rhs_obj) {
				if (Value.second.is_null()) {
					auto iter = lhs_obj.find(Value.first);
					if (iter != lhs_obj.end())
						lhs_obj.erase(iter);
				}
				else {
					MergePatch(lhs_obj[Value.first], Value.second);
				}
			}
		}
		else {
			Target = Patch;
		}
	}

	void json::_minify(json_string& str)const {
		switch (vtype) {
			case (uint8_t)(value_t::null) : {
				str.append("null");
				break;
			}
			case (uint8_t)(value_t::boolean) : {
				if (to_boolean())
					str.append("true");
				else str.append("false");
				break;
			}
			case (uint8_t)(value_t::number) : {
				str.append(json_string::fixed_number(to_number()));
				break;
			}
			case (uint8_t)(value_t::string) : {
				str.multiple_append('"', to_string(), '"');
				break;
			}
			case (uint8_t)(value_t::object) : {
				str.push_back('{');
				auto& obj = to_object();
				if (!obj.empty()) {
					bool head = true;
					for (auto& i : obj) {
						if (head) head = false;
						else str.push_back(',');
						str.multiple_append('"', i.first, "\":");
						i.second._minify(str);
					}
				}
				str.push_back('}');
				break;
			}
			case (uint8_t)(value_t::array) : {
				str.push_back('[');
				const auto& umap = to_array();
				if (!umap.empty()) {
					bool head = true;
					for (auto& i : umap) {
						if (head) head = false;
						else str.push_back(',');
						i._minify(str);
					}
				}
				str.push_back(']');
				break;
			}
			default: {
				assert(false);
				break;
			}
		}
	}

	const char* json::type_name()const {
		switch (vtype) {
			case (uint8_t)(value_t::undefined) : {
				return "undefined";
			}
			case (uint8_t)(value_t::null) : {
				return "null";
			}
			case (uint8_t)(value_t::boolean) : {
				return "boolean";
			}
			case (uint8_t)(value_t::number) : {
				return "number";
			}
			case (uint8_t)(value_t::string) : {
				return "string";
			}
			case (uint8_t)(value_t::object) : {
				return "object";
			}
			case (uint8_t)(value_t::array) : {
				return "array";
			}
			default: {
				assert(false);
				return "error";
			}
		}
	}

	void json::merge_patch(const json& apply_patch) {
		MergePatch(*this, apply_patch);
	}

	json json::parse(String_view str) {
		assert(accept(str));
		json x;
		const uint8_t* ptr = (const uint8_t*)str.data();
		x.dfs_parse(ptr, ptr + str.length());
		return x;
	}

	bool check(const uint8_t*& s, const uint8_t* e, uint8_t state) {
		bool head = true;
		for (;; ++s) {
			s = (uint8_t*)skip_whitespace(s, e);
			if (s == e)return false;
			if (state == '}') { // object
				if (*s == '}') {
					if (!head) return false;
					++s;
					return true;
				}
				if (*s != '"')return false;
				if (!check_string(s, e))
					return false;
				++s;
				s = (uint8_t*)skip_whitespace(s, e);
				if (s == e || *s != ':') return false;
				++s;
				s = (uint8_t*)skip_whitespace(s, e);
				if (s == e) return false;
			}
			else {
				if (*s == ']') {
					if (!head)return false;
					++s;
					return true;
				}
			}
			head = false;

			switch (*s) {
			case 'n':
				if (memcmp(s, "null", sizeof(char) * 4) != 0)
					return false;
				s += 4;
				break;
			case 't':
				if (memcmp(s, "true", sizeof(char) * 4) != 0)
					return false;
				s += 4;
				break;
			case 'f':
				if (memcmp(s, "false", sizeof(char) * 5) != 0)
					return false;
				s += 5;
				break;
			case '"':
				if (!check_string(s, e))
					return false;
				++s;
				break;
			case '[':
				++s;
				if (!check(s, e, ']'))
					return false;
				break;
			case '{':
				++s;
				if (!check(s, e, '}'))
					return false;
				break;
			default:
				if (!check_num(s, e))
					return false;
				break;
			}

			s = (uint8_t*)skip_whitespace(s, e);
			if (*s == ',')
				continue;
			if (*s != state)
				return false;
			++s;
			return true;
		}
	}

	bool json::accept(String_view str) {
		const uint8_t* s = (const uint8_t*)str.data();
		const uint8_t* e = s + str.length();

		s = (uint8_t*)skip_whitespace(s, e);

		if (s == e) // empty
			return false;

		switch (*s) {
		case '[':
			++s;
			if (!check(s, e, ']'))
				return false;
			s = (uint8_t*)skip_whitespace(s, e);
			return s == e;
		case '{':
			++s;
			if (!check(s, e, '}'))
				return false;
			s = (uint8_t*)skip_whitespace(s, e);
			return s == e;
		default:
			return false;
		}
	}

	json json::array(std::initializer_list<json> il) {
		json x;
		x.vtype = (uint8_t)(json::value_t::array);
		x._Array = mallocator<Array>().allocate(1);
		new (x._Array) Array(il);
		return x;
	}

	json json::object(std::initializer_list<std::pair<const String, json>> il) {
		json x;
		x.vtype = (uint8_t)(json::value_t::object);
		x._Object = mallocator<Object>().allocate(1);
		new (x._Object) Object(il);
		return x;
	}

	bool operator==(const json& lhs, const json& rhs) {
		if (lhs.vtype != rhs.vtype)return false;
		switch (lhs.vtype) {
			case (uint8_t)(json::value_t::boolean) : {
				return lhs._Boolean == rhs._Boolean;
			}
			case (uint8_t)(json::value_t::number) : {
				constexpr static double eps = 1e-3;
				return std::fabs(lhs._Number - rhs._Number) < eps;
			}
			case (uint8_t)(json::value_t::string) : {
				return lhs.to_string() == rhs.to_string();
			}
			case (uint8_t)(json::value_t::array) : {
				return lhs.to_array() == rhs.to_array();
			}
			case (uint8_t)(json::value_t::object) : {
				return lhs.to_object() == rhs.to_object();
			}
			default: {
				return true;
			}
		}
	}

	bool operator!=(const json& lhs, const json& rhs) {
		return !(lhs == rhs);
	}

	bool operator<(const json& lhs, const json& rhs) {
		assert(lhs.vtype == rhs.vtype);
		switch (lhs.vtype) {
			case (uint8_t)(json::value_t::boolean) : {
				return lhs._Boolean < rhs._Boolean;
			}
			case (uint8_t)(json::value_t::number) : {
				return lhs._Number < rhs._Number;
			}
			case (uint8_t)(json::value_t::string) : {
				return lhs.to_string() < rhs.to_string();
			}
			case (uint8_t)(json::value_t::array) : {
				return lhs.to_array() < rhs.to_array();
			}
			case (uint8_t)(json::value_t::object) : {
				return lhs.to_object() < rhs.to_object();
			}
			default: {
				return false;
			}
		}
	}

	bool operator<=(const json& lhs, const json& rhs) {
		assert(lhs.vtype == rhs.vtype);
		switch (lhs.vtype) {
			case (uint8_t)(json::value_t::boolean) : {
				return lhs._Boolean <= rhs._Boolean;
			}
			case (uint8_t)(json::value_t::number) : {
				constexpr static double eps = 1e-3;
				return lhs._Number < rhs._Number + eps;
			}
			case (uint8_t)(json::value_t::string) : {
				return lhs.to_string() <= rhs.to_string();
			}
			case (uint8_t)(json::value_t::array) : {
				return lhs.to_array() <= rhs.to_array();
			}
			case (uint8_t)(json::value_t::object) : {
				return lhs.to_object() <= rhs.to_object();
			}
			default: {
				return true;
			}
		}
	}

	bool operator>(const json& lhs, const json& rhs) {
		return !(rhs <= lhs);
	}

	bool operator>=(const json& lhs, const json& rhs) {
		return !(rhs < lhs);
	}

	std::ostream& operator<<(std::ostream& out, const json& x) {
		return out << x.stringify();
	}

	json_string json::format_tab(int tab) {
		return json_string(tab, ' ');
	}

	json_string json::stringify(int tab, int delta)const {
		switch (vtype) {
			case (uint8_t)(value_t::null) : {
				return json_string("null");
			}
			case (uint8_t)(value_t::boolean) : {
				return json_string(to_boolean() ? "true" : "false");
			}
			case (uint8_t)(value_t::number) : {
				return json_string::fixed_number(to_number());
			}
			case (uint8_t)(value_t::string) : {
				return json_string::connect('"', to_string(), '"');
			}
			case (uint8_t)(value_t::object) : {
				json_string str;
				str.push_back('{');
				auto& obj = to_object();
				if (!obj.empty()) {
					bool head = true;
					for (auto& i : obj) {
						if (head) head = false;
						else str.push_back(',');
						str.multiple_append('\n', format_tab(tab + delta),
							'"', i.first, "\": ", i.second.stringify(tab + delta, delta));
					}
					str.multiple_append('\n', format_tab(tab));
				}
				str.push_back('}');
				return str;
			}
			case (uint8_t)(value_t::array) : {
				json_string str;
				str.push_back('[');
				const auto& umap = to_array();
				if (!umap.empty()) {
					bool head = true;
					for (auto& i : umap) {
						if (head) head = false;
						else str.push_back(',');
						str.multiple_append('\n',
							format_tab(tab + 2), i.stringify(tab + delta, delta));
					}
					str.multiple_append('\n', format_tab(tab));
				}
				str.push_back(']');
				return str;
			}
			default: {
				assert(false);
				return json_string("");
			}
		}
	}

	void json::dfs_parse(const uint8_t*& s, const uint8_t* e) {
		s = (uint8_t*)skip_whitespace(s, e);
		switch (*s) {
		case 'n': {
			vtype = (uint8_t)(value_t::null);
			s += 4;
			break;
		}
		case 't': {
			vtype = (uint8_t)(value_t::boolean);
			_Boolean = true;
			s += 4;
			break;
		}
		case 'f': {
			vtype = (uint8_t)(value_t::boolean);
			_Boolean = false;
			s += 5;
			break;
		}
		case '{': {
			vtype = (uint8_t)(value_t::object);
			_Object = mallocator<Object>().allocate(1);
			new (_Object) Object();
			auto& obj = *_Object;
			++s;
			s = (uint8_t*)skip_whitespace(s, e);
			// if is empty
			if (*s == '}') {
				++s;
				break;
			}
			for (;;) {
				s = (uint8_t*)skip_whitespace(s, e);
				assert(*s == '"');
				++s;
				auto p = skip_string(s, e);
				auto& iter = obj[String((const char*)s, p - s)] = json();
				s = (uint8_t*)skip_whitespace(p + 1, e);
				assert(*s == ':');
				++s;
				iter.dfs_parse(s, e);
				s = (uint8_t*)skip_whitespace(s, e);
				if (*s == '}') {
					++s;
					break;
				}
				assert(*s == ',');
				++s;
			}
			break;
		}
		case '[': {
			vtype = (uint8_t)(value_t::array);
			_Array = mallocator<Array>().allocate(1);
			new (_Array) Array();
			auto& arr = *_Array;
			++s;
			s = (uint8_t*)skip_whitespace(s, e);
			if (*s == ']') {
				++s;
				break;
			}
			for (;;) {
				arr.push_back(json());
				arr.back().dfs_parse(s, e);
				s = (uint8_t*)skip_whitespace(s, e);
				if (*s == ']') {
					++s;
					break;
				}
				assert(*s == ',');
				++s;
			}
			arr.shrink_to_fit();
			break;
		}
		case '"': {
			++s;
			auto t = skip_string(s, e);
			vtype = (uint8_t)(value_t::string);
			_String = mallocator<String>().allocate(1);
			new (_String) String((const char*)s, t - s);
			s = t + 1;
			break;
		}
		case '-':
		case '+':
		default: {
			vtype = (uint8_t)(value_t::number);
			const char* ptr = (const char*)s;
			_Number = read_double((const char*)s, (const char*)e, ptr);
			s = (const uint8_t*)ptr;
			break;
		}
		}
	}

	json operator ""_json(const char* str, const size_t) {
		json x;
		x.parse(str);
		return x;
	}
}