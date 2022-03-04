#pragma once

namespace bhw
{
	template<class T>
	bool find(const std::vector<T> &v, const T &el)
	{
		for (auto it = v.begin(); it != v.end(); ++it)
			if (*it == el)
				return 1;
		return 0;
	}

	template<class U>
	std::set<U> &addSets(std::set<U> &s1, const std::set<U> &s2)
	{
		for_each(s2.begin(), s2.end(), [&s1](const U &el) { s1.insert(el); });
		return s1;
	}
}