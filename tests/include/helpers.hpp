#ifndef KWAYMERGE_HELPERS_HPP
#define KWAYMERGE_HELPERS_HPP

#include <functional>
#include <random>

template <typename Comp = std::less<double>>
std::vector<double> randomly_sorted(std::size_t size,
                                    double min = 0.0,
                                    double max = 1.0,
                                    Comp comp = Comp()) {
	std::random_device r;
	std::default_random_engine random_engine(r());
	std::uniform_real_distribution<double> random_double(min, max);

	std::vector<double> ret;
	ret.reserve(size);
	for(std::size_t i{0}; i < size; ++i) {
		ret.push_back(random_double(random_engine));
	}

	std::sort(ret.begin(), ret.end(), comp);
	return ret;
}


#endif //KWAYMERGE_HELPERS_HPP
