// File: docs/reference/utf_std_api.md

# LibUTF std layer API reference

This document describes the public API of the `utf_std` layer, grouped by
category. It does not define new semantics; it reflects the behaviour implied
by `utf_std.h` and the overview in `std_overview.md`.

All functions are `noexcept`, do not allocate, and do not throw exceptions.


## Core types

### `unicode_t`

    typedef int32_t unicode_t;

Integral type used for Unicode scalar values and internal error codes.


### `struct utf_text`

    struct utf_text
    {
        uint32_t    length; // buffer size in bytes
        uint32_t    offset; // byte offset into buffer
        uint8_t*    buffer;
    };

Non-owning view of an encoded buffer.

- `buffer`
  - Pointer to first byte of the buffer, or `nullptr`.
- `length`
  - Size of `buffer` in bytes.
- `offset`
  - Current position in bytes from `buffer`.

Used by `IUTF` non-virtual helpers for sequential processing.


### `enum class UTF_TYPE`

    enum class UTF_TYPE : int32_t
    {
        UTF8    = 0,
        UTF16le = 1,
        UTF16be = 2,
        UTF32le = 3,
        UTF32be = 4,
        OTHER   = 5,
        COUNT   = 6
    };

Encoding type for standard UTF encodings plus an `OTHER` category for
non-UTF or unidentified encodings. See `std_overview.md` for usage patterns.


### `enum class UTF_OTHER_TYPE`

    enum class UTF_OTHER_TYPE : int32_t
    {
        JUTF8     = 0,
        ISO8859_1 = 1,
        Ascii     = 2,
        CP1252    = 3,
        COUNT     = 4
    };

Encoding type for JUTF8 (Java-style UTF-8), ISO-8859-1, ASCII, and CP1252.


## Encoding identification

### `UTF_TYPE identifyUTF(...)`

    UTF_TYPE identifyUTF(
        const uint8_t* const buffer,
        const uint32_t size,
        uint32_t& bytes) noexcept;

Attempts to identify a UTF encoding from:

- A byte-order marker (BOM) at the start of `buffer`.
- The first two ASCII-range bytes.

Parameters:

- `buffer`
  - Pointer to the start of the data.
- `size`
  - Number of bytes available at `buffer`.
- `bytes` (out)
  - On return, the number of BOM bytes consumed (0, 2, 3, or 4).

Returns:

- A `UTF_TYPE` value describing the likely encoding.
- Does not validate the entire buffer; it is a helper, not a validator.


## CP1252 helpers

### `uint32_t lenCP1252(unicode_t unicode)`

    uint32_t lenCP1252(const unicode_t unicode) noexcept;

- Returns 1 if `unicode` is encodable as a single CP1252 byte.
- Returns 0 if it is not encodable.


### `bool getCP1252(...)`

    bool getCP1252(const uint8_t* const buffer,
                   const uint32_t size,
                   unicode_t& unicode,
                   uint32_t& bytes) noexcept;

Decode a CP1252 code point.

Parameters:

- `buffer`, `size`
  - Input bytes and available length.
- `unicode` (out)
  - On success, decoded code point.
- `bytes` (out)
  - On success, the number of bytes consumed (1).
  - On failure, set to the code-unit size (1) or 0 at end-of-buffer.

Return value:

- `true` if decoding succeeds.
- `false` if the value is not decodable or non-compliant.


### `bool setCP1252(...)`

    bool setCP1252(uint8_t* const buffer,
                   const uint32_t size,
                   const unicode_t unicode,
                   uint32_t& bytes) noexcept;

Encode a single CP1252 code point.

Parameters:

- `buffer`, `size`
  - Output buffer and its size.
- `unicode`
  - Code point to encode.
- `bytes` (out)
  - On success, number of bytes written (1).
  - On failure, either 0 (no write) or the code-unit size.

Return value:

- `true` if encoding succeeds.
- `false` if `unicode` is not encodable or the buffer is too small.


## Length helpers

### `lenBYTE`, `lenUTF8`, `lenUTF16`, `lenUTF32`

    inline constexpr uint32_t lenBYTE(const unicode_t unicode,
                                      const bool use_ascii = false) noexcept;
    inline constexpr uint32_t lenUTF8(const unicode_t unicode,
                                      const bool use_java = false) noexcept;
    inline constexpr uint32_t lenUTF16(const unicode_t unicode) noexcept;
    inline constexpr uint32_t lenUTF32(const unicode_t unicode) noexcept;

Return the number of bytes required to encode `unicode` in the given encoding.

- `lenBYTE`
  - Returns 1 if encodable as a single byte.
  - If `use_ascii` is `true`, allows only values up to 0x7f.
- `lenUTF8`
  - Returns 1, 2, 3, or 4 depending on the code point.
  - Returns 0 if `unicode` is not encodable as strict UTF-8.
  - If `use_java` is `true`, encodes U+0000 as a 2-byte sequence.
- `lenUTF16`
  - Returns 2 or 4 depending on the code point (or 0 if not encodable).
- `lenUTF32`
  - Returns 4 for encodable values, 0 otherwise.


## Generic byte-based encode/decode

### `getBYTE` / `setBYTE`

    bool getBYTE(const uint8_t* const buffer,
                 const uint32_t size,
                 unicode_t& unicode,
                 uint32_t& bytes,
                 const bool use_ascii = false) noexcept;

    bool setBYTE(uint8_t* const buffer,
                 const uint32_t size,
                 const unicode_t unicode,
                 uint32_t& bytes,
                 const bool use_ascii = false) noexcept;

Single-byte encode/decode helpers.

- `getBYTE`
  - Decodes a single byte to `unicode`.
  - If `use_ascii` is `true`, treats values above 0x7f as non-compliant.
- `setBYTE`
  - Encodes `unicode` into one byte.
  - If `use_ascii` is `true`, only encodes values up to 0x7f.

Both follow the general success/failure and `bytes` conventions described in
`std_overview.md`.


### `getUTF8` / `setUTF8`

    bool getUTF8(const uint8_t* const buffer,
                 const uint32_t size,
                 unicode_t& unicode,
                 uint32_t& bytes,
                 const bool use_java = false) noexcept;

    bool setUTF8(uint8_t* const buffer,
                 const uint32_t size,
                 const unicode_t unicode,
                 uint32_t& bytes,
                 const bool use_java = false) noexcept;

Decode and encode UTF-8 code points.

- Intended to match the standard behaviour of most web browsers for UTF-8.
- `use_java` enables Java-style 2-byte encoding for U+0000.

On failure:

- For non-compliant or undecodable sequences:
  - `unicode` is set to `0x800000XX` where `XX` is the lead byte.
  - `bytes` is set to the code-unit size (1).
- For buffer underflow or `nullptr`:
  - `unicode` is 0, `bytes` is 0, and the function returns `false`.


### UTF-16 and UTF-32 encode/decode

    bool getUTF16le(const uint8_t* const buffer,
                    const uint32_t size,
                    unicode_t& unicode,
                    uint32_t& bytes) noexcept;
    bool setUTF16le(uint8_t* const buffer,
                    const uint32_t size,
                    const unicode_t unicode,
                    uint32_t& bytes) noexcept;

    bool getUTF16be(const uint8_t* const buffer,
                    const uint32_t size,
                    unicode_t& unicode,
                    uint32_t& bytes) noexcept;
    bool setUTF16be(uint8_t* const buffer,
                    const uint32_t size,
                    const unicode_t unicode,
                    uint32_t& bytes) noexcept;

    bool getUTF32le(const uint8_t* const buffer,
                    const uint32_t size,
                    unicode_t& unicode,
                    uint32_t& bytes) noexcept;
    bool setUTF32le(uint8_t* const buffer,
                    const uint32_t size,
                    const unicode_t unicode,
                    uint32_t& bytes) noexcept;

    bool getUTF32be(const uint8_t* const buffer,
                    const uint32_t size,
                    unicode_t& unicode,
                    uint32_t& bytes) noexcept;
    bool setUTF32be(uint8_t* const buffer,
                    const uint32_t size,
                    const unicode_t unicode,
                    uint32_t& bytes) noexcept;

Decode and encode single code points for the respective encoding and
endianness.

On decode failure:

- `unicode` is set to `0x80000000`.
- `bytes` is set to the code-unit size in bytes (2 for UTF-16, 4 for UTF-32),
  or 0 at end-of-buffer or on underflow.
- Return value is `false`.

On encode failure (non-encodable value or insufficient space):

- Return value is `false`.
- `bytes` is set to 0 or to the required size, depending on the implementation.


## BOM and null helpers

    bool setUTF8_BOM(uint8_t* const buffer,
                     const uint32_t size,
                     uint32_t& bytes) noexcept;

    bool setUTF16le_BOM(uint8_t* const buffer,
                        const uint32_t size,
                        uint32_t& bytes) noexcept;

    bool setUTF16be_BOM(uint8_t* const buffer,
                        const uint32_t size,
                        uint32_t& bytes) noexcept;

    bool setUTF32le_BOM(uint8_t* const buffer,
                        const uint32_t size,
                        uint32_t& bytes) noexcept;

    bool setUTF32be_BOM(uint8_t* const buffer,
                        const uint32_t size,
                        uint32_t& bytes) noexcept;

    bool setUTF8_NULL(uint8_t* const buffer,
                      const uint32_t size,
                      uint32_t& bytes) noexcept;

    bool setUTF16_NULL(uint8_t* const buffer,
                       const uint32_t size,
                       uint32_t& bytes) noexcept;

    bool setUTF32_NULL(uint8_t* const buffer,
                       const uint32_t size,
                       uint32_t& bytes) noexcept;

Write BOMs and null terminators for the respective encodings.

- `buffer`, `size`
  - Output buffer and its size.
- `bytes` (out)
  - On success, number of bytes written.
  - On failure, set to 0.

Return `true` on success, `false` if `buffer` is `nullptr` or `size` is too
small.


## String size and length helpers

### Null-terminated string byte sizes

    uint32_t strsizeUTF8(const uint8_t* const buffer) noexcept;
    uint32_t strsizeUTF16(const uint8_t* const buffer) noexcept;
    uint32_t strsizeUTF32(const uint8_t* const buffer) noexcept;

Return the number of bytes in a null (0) terminated string, excluding the
terminator itself.


### Null-terminated code-point counts

    uint32_t strlenUTF8(const uint8_t* const buffer) noexcept;
    uint32_t strlenUTF16le(const uint8_t* const buffer) noexcept;
    uint32_t strlenUTF16be(const uint8_t* const buffer) noexcept;
    uint32_t strlenUTF32(const uint8_t* const buffer) noexcept;

Return the number of code points in a null-terminated string.

- `strlenUTF8` returns the correct count only for well-formed UTF-8 (including
  Java-style UTF-8).


### Fixed-buffer code-point counts

    uint32_t strlenUTF8(const uint8_t* const buffer,
                        const uint32_t size) noexcept;

    uint32_t strlenUTF16le(const uint8_t* const buffer,
                           const uint32_t size) noexcept;

    uint32_t strlenUTF16be(const uint8_t* const buffer,
                           const uint32_t size) noexcept;

Count code points in a buffer of fixed size. The buffer may or may not be
null-terminated.


## Format conversion size calculation

All of these functions estimate the number of bytes required to store a
null-terminated string when converting between encodings, excluding the size
of the terminating null in the destination encoding.

### From null-terminated UTF-16/UTF-32 to UTF-8

    uint32_t strsizeUTF8fromUTF16le(const uint8_t* const buffer) noexcept;
    uint32_t strsizeUTF8fromUTF16be(const uint8_t* const buffer) noexcept;
    uint32_t strsizeUTF8fromUTF32le(const uint8_t* const buffer) noexcept;
    uint32_t strsizeUTF8fromUTF32be(const uint8_t* const buffer) noexcept;

Estimate the byte size of the UTF-8 representation of a null-terminated
UTF-16/UTF-32 string.


### From null-terminated UTF-8/UTF-32 to UTF-16

    uint32_t strsizeUTF16fromUTF8(const uint8_t* const buffer,
                                  const bool use_java = false) noexcept;

    uint32_t strsizeUTF16fromUTF32le(const uint8_t* const buffer) noexcept;
    uint32_t strsizeUTF16fromUTF32be(const uint8_t* const buffer) noexcept;

Estimate the byte size of the UTF-16 representation of a null-terminated
UTF-8 or UTF-32 string.

- `use_java`:
  - When `true`, permits Java-style UTF-8 2-byte over-long U+0000 encoding.


### Fixed-buffer conversion size calculation

    uint32_t strsizeUTF8fromUTF16le(const uint8_t* const buffer,
                                    const uint32_t size,
                                    const bool use_java = false) noexcept;

    uint32_t strsizeUTF8fromUTF16be(const uint8_t* const buffer,
                                    const uint32_t size,
                                    const bool use_java = false) noexcept;

    uint32_t strsizeUTF8fromUTF32le(const uint8_t* const buffer,
                                    const uint32_t size,
                                    const bool use_java = false) noexcept;

    uint32_t strsizeUTF8fromUTF32be(const uint8_t* const buffer,
                                    const uint32_t size,
                                    const bool use_java = false) noexcept;

    uint32_t strsizeUTF16fromUTF8(const uint8_t* const buffer,
                                  const uint32_t size,
                                  const bool use_java = false) noexcept;

    uint32_t strsizeUTF16fromUTF32le(const uint8_t* const buffer,
                                     const uint32_t size) noexcept;

    uint32_t strsizeUTF16fromUTF32be(const uint8_t* const buffer,
                                     const uint32_t size) noexcept;

Estimate the destination byte sizes for conversions between encodings when the
input buffer has a fixed size and may not be null-terminated.

These functions are typically used to size destination buffers before calling
encode/decode routines or `IUTF` methods.


## `IUTF` handler interface

### Obtaining handlers

    struct IUTF
    {
        static const IUTF& getHandler(const UTF_TYPE utfType = UTF_TYPE::OTHER) noexcept;
        static const IUTF& getHandler(const UTF_OTHER_TYPE utfOtherType) noexcept;
        ...
    };

- `getHandler(UTF_TYPE utfType)`
  - Returns a handler for the specified UTF encoding.
  - If `utfType == UTF_TYPE::OTHER`, the handler corresponds to the default
    "other" encoding (Java-style UTF-8).

- `getHandler(UTF_OTHER_TYPE utfOtherType)`
  - Returns a handler for JUTF8, ISO-8859-1, ASCII, or CP1252.


### Virtual interface

    virtual UTF_TYPE   utfType() const noexcept = 0;
    virtual uint32_t   unitSize() const noexcept = 0;

    virtual uint32_t   len(const unicode_t unicode) const noexcept = 0;
    virtual uint32_t   lenBOM() const noexcept = 0;
    virtual uint32_t   lenNull() const noexcept = 0;

    virtual bool get(const uint8_t* const buffer,
                     const uint32_t size,
                     unicode_t& unicode,
                     uint32_t& bytes) const noexcept = 0;

    virtual bool set(uint8_t* const buffer,
                     const uint32_t size,
                     const unicode_t unicode,
                     uint32_t& bytes) const noexcept = 0;

    virtual bool setBOM(uint8_t* const buffer,
                        const uint32_t size,
                        uint32_t& bytes) const noexcept = 0;

    virtual bool setNull(uint8_t* const buffer,
                         const uint32_t size,
                         uint32_t& bytes) const noexcept = 0;

    virtual uint32_t strsize(const uint8_t* const buffer) const noexcept = 0;
    virtual uint32_t strlen(const uint8_t* const buffer) const noexcept = 0;
    virtual uint32_t strlen(const uint8_t* const buffer,
                            const uint32_t size) const noexcept = 0;

See `std_overview.md` for detailed semantics and error conventions. In
summary:

- `len`, `lenBOM`, `lenNull`
  - Query sizes in bytes for individual code points, BOMs, and null
    terminators.
- `get`, `set`, `setBOM`, `setNull`
  - Single-code-point encode/decode plus BOM and null helpers.
- `strsize`, `strlen`
  - String size and code-point count for null-terminated and fixed-size
    buffers.


### Non-virtual `utf_text` helpers

    bool get(const utf_text& text,
             unicode_t& unicode,
             uint32_t& bytes) const noexcept;

    bool set(utf_text& text,
             const unicode_t unicode,
             uint32_t& bytes) const noexcept;

    bool setBOM(utf_text& text,
                uint32_t& bytes) const noexcept;

    bool setNull(utf_text& text,
                 uint32_t& bytes) const noexcept;

    bool read(utf_text& text,
              unicode_t& unicode) const noexcept;

    bool write(utf_text& text,
               const unicode_t unicode) const noexcept;

    bool writeBOM(utf_text& text) const noexcept;
    bool writeNull(utf_text& text) const noexcept;

    bool validate(const utf_text& text) const noexcept;

- `get` / `set`
  - Operate at `text.buffer + text.offset`, do not change `offset`.
- `read` / `write`
  - Wrap `get` / `set` and advance `offset` by `bytes` on success.
- `setBOM` / `setNull`
  - Write without advancing `offset`.
- `writeBOM` / `writeNull`
  - Write and advance `offset` on success.
- `validate`
  - Validates the entire range `[text.buffer, text.buffer + text.length)` and
    returns `true` if well-formed, `false` otherwise.


### Normalised line-feed and line-reading helpers

    bool getNLF(const utf_text& text,
                unicode_t& unicode,
                uint32_t& bytes) const noexcept;

    bool readNLF(utf_text& text,
                 unicode_t& unicode) const noexcept;

    bool getLine(const utf_text& text,
                 utf_text& line,
                 uint32_t& bytes) const noexcept;

    bool readLine(utf_text& text,
                  utf_text& line) const noexcept;

- `getNLF`
  - Searches at `text.buffer + text.offset` for a line terminator in the
    normalized line-feed set.
  - Sets `unicode` to U+000a for normalized terminators.
  - `bytes` is set to the number of bytes covered by the terminator, or 0 at
    end-of-buffer or when `buffer` is `nullptr`.
  - Returns `true` if a terminator is read, `false` otherwise.

- `readNLF`
  - Wraps `getNLF`.
  - Advances `text.offset` by `bytes` on success.

- `getLine`
  - Collects bytes from the start of the current line up to the next line
    terminator found by `getNLF`, or to the U+0000 string terminator or to
    end-of-buffer if no terminator exists.
  - Does not modify `text`.
  - On success:
    - `line.buffer` points to the first byte of the line.
    - `line.offset` is 0.
    - `line.length` excludes the terminator.
    - `bytes` includes the line bytes plus the terminator bytes (if any).
  - On empty lines (for example a bare terminator), `line.length` is 0 and
    `bytes` is the terminator size.
  - On end-of-buffer with remaining bytes and no terminator, `line` covers
    the remaining bytes and `bytes` equals `line.length`.

- `readLine`
  - Wraps `getLine`.
  - On success, advances `text.offset` by `bytes`.

For more detailed semantics, see `std_overview.md`.
