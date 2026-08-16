#include "core/Time.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("UTC calendar construction validates dates and leap years", "[time]")
{
    CHECK(solar::core::makeUtcTimePoint({2028, 2, 29, 23, 59, 59}).has_value());
    CHECK_FALSE(solar::core::makeUtcTimePoint({2027, 2, 29, 0, 0, 0}));
    CHECK_FALSE(solar::core::makeUtcTimePoint({2027, 2, 30, 0, 0, 0}));
    CHECK_FALSE(solar::core::makeUtcTimePoint({2027, 13, 1, 0, 0, 0}));
    CHECK_FALSE(solar::core::makeUtcTimePoint({2027, 1, 1, 24, 0, 0}));
    CHECK_FALSE(solar::core::makeUtcTimePoint({2027, 1, 1, 0, 60, 0}));
    CHECK_FALSE(solar::core::makeUtcTimePoint({2027, 1, 1, 0, 0, 60}));
}

TEST_CASE("UTC chrono formatting is stable across supported ephemeris years", "[time]")
{
    for (const int year : {1900, 2000, 2031, 2100}) {
        const auto time = solar::core::makeUtcTimePoint({year, 4, 21, 13, 42, 18});
        REQUIRE(time);
        const auto components = solar::core::utcComponents(*time);
        CHECK(components.year == year);
        CHECK(components.month == 4);
        CHECK(components.day == 21);
        CHECK(components.hour == 13);
        CHECK(components.minute == 42);
        CHECK(components.second == 18);
        CHECK(solar::core::formatUtcIso8601(*time) ==
              std::to_string(year) + "-04-21T13:42:18Z");
    }
}
