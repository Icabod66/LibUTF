# File: docs/utf/variants_utf_sub_type.md

# UTF variant and sub-type model (UTF_SUB_TYPE)

## Purpose and scope

`UTF_SUB_TYPE` is the toolkit-layer selector for exact encode and decode
behavior.

It exists because "UTF-8", "UTF-16", and "UTF-32" are not single behaviors in
practice. Real-world text handling must account for:

- strict standards-compliant forms
- permissive forms used by browsers and editors
- compatibility encodings (CESU)
- modified encodings used by specific runtimes (Java modified UTF-8)
- legacy single-byte encodings (ASCII, ISO-8859-1 style 8-bit Unicode as BYTE,
  CP1252)

The toolkit layer exposes these behaviors explicitly so callers can choose a
precise policy, rather than relying on implicit defaults.

This document is a conceptual reference for the `UTF_SUB_TYPE` model used by the
UTF toolkit layer. It describes the toolkit layer only.

## UTF_TYPE vs UTF_SUB_TYPE

`UTF_TYPE` is a coarse-grained categorization, such as UTF8, UTF16, UTF32, or
OTHER. It answers the question: "Which encoding family is this?"

`UTF_SUB_TYPE` is a toolkit-layer concept that fully specifies encode and decode
behavior. It answers the question: "Exactly how do bytes map to code points, and
how are errors handled?"

A `UTF_SUB_TYPE` fixes:

- the base encoding family
- variant behavior (Java modified UTF-8, CESU compatibility schemes)
- strictness and error consumption behavior
- UCS2 or UCS4 constraints where applicable
- endianness where applicable

In the toolkit layer, a chosen `UTF_SUB_TYPE` uniquely selects an IUTFTK handler
via `IUTFTK::getHandler(UTF_SUB_TYPE)`.

## High-level model

A `UTF_SUB_TYPE` combines several orthogonal concerns.

### Base encoding family

- UTF8 family: variable-length byte sequences
- UTF16 family: 16-bit code units, possibly surrogate pairs
- UTF32 family: 32-bit code units
- Other: single-byte encodings (BYTE, ASCII, CP1252)

### Variant behavior

- UTF: standard UTF behavior for the family
- J (Java): modified UTF-8 behavior for U+0000
- CESU: compatibility encoding scheme using UTF-16 surrogate mechanics for
  supplementary planes when used with the UTF8 and UTF32 encoding families

### Strictness and error consumption

- permissive (no suffix)
- `ns` (non-skipping)
- `st` (strict, UTF8 family only)

Strictness controls how decode failures are classified and how many bytes are
consumed on failure. The definition of an invalid encoding sequence, and the
rules for byte consumption on failure, are shared across the toolkit layer; see
docs/utf/toolkit_overview.md for the core invalid-sequence model.

### Endianness

- `le` little-endian
- `be` big-endian

Endianness is always explicit for multi-byte encodings in `UTF_SUB_TYPE`. There
are no UTF16, UTF32, or CESU sub-types with implicit byte order.

A `UTF_SUB_TYPE` fully determines encode and decode behavior. Callers should not
attempt to infer behavior beyond what the sub-type specifies.

## Naming conventions

### Prefixes

- `UTF...`
  Standard UTF behavior for that code-unit size.

- `UCS...`
  Universal Character Set constraints or extensions.
  - `UCS2` restricts Unicode to the BMP (U+0000 to U+FFFF).
  - `UCS4` treats the extended UCS4 range (U+00110000 to U+7FFFFFFF) as
    standards-compliant.

- `CESU...`
  Compatibility Encoding Scheme for UTF-16.
  Uses UTF-16 surrogate pairs to represent supplementary plane code points.
  By definition, CESU variants only exist for the UTF8 and UTF32 families.

- `J...`
  Java modified UTF-8 behavior.
  Enables a Java-specific modified encoding of U+0000.

### Suffixes

- `ns`
  Non-skipping. Decode failures consume exactly one byte for byte-oriented
  encodings.

- `st`
  Strict. Applies to UTF8-family encodings. Irregular forms are treated as
  decode failures, and all decode failures consume one byte.

### Meaning of permissive (no suffix)

When no `ns` or `st` suffix is present, the handler operates in permissive mode.

Permissive behavior includes:

- acceptance of irregular but decodable forms, reported via warnings such as
  `cp_errors::bits::IrregularForm`
- UTF8-family support for extended UTF-8 encodings (more than 4 bytes) and
  overlong encodings
- surrogate code points are not automatically fatal errors
- for byte-oriented encodings, multiple adjacent illegal or invalid bytes may
  be coalesced into a single invalid sequence

Permissive does not mean "anything goes". Code-point values in the range
U+80000000 to U+FFFFFFFF are always considered invalid, even in permissive
handlers.

## UTF8-related sub-types

### UTF8, UTF8ns, UTF8st

- `UTF8`
  Permissive UTF-8 encode and decode. Supports extended and overlong encodings.
  Adjacent illegal or invalid bytes may be coalesced into a single invalid
  sequence.

- `UTF8ns`
  Non-skipping UTF-8 decode. Each decode failure consumes exactly one byte.
  This preserves alignment and simplifies diagnostics and repair.

- `UTF8st`
  Strict UTF-8 decode. Irregular forms are treated as decode failures, and all
  decode failures consume one byte. This behavior matches the "malformed
  sequence as single bytes" interpretation used by many browsers and editors,
  and passes Markus Kuhn's decoder stress test under that variation.

### Java modified UTF-8 (JUTF8)

`JUTF8`, `JUTF8ns`, and `JUTF8st` are Java-style modified UTF-8 variants.

The defining rule is Java-specific:

- U+0000 (NULL) may be encoded as a 2-byte overlong UTF-8 sequence.

In toolkit-layer diagnostics:

- Java-style modified NULL encodings are flagged with
  `cp_errors::bits::ModifiedUTF8` (warning).
- Other overlong encodings are flagged with
  `cp_errors::bits::OverlongUTF8` (warning).

These two flags are mutually exclusive and must both be checked to detect all
overlong encodings.

For details on overlong handling and indexing, see docs/utf/overlong_utf8.md.

### CESU8 and JCESU8

CESU encodings apply to the UTF8 and UTF32 families.

CESU-8 represents supplementary plane code points
(U+00010000 to U+0010FFFF) using UTF-16 surrogate pairs, then encodes each
surrogate separately as UTF-8. The resulting byte stream is not standard UTF-8
for supplementary characters, but is useful for compatibility with systems
built around UTF-16 code units.

- `CESU8`, `CESU8ns`, `CESU8st`
  CESU-8 behavior with permissive, non-skipping, and strict handling.

- `JCESU8`, `JCESU8ns`, `JCESU8st`
  CESU-8 combined with Java modified UTF-8 NULL behavior.

### Error coalescing vs non-skipping in UTF8

When decoding UTF8-family encodings, error handling operates at the level of a
code-point decode attempt.

If a decode attempt starts at a given byte and any byte that should form part
of that code point is illegal or unexpected at its position, then the entire
attempted sequence is considered invalid. This definition matches the invalid
sequence model described in docs/utf/toolkit_overview.md.

- In permissive variants, multiple adjacent invalid bytes may be coalesced
  into a single invalid sequence.
- In `ns` and `st` variants, decode failures always consume exactly one byte.

This distinction affects code-point counting and diagnostics but does not
change which bytes are considered invalid.

## UTF16 and UTF32 sub-types

### UTF16 vs UCS2

Both use 16-bit code units.

- `UTF16le` / `UTF16be`
  Standard UTF-16 behavior, including surrogate pairs for supplementary planes.

- `UCS2le` / `UCS2be`
  UCS2 restricts Unicode to the BMP (U+0000 to U+FFFF). Surrogate pairs are
  disabled.

Choose UCS2 variants when supplementary plane characters must be rejected or
treated as invalid.

### UTF32 vs UCS4

Both use 32-bit code units.

- `UTF32le` / `UTF32be`
  Standard UTF-32 behavior.

- `UCS4le` / `UCS4be`
  UCS4 treats the extended UCS4 range (U+00110000 to U+7FFFFFFF) as
  standards-compliant.

Even in permissive modes, values in the range U+80000000 to U+FFFFFFFF are
always invalid and will be reported as such.

### CESU32 and CESU4

CESU variants also exist for the UTF32 family.

- `CESU32le` / `CESU32be`
  CESU behavior layered onto UTF32.

- `CESU4le` / `CESU4be`
  CESU behavior layered onto UCS4.

These are compatibility formats intended for specialized interoperability and
round-tripping of surrogate-based representations.

## Non-UTF sub-types

### BYTE and BYTEns

- `BYTE`
  Single-byte encoding where all byte values are representable. When ASCII
  restrictions are applied, high-bit bytes are treated as illegal. Adjacent
  illegal or invalid bytes may be coalesced.

- `BYTEns`
  Non-skipping variant. Each decode failure consumes exactly one byte.

### ASCII and ASCIIns

ASCII is treated as a 7-bit encoding, even though stored as bytes.

- `ASCII`
  Bytes with the top bit set are illegal. Failures may be coalesced.

- `ASCIIns`
  Non-skipping ASCII. Each illegal byte is reported individually.

### CP1252 variants

- `CP1252`
  Permissive Windows Code Page 1252 behavior.

- `CP1252ns`
  Non-skipping CP1252.

- `CP1252st`
  Strict CP1252 mapping. Rejects code points or bytes not encodable or
  decodable under the strict mapping.

For single-byte encodings, `st` controls mapping strictness, while `ns`
controls error coalescing. They are independent concerns.

## How to choose a UTF_SUB_TYPE

### Validating input

- Use `UTF8st` or `JUTF8st` for strong validation with browser-like behavior.
- Use `UTF8ns` or `JUTF8ns` when alignment and precise diagnostics matter.
- Use UCS2 or UCS4 variants when the valid Unicode range must be constrained or
  extended explicitly.

Consider calling `IUTFTK::validate` on untrusted input.

### Matching browser or editor behavior

- `UTF8st` and `JUTF8st` produce results similar to common browsers and editors
  and pass Markus Kuhn's decoder stress test using the single-byte failure
  variation.

### Handling legacy data

- Use `CP1252` or `CP1252ns` for Windows-era text.
- Use `ASCII` or `ASCIIns` when high-bit bytes must be rejected.
- Use `BYTE` when raw byte round-tripping is required.

### Diagnostics and repair

For diagnostics and repair of corrupted data, predictable alignment is
critical.

- Prefer `ns` variants for byte-oriented encodings.
- Non-skipping ensures each bad byte is surfaced individually.

When decode fails, the toolkit layer may return the first byte of the attempted
sequence as the reported code-point value, preserving information needed for
analysis and repair. This is a diagnostic behavior; see
docs/utf/toolkit_overview.md for the core decode semantics.

## Examples

Example 1: selecting a handler by UTF_SUB_TYPE

    using namespace unicode::utf::toolkit;

    const IUTFTK& h = IUTFTK::getHandler(UTF_SUB_TYPE::UTF8st);

    utf_text t = ...;
    unicode_t cp = 0;
    uint32_t bytes = 0;

    cp_errors e = h.get(t, cp, bytes);
    if (e.failed())
    {
        // handle error
    }

Example 2: switching between strict and permissive variants

    using namespace unicode::utf::toolkit;

    UTF_SUB_TYPE sub = UTF_SUB_TYPE::UTF8;
    if (need_strict_validation)
        sub = UTF_SUB_TYPE::UTF8st;

    const IUTFTK& h = IUTFTK::getHandler(sub);

    cp_errors e = h.validate(input_text);
    if (e.error())
    {
        // reject input
    }

Example 3: diagnostics vs production decoding

    using namespace unicode::utf::toolkit;

    const IUTFTK& diag = IUTFTK::getHandler(UTF_SUB_TYPE::UTF8ns);
    const IUTFTK& prod = IUTFTK::getHandler(UTF_SUB_TYPE::UTF8st);

    // diagnostics pass
    for (;;)
    {
        unicode_t cp = 0;
        cp_errors e = diag.read(text, cp);
        if (e.any(cp_errors::bits::ReadExhausted))
            break;

        if (e.failed())
        {
            // record precise failure information
        }
    }

    // production validation
    if (prod.validate(text).no_error())
    {
        // safe to process
    }

## Cross-links

- docs/utf/toolkit_overview.md
- docs/utf/cp_errors.md
- docs/utf/overlong_utf8.md
