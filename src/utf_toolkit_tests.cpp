//  File:   utf_toolkit_tests.cpp
//  Author: ChatGPT 5.1
//  Date:   24 November 2025
//  
//  Description:
//  
//      Basic tests for the cp_errors helper type.

#include "utf_toolkit.h"

namespace unicode
{

namespace utf
{

namespace toolkit
{

bool test_cp_errors()
{
    using cp = cp_errors;
    using bits = cp_errors::bits;

    bool ok = true;

    // Initial state: no errors/warnings
    {
        cp e;
        ok = ok && !e.any();
        ok = ok && e.none();
        ok = ok && !e.failed();
        ok = ok && !e.error();
    }

    // Simple error flags
    {
        cp e(bits::Failed);
        ok = ok && e.failed();
        ok = ok && e.any(bits::Failed);
        ok = ok && !e.none(bits::Failed);

        cp f(bits::InvalidBuffer);
        cp g = e | f;
        ok = ok && g.any(bits::Failed);
        ok = ok && g.any(bits::InvalidBuffer);
        ok = ok && g.error();
    }

    // errors_only / warnings_only separation
    {
        cp e;
        e |= bits::Failed;
        e |= bits::InvalidBuffer;
        e |= bits::ReadExhausted;   // classified as a warning
        e |= bits::InvalidPoint;    // warning

        cp only_errors = e.errors_only();
        cp only_warnings = e.warnings_only();

        ok = ok && only_errors.any(bits::Failed);
        ok = ok && only_errors.any(bits::InvalidBuffer);
        ok = ok && !only_errors.any(bits::ReadExhausted);
        ok = ok && !only_errors.any(bits::InvalidPoint);

        ok = ok && !only_warnings.any(bits::Failed);
        ok = ok && !only_warnings.any(bits::InvalidBuffer);
        ok = ok && only_warnings.any(bits::ReadExhausted);
        ok = ok && only_warnings.any(bits::InvalidPoint);
    }

    // Buffer error classification
    {
        cp e;
        e |= bits::InvalidBuffer;
        e |= bits::InvalidOffset;
        ok = ok && e.buffer_error();
        cp non_buffer = cp(bits::Failed);
        ok = ok && !non_buffer.buffer_error();
    }

    // Byte index helpers
    {
        cp e;
        e.set_byte_index(5u);
        ok = ok && (e.get_byte_index() == 5u);
        e.set_byte_index(0u);
        ok = ok && (e.get_byte_index() == 0u);
    }

    // Replacement-character decision mask
    {
        cp e(bits::NotDecodable);
        ok = ok && e.use_replacement_character();
        cp f(bits::NonCharacter);
        ok = ok && f.use_replacement_character();
    }

    return ok;
}

};  //  namespace toolkit

};  //  namespace utf

};  //  namespace unicode
