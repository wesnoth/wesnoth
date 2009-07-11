#include "skiplist_multimap.hpp"
#include "skiplist_map.hpp"

#include <iostream>
#include <cassert>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#define FORMAT_ITER(x) x->first << ":" << x->second

template <typename T, typename U>
bool equal_range(const T& a, const U& b) {
	typename T::const_iterator a1 = a.begin(), a2 = a.end();
	typename U::const_iterator b1 = b.begin(), b2 = b.end();

	bool ret = true;
	while (a1 != a2 && b1 != b2) {
		std::cout << a1->first << ":" << a1->second  << " " << b1->first << ":" << b1->second << std::endl;
		if (*a1 != *b1) ret = false;
		++a1; ++b1;
	}
	std::cout << std::endl << std::endl;
	return ret && a1 == a2 && b1 == b2;
}

int keys[] = { 5, -1, 3, 2, 7, 5, 3, 2, 7, 8, 3, 4, 5, 6, 7, 3, 2, 1, -5, -3, -4, -5, -6, 2 };
std::string values[] = { "five1", "-one1", "three1", "two1", "seven1", "five2", "three2", "two2", "seven2", "eight1",
	"three3", "four1", "five3", "six1", "seven3", "three4", "two3", "one1", "-five1", "-three1", "-four1", "-five2", "-six1", "two4"};
int size = 24;

void test_multimap() {
	std::cout << "construct()" << std::endl;
	typedef skiplist_multimap<int, std::string> slmap_t;
	slmap_t slmap;
	typedef skiplist_multimap<int, std::string> stdmap_t;
	stdmap_t stdmap;
	assert(equal_range(slmap, stdmap));
	assert(slmap.size() == stdmap.size());
	assert(slmap.empty());

	std::cout << "insert(pair)" << std::endl;
	for (int i = 0; i < size; i++) {
		slmap.insert(std::make_pair(keys[i], values[i]));
		stdmap.insert(std::make_pair(keys[i], values[i]));
		assert(equal_range(slmap, stdmap));
	}

	std::cout << "size()" << std::endl;
	assert(slmap.size() == stdmap.size());

	std::cout << "iterators" << std::endl;
	assert(equal_range(slmap, stdmap));
	{
		slmap_t::iterator slbegin = slmap.begin();
		stdmap_t::iterator stdbegin = stdmap.begin();
		while (slbegin != slmap.end() && stdbegin != stdmap.end()) {
			slbegin++; stdbegin++;
			if (slbegin == slmap.end() || stdbegin == stdmap.end()) break;
			assert(*--slbegin == *--stdbegin);
			assert(*++slbegin == *++stdbegin);
			assert(*slbegin-- == *stdbegin--);
			assert(!(*slbegin++ != *stdbegin++));
		}
	}

	std::cout << "reverse iterators" << std::endl;
	assert(equal_range(slmap, stdmap));
	{
		slmap_t::reverse_iterator slbegin = slmap.rbegin();
		stdmap_t::reverse_iterator stdbegin = stdmap.rbegin();
		while (slbegin != slmap.rend() && stdbegin != stdmap.rend()) {
			slbegin++; stdbegin++;
			if (slbegin == slmap.rend() || stdbegin == stdmap.rend()) break;
			assert(*--slbegin == *--stdbegin);
			assert(*++slbegin == *++stdbegin);
			assert(*slbegin-- == *stdbegin--);
			assert(!(*slbegin++ != *stdbegin++));
		}
	}

	std::cout << "clear" << std::endl;
	slmap.clear();

	std::cout << "empty" << std::endl;
	assert(slmap.empty());
	assert(slmap.size() == 0);

	std::cout << "constructor(forwarditer, forwarditer)" << std::endl;
	std::vector<std::pair<int, std::string> > pv;
	for (int i = 0; i < 100; i++) {
		int t = rand() % size;
		pv.push_back(std::make_pair(keys[t], values[t]));
	}

	for (int i = 0; i < size; i++) {
		slmap.insert(std::make_pair(keys[i], values[i]));
		stdmap.insert(std::make_pair(keys[i], values[i]));
	}

	slmap_t slmap2(pv.begin(), pv.end());
	stdmap_t stdmap2(pv.begin(), pv.end());

	{
		// test destruction after iterator constructor
		slmap_t slmap3(pv.begin(), pv.end());
	}
	assert(equal_range(slmap2, stdmap2));

	std::cout << "constructor(skiplist)" << std::endl;
	{
		slmap_t slmap3(slmap2);
		stdmap_t stdmap3(stdmap2);
		assert(equal_range(slmap3, stdmap3));
	}

	assert(equal_range(slmap2, stdmap2));

	std::cout << "operator=" << std::endl;
	slmap = slmap2;
	stdmap = stdmap2;

	assert(equal_range(slmap, stdmap));
	assert(equal_range(slmap2, stdmap2));


	std::cout << "insert(iter, iter)" << std::endl;
	slmap.insert(pv.begin(), pv.end());
	stdmap.insert(pv.begin(), pv.end());
	assert(equal_range(slmap, stdmap));

	std::cout << "insert(iter, pair)" << std::endl;
	{
		slmap_t slmap2(slmap);
		slmap2.insert(slmap2.insert(std::make_pair(0, "zeroins1")), std::make_pair(0, "zeroins2"));
		assert(std::adjacent_find(slmap.rbegin(), slmap.rend(), slmap.value_comp()) == slmap.rend());
	}

	std::cout << "erase(key)" << std::endl;
	int erase_vals[] = { -3, -10, 3, 5, 100 };
	for (int i = 0; i < 5; i++) {
		slmap.erase(erase_vals[i]);
		stdmap.erase(erase_vals[i]);
	}
	assert(equal_range(slmap, stdmap));

	std::cout << "erase(iter)" << std::endl;
	for (int i = 0; i < 10; i++) {
		assert(std::adjacent_find(slmap.rbegin(), slmap.rend(), slmap.value_comp()) == slmap.rend());
		slmap_t::iterator sliter = slmap.insert(std::make_pair(keys[i], values[i]));
		assert(std::adjacent_find(stdmap.rbegin(), stdmap.rend(), stdmap.value_comp()) == stdmap.rend());
		stdmap_t::iterator stditer = stdmap.insert(std::make_pair(keys[i], values[i]));
		assert(*sliter == *stditer);
		assert(equal_range(slmap, stdmap));
		slmap.erase(sliter);
		stdmap.erase(stditer);
		assert(equal_range(slmap, stdmap));
	}

	std::cout << "erase(iter, iter)" << std::endl;
	for (int i = 0; i < 10; i+=3) {
		slmap_t::iterator slbegin = slmap.insert(std::make_pair(i, values[i])),
			slend = slmap.insert(std::make_pair(i + 1, values[i + 1]));
		stdmap_t::iterator stdbegin = stdmap.insert(std::make_pair(i, values[i])),
			stdend = stdmap.insert(std::make_pair(i + 1, values[i + 1]));
		assert(equal_range(slmap, stdmap));
		slmap.erase(slbegin, slend);
		stdmap.erase(stdbegin, stdend);
		assert(equal_range(slmap, stdmap));
	}
	assert(equal_range(slmap, stdmap));

	std::cout << "swap" << std::endl;
	{
		slmap_t slmap2(pv.begin(), pv.end());
		stdmap_t stdmap2(pv.begin(), pv.end());
		assert(equal_range(slmap2, stdmap2));
		slmap.swap(slmap2);
		stdmap.swap(stdmap2);
		assert(equal_range(slmap, stdmap));
		assert(equal_range(slmap2, stdmap2));
		slmap.swap(slmap2);
		stdmap.swap(stdmap2);
	}

	assert(equal_range(slmap, stdmap));
	slmap.swap(slmap);
	stdmap.swap(stdmap);
	assert(equal_range(slmap, stdmap));

	std::cout << "find" << std::endl;

	for (int i = 0; i < 30; i++) {
		if (slmap.find(i) == slmap.end()) {
			assert(stdmap.find(i) == stdmap.end());
			continue;
		}
		assert(*slmap.find(i) == *stdmap.find(i));
	}

	std::cout << "count" << std::endl;
	for (int i = 0; i < 30; i++) {
		assert(slmap.count(i) == stdmap.count(i));
	}

	std::cout << "lower_bound" << std::endl;

	for (int i = 0; i < 30; i++) {
		if (slmap.lower_bound(i) == slmap.end()) {
			assert(stdmap.lower_bound(i) == stdmap.end());
			continue;
		}
		assert(*slmap.lower_bound(i) == *stdmap.lower_bound(i));
	}

	std::cout << "upper_bound" << std::endl;

	for (int i = 0; i < 30; i++) {
		if (slmap.upper_bound(i) == slmap.end()) {
			assert(stdmap.upper_bound(i) == stdmap.end());
			continue;
		}
		assert(*slmap.upper_bound(i) == *stdmap.upper_bound(i));
	}

	std::cout << "equal_range" << std::endl;

	for (int i = 0; i < 30; i++) {
		std::pair<slmap_t::iterator, slmap_t::iterator> slrange = slmap.equal_range(i);
		std::pair<stdmap_t::iterator, stdmap_t::iterator> stdrange = stdmap.equal_range(i);
		assert(std::lexicographical_compare(slrange.first, slrange.second, stdrange.first, stdrange.second) == 0);
		assert(std::lexicographical_compare(stdrange.first, stdrange.second, slrange.first, slrange.second) == 0);
	}

}







void test_map() {
	std::cout << "construct()" << std::endl;
	typedef skiplist_map<int, std::string> slmap_t;
	slmap_t slmap;
	typedef std::map<int, std::string> stdmap_t;
	stdmap_t stdmap;
	assert(equal_range(slmap, stdmap));
	assert(slmap.size() == stdmap.size());
	assert(slmap.empty());

	std::cout << "insert(pair)" << std::endl;
	for (int i = 0; i < size; i++) {
		slmap.insert(std::make_pair(keys[i], values[i]));
		stdmap.insert(std::make_pair(keys[i], values[i]));
		assert(equal_range(slmap, stdmap));
	}

	std::cout << "size()" << std::endl;
	assert(slmap.size() == stdmap.size());

	std::cout << "iterators" << std::endl;
	assert(equal_range(slmap, stdmap));
	{
		slmap_t::iterator slbegin = slmap.begin();
		stdmap_t::iterator stdbegin = stdmap.begin();
		while (slbegin != slmap.end() && stdbegin != stdmap.end()) {
			slbegin++; stdbegin++;
			if (slbegin == slmap.end() || stdbegin == stdmap.end()) break;
			assert(*--slbegin == *--stdbegin);
			assert(*++slbegin == *++stdbegin);
			assert(*slbegin-- == *stdbegin--);
			assert(!(*slbegin++ != *stdbegin++));
		}
	}

	std::cout << "reverse iterators" << std::endl;
	assert(equal_range(slmap, stdmap));
	{
		slmap_t::reverse_iterator slbegin = slmap.rbegin();
		stdmap_t::reverse_iterator stdbegin = stdmap.rbegin();
		while (slbegin != slmap.rend() && stdbegin != stdmap.rend()) {
			slbegin++; stdbegin++;
			if (slbegin == slmap.rend() || stdbegin == stdmap.rend()) break;
			assert(*--slbegin == *--stdbegin);
			assert(*++slbegin == *++stdbegin);
			assert(*slbegin-- == *stdbegin--);
			assert(!(*slbegin++ != *stdbegin++));
		}
	}

	std::cout << "clear" << std::endl;
	slmap.clear();

	std::cout << "empty" << std::endl;
	assert(slmap.empty());
	assert(slmap.size() == 0);

	std::cout << "constructor(forwarditer, forwarditer)" << std::endl;
	std::vector<std::pair<int, std::string> > pv;
	for (int i = 0; i < 100; i++) {
		int t = rand() % size;
		pv.push_back(std::make_pair(keys[t], values[t]));
	}

	for (int i = 0; i < size; i++) {
		slmap.insert(std::make_pair(keys[i], values[i]));
		stdmap.insert(std::make_pair(keys[i], values[i]));
	}

	slmap_t slmap2(pv.begin(), pv.end());
	stdmap_t stdmap2(pv.begin(), pv.end());

	{
		// test destruction after iterator constructor
		slmap_t slmap3(pv.begin(), pv.end());
	}
	assert(equal_range(slmap2, stdmap2));

	std::cout << "constructor(skiplist)" << std::endl;
	{
		slmap_t slmap3(slmap2);
		stdmap_t stdmap3(stdmap2);
		assert(equal_range(slmap3, stdmap3));
	}

	assert(equal_range(slmap2, stdmap2));

	std::cout << "operator=" << std::endl;
	slmap = slmap2;
	stdmap = stdmap2;

	assert(equal_range(slmap, stdmap));
	assert(equal_range(slmap2, stdmap2));


	std::cout << "insert(iter, iter)" << std::endl;
	slmap.insert(pv.begin(), pv.end());
	stdmap.insert(pv.begin(), pv.end());
	assert(equal_range(slmap, stdmap));

	std::cout << "insert(iter, pair)" << std::endl;
	{
		slmap_t slmap2(slmap);
		slmap2.insert(slmap2.insert(std::make_pair(0, "zeroins1")).first, std::make_pair(0, "zeroins2"));
		stdmap2.insert(stdmap2.insert(std::make_pair(0, "zeroins1")).first, std::make_pair(0, "zeroins2"));
		assert(std::adjacent_find(slmap.rbegin(), slmap.rend(), slmap.value_comp()) == slmap.rend());
		assert(equal_range(slmap, stdmap));
	}

	std::cout << "erase(key)" << std::endl;
	int erase_vals[] = { -3, -10, 3, 5, 100 };
	for (int i = 0; i < 5; i++) {
		slmap.erase(erase_vals[i]);
		stdmap.erase(erase_vals[i]);
	}
	assert(equal_range(slmap, stdmap));

	std::cout << "erase(iter)" << std::endl;
	for (int i = 0; i < 10; i++) {
		assert(std::adjacent_find(slmap.rbegin(), slmap.rend(), slmap.value_comp()) == slmap.rend());
		slmap_t::iterator sliter = slmap.insert(std::make_pair(keys[i], values[i])).first;
		assert(std::adjacent_find(stdmap.rbegin(), stdmap.rend(), stdmap.value_comp()) == stdmap.rend());
		stdmap_t::iterator stditer = stdmap.insert(std::make_pair(keys[i], values[i])).first;
		std::cout << "~" << FORMAT_ITER(sliter) << " " << FORMAT_ITER(stditer) << std::endl;
		assert(*sliter == *stditer);
		assert(equal_range(slmap, stdmap));
		slmap.erase(sliter);
		stdmap.erase(stditer);
		assert(equal_range(slmap, stdmap));
	}

	std::cout << "erase(iter, iter)" << std::endl;
	for (int i = 0; i < 10; i+=3) {
		slmap_t::iterator slbegin = slmap.insert(std::make_pair(i, values[i])).first,
			slend = slmap.insert(std::make_pair(i + 1, values[i + 1])).first;
		stdmap_t::iterator stdbegin = stdmap.insert(std::make_pair(i, values[i])).first,
			stdend = stdmap.insert(std::make_pair(i + 1, values[i + 1])).first;
		assert(equal_range(slmap, stdmap));
		slmap.erase(slbegin, slend);
		stdmap.erase(stdbegin, stdend);
		assert(equal_range(slmap, stdmap));
	}
	assert(equal_range(slmap, stdmap));

	std::cout << "swap" << std::endl;
	{
		slmap_t slmap2(pv.begin(), pv.end());
		stdmap_t stdmap2(pv.begin(), pv.end());
		assert(equal_range(slmap2, stdmap2));
		slmap.swap(slmap2);
		stdmap.swap(stdmap2);
		assert(equal_range(slmap, stdmap));
		assert(equal_range(slmap2, stdmap2));
		slmap.swap(slmap2);
		stdmap.swap(stdmap2);
	}

	assert(equal_range(slmap, stdmap));
	slmap.swap(slmap);
	stdmap.swap(stdmap);
	assert(equal_range(slmap, stdmap));

	std::cout << "find" << std::endl;

	for (int i = 0; i < 30; i++) {
		if (slmap.find(i) == slmap.end()) {
			assert(stdmap.find(i) == stdmap.end());
			continue;
		}
		assert(*slmap.find(i) == *stdmap.find(i));
	}

	std::cout << "count" << std::endl;
	for (int i = 0; i < 30; i++) {
		assert(slmap.count(i) == stdmap.count(i));
	}

	std::cout << "lower_bound" << std::endl;

	for (int i = 0; i < 30; i++) {
		if (slmap.lower_bound(i) == slmap.end()) {
			assert(stdmap.lower_bound(i) == stdmap.end());
			continue;
		}
		assert(*slmap.lower_bound(i) == *stdmap.lower_bound(i));
	}

	std::cout << "upper_bound" << std::endl;

	for (int i = 0; i < 30; i++) {
		if (slmap.upper_bound(i) == slmap.end()) {
			assert(stdmap.upper_bound(i) == stdmap.end());
			continue;
		}
		assert(*slmap.upper_bound(i) == *stdmap.upper_bound(i));
	}

	std::cout << "equal_range" << std::endl;

	for (int i = 0; i < 30; i++) {
		std::pair<slmap_t::iterator, slmap_t::iterator> slrange = slmap.equal_range(i);
		std::pair<stdmap_t::iterator, stdmap_t::iterator> stdrange = stdmap.equal_range(i);
		assert(std::lexicographical_compare(slrange.first, slrange.second, stdrange.first, stdrange.second) == 0);
		assert(std::lexicographical_compare(stdrange.first, stdrange.second, slrange.first, slrange.second) == 0);
	}

}

int main() {
	test_map();
	return 0;
}
