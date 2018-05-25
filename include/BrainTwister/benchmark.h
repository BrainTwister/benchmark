#pragma once

#include "BrainTwister/Record.h"
#include "BrainTwister/JSON.h"
#include "clock.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <set>

namespace BrainTwister {

/// An empty function object as default object if initialization step in benchIt is not needed.
static auto empty_function = [](){};

/**
 * Accurate time measurement by repeated execution
 *
 * For accurate timings a minimum execution time is needed. Therefore for fast routines
 * the execution will be repeated until minExecution_time is reached. The total time
 * will be divided by the number of cycles. If execution time is larger than minExecution_time
 * the execution will repeated at least as min_replications. min_replications should
 * not smaller than 3 to detect spikes.
 * Spikes are measurements outside the mean_value +/- spike_detection_factor * mean_value.
 */
class Benchmark
{
public:

    BRAINTWISTER_RECORD(Settings, \
        ((size_t, min_replications, 3)) \
        ((size_t, max_replications, 100000)) \
        ((myclock::duration, min_execution_time, std::chrono::seconds(1))) \
        ((bool, spike_detection, true)) \
        ((double, spike_detection_factor, 0.1)) \
        ((size_t, warm_up_runs, 2)) \
        ((int, verbosity, 0)) \
    )

    BRAINTWISTER_RECORD(Results, \
        ((myclock::duration, average_time, myclock::duration(0))) \
        ((myclock::duration, shortest_time, myclock::duration(0))) \
        ((myclock::duration, longest_time, myclock::duration(0))) \
        ((size_t, nb_replications, 0)) \
        ((size_t, nb_spikes, 0))
    )

    Benchmark(Settings const& settings = Settings())
     : settings(settings)
    {}

    template <typename Action, typename Init = decltype(empty_function)>
    Results benchIt(Action const& action, Init const& init = empty_function) const
    {
        // Warm up
        for (size_t i(0); i != settings.warm_up_runs; ++i)
        {
            init();
            auto time = measure_time(action);
            if (settings.verbosity >= 1) std::cout << "warm-up #" << i+1 << " time: " << time << std::endl;
        }

        // Now the time will measured
        Results results;
        myclock::duration total_time(0);
        std::set<myclock::duration> times;
        while ((total_time < settings.min_execution_time or
                results.nb_replications < settings.min_replications) and
                !(results.nb_replications > settings.max_replications))
        {
            ++results.nb_replications;
            init();
            auto time = measure_time(action);
            if (settings.verbosity >= 1) std::cout << "#" << results.nb_replications << " time: " << time << std::endl;
            times.insert(time);
            total_time += time;
        }

        // Remeasure spikes
        if (settings.spike_detection) {
            while (*times.rbegin() > *times.begin() * (settings.spike_detection_factor + 1.0))
            {
                ++results.nb_spikes;
                init();
                auto time = measure_time(action);
                if (settings.verbosity >= 1) std::cout << "rerun #" << results.nb_spikes << " time: " << time << std::endl;
                times.insert(time);
                total_time += time;
                total_time -= *times.rbegin();
                times.erase(std::prev(times.end()));
            }
        }

        results.average_time = total_time / results.nb_replications;
        results.shortest_time = *times.begin();
        results.longest_time = *times.rbegin();

        return results;
    }

    Settings const& get_settings() const { return settings; }

private:

    /// Determine a single execution time
    template <class Action>
    myclock::duration measure_time(Action const& action) const
    {
        std::chrono::time_point<myclock> start = myclock::now();
        action();
        return myclock::now() - start;
    }

    /// General settings
    Settings settings;

};

} // namespace BrainTwister
