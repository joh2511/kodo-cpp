// Copyright Steinwurf ApS 2016.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

/// @example uncoded_symbols.cpp
///
/// Example showing how to process original symbols that were transmitted
/// without kodo headers. These symbols are inserted manually to the decoder
/// before the coded packets are processed.

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <vector>

#include <kodocpp/kodocpp.hpp>

int main()
{
    // Seed the random number generator to produce different data every time
    srand((uint32_t)time(0));

    // Set the number of symbols and the symbol size
    uint32_t symbols = 10;
    uint32_t symbol_size = 100;

    // Create encoder/decoder factories that we will use to build the actual
    // encoder and decoder
    kodocpp::encoder_factory encoder_factory(
        kodocpp::codec::sparse_full_vector,
        kodocpp::field::binary,
        symbols,
        symbol_size);

    kodocpp::encoder encoder = encoder_factory.build();

    kodocpp::decoder_factory decoder_factory(
        kodocpp::codec::sparse_full_vector,
        kodocpp::field::binary,
        symbols,
        symbol_size);

    kodocpp::decoder decoder = decoder_factory.build();

    // The coding vector density on the encoder can be set with set_density().
    // Note: the density can be adjusted at any time. This feature can be used
    // to adapt to changing network conditions.
    encoder.set_density(0.4);
    std::cout << "The density was set to: " << encoder.density() << std::endl;
    // A low density setting can lead to a large number of redundant symbols.
    // In practice, the value should be tuned to the specific scenario.

    // In this example, we do not use systematic coding. The original symbols
    // will be transferred to the decoder without using kodo.
    //
    // We explicitly turn off the systematic mode on the encoder:
    if (encoder.has_systematic_interface())
    {
        encoder.set_systematic_off();
    }
    std::cout << "Systematic encoding disabled" << std::endl;

    std::vector<uint8_t> payload(encoder.payload_size());

    // Allocate symbols in non-contiguous buffers
    std::vector<std::vector<uint8_t>> input_symbols(symbols);
    std::vector<std::vector<uint8_t>> output_symbols(symbols);

    uint32_t i, j;
    // Create the original symbols and store them in the encoder
    for (i = 0; i < symbols; ++i)
    {
        // Create the individual symbols for the encoder
        input_symbols[i].resize(symbol_size);

        // Randomize input data
        for (j = 0; j < symbol_size; ++j)
            input_symbols[i][j] = rand() % 256;

        // Store the symbol pointer in the encoder
        encoder.set_const_symbol(i, input_symbols[i].data(), symbol_size);
    }

    // Transfer the original symbols to the decoder with some losses
    uint32_t lost_payloads = 0;
    for (i = 0; i < symbols; ++i)
    {
        // Create the output symbol buffers for the decoder
        output_symbols[i].resize(symbol_size);

        // Specify the output buffers used for decoding
        decoder.set_mutable_symbol(i, output_symbols[i].data(), symbol_size);

        // Simulate a channel with a 50% loss rate
        if (rand() % 2)
        {
            lost_payloads++;
            std::cout << "Symbol " << i << " lost on channel" << std::endl
                      << std::endl;
            continue;
        }

        // If the symbol was not lost, then insert that symbol into the decoder
        // using the raw symbol data (no additional headers are needed)
        // This will copy the data from input_symbols[i] to output_symbols[i]
        decoder.read_uncoded_symbol(input_symbols[i].data(), i);
    }

    std::cout << "Number of lost payloads: " << lost_payloads << std::endl;

    // Now we generate coded packets with the encoder in order to recover the
    // lost packets on the decoder side
    while (!decoder.is_complete())
    {
        // The encoder will use a certain amount of bytes of the payload buffer
        uint32_t bytes_used = encoder.write_payload(payload.data());
        std::cout << "Payload generated by encoder, bytes used = "
                  << bytes_used << std::endl;

        // Pass the generated packet to the decoder
        decoder.read_payload(payload.data());
        std::cout << "Payload processed by decoder, current rank = "
                  << decoder.rank() << std::endl << std::endl;
    }

    // Compare the input and output symbols one-by-one
    uint32_t failures = 0;
    for (i = 0; i < symbols; ++i)
    {
        if (input_symbols[i] != output_symbols[i])
        {
            std::cout << "Error: Symbol " << i << " was not decoded correctly"
                      << std::endl;
            failures++;
        }
    }

    if (failures == 0)
    {
        std::cout << "Data decoded correctly" << std::endl;
    }

    return 0;
}
