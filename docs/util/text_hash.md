File: docs/util/text_hash.md

# Text hashing utilities (CRC-CCITT-FALSE)

## Purpose and scope

This document describes a small set of text hashing utilities based on a
16-bit CCITT CRC and a compact ASCII-hex representation of that CRC.

The functionality described here is a standalone utility that is distributed
alongside SuiteUTF for practical reasons. It is commonly useful in the same
contexts as Unicode and UTF tooling, but it is not part of Unicode encoding or
decoding, and it does not depend on Unicode semantics.

All operations described here work on byte sequences.

## CRC and ASCII hash model

### CRC-CCITT-FALSE (as used here)

The CRC produced by these utilities is a 16-bit CCITT CRC using the commonly
known "FALSE" variant.

Its intended use is as a lightweight, non-cryptographic integrity check for
byte sequences. Typical use cases include:

- Detecting accidental data corruption in small to moderate-sized data blocks.
- Verifying integrity of transmitted or stored data where simplicity and
  performance matter more than cryptographic strength.
- Providing stable, deterministic fingerprints for identifiers, records, or
  protocol elements.
- Acting as a quick consistency check in tooling, embedded systems, logging,
  or diagnostic paths.

The CRC is deterministic and purely byte-oriented. The result is a `uint16_t`
value, and the same input byte sequence will always produce the same CRC.

The exact polynomial and initialization details are fixed by the implementation
and are not configurable through this interface.

### 16-bit CRC represented as a 32-bit ASCII hex hash

In addition to returning the raw 16-bit CRC, the utility supports a compact
32-bit representation referred to here as an "ASCII hash":

- The 16-bit CRC is formatted as four uppercase hexadecimal digits
  (`0-9`, `A-F`).
- Each hex digit is stored as an ASCII byte.
- The four bytes are packed into a single `uint32_t`.

Conceptually, this is equivalent to the four-character ASCII string
"ABCD", but stored in an integer rather than a character array.

This representation is useful for fixed-width tags that are easy to log,
embed into identifiers, or compare, without allocating or managing strings.

### Transformation helpers

Three helpers define the relationship between the two representations:

- `crc_to_ascii_hash`
  Converts a 16-bit CRC value into a packed 4-byte ASCII hex hash using
  uppercase digits.

- `ascii_hash_to_crc`
  Converts a packed ASCII hex hash back into the original 16-bit CRC value.
  This assumes the input consists of valid ASCII hex characters.

- `is_valid_ascii_hash`
  Checks whether a packed 32-bit value consists of four ASCII hex characters
  in the supported range (`0-9`, `A-F`).

A typical flow is to compute a CRC from bytes, convert it to an ASCII hash for
external use, and validate and decode it later if required.

## Functions and typical usage

### CRC calculation

Two primary CRC calculation forms are provided:

- Null-terminated input

  Computes the CRC over a conventional C-style string. Processing stops at
  the first NUL (`'\0'`) byte.

- Explicit-length input

  Computes the CRC over exactly `length` bytes. This form must be used when
  the data may contain embedded NUL bytes or is not a string.

Both forms operate on byte values and make no assumptions about text encoding.

Convenience overloads exist for both `uint8_t*` and `char*` inputs. The `char*`
overloads are simple pointer conversions and do not alter behavior.

### ASCII hash convenience helpers

For convenience, combined helpers are provided that compute the CRC and
immediately convert it to the packed ASCII hash form. These exist for both
null-terminated and explicit-length inputs.

These helpers are equivalent to calling the CRC function followed by
`crc_to_ascii_hash`.

### Test helpers

Simple test functions are provided as part of the same distribution. Their
implementations live in a separate source file alongside the non-inlined
function bodies.

The test helpers perform basic validation of the CRC calculation and the
ASCII-hash transformation logic. They are intended to help detect breaking
changes during maintenance or refactoring rather than to serve as a full
conformance test suite.

## Safety and constraints

- Byte-oriented processing

  All hashing operates on raw byte sequences. No Unicode interpretation,
  validation, or normalization is performed.

- Embedded NUL handling

  Null-terminated helpers stop at the first NUL byte. Use the explicit-length
  form when hashing binary data or text that may contain embedded NULs.

- ASCII hash validation

  Decoding a packed ASCII hash assumes the input is well-formed. Use the
  validation helper before decoding values that originate outside the program.

## Examples

Computing a CRC from a null-terminated string:

    const char* s = "Hello";
    uint16_t crc = crc_ccitt_false(s);

Computing a CRC from a buffer with explicit length:

    const uint8_t data[] = { 'A', 0, 'B', 'C' };
    uint16_t crc = crc_ccitt_false(data, 4u);

Converting a CRC to an ASCII hash and validating it:

    const char* s = "Hello";
    uint16_t crc = crc_ccitt_false(s);
    uint32_t hash = crc_to_ascii_hash(crc);

    if (is_valid_ascii_hash(hash))
    {
        uint16_t recovered = ascii_hash_to_crc(hash);
        (void)recovered;
    }

## Used alongside SuiteUTF

Although independent of Unicode processing, these utilities are often useful
near UTF-related code. Common uses include generating compact debug tags,
lightweight fingerprints for identifiers, and stable markers for logging or
testing encoded text paths.
