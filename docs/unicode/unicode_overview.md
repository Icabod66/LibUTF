# File: docs/unicode/unicode_overview.md

The Unicode basics layer provides the foundational types, classification helpers,
and small utility functions that underpin general Unicode-aware logic throughout
the codebase. It offers simple, allocation-free, exception-free building blocks
for code-point analysis, ASCII/XML/JSON rules, hexadecimal and escape handling,
and limited 8-bit transcoding.

Unlike the UTF encoding layers, this component does not perform UTF
encode/decode. Instead, it focuses on reasoning about Unicode code points
(unicode_t) and providing semantic tools that are broadly useful in parsers,
validators, and higher-level utilities.

This layer is conceptually foundational, but it is not the core implementation
layer for UTF encoding and decoding. The UTF layers implement their own
specialized logic tailored to their correctness and performance requirements.


## Core type: unicode_t

unicode_t is defined as a 32-bit signed integer:

    typedef int32_t unicode_t;

This type is used throughout the library and by external utilities to represent:

- Unicode scalar values.
- Surrogate and special code points.
- The input/output domain of classification and utility functions.

In the Unicode basics layer, unicode_t represents only Unicode-related values.
Special error signalling via unicode_t values is defined by the utf_std layer
and does not apply to the functions described in this document.


## Classification functions

The classification API (unicode_classification.h) provides a comprehensive set
of predicates for identifying the role, category, or validity of a Unicode code
point. All functions take a unicode_t, return bool, and are noexcept.

### General Unicode classification

These functions operate across the full Unicode range:

- isUnicode  
  Whether the value falls within the valid Unicode range.

- isCharacter, isNonCharacter  
  Distinguish valid scalar characters from the explicitly reserved
  non-character ranges.

- isCombining  
  Identify combining marks.

- isPrivateUse  
  Check whether the code point lies in one of the Private Use Areas.

- isSpecial  
  Detect special-purpose values that are not standard characters.

- isSurrogate, isHighSurrogate, isLowSurrogate  
  Recognise UTF-16 surrogate code points.

- isC0, isC1, isCC  
  Detect control characters from the C0, C1, or combined control sets.

- isBreakingWhite, isTrivialWhite  
  Identify whitespace classes relevant to layout and tokenisation.

These helpers are frequently used by lexical analysers, validators, and other
tools that reason about Unicode properties independently of any encoding.

### ASCII classification

ASCII-focused helpers are useful for lightweight filtering and validation:

- isAsciiCC  
  ASCII control characters.

- isAsciiText / isStrictAsciiText  
  Printable ASCII ranges, with or without VT/FF.

- isAsciiWhite / isStrictAsciiWhite  
  Standard vs. strict ASCII whitespace.

- isAsciiBlack  
  Printable non-whitespace ASCII characters.

These predicates avoid the need for any larger Unicode table when working
with ASCII-dominant formats.

### XML classification

XML defines strict character-set rules. The helpers include:

- isNameStartXML, isNameExtraXML, isNameXML  
  Classification for allowed XML name characters.

- isPostNameXML  
  Characters allowed immediately after a name.

- isWhiteXML  
  XML-defined whitespace.

- isCleanXML  
  Characters permitted by XML and not in the discouraged set.

These functions allow XML parsers to precisely match XML's permitted character
sets without embedding custom tables.

### JSON classification

JSON's character requirements are simple but important:

- isWhiteJSON  
  Whitespace defined by RFC 7159.

- isHexEscapedJSON  
  Characters that JSON requires to be represented as \uXXXX escapes.

These checks support JSON emitters and validators that follow the standard's
strict formatting rules.


## Utility functions

The utility functions (unicode_utilities.h) provide general-purpose helpers for
conversions, escape handling, and CP1252 transcoding.

### Hexadecimal conversion

- unicodeToHex  
  Convert a Unicode code point representing a hexadecimal digit to its numeric
  value (0-15), or return -1 if not valid.

- hexToLowerUnicode / hexToUpperUnicode  
  Convert a 4-bit integer (0x0-0xf) into a lowercase or uppercase hexadecimal
  Unicode character.

### Escape translation

- toShortEscape / fromShortEscape  
  Map between Unicode code points and standard short escape characters.

- toShortEscapeJSON / fromShortEscapeJSON  
  JSON-restricted versions following the smaller escape set permitted by the
  JSON specification.

### CP1252 transcoding

CP1252 support is provided for handling Windows-compatible text:

- cp1252ToUnicode  
  Convert a CP1252 byte to a Unicode code point.

- unicodeToCP1252  
  Attempt the reverse conversion, returning false if the value is not
  representable.


## Relationship to the utf_std and utf_toolkit layers

The Unicode basics layer is largely independent of the UTF encoding layers.

The utf_std and utf_toolkit layers implement their own optimized validation and
classification logic to ensure predictable behavior, precise error handling,
and minimal overhead during encoding and decoding.

Only the CP1252 transcoding helpers are reused directly by these layers, as
CP1252 is an external encoding that benefits from a shared implementation.


## When to use these functions directly

Direct use of the Unicode basics layer is appropriate when you need:

- Fast, allocation-free classification of code points.
- Rule-based validation for ASCII, XML, or JSON text.
- Escape processing for text serialization or parsing.
- CP1252 interoperability.
- A semantic layer independent of UTF encoding details.

For UTF encoding, decoding, and buffer-level processing, use the utf_std or
utf_toolkit layers and their handler interfaces.
