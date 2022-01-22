#pragma once

#include <stdexcept>

class RunTimeException : public std::runtime_error {
public:
    // Exception to be thrown when decoding error.
    RunTimeException(const std::string &reason) : std::runtime_error(reason) {}
};
