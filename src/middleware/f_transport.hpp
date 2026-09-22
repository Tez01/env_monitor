#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

class Transport{
    public:
        Transport() = default;
        virtual ~Transport() = default;

        virtual std::size_t read(std::span<std::uint8_t> buffer, std::size_t bytes_to_read) = 0;

        virtual std::size_t write(std::span<const std::uint8_t> buffer, std::size_t bytes_to_write) = 0;
};