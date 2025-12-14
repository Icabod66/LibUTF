# File: docs/utf/toolkit_overview.md

# LibUTF toolkit layer overview (utf_toolkit.h)

The LibUTF toolkit layer (`unicode::utf::toolkit`) provides robust,
configurable UTF code-point processing for real-world data. It is designed
for low-level use: allocation-free, exception-free, and explicit about buffer
boundaries.

Where the std layer focuses on simple, strict, and common UTF handling, the
toolkit layer is intended for diagnostics, interoperability, and controlled
handling of non-standard or malformed data.

The toolkit is most commonly used via its handler interface (`IUTFTK`),
configured with a specific encoding and behavior model using `UTF_SUB_TYPE`.
Direct function calls are available for advanced use cases where finer-grained
control is required.

See also:
- docs/utf/handlers_usage.md
- docs/utf/cp_errors.md
- docs/utf/variants_utf_sub_type.md
- docs/utf/overlong_utf8.md

## When to use the toolkit layer

Use the toolkit layer when you need one or more of the following:

- Detailed error and warning reporting during encode and decode.
- Explicit control over permissive, non-skipping, or strict decoding behavior.
- Support for compatibility encodings (CESU forms, modified UTF-8).
- Controlled handling of malformed, truncated, or corrupted data.
- Buffer-walking utilities that remain usable even on invalid input.
- Overlong UTF-8 indexing for intentional private signaling.

For simple, standards-first UTF handling without detailed diagnostics, prefer
the std layer (`unicode::utf::std`).

## Std layer vs toolkit layer in practical terms

The std layer (`unicode::utf::std`) provides quick, convenience-oriented
functions such as `getUTF8`, `setUTF8`, and `strlenUTF8`, along with the `IUTF`
handler interface. These APIs are intentionally strict and compact:

- Operations succeed or fail with minimal diagnostic detail.
- Non-decodable or non-compliant sequences typically cause failure.
- The API surface is optimized for common, well-formed input.

The toolkit layer is designed differently:

- Encode and decode functions return `cp_errors`, a structured bitset
  describing both errors and warnings.
- Operations are centered around `utf_text`, which tracks buffer, length, and
  byte offset explicitly.
- Multiple encoding sub-types are supported for each encoding family.
- Invalid or irregular input can often be processed in a controlled way,
  allowing scanning, diagnostics, or recovery.
- Forward and backward stepping operates in code points, even on malformed
  data.
- Overlong UTF-8 encodings are explicitly supported and indexable.

In typical usage, applications select an encoding model by choosing a
`UTF_SUB_TYPE` and then use an `IUTFTK` handler for most operations. Direct
function calls are primarily used when per-call policy control is required or
when building higher-level parsers and scanners.

## Core data model: utf_text

Both layers use `unicode::utf::utf_text` to represent an encoded text stream:

- `buffer` points to the byte buffer (may be `nullptr`).
- `length` is the buffer size in bytes.
- `offset` is the current byte offset into the buffer.

Toolkit functions treat `utf_text` as a stream cursor. Reads and writes operate
at `buffer + offset`, and the number of bytes consumed or produced is reported
separately (usually via a `bytes` output parameter).

Misaligned offsets or lengths are treated as precondition violations. When such
a condition is detected, the operation fails and the returned `cp_errors` value
has the `Failed` bit set together with the specific diagnostic reason (for
example `MisalignedOffset` or `MisalignedLength`).

These conditions indicate incorrect use of the API rather than malformed input
data, but are still reported diagnostically via `cp_errors` to allow callers to
detect, log, or debug the failure explicitly.

Alignment checks apply to the `offset` and `length` values only and are defined
relative to the start of the byte stream represented by `utf_text.buffer`. The
toolkit does not validate or enforce the physical alignment of the buffer
pointer itself.

This allows advanced callers to deliberately scan data starting at arbitrary
memory addresses by adjusting the buffer pointer, while still maintaining
correct logical alignment within the abstract byte stream.

Validation helpers allow detection of structural issues such as:

- Null buffer pointers.
- Offsets beyond the buffer length.
- Misalignment for encodings with multi-byte code units.

All offsets and lengths are expressed in bytes, even for UTF-16 and UTF-32.

## Encoding models and UTF_SUB_TYPE

`UTF_SUB_TYPE` describes a complete encoding and behavior model. It combines
the encoding family with specific policy choices, including permissiveness,
strictness, endianness, and compatibility modes.

Examples of supported models include:

- UTF-8 variants:
  - Permissive, non-skipping, and strict forms.
  - Modified UTF-8 (Java-style NULL handling).
  - CESU-8 compatibility encodings.
- UTF-16 variants:
  - Big-endian and little-endian.
  - UCS-2 restricted forms (BMP only, no surrogate pairs).
- UTF-32 variants:
  - Big-endian and little-endian.
  - UCS-4 extended range support.
  - CESU compatibility forms.
- Single-byte encodings:
  - ISO-8859-1 style byte encoding.
  - ASCII (7-bit).
  - Windows Code Page 1252, with permissive, non-skipping, and strict modes.

In most applications, users select a `UTF_SUB_TYPE` and obtain an `IUTFTK`
handler. That handler encapsulates the encoding rules and behavior, allowing
uniform use across encode, decode, stepping, and validation operations.

For detailed discussion of sub-types, see
docs/utf/variants_utf_sub_type.md.

## cp_errors: errors and warnings

Toolkit encode and decode operations return a `cp_errors` value. This type
records what occurred during the operation and distinguishes between errors
and warnings.

- Errors indicate that the requested operation could not be performed
  correctly (for example: invalid buffer, write overflow, not encodable,
  truncated read, disallowed or unexpected bytes).
- Warnings indicate unusual or non-ideal conditions that were still
  processable (for example: non-characters, surrogate usage, overlong UTF-8,
  irregular but decodable forms).

Key properties:

- Errors and warnings are independent; a result may contain both.
- `failed()` and `error()` allow quick detection of failure conditions.
- Filtering helpers allow extraction of only errors or only warnings.

Some warning and error combinations indicate that a replacement character may
be appropriate. The helper `use_replacement_character()` identifies those
cases, leaving the final policy decision to the caller. How this guidance is
applied depends on the selected strictness and behavior model; see
docs/utf/variants_utf_sub_type.md for strict versus permissive decoding rules.

For recommended handling strategies, see docs/utf/cp_errors.md.

## Control flags and behavior tuning

Low-level toolkit functions expose explicit control flags to fine-tune
behavior. Common flags include:

- `use_ascii`:
  - Restricts byte decoding to 7-bit values.
- `use_ucs2`:
  - Disables surrogate pairs and restricts Unicode to the BMP.
- `use_ucs4`:
  - Enables the extended UCS-4 range beyond U+10FFFF.
- `use_cesu`:
  - Enables surrogate-based compatibility encodings.
- `use_java`:
  - Enables modified UTF-8 behavior, including 2-byte NULL encoding.
- `strict`:
  - Enforces strict UTF-8 rules and treats irregular forms as failures.
- `coalesce`:
  - Coalesces runs of illegal or invalid bytes into a single invalid sequence.
- `le`:
  - Selects little-endian behavior for multi-byte code units.

These flags are primarily intended for advanced usage. Most applications rely
on `UTF_SUB_TYPE` selection via `IUTFTK` rather than passing flags manually.

## Invalid sequences and decode semantics

An invalid encoding sequence is not defined by a single offending byte alone.
If a multi-byte character encoding contains an invalid or mispositioned byte,
the entire attempted character sequence is invalid because it does not form a
complete, valid code point.

As a result:

- All bytes consumed as part of the attempted sequence are considered part of
  the invalid encoding.
- Decode functions may report the lead byte of the sequence as the returned
  code point value.
- Stepping behavior depends on strictness and coalescing policy.

The returned code-point value on failure is a diagnostic artifact, not a valid
decoded Unicode value. The associated `cp_errors` flags describe the failure
mode in detail. Exact behavior in strict versus permissive decoding is defined
by the selected `UTF_SUB_TYPE`; see docs/utf/variants_utf_sub_type.md.

This behavior allows predictable progress through malformed data while still
preserving diagnostic information.

## Step and back semantics

The toolkit provides forward and backward stepping functions for all supported
encodings.

Important properties:

- Stepping operates in code points, not bytes.
- The return value is the number of code points successfully skipped.
- The underlying byte offset is adjusted according to the encoding and
  encountered sequences.

In permissive or coalescing modes, a single invalid code point may correspond
to multiple bytes. In strict or non-skipping modes, invalid sequences may
advance by only one byte per code point. The definition of invalid sequences
is shared with the decode semantics described above.

## BOM and NULL helpers

The toolkit provides stream-oriented helpers for writing byte order markers
and NULL code points:

- UTF-8, UTF-16, and UTF-32 BOM encoding.
- UTF-8, UTF-16, and UTF-32 NULL encoding.

These helpers operate on `utf_text` and return `cp_errors`, making them suitable
for sequential output pipelines.

## Overlong UTF-8 support and indexing

The toolkit explicitly supports detection and construction of overlong UTF-8
encodings:

- Identification of overlong encodings.
- Mapping between overlong encodings and a numeric index.
- Construction of overlong encodings from an index.

Index 0 corresponds to the Java-style 2-byte NULL encoding. Other indices are
available for private or experimental signaling. Strict decoders are expected
to reject or ignore overlong forms.

For details, see docs/utf/overlong_utf8.md.

## Common use cases

### Diagnostics and validation

When processing buffers of unknown provenance, the toolkit allows you to:

- Distinguish structural buffer errors from encoding errors.
- Detect truncation versus clean end-of-buffer.
- Record detailed information about malformed sequences.

This makes it possible to scan hostile or damaged input without losing
progress.

### Legacy and corrupted data handling

Real-world data may contain malformed sequences due to file corruption,
partial writes, truncation, or transmission errors. The toolkit provides
controlled ways to tolerate and diagnose such data without undefined behavior.

### Transcoding with explicit policy

The toolkit enables decoding with one policy and re-encoding with another.
For example, permissive input decoding combined with strict output encoding,
or UCS-2 restricted output for legacy systems.

### Intentional overlong output

Although overlong UTF-8 is not suitable for standard interchange, the toolkit
supports intentional use for private signaling or specialized compatibility
scenarios.

## Handlers versus free functions

Use `IUTFTK` handlers when:

- A consistent encoding and behavior model applies across many operations.
- You want a uniform interface for encode, decode, stepping, and validation.
- The encoding policy can be expressed as a `UTF_SUB_TYPE`.

Use free functions when:

- Per-call behavior must vary.
- Custom or experimental encodings are required.
- Building higher-level parsers or scanners that need precise control over
  error handling and progression.

In most applications, handlers are the primary interface, with free functions
reserved for advanced or specialized scenarios.

## Safety and validation considerations

The toolkit is designed for low-level, defensive use, but callers must still
apply care:

- Validate `utf_text` before assuming alignment or available space.
- Distinguish end-of-buffer conditions from truncation errors.
- Do not assume bytes were written unless no error is reported.
- Decide explicitly how warnings and replacement-character conditions are
  handled downstream.

The toolkit provides the information needed for safe handling; policy decisions
remain with the caller.
