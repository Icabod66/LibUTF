# wchar_t guidance for SuiteUTF

This document provides practical guidance on handling wchar_t when using
SuiteUTF. It focuses on common platform behavior and the failure modes that
matter in real code. It is engineering guidance, not a standards reference.

## Summary

- wchar_t is an implementation-defined code unit type.
- It does not represent Unicode scalar values.
- Element-wise processing of wchar_t is unsafe.
- wchar_t should be treated as a boundary or legacy type.
- SuiteUTF handles wchar_t by explicit decoding, never by assumption.

## What matters about wchar_t

The C and C++ standards intentionally leave the size and encoding interpretation
of wchar_t unspecified. In practice, two patterns dominate.

### 16-bit wchar_t (commonly Windows)

- sizeof(wchar_t) == 2
- Used by system APIs as UTF-16 code units
- A single wchar_t value may be:
  - a complete BMP code point, or
  - one half of a surrogate pair

Unpaired or ill-formed surrogates can appear in real data.

### 32-bit wchar_t (many Unix-like systems)

- sizeof(wchar_t) == 4
- Often treated as UTF-32-like by system libraries

Even in this case:
- Not all values are valid Unicode scalar values
- Invalid or reserved code points may appear

In both cases, wchar_t represents code units, not characters.

## Consequences

Because wchar_t elements are code units:

- Iteration, indexing, and length calculations are unreliable
- Non-BMP characters may be split or miscounted
- Invalid data may be silently accepted

Element-wise interpretation of wchar_t as characters is incorrect.

## Guidance for new code

For portable Unicode text handling:

- Avoid wchar_t as a primary text type in new APIs
- Prefer explicit-width types:
  - char8_t for UTF-8
  - char16_t for UTF-16
  - char32_t for UTF-32
- Treat wchar_t as an interchange or boundary type

Explicit-width types make encoding assumptions visible and auditable.

## Handling wchar_t in SuiteUTF

SuiteUTF does not treat wchar_t as a first-class Unicode representation.

Instead:

- Determine the effective width of wchar_t
- Decode explicitly using the corresponding UTF path
- Operate on Unicode scalar values
- Re-encode explicitly if required

### Practical handling

- sizeof(wchar_t) == 2
  - Decode as UTF-16
  - Allow SuiteUTF to handle surrogate pairs

- sizeof(wchar_t) == 4
  - Decode as UTF-32
  - Validate scalar values explicitly

In all cases, decoding precedes iteration or transformation.

## Decision summary

- wchar_t is a code unit type
- Its encoding depends on platform conventions
- Element-wise use is incorrect
- Explicit decoding is required
- SuiteUTF treats wchar_t as boundary data
