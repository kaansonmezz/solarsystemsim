#include "core/Time.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace solar::core {
namespace {

void writeDate(std::ostringstream& output, const UtcComponents& value)
{
    output << std::setfill('0') << std::setw(4) << value.year << '-'
           << std::setw(2) << value.month << '-' << std::setw(2) << value.day;
}

void writeTime(std::ostringstream& output, const UtcComponents& value)
{
    output << std::setfill('0') << std::setw(2) << value.hour << ':'
           << std::setw(2) << value.minute << ':' << std::setw(2) << value.second;
}

} // namespace

UtcComponents utcComponents(const SimulationTimePoint timePoint) noexcept
{
    using namespace std::chrono;
    const auto dayPoint = floor<days>(timePoint);
    const year_month_day calendar{dayPoint};
    const hh_mm_ss timeOfDay{floor<seconds>(timePoint) - dayPoint};
    return {
        static_cast<int>(calendar.year()),
        static_cast<unsigned int>(calendar.month()),
        static_cast<unsigned int>(calendar.day()),
        static_cast<unsigned int>(timeOfDay.hours().count()),
        static_cast<unsigned int>(timeOfDay.minutes().count()),
        static_cast<unsigned int>(timeOfDay.seconds().count()),
    };
}

std::optional<SimulationTimePoint> makeUtcTimePoint(const UtcComponents& value) noexcept
{
    using namespace std::chrono;
    if (value.hour > 23 || value.minute > 59 || value.second > 59) return std::nullopt;
    const year_month_day calendar{year{value.year}, month{value.month}, day{value.day}};
    if (!calendar.ok()) return std::nullopt;
    return SimulationTimePoint{sys_days{calendar}.time_since_epoch()} +
           hours{value.hour} + minutes{value.minute} + seconds{value.second};
}

UtcDateTime formatUtc(const SimulationTimePoint timePoint)
{
    const UtcComponents value = utcComponents(timePoint);
    std::ostringstream date;
    writeDate(date, value);
    std::ostringstream time;
    writeTime(time, value);
    time << " UTC";
    return {date.str(), time.str()};
}

std::string formatUtcIso8601(const SimulationTimePoint timePoint)
{
    const auto value = formatUtc(timePoint);
    return value.date + "T" + value.time.substr(0, 8) + "Z";
}

std::string formatUtcForSpice(const SimulationTimePoint timePoint)
{
    using namespace std::chrono;
    const auto wholeSeconds = floor<seconds>(timePoint);
    const auto microseconds = duration_cast<std::chrono::microseconds>(timePoint - wholeSeconds);
    const UtcComponents value = utcComponents(wholeSeconds);
    std::ostringstream output;
    writeDate(output, value);
    output << 'T';
    writeTime(output, value);
    output << '.' << std::setfill('0') << std::setw(6) << microseconds.count() << " UTC";
    return output.str();
}

} // namespace solar::core
