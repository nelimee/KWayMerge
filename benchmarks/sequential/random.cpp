#include <benchmark/benchmark.h>
#include "merge.h"

#include <random>
#include <vector>

template <typename ContainerOfContainer>
class RandomContainerOfContainer : public ::benchmark::Fixture {
public:
	void SetUp(const ::benchmark::State& st) final {
		for(std::size_t i{0}; i < st.range(0); ++i)
			container_of_container.push_back(std::move(randomly_filled_container(st.range(1))));
	}

	void TearDown(const ::benchmark::State&) final {
		container_of_container.clear();
	}

	ContainerOfContainer container_of_container;

private:
	typename ContainerOfContainer::value_type randomly_filled_container(std::size_t size) {
		typename ContainerOfContainer::value_type container;
		std::random_device r;
		std::default_random_engine random_engine(r());
		std::uniform_real_distribution<double> random_double;
		for(std::size_t i{0}; i < size; ++i)
			container.push_back(random_double(random_engine));
		return container;
	}
};

#define KWAYMERGE_BENCHMARK_DATASTRUCTURE(DATA_STRUCTURE, DATA_STRUCTURE_REPR) \
BENCHMARK_TEMPLATE_DEFINE_F(RandomContainerOfContainer, BM_MergeRandom##DATA_STRUCTURE_REPR, DATA_STRUCTURE)(benchmark::State& st) { \
	for(auto _ : st) { \
		::benchmark::DoNotOptimize(merge_arrays(this->container_of_container));\
	}\
	st.SetComplexityN(st.range(0) * st.range(1));\
}\
BENCHMARK_REGISTER_F(RandomContainerOfContainer, BM_MergeRandom##DATA_STRUCTURE_REPR)->RangeMultiplier(8) \
		->Ranges({{1, 1<<8}, {32, 32}})->Ranges({{1, 1<<8}, {1024, 1024}})->Complexity(benchmark::oN);\


KWAYMERGE_BENCHMARK_DATASTRUCTURE(std::vector<std::vector<float>>, VectorVectorFloat)
KWAYMERGE_BENCHMARK_DATASTRUCTURE(std::list<std::vector<float>>, ListVectorFloat)


#undef KWAYMERGE_BENCHMARK_DATASTRUCTURE