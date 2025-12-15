# docs/utf/wchar_t_guidance.md

# wchar_t guidance for LibUTF

This document provides pragmatic guidance on wchar_t, its platform-dependent
meaning, and how to handle it safely when using LibUTF. It is intended as
engineering guidance, not a standards compliance or conformance document.

The focus is on clarifying how wchar_t is commonly used, why it does not map
cleanly to Unicode characters, and how LibUTF handles such data when it must be
supported.

## Framing and takeaway

This document assumes familiarity with the design intent of the LibUTF
standard and toolkit layers. See std_overview.md and
toolkit_overview.md for background on their respective roles.

The key points to keep in mind are:

- wchar_t is not a Unicode character type.
- Its size and interpretation are implementation-defined.
- It represents code units, not Unicode scalar values.
- wchar_t is best treated as a boundary or legacy type.
- LibUTF handles wchar_t safely by decoding it explicitly, not by assuming
  element-wise semantics.

## What wchar_t actually represents

wchar_t is a built-in character type in C and C++ intended to represent "wide
characters". The language standards intentionally leave its representation and
encoding interpretation implementation-defined.

In particular:

- The standard does not define wchar_t as UTF-16, UTF-32, or any other
  Unicode encoding.
- The size of wchar_t may vary between platforms and environments.
- A wchar_t value is a code unit whose interpretation depends on
  external context.

The name "wide character" is historical and does not imply a direct or stable
mapping to Unicode characters.

## How platforms commonly use wchar_t

Although not guaranteed by the language, most modern systems fall into a small
number of patterns.

### Windows

- wchar_t is typically 16 bits.
- Windows wide-character APIs use wchar_t arrays as UTF-16 code units.
- A single wchar_t value may represent:
  - A complete Unicode code point in the Basic Multilingual Plane.
  - One half of a surrogate pair.

In this environment, wchar_t behaves like a UTF-16 code unit type.

### Many Unix-like systems

- wchar_t is typically 32 bits.
- It is often treated as UTF-32-like by system libraries and locales.

Even in this case:

- Not all 32-bit values are necessarily valid Unicode scalar values.
- Invalid or reserved code points may appear in practice.
- The interpretation still depends on the surrounding environment.

## Why naive use breaks

Using wchar_t as a Unicode character type commonly results in iteration,
indexing, and length-counting errors.

Typical consequences include:

- Characters being split or miscounted during iteration.
- Incorrect indexing, especially with non-BMP characters.
- Data corruption when truncating or slicing buffers.
- Silent acceptance of invalid code points or unpaired surrogates.

These issues arise because wchar_t elements are code units, not characters, and
because the encoding they participate in is not implicit in the type itself.

## Guidance for new code

For new, portable code, wchar_t is generally a poor choice for representing
Unicode text.

Recommended practices include:

- It is generally preferable to avoid wchar_t as a primary text type in new
  APIs.
- Prefer explicit-width types with clear intent:
  - char8_t for UTF-8 code units.
  - char16_t for UTF-16 code units.
  - char32_t for UTF-32 code units.
- Treat wchar_t as an interchange, boundary, or legacy type rather than a core
  representation.

Using explicit-width types makes encoding assumptions visible and reduces the
need for platform-specific reasoning.

## How to handle wchar_t with LibUTF

LibUTF does not treat wchar_t as a first-class Unicode type. Instead, it expects
wchar_t buffers to be decoded explicitly based on their effective width and
context.

A common and effective approach is to decode first, then operate on Unicode
scalar values.

### 16-bit wchar_t handling

When sizeof(wchar_t) is 2, as is typical on Windows:

- Treat wchar_t* as a sequence of UTF-16 code units.
- Pass the buffer through LibUTF UTF-16 decode paths.
- Allow LibUTF to handle surrogate pairs during decoding.

Element-wise processing of wchar_t values as characters is not appropriate.

### 32-bit wchar_t handling

When sizeof(wchar_t) is 4:

- Treat wchar_t values as UTF-32 code units only after validation.
- Pass the buffer through LibUTF UTF-32 decode paths.
- Reject or report invalid scalar values explicitly.

Even with 32-bit wchar_t, decoding and validation remain important.

### Decoding, not element-wise processing

In all cases:

- Decode wchar_t buffers into Unicode scalar values using LibUTF.
- Perform iteration, counting, and transformation on decoded values.
- Re-encode explicitly to the desired output encoding.

This approach avoids surrogate handling errors and makes error handling
intentional and visible.

## Choosing between utf_std and utf_toolkit

wchar_t data often originates at platform or API boundaries, where encoding
assumptions and data quality are not always under the program's control.

LibUTF provides two layers that can both be used for wchar_t decoding, depending
on intent.

### utf_toolkit

- Provides validation and detailed diagnostics.
- Reports observed issues using cp_errors rather than sentinel values.
- Never returns sentinel unicode values.
- Often the better default for wchar_t, especially when handling legacy data,
  platform APIs, or uncertain inputs.

This layer is useful when you want to understand what the input contains and
decide how to respond.

### utf_std

- Focused on strict, deterministic behavior.
- Uses sentinel unicode values to signal decode failure.
- Appropriate when inputs are trusted and well-formed by construction.

This layer is suitable when wchar_t data is already constrained by design and
early failure is acceptable.

The choice between the two is primarily about how much information and
flexibility you want when handling boundary data, not about correctness versus
incorrectness.

## Decision summary

A practical summary of handling strategies is as follows:

- sizeof(wchar_t) == 2
  - Treat as UTF-16 code units.
  - Decode using LibUTF UTF-16 paths.
  - Handle surrogate pairs during decoding.

- sizeof(wchar_t) == 4
  - Treat as UTF-32 code units after validation.
  - Decode using LibUTF UTF-32 paths.
  - Reject or report invalid scalar values.

In all cases:

- Element-wise interpretation is not appropriate.
- Explicit decoding followed by operation on Unicode scalar values is preferred.

## References

This guidance reflects common platform behavior, Unicode encoding rules, and
widely used API conventions such as Windows wide-character interfaces and
Unicode surrogate ranges. It does not claim formal standards compliance and
should be read as pragmatic engineering advice.
