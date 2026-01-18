# File: docs/reference/unicode_api.md

# SuiteUTF Unicode basics API reference

This document describes the public API of the Unicode basics layer as declared in:

- unicode_type.h
- unicode_classification.h
- unicode_utilities.h

The content reflects only the semantics implied by the headers and the known implementation.  
All functions documented here are noexcept.


## Core types

### unicode_t

    typedef int32_t unicode_t;

Fundamental code-point type used throughout the library.

- Represents Unicode scalar values and other code points.
- Serves as the input/output type for all classification and utility functions.


## Classification API (unicode_classification.h)

All classification functions follow the signature:

    bool function(const unicode_t unicode) noexcept;

They return true if the code point belongs to the category, otherwise false.


### General Unicode classification

    bool isBOM(const unicode_t unicode) noexcept;
    bool isUnicode(const unicode_t unicode) noexcept;
    bool isCharacter(const unicode_t unicode) noexcept;
    bool isNonCharacter(const unicode_t unicode) noexcept;
    bool isCombining(const unicode_t unicode) noexcept;
    bool isPrivateUse(const unicode_t unicode) noexcept;
    bool isSpecial(const unicode_t unicode) noexcept;
    bool isSurrogate(const unicode_t unicode) noexcept;
    bool isHighSurrogate(const unicode_t unicode) noexcept;
    bool isLowSurrogate(const unicode_t unicode) noexcept;
    bool isC0(const unicode_t unicode) noexcept;
    bool isC1(const unicode_t unicode) noexcept;
    bool isCC(const unicode_t unicode) noexcept;
    bool isBreakingWhite(const unicode_t unicode) noexcept;
    bool isTrivialWhite(const unicode_t unicode) noexcept;

Notes:

- Functions such as isBreakingWhite never return true for invalid Unicode
  values, so users do not need to call isUnicode beforehand.

Example (simple usage):

    if (unicode::isBreakingWhite(ch))
    {
        // treat as a breaking whitespace boundary
    }


### ASCII classification

    bool isAsciiCC(const unicode_t unicode) noexcept;
    bool isAsciiText(const unicode_t unicode) noexcept;
    bool isAsciiWhite(const unicode_t unicode) noexcept;
    bool isAsciiBlack(const unicode_t unicode) noexcept;
    bool isStrictAsciiText(const unicode_t unicode) noexcept;
    bool isStrictAsciiWhite(const unicode_t unicode) noexcept;

These functions classify ASCII subsets.

- It is safe to call any of them on any unicode_t; none will incorrectly
  classify an invalid or non-ASCII value as ASCII.
- A common usage pattern is to check isAsciiText() first when operating in an
  ASCII-only processing context.

Example (common contextual usage):

    if (unicode::isAsciiText(ch))
    {
        ...
        if (!unicode::isAsciiWhite(ch))
        {
            // printable non-whitespace ASCII
        }
        ...
    }


### XML classification

    bool isNameStartXML(const unicode_t unicode) noexcept;
    bool isNameExtraXML(const unicode_t unicode) noexcept;
    bool isNameXML(const unicode_t unicode) noexcept;
    bool isPostNameXML(const unicode_t unicode) noexcept;
    bool isWhiteXML(const unicode_t unicode) noexcept;
    bool isCleanXML(const unicode_t unicode) noexcept;

Example:

    bool isValidXmlNameChar(unicode::unicode_t ch, bool is_first)
    {
        return is_first
            ? unicode::isNameStartXML(ch)
            : unicode::isNameXML(ch);
    }


### JSON classification

    bool isWhiteJSON(const unicode_t unicode) noexcept;
    bool isHexEscapedJSON(const unicode_t unicode) noexcept;

Example:

    if (unicode::isHexEscapedJSON(ch))
    {
        // must output \\uXXXX escape in JSON
    }


## Utility API (unicode_utilities.h)

### Hexadecimal helpers

    int32_t   unicodeToHex(const unicode_t unicode) noexcept;
    unicode_t hexToLowerUnicode(const int32_t hex) noexcept;
    unicode_t hexToUpperUnicode(const int32_t hex) noexcept;

- unicodeToHex
  - Returns 0 to 15 if unicode is '0'-'9', 'a'-'f', or 'A'-'F'.
  - Returns -1 otherwise.

- hexToLowerUnicode / hexToUpperUnicode
  - Convert a 4-bit integer (0x0-0xf) to the corresponding hex digit.
  - Values outside this domain result in implementation-defined behavior.

Example:

    int32_t v = unicode::unicodeToHex('E');   // 14
    unicode::unicode_t lo = unicode::hexToLowerUnicode(v); // 'e'
    unicode::unicode_t up = unicode::hexToUpperUnicode(v); // 'E'


### General short escape helpers

    unicode_t toShortEscape(const unicode_t unicode) noexcept;
    unicode_t fromShortEscape(const unicode_t unicode) noexcept;

These implement the library's complete short escape map:

- 0x0007 (BEL)        -> 'a'
- 0x0008 (BS)         -> 'b'
- 0x0009 (TAB)        -> 't'
- 0x000A (LF)         -> 'n'
- 0x000B (VT)         -> 'v'
- 0x000C (FF)         -> 'f'
- 0x000D (CR)         -> 'r'
- 0x0022 (")          -> '"'
- 0x0027 (')          -> "'"
- 0x002F (/)          -> '/'
- 0x003F (?)          -> '?'
- 0x005C (\)          -> '\\'

All others -> -1.

fromShortEscape is the inverse mapping for the supported escape characters.


### JSON short escape helpers

    unicode_t toShortEscapeJSON(const unicode_t unicode) noexcept;
    unicode_t fromShortEscapeJSON(const unicode_t unicode) noexcept;

JSON defines a restricted escape set:

- 0x0008 (BS)         -> 'b'
- 0x0009 (TAB)        -> 't'
- 0x000A (LF)         -> 'n'
- 0x000C (FF)         -> 'f'
- 0x000D (CR)         -> 'r'
- 0x0022 (")          -> '"'
- 0x002F (/)          -> '/'
- 0x005C (\)          -> '\\'

All others -> -1.

fromShortEscapeJSON is the inverse map.


## CP1252 transcoding

These functions convert between Windows code page 1252 and Unicode.  
They are the only consumers of CP1252Strictness.


### CP1252Strictness

    enum class CP1252Strictness : uint8_t
    {
        WindowsCompatible = 0,
        StrictUndefined
    };

Controls strictness of CP1252 conversions.

- WindowsCompatible
  - All 256 CP1252 bytes map to Unicode.
  - Round-trip conversions always succeed.
  - cp1252ToUnicode never returns false.
- StrictUndefined
  - Rejects 5 undefined C1 control codes:
    U+0081, U+008D, U+008F, U+0090, U+009D.
  - Both cp1252ToUnicode and unicodeToCP1252 return false for these.


### cp1252ToUnicode(...)

    bool cp1252ToUnicode(
        const uint8_t cp1252,
        unicode_t& unicode,
        const CP1252Strictness strictness = CP1252Strictness::WindowsCompatible) noexcept;

Parameters:

- cp1252 - CP1252 byte.
- unicode - receives the mapped Unicode code point.
- strictness - see above.

Behavior:

- WindowsCompatible: always returns true.
- StrictUndefined: returns false only for the 5 undefined C1 codes.

Example:

    uint8_t b = 0x81;
    unicode::unicode_t u = 0;
    bool ok = unicode::cp1252ToUnicode(
        b, u, unicode::CP1252Strictness::StrictUndefined );
    // ok == false


### unicodeToCP1252(...)

    bool unicodeToCP1252(
        const unicode_t unicode,
        uint8_t& cp1252,
        const CP1252Strictness strictness = CP1252Strictness::WindowsCompatible) noexcept;

Parameters:

- unicode - input code point.
- cp1252 - output byte.
- strictness - as above.

Behavior:

- Many Unicode values are not representable in CP1252.
- StrictUndefined mode additionally rejects the 5 undefined C1 code points.

Example:

    unicode::unicode_t ch = 0x00A3; // POUND SIGN
    uint8_t out = 0;
    if (unicode::unicodeToCP1252(ch, out))
    {
        // CP1252 mapping succeeded
    }
    else
    {
        // code point is not representable or was rejected
    }
