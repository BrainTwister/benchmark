#include "BrainTwister/benchmark.h"
#include "BrainTwister/JSON.h"
#include "gtest/gtest.h"
#include <random>
#include <thread>

using namespace BrainTwister;

/// A simple Action initialized with a list of times in microseconds.
struct Action
{
    Action(std::initializer_list<size_t> const& list) : time_list(begin(list), end(list)) {}

    void operator()() const
    {
        if (iter_time == time_list.end()) throw std::runtime_error("End of time list reached.");
        std::this_thread::sleep_for(std::chrono::milliseconds(*iter_time++));
    }

    std::vector<size_t> time_list;
    mutable std::vector<size_t>::const_iterator iter_time = time_list.begin();
};

TEST(benchmark, default)
{
    Benchmark benchmark;

    EXPECT_EQ(3UL, benchmark.get_settings().min_replications);
    EXPECT_EQ(std::chrono::seconds(1), benchmark.get_settings().min_execution_time);
    EXPECT_DOUBLE_EQ(0.1, benchmark.get_settings().spike_detection_factor);
    EXPECT_EQ(2UL, benchmark.get_settings().warm_up_runs);
}

TEST(benchmark, own_settings)
{
    Benchmark benchmark(Benchmark::Settings().set_min_replications(5).set_min_execution_time(std::chrono::seconds(3)));

    EXPECT_EQ(5UL, benchmark.get_settings().min_replications);
    EXPECT_EQ(std::chrono::seconds(3), benchmark.get_settings().min_execution_time);
    EXPECT_DOUBLE_EQ(0.1, benchmark.get_settings().spike_detection_factor);
}

TEST(benchmark, json_settings)
{
    auto settings = R"(
        {
            "min_replications": 10,
            "min_execution_time": "00:00:05.000",
            "spike_detection_factor": 0.01
        }
    )";
    Benchmark benchmark(Benchmark::Settings(JSON{settings}));

    EXPECT_EQ(10UL, benchmark.get_settings().min_replications);
    EXPECT_EQ(std::chrono::seconds(5), benchmark.get_settings().min_execution_time);
    EXPECT_DOUBLE_EQ(0.01, benchmark.get_settings().spike_detection_factor);
}

TEST(benchmark, Action)
{
    Benchmark benchmark(Benchmark::Settings().set_min_execution_time(std::chrono::milliseconds(100))
                                             .set_spike_detection_factor(0.3));
    Action action{130, 120, 110, 90, 100};
    Benchmark::Results results = benchmark.benchIt(action);

    EXPECT_EQ(3UL, results.nb_replications);
    EXPECT_EQ(0UL, results.nb_spikes);
    EXPECT_EQ(10, std::chrono::duration_cast<std::chrono::milliseconds>(results.average_time).count()/10);
    EXPECT_EQ(9, std::chrono::duration_cast<std::chrono::milliseconds>(results.shortest_time).count()/10);
    EXPECT_EQ(11, std::chrono::duration_cast<std::chrono::milliseconds>(results.longest_time).count()/10);
}

TEST(benchmark, spike_detection)
{
    Benchmark benchmark(Benchmark::Settings().set_min_execution_time(std::chrono::milliseconds(100))
                                             .set_spike_detection_factor(0.3));
    Action action{130, 120, 110, 150, 100, 90};
    Benchmark::Results results = benchmark.benchIt(action);

    EXPECT_EQ(3UL, results.nb_replications);
    EXPECT_EQ(1UL, results.nb_spikes);
    EXPECT_EQ(10, std::chrono::duration_cast<std::chrono::milliseconds>(results.average_time).count()/10);
    EXPECT_EQ(9, std::chrono::duration_cast<std::chrono::milliseconds>(results.shortest_time).count()/10);
    EXPECT_EQ(11, std::chrono::duration_cast<std::chrono::milliseconds>(results.longest_time).count()/10);
}

TEST(benchmark, warm_up)
{
    Benchmark benchmark(Benchmark::Settings().set_min_execution_time(std::chrono::milliseconds(100))
                                             .set_spike_detection_factor(0.3));
    Action action{130, 120, 110, 100, 90};
    Benchmark::Results results = benchmark.benchIt(action);

    EXPECT_EQ(3UL, results.nb_replications);
    EXPECT_EQ(0UL, results.nb_spikes);
    EXPECT_EQ(10, std::chrono::duration_cast<std::chrono::milliseconds>(results.average_time).count()/10);
    EXPECT_EQ(9, std::chrono::duration_cast<std::chrono::milliseconds>(results.shortest_time).count()/10);
    EXPECT_EQ(11, std::chrono::duration_cast<std::chrono::milliseconds>(results.longest_time).count()/10);
}

TEST(benchmark, lambda_sqrt)
{
    Benchmark benchmark{JSON{R"(
        {
            "min_replications": 10,
            "max_replications": 100000,
            "min_execution_time": "00:00:01.000000000",
            "spike_detection": 1,
            "spike_detection_factor": 0.1,
            "warm_up_runs": 2
        }
    )"}};

    double x = 0.0;
    auto results = benchmark.benchIt([&x](){
        x = std::sqrt(x);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }, [&x](){
        x = 9.0;
    });

    EXPECT_DOUBLE_EQ(3.0, x);
    EXPECT_EQ(10UL, results.nb_replications);
}

TEST(benchmark, sort_vector)
{
    Benchmark benchmark{JSON{R"(
        {
            "min_replications": 4,
            "max_replications": 100000,
            "min_execution_time": "00:00:01.000000000",
            "spike_detection": 1,
            "spike_detection_factor": 0.1,
            "warm_up_runs": 2
        }
    )"}};

    int N = 100000;
    std::vector<int> v, v_init(N);

    std::random_device rnd_device;
    std::mt19937 mersenne_engine(rnd_device());
    std::uniform_int_distribution<int> dist(1, 100000);

    auto gen = std::bind(dist, mersenne_engine);
    generate(std::begin(v_init), std::end(v_init), gen);

    benchmark.benchIt(
        [&v](){
            std::sort(std::begin(v), std::end(v));
        },
        [&v, v_init](){
            v = v_init;
        }
    );

    bool sorted = true;
    for (size_t i = 0; i != v.size() - 1; ++i) if (v[i] > v[i+1]) {
        sorted = false;
        break;
    }

    EXPECT_EQ(true, sorted);
}
