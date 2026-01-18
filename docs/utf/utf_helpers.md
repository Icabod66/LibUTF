# File: docs/utf/utf_helpers.md

# utf_helpers.h

This header provides small, inline, constexpr helpers that are exposed as a
courtesy to users of SuiteUTF.

None of the other headers or implementation files in SuiteUTF depend on or
reference utf_helpers.h directly. Instead, these functions are provided because
they fit naturally within the same conceptual framework as the rest of the
library and have proven useful when writing higher-level code that works with
byte streams, text decoding, or encoding detection alongside SuiteUTF.

Links:
- docs/utf/toolkit_overview.md
- docs/utf/variants_utf_sub_type.md
- docs/utf/overlong_utf8.md

## Purpose and scope

utf_helpers.h exists to support upstream code that uses SuiteUTF, rather than to
support SuiteUTF's own internal implementation.

It provides small, fast helpers that answer local, byte-level questions such as:

- Could this byte plausibly appear in a UTF-8 sequence?
- If I treat this byte as a UTF-8 lead byte, how many bytes would the sequence
  claim to use?
- For a given UTF-8 byte length, how many bits are encodable, and what is the
  maximum representable code point value?
- Is this byte outright illegal under GB18030 or Shift-style encodings?
- Could this byte appear in a given position of a multi-byte sequence?

These helpers are intended to be composed by user code when building scanners,
decoders, validators, or heuristics that sit above SuiteUTF.

They intentionally operate at a lower semantic level than the toolkit decode
functions and do not apply full encoding validity or policy rules. Where these
helpers interact with extended or irregular forms, the interpretation of those
forms is defined by the toolkit layer; see docs/utf/overlong_utf8.md and
docs/utf/toolkit_overview.md.

## What it provides

- UTF-8 byte identification helpers:
  - isLeadUTF8, isContUTF8, isBadUTF8
- UTF-8 small utilities:
  - leadToBytesUTF8
  - bitCountUTF8
  - maxUnicodeUTF8
- GB18030 byte identification helpers:
  - isIllegalGB18030_Byte
  - possibleGB18030_Byte0/1/2/3
  - possibleGB18030_1Byte/2Byte/4Byte
- Shift encoding (SJIS and similar) byte identification helpers:
  - isIllegalSHIFT_Byte
  - possibleSHIFT_Byte0/1
  - possibleSHIFT_1Byte/2Byte

All functions are inline constexpr and noexcept.

## What it does not provide

- It is not a full UTF-8 validator.
- It is not a transcoder.
- It does not enforce overlong rules, surrogate exclusions, Unicode range
  ceilings, or canonical constraints by itself.
- "possible" predicates do not guarantee a valid sequence; they only encode
  local per-byte range checks.

## UTF-8 helpers

### isLeadUTF8

    inline constexpr bool isLeadUTF8(uint8_t byte) noexcept;

Returns true when the byte is a plausible UTF-8 lead byte under SuiteUTF's rules:

- It must not be a continuation byte (i.e. not 0x80..0xBF).
- It must be less than 0xFE (so 0xFE and 0xFF are not treated as leads).

In terms of the implementation, it is:

- true if (byte & 0xC0) != 0x80 and byte < 0xFE.

This is a byte-classification helper, not a correctness proof.

### isContUTF8

    inline constexpr bool isContUTF8(uint8_t byte) noexcept;

Returns true for UTF-8 continuation bytes (0x80..0xBF), i.e. (byte & 0xC0) ==
0x80.

### isBadUTF8

    inline constexpr bool isBadUTF8(uint8_t byte) noexcept;

Returns true for bytes >= 0xFE. Under this header, 0xFE and 0xFF are treated as
bad bytes for UTF-8 purposes.

### leadToBytesUTF8

    inline constexpr uint32_t leadToBytesUTF8(uint8_t lead) noexcept;

Infers a byte count from a lead byte. This function answers: "If I treat this
byte as a lead, how many bytes would the encoded sequence claim to occupy?"

Important notes about its meaning:

- It does not validate the sequence.
- It intentionally maps certain unexpected or illegal inputs to 1.

Behavior summary:

- Returns 1 for:
  - ASCII (0x00..0x7F)
  - unexpected continuation bytes (0x80..0xBF)
  - illegal lead bytes (0xFE..0xFF)
- Returns 2 for lead bytes in 0xC0..0xDF.
- Returns 3 for lead bytes in 0xE0..0xEF.
- Returns 4 for lead bytes in 0xF0..0xF7.
- Returns 5 or 6 for extended lead bytes in 0xF8..0xFD.

Extended UTF-8 behavior:

This helper intentionally recognizes extended UTF-8 lead bytes and lengths
beyond modern standard UTF-8. These forms are part of the SuiteUTF toolkit
diagnostic and indexing model and are not indicators of valid UTF-8 text; see
docs/utf/overlong_utf8.md for how extended lengths and overlong forms are
interpreted by the toolkit layer.

### bitCountUTF8

    inline constexpr uint32_t bitCountUTF8(uint32_t bytes) noexcept;

Returns the number of bits encodable in a UTF code point based on the number of
bytes used by the UTF-8 encoding, as defined by this library.

Practically:

- For bytes in the supported range 1..6, it returns a bit capacity for the
  payload bits of the code point.
- For other values, it returns 0.

This value reflects bit capacity, not Unicode scalar value limits or modern
UTF-8 constraints.

### maxUnicodeUTF8

    inline constexpr unicode_t maxUnicodeUTF8(uint32_t bytes) noexcept;

Returns the maximum unicode value that can be encoded in the specified number
of bytes, as defined by this library.

Practically:

- For bytes in the supported range 1..6, it returns
  (2^bitCountUTF8(bytes)) - 1 as a unicode_t.
- For other values, it returns -1.

This value reflects bit capacity, not Unicode scalar value limits or modern
UTF-8 constraints.

## GB18030 helpers

The GB18030 helpers are "possible" predicates. They answer: "Could this byte be
part of a GB18030 sequence at this position?" They are range checks only.

### isIllegalGB18030_Byte

    inline constexpr bool isIllegalGB18030_Byte(uint8_t byte) noexcept;

Marks 0xFF as illegal:

- Returns true if byte == 0xFF.

### possibleGB18030_Byte0

    inline constexpr bool possibleGB18030_Byte0(uint8_t byte0) noexcept;

Returns true if byte0 <= 0xFE.

### possibleGB18030_Byte1

    inline constexpr bool possibleGB18030_Byte1(uint8_t byte1) noexcept;

Returns true if byte1 is either:

- in 0x40..0xFE excluding 0x7F, or
- in 0x30..0x39.

### possibleGB18030_Byte2

    inline constexpr bool possibleGB18030_Byte2(uint8_t byte2) noexcept;

Returns true if byte2 is in 0x81..0xFE.

### possibleGB18030_Byte3

    inline constexpr bool possibleGB18030_Byte3(uint8_t byte3) noexcept;

Returns true if byte3 is in 0x30..0x39.

### possibleGB18030_1Byte, possibleGB18030_2Byte, possibleGB18030_4Byte

These predicates are used once a candidate length has been chosen.

- possibleGB18030_1Byte: byte0 <= 0x80.
- possibleGB18030_2Byte: byte1 in 0x40..0xFE excluding 0x7F.
- possibleGB18030_4Byte: byte1 in 0x30..0x39.

What "possible" means:

- The byte is within an allowed range for that position.
- It does not imply the overall sequence is valid, mapped, or decodable.

Illegal byte rule:

- 0xFF is explicitly illegal.

## Shift encoding helpers (SJIS and similar)

These helpers support Shift-style encodings where some bytes are single-byte
characters and others form two-byte sequences.

As with GB18030, these are range checks only.

### isIllegalSHIFT_Byte

    inline constexpr bool isIllegalSHIFT_Byte(uint8_t byte) noexcept;

Returns true for bytes >= 0xFD.

### possibleSHIFT_Byte0

    inline constexpr bool possibleSHIFT_Byte0(uint8_t byte0) noexcept;

Returns true if:

- byte0 <= 0xFC,
- byte0 is not 0x80,
- byte0 is not 0xA0.

### possibleSHIFT_Byte1

    inline constexpr bool possibleSHIFT_Byte1(uint8_t byte1) noexcept;

Returns true if byte1 is in 0x40..0xFC excluding 0x7F.

### possibleSHIFT_1Byte

    inline constexpr bool possibleSHIFT_1Byte(uint8_t byte0) noexcept;

Returns true if byte0 is:

- 0x00..0x7F, or
- 0xA1..0xDF.

### possibleSHIFT_2Byte

    inline constexpr bool possibleSHIFT_2Byte(uint8_t byte0) noexcept;

Returns true if byte0 is:

- 0x81..0x9F, or
- 0xE0..0xFC.

## Typical usage patterns

These helpers are typically used in upstream code that sits alongside SuiteUTF,
for example:

- fast scanning or classification of byte streams,
- choosing a likely decode path before invoking higher-level logic,
- rejecting obviously impossible bytes early.

They are deliberately low-level and do not replace full decoding or validation.

Example 1: UTF-8 scanning using leadToBytesUTF8

    const uint8_t b0 = p[0];

    if (unicode::utf::isBadUTF8(b0))
        return Error;

    const uint32_t n = unicode::utf::leadToBytesUTF8(b0);

    if (n == 1)
    {
        // ASCII or otherwise treated as single-byte.
        consume(1);
    }
    else
    {
        if (remaining < n)
            return NeedMoreInput;

        for (uint32_t i = 1; i < n; ++i)
        {
            if (!unicode::utf::isContUTF8(p[i]))
                return Error;
        }

        consume(n);
    }

Example 2: GB18030 length decision by inspecting the second byte

    const uint8_t b0 = p[0];

    if (!unicode::utf::possibleGB18030_Byte0(b0))
        return Error;

    if (b0 <= 0x80)
    {
        consume(1);
    }
    else
    {
        const uint8_t b1 = p[1];

        if (!unicode::utf::possibleGB18030_Byte1(b1))
            return Error;

        if (unicode::utf::possibleGB18030_2Byte(b1))
        {
            consume(2);
        }
        else
        {
            if (!unicode::utf::possibleGB18030_Byte2(p[2]))
                return Error;
            if (!unicode::utf::possibleGB18030_Byte3(p[3]))
                return Error;

            consume(4);
        }
    }
