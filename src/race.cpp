#include "global.hpp"

#include "race.hpp"
#include "random.hpp"
#include "serialization/string_utils.hpp"

#include <cstdlib>

namespace {

config::child_list empty_traits;

void add_prefixes(const wide_string& str, size_t length, markov_prefix_map& res)
{
	for(size_t i = 0; i <= str.size(); ++i) {
		const size_t start = i > length ? i - length : 0;
		const wide_string key(str.begin() + start, str.begin() + i);
		const wchar_t c = i != str.size() ? str[i] : 0;
		res[key].push_back(c);
	}
}

markov_prefix_map markov_prefixes(const std::vector<std::string>& items, size_t length)
{
	markov_prefix_map res;

	for(std::vector<std::string>::const_iterator i = items.begin(); i != items.end(); ++i) {
		add_prefixes(utils::string_to_wstring(*i),length,res);
	}

	return res;
}

wide_string markov_generate_name(const markov_prefix_map& prefixes, size_t chain_size, size_t max_len)
{
	if(chain_size == 0)
		return wide_string();

	wide_string prefix, res;

	while(res.size() < max_len) {
		const markov_prefix_map::const_iterator i = prefixes.find(prefix);
		if(i == prefixes.end() || i->second.empty()) {
			return res;
		}
	
		const wchar_t c = i->second[get_random()%i->second.size()];
		if(c == 0) {
			return res;
		}

		res.resize(res.size()+1);
		res[res.size()-1] = c;
		prefix.resize(prefix.size()+1);
		prefix[prefix.size()-1] = c;
		while(prefix.size() > chain_size) {
			prefix.erase(prefix.begin());
		}
	}

	// Getting here means that the maximum length was reached when
	// generating the name, hence the ending of the name has to be
	// made valid. Otherwise weird names like Unárierinil- and
	// Thramboril-G may occur.

	// Strip characters from the end until the last prefix of the
	// name has end-of-string as a possible next character in the
	// markov prefix map. If no valid ending is found, use the
	// originally generated name.
	wide_string originalRes = res;
	int prefixLen;
	while (res.size() > 0) {
		prefixLen = chain_size < res.size() ? chain_size : res.size();
		prefix = wide_string(res.end() - prefixLen, res.end());

		const markov_prefix_map::const_iterator i = prefixes.find(prefix);
		if (i == prefixes.end() || i->second.empty()) {
			return res;
		}
		if (std::find(i->second.begin(), i->second.end(), 0)
				!= i->second.end()) { 
			// This ending is valid. 
			return res;
		}
		// The current ending is invalid, remove the last character
		// and retry.
		res.pop_back();
	}
	// No valid ending at all could be found. This generally should
	// not happen, unless the chain length is very long or the
	// maximum length is very small. Return the originally generated
	// name, it's not much we can do about it.
	return originalRes;
}

}

unit_race::unit_race() : ntraits_(0), chain_size_(0), not_living_(false), traits_(&empty_traits), global_traits_(true)
{
}

unit_race::unit_race(const config& cfg) : name_(cfg["name"]), ntraits_(atoi(cfg["num_traits"].c_str())),
                                          not_living_(cfg["not_living"] == "yes"),
										  traits_(&cfg.get_children("trait")), global_traits_(cfg["ignore_global_traits"] != "yes")
{
	names_[MALE] = utils::split(cfg["male_names"]);
	names_[FEMALE] = utils::split(cfg["female_names"]);

	chain_size_ = atoi(cfg["markov_chain_size"].c_str());
	if(chain_size_ <= 0)
		chain_size_ = 2;

	next_[MALE] = markov_prefixes(names_[MALE],chain_size_);
	next_[FEMALE] = markov_prefixes(names_[FEMALE],chain_size_);
}

const std::string& unit_race::name() const { return name_; }

std::string unit_race::generate_name(unit_race::GENDER gender) const
{
	return utils::wstring_to_string(markov_generate_name(next_[gender],chain_size_,12));
}

bool unit_race::uses_global_traits() const
{
	return global_traits_;
}

const config::child_list& unit_race::additional_traits() const
{
	return *traits_;
}

int unit_race::num_traits() const { return ntraits_; }

bool unit_race::not_living() const { return not_living_; }
