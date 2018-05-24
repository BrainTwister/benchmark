[![Build Status](https://jenkins.braintwister.eu/buildStatus/icon?job=BrainTwister/record/master)](https://jenkins.braintwister.eu/job/BrainTwister/job/record/job/master/)

BrainTwister benchmark
======================

Accurate benchmark library for C++

Copyright (C) 2018 Bernd Doser, <bernd.doser@braintwister.eu>

All rights reserved.

BrainTwister benchmark is free software made available under the [MIT License]
(http://opensource.org/licenses/MIT). For details see [LICENSE](LICENSE.md).

conan.io
--------

C++ conan.io package is available at https://bintray.com/braintwister/conan


Description
-----------

BrainTwister benchmark determine accurate timings of a C++ function
be executing it several times to get a reliable result.
Outliers (spikes) will be detected and repeated.
Number of "warm up" runs can be defined 

``` C++
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

auto results = benchmark.benchIt(
    [&v](){
        std::sort(std::begin(v), std::end(v));
    },
    [&v, v_init](){
        v = v_init;
    }
);

```

The time format is `hh:mm:ss.nnnnnnnnn` with (h)ours, (m)inutes, (s)econds, and
(n)anoseconds. The benchmark execution function `benchIt` expect two
functions. The first one is the function which will be benchmarked. The second
one is optional and can be used if the benchmark function needs some
initialization. In the given example the unsorted vector will be copied back
before the next run.  
