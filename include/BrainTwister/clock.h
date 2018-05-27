#pragma once

#include <chrono>
#include <iomanip>
#include <string>
#include <vector>

/// Take the clock with the highest precision
using myclock = std::chrono::high_resolution_clock;

template <typename Container, typename Fun>
void tuple_for_each(const Container& c, Fun fun)
{
    for (auto& e : c)
        fun(std::get<0>(e), std::get<1>(e), std::get<2>(e));
}

auto split(std::string const& s, char separator) -> std::vector<std::string>
{
    std::vector<std::string> result;
    std::string::size_type p = 0;
    std::string::size_type q;

    while ((q = s.find(separator, p)) != std::string::npos) {
        result.emplace_back(s, p, q - p);
        p = q + 1;
    }

    result.emplace_back(s, p);
    return result;
}

/// Converts a string to duration
std::ostream& operator << (std::ostream& os, myclock::duration const& d)
{
    using T = std::tuple<std::chrono::nanoseconds, int, const char*>;

    constexpr T formats[] = {
        T{std::chrono::hours(1), 2, ""},
        T{std::chrono::minutes(1), 2, ":"},
        T{std::chrono::seconds(1), 2, ":"},
        T{std::chrono::milliseconds(1), 3, "."},
        T{std::chrono::microseconds(1), 3, ""},
        T{std::chrono::nanoseconds(1), 3, ""}
    };

    std::chrono::nanoseconds time = std::chrono::duration_cast<std::chrono::nanoseconds>(d);
    tuple_for_each(formats, [&time, &os](auto denominator, auto width, auto separator) {
        os << separator << std::setw(width) << std::setfill('0') << (time / denominator);
        time = time % denominator;
    });

    return os;
}

/// Custom translator for myclock::duration (only supports std::string)
struct DurationTranslator
{
    typedef std::string internal_type;
    typedef myclock::duration external_type;

    /// Converts a string to duration
    boost::optional<external_type> get_value(internal_type const& s)
    {
        std::vector<std::string> s2 = split(s, '.');
        std::vector<std::string> s3 = split(s2[0], ':');

        myclock::duration d = std::chrono::duration_cast<myclock::duration>(std::chrono::hours(stoi(s3[0])))
                            + std::chrono::duration_cast<myclock::duration>(std::chrono::minutes(stoi(s3[1])))
                            + std::chrono::duration_cast<myclock::duration>(std::chrono::seconds(stoi(s3[2])))
                            + std::chrono::duration_cast<myclock::duration>(std::chrono::nanoseconds(stoi(s2[1])));

        return boost::optional<external_type>(d);
    }

    /// Converts a duration to string
    boost::optional<internal_type> put_value(external_type const& d)
    {
        using T = std::tuple<std::chrono::microseconds, int, const char*>;

        constexpr T formats[] = {
            T{std::chrono::hours(1), 2, ""},
            T{std::chrono::minutes(1), 2, ":"},
            T{std::chrono::seconds(1), 2, ":"},
            T{std::chrono::milliseconds(1), 3, "."},
            T{std::chrono::microseconds(1), 3, ""}
        };

        std::ostringstream oss;
        std::chrono::microseconds time = std::chrono::duration_cast<std::chrono::microseconds>(d);
        tuple_for_each(formats, [&time, &oss](auto denominator, auto width, auto separator) {
            oss << separator << std::setw(width) << std::setfill('0') << (time / denominator);
            time = time % denominator;
        });

        return boost::optional<internal_type>(oss.str());
    }
};

namespace boost {
namespace property_tree {

template<typename Ch, typename Traits, typename Alloc>
struct translator_between<std::basic_string< Ch, Traits, Alloc >, myclock::duration>
{
    typedef DurationTranslator type;
};

} // namespace property_tree
} // namespace boost
