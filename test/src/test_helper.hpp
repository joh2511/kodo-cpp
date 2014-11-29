// Copyright Steinwurf ApS 2014.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#pragma once

#include <cstdint>
#include <vector>
#include <functional>

#include <kodocpp/kodocpp.hpp>

using test_function = std::function<
    void (uint32_t, uint32_t, kodocpp::code_type, kodocpp::finite_field, bool)>;

inline void test_combinations(
    test_function coder_test,
    uint32_t max_symbols,
    uint32_t max_symbol_size,
    bool trace_enabled)
{
    std::vector<kodocpp::code_type> code_types =
    {
        kodocpp::code_type::full_rlnc,
        kodocpp::code_type::on_the_fly,
        kodocpp::code_type::sliding_window
    };

    std::vector<kodocpp::finite_field> fields =
    {
        kodocpp::finite_field::binary,
        kodocpp::finite_field::binary8,
        kodocpp::finite_field::binary16
    };

    for (auto& code : code_types)
    {
        for (auto& field : fields)
        {
            if (coder_test)
            {
                coder_test(max_symbols, max_symbol_size,
                           code, field, trace_enabled);
            }
        }
    }
}
