///
/// @defgroup   RANGE range
///
/// This file implements the range template.
///
/// @author     J
/// @date       2020-12-31

#pragma once

namespace jwezel {

template<class T>
class range
{
	public:

	///
	/// Construct a new range instance with begin.
	///
	/// @param      container  The container
	inline range(typename T::iterator begin, typename T::iterator end):
	begin_(begin), end_(end)
	{}

	///
	/// Get begin iterator.
	///
	/// @return     Iterator to start of range
	inline typename T::iterator begin() const {
		return begin_;
	};

	///
	/// Get end iterator.
	///
	/// @return     Iterator to end of range
	inline typename T::iterator end() const {
		return end_;
	}

	private:
	typename T::iterator begin_;

	typename T::iterator end_;
};

}
