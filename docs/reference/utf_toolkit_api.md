File: docs/reference/utf_toolkit_api.md

# LibUTF toolkit API reference (utf_toolkit.h)

This document is a reference for the public API provided by the LibUTF
toolkit layer, declared in `unicode::utf::toolkit`.

It lists public types, enums, interfaces, and free functions, grouped by
purpose. Conceptual guidance and usage patterns are described in:

- docs/utf/toolkit_overview.md

All APIs are allocation-free and exception-free. Error reporting is performed
via the `cp_errors` return type.

## Namespaces

All entities documented here are defined in:

- `namespace unicode::utf::toolkit`

The toolkit builds on shared types from:

- `namespace unicode`
- `namespace unicode::utf`

## Fundamental types

### unicode::utf::utf_text

Stream-oriented encoded text cursor shared with the std layer.

Members:
- `uint32_t length`
  - Buffer size in bytes.
- `uint32_t offset`
  - Current byte offset into the buffer.
- `uint8_t* buffer`
  - Pointer to the byte buffer (may be null).

All toolkit operations that read or write encoded data operate on `utf_text`.

## Encoding sub-type selection

### enum class UTF_SUB_TYPE : int32_t

Defines a complete encoding and behavior model. A `UTF_SUB_TYPE` combines:

- Encoding family (UTF-8, UTF-16, UTF-32, single-byte).
- Endianness where applicable.
- Compatibility modes (CESU, modified UTF-8).
- Permissive, non-skipping, or strict behavior.

Representative values include:

- UTF-8 family:
  - `UTF8`, `UTF8ns`, `UTF8st`
  - `JUTF8`, `JUTF8ns`, `JUTF8st`
  - `CESU8`, `CESU8ns`, `CESU8st`
  - `JCESU8`, `JCESU8ns`, `JCESU8st`
- UTF-16 family:
  - `UTF16le`, `UTF16be`
  - `UCS2le`, `UCS2be`
- UTF-32 family:
  - `UTF32le`, `UTF32be`
  - `UCS4le`, `UCS4be`
  - `CESU32le`, `CESU32be`
  - `CESU4le`, `CESU4be`
- Single-byte encodings:
  - `BYTE`, `BYTEns`
  - `ASCII`, `ASCIIns`
  - `CP1252`, `CP1252ns`, `CP1252st`

`COUNT` represents the number of sub-types.

`UTF_SUB_TYPE` is most commonly used with the `IUTFTK` handler interface.

## Error and warning reporting

### class cp_errors

Bitset type used to report errors and warnings from toolkit operations.

`cp_errors` stores a 32-bit state value and provides bitwise operations,
queries, and helpers for common classifications.

#### cp_errors::bits

Enumeration of individual error and warning bits.

Categories include:

- Buffer and structural errors:
  - `InvalidBuffer`
  - `InvalidOffset`
  - `MisalignedOffset`
  - `MisalignedLength`
  - `WriteOverflow`
  - `ReadTruncated`
- General encoding errors:
  - `NotEncodable`
  - `NotDecodable`
  - `BadSizeUTF8`
  - `Untransformable`
  - `NotEnoughBits`
  - `DisallowedByte`
  - `UnexpectedByte`
- Warnings and properties:
  - `InvalidPoint`
  - `ExtendedUCS4`
  - `Supplementary`
  - `NonCharacter`
  - `SurrogatePair`
  - `HighSurrogate`
  - `LowSurrogate`
  - `IrregularForm`
  - `ModifiedUTF8`
  - `OverlongUTF8`
  - `ExtendedUTF8`
- State flags:
  - `Failed`

Lower bits are reserved for internal byte index information.

#### Constructors and basic access

- `cp_errors()`
  - Construct with no flags set.
- `cp_errors(bits bit)`
  - Construct from a single bit.
- `explicit cp_errors(uint32_t raw)`
  - Construct from a raw bit mask.
- `uint32_t raw() const`
  - Return raw underlying value.

#### Query helpers

- `bool any() const`
- `bool any(bits bit) const`
- `bool any(cp_errors mask) const`
- `bool none() const`
- `bool failed() const`
- `bool error() const`
- `bool no_error() const`
- `bool buffer_error() const`
- `bool use_replacement_character() const`

#### Filtering helpers

- `cp_errors errors_only() const`
- `cp_errors warnings_only() const`
- `cp_errors buffer_errors_only() const`

#### Byte index helpers

- `uint32_t get_byte_index() const`
- `void set_byte_index(uint32_t index)`

#### Bitwise operators

Supports `~`, `&`, `|`, `^`, and corresponding assignment operators for both
`bits` and `cp_errors`.

Implicit conversion to `bool` tests whether any non-reserved bits are set.

## utf_text validation helpers

### cp_errors get_errors(const utf_text& text)

Checks basic structural validity:

- Null buffer pointer.
- Offset greater than length.

Returns a `cp_errors` value describing detected issues.

### cp_errors get_errors(const utf_text& text, uint32_t alignment_mask)

Extends basic validation with alignment checks:

- Misaligned offset.
- Misaligned length.

`alignment_mask` is typically `(unit_size - 1)` for UTF-16 or UTF-32.

## Encoded length calculation

### uint32_t lenUTF8(unicode_t unicode,
                    bool use_cesu = false,
                    bool use_java = false)

Returns the number of bytes required to encode a code point in UTF-8 under
the specified rules. Returns 0 if not encodable.

### uint32_t lenUTF16(unicode_t unicode,
                     bool use_ucs2 = false)

Returns the number of bytes required to encode a code point in UTF-16.
If `use_ucs2` is true, surrogate pairs are disabled.

### uint32_t lenUTF32(unicode_t unicode,
                     bool use_cesu = false,
                     bool use_ucs4 = false)

Returns the number of bytes required to encode a code point in UTF-32.
Flags control CESU compatibility and extended UCS-4 range support.

## Low-level encoding functions

All encoding functions write to `utf_text` at the current offset and return
`cp_errors`. The `bytes` output parameter reports the number of bytes written.

### cp_errors encodeBYTE(utf_text& text,
                         unicode_t unicode,
                         uint32_t& bytes,
                         bool use_ascii = false)

Encode a single-byte value. With `use_ascii`, only 7-bit values are allowed.

### cp_errors encodeUTF8(utf_text& text,
                         unicode_t unicode,
                         uint32_t& bytes,
                         bool use_cesu = false,
                         bool use_java = false)

Encode a UTF-8 code point with optional CESU and modified UTF-8 behavior.

### cp_errors encodeUTF8n(utf_text& text,
                          unicode_t unicode,
                          uint32_t bytes,
                          bool use_java = false)

Encode UTF-8 using an explicitly specified byte length.

### cp_errors encodeUTF16(utf_text& text,
                          unicode_t unicode,
                          uint32_t& bytes,
                          bool le = false,
                          bool use_ucs2 = false)

Encode UTF-16 with selectable endianness and UCS-2 restriction.

### cp_errors encodeUTF32(utf_text& text,
                          unicode_t unicode,
                          uint32_t& bytes,
                          bool le = false,
                          bool use_cesu = false,
                          bool use_ucs4 = false)

Encode UTF-32 with selectable endianness and compatibility options.

### cp_errors encodeCP1252(utf_text& text,
                           unicode_t unicode,
                           uint32_t& bytes,
                           bool strict = false)

Encode Windows Code Page 1252. Strict mode rejects unmapped values.

## Low-level decoding functions

All decoding functions read from `utf_text` at the current offset and return
`cp_errors`. The `bytes` output parameter reports the number of bytes consumed.

### cp_errors decodeBYTE(const utf_text& text,
                         unicode_t& unicode,
                         uint32_t& bytes,
                         bool use_ascii = false,
                         bool coalesce = true)

Decode a single-byte encoding with optional ASCII restriction and coalescing.

### cp_errors decodeUTF8(const utf_text& text,
                         unicode_t& unicode,
                         uint32_t& bytes,
                         bool use_cesu = false,
                         bool use_java = false,
                         bool strict = false,
                         bool coalesce = true)

Decode UTF-8 with configurable permissiveness, strictness, and coalescing.

### cp_errors decodeUTF16(const utf_text& text,
                          unicode_t& unicode,
                          uint32_t& bytes,
                          bool le = false,
                          bool use_ucs2 = false)

Decode UTF-16 with selectable endianness and UCS-2 restriction.

### cp_errors decodeUTF32(const utf_text& text,
                          unicode_t& unicode,
                          uint32_t& bytes,
                          bool le = false,
                          bool use_cesu = false,
                          bool use_ucs4 = false)

Decode UTF-32 with compatibility and extended range options.

### cp_errors decodeCP1252(const utf_text& text,
                           unicode_t& unicode,
                           uint32_t& bytes,
                           bool strict = false,
                           bool coalesce = true)

Decode Windows Code Page 1252 with optional strictness and coalescing.

## BOM and NULL encoding helpers

### cp_errors encodeUTF8_BOM(utf_text& text, uint32_t& bytes)
### cp_errors encodeUTF16_BOM(utf_text& text,
                             uint32_t& bytes,
                             bool le = false)
### cp_errors encodeUTF32_BOM(utf_text& text,
                             uint32_t& bytes,
                             bool le = false)

Write a byte order marker for the specified encoding.

### cp_errors encodeUTF8_NULL(utf_text& text, uint32_t& bytes)
### cp_errors encodeUTF16_NULL(utf_text& text, uint32_t& bytes)
### cp_errors encodeUTF32_NULL(utf_text& text, uint32_t& bytes)

Write a NULL code point for the specified encoding.

## Step and back functions

These functions move the `utf_text` offset forward or backward by code points.
They return the number of code points successfully skipped.

### uint32_t stepBYTE(utf_text& text,
                      uint32_t count,
                      bool use_ascii = false,
                      bool coalesce = true)

### uint32_t backBYTE(utf_text& text,
                      uint32_t count,
                      bool use_ascii = false,
                      bool coalesce = true)

### uint32_t stepUTF8(utf_text& text,
                      uint32_t count,
                      bool use_cesu = false,
                      bool use_java = false,
                      bool strict = false,
                      bool coalesce = true)

### uint32_t backUTF8(utf_text& text,
                      uint32_t count,
                      bool use_cesu = false,
                      bool use_java = false,
                      bool strict = false,
                      bool coalesce = true)

### uint32_t stepUTF16(utf_text& text,
                       uint32_t count,
                       bool le = false,
                       bool use_ucs2 = false)

### uint32_t backUTF16(utf_text& text,
                       uint32_t count,
                       bool le = false,
                       bool use_ucs2 = false)

### uint32_t stepUTF32(utf_text& text,
                       uint32_t count,
                       bool le = false,
                       bool use_cesu = false)

### uint32_t backUTF32(utf_text& text,
                       uint32_t count,
                       bool le = false,
                       bool use_cesu = false)

### uint32_t stepCP1252(utf_text& text,
                        uint32_t count,
                        bool strict = false,
                        bool coalesce = true)

### uint32_t backCP1252(utf_text& text,
                        uint32_t count,
                        bool strict = false,
                        bool coalesce = true)

## Overlong UTF-8 companion helpers to be used in combination with encodeUTF8n()

### bool isOverlongUTF8(unicode_t unicode, uint32_t bytes)

Returns true if the given code point and byte length represent an overlong
UTF-8 encoding.

### bool overlongToIndexUTF8(unicode_t unicode,
                             uint32_t bytes,
                             uint32_t& index)

Convert an overlong UTF-8 encoding to an index value.

### bool indexToOverlongUTF8(uint32_t index,
                             unicode_t& unicode,
                             uint32_t& bytes)

Convert an index value to an overlong UTF-8 encoding.

## Handler interface

### struct IUTFTK

Abstract interface providing unified access to toolkit functionality using a
selected encoding model.

#### Handler acquisition

- `static const IUTFTK& getHandler(UTF_TYPE utfType)`
- `static const IUTFTK& getHandler(UTF_SUB_TYPE utfSubType)`
- `static const IUTFTK& getHandler(UTF_OTHER_TYPE utfOtherType)`

#### Virtual interface

- `UTF_TYPE utfType() const`
- `UTF_SUB_TYPE utfSubType() const`
- `uint32_t unitSize() const`
- `uint32_t len(unicode_t unicode) const`
- `uint32_t lenBOM() const`
- `uint32_t lenNull() const`
- `cp_errors get(const utf_text& text,
                 unicode_t& unicode,
                 uint32_t& bytes) const`
- `cp_errors set(utf_text& text,
                 unicode_t unicode,
                 uint32_t& bytes) const`
- `cp_errors setBOM(utf_text& text, uint32_t& bytes) const`
- `cp_errors setNull(utf_text& text, uint32_t& bytes) const`
- `uint32_t step(utf_text& text, uint32_t count) const`
- `uint32_t back(utf_text& text, uint32_t count) const`

#### Utility functions

Non-virtual helpers implemented in terms of the virtual interface:

- `cp_errors read(utf_text& text, unicode_t& unicode) const`
- `cp_errors write(utf_text& text, unicode_t unicode) const`
- `cp_errors writeBOM(utf_text& text) const`
- `cp_errors writeNull(utf_text& text) const`
- `cp_errors validate(const utf_text& text) const`
- Normalized line-feed helpers:
  - `getNLF`
  - `readNLF`
  - `getLine`
  - `readLine`

These helpers provide higher-level stream operations while preserving the
error reporting and behavior defined by the selected `UTF_SUB_TYPE`.
