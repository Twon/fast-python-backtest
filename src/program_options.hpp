#pragma once

#include "enum.hpp"
#include "format.hpp"

#include <boost/program_options.hpp>
#include <boost/throw_exception.hpp>
#include <boost/log/trivial.hpp>

#include <concepts>
#include <filesystem>
#include <iostream>
#include <optional>

namespace profitview
{

template<BoostDescribeEnum Enum>
void validate(boost::any& v, const std::vector<std::string>& values, Enum*, int)
{
    namespace po = boost::program_options;
    po::validators::check_first_occurrence(v);
    std::string const& s = po::validators::get_single_string(values);

    if (auto const value = fromString<Enum>(s); value.has_value())
    {
        v = value.value();
    }
    else
    {
        boost::throw_exception(po::invalid_option_value(s));
    }
}

// clang-format off
template<typename T>
concept CustomProgramOptions = requires(T& t, boost::program_options::options_description& options)
{
    { t.addOptions(options) } -> std::same_as<void>;
};
// clang-format on

std::optional<int> parseProgramOptions(int argc, char* argv[], CustomProgramOptions auto&... options)
{
    namespace po = boost::program_options;
    try
    {
        po::options_description desc(fmt_ns::format("{}\n\nUsage: ", std::filesystem::path(argv[0]).filename().string()));

        desc.add_options()("help", "produce help message");

        (options.addOptions(desc), ...);

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            return 1;
        }

        po::notify(vm);
    }
    catch (po::required_option& e)
    {
        BOOST_LOG_TRIVIAL(error) << e.what() << std::endl;
        return 1;
    }
    return std::nullopt;
}

}    // namespace profitview