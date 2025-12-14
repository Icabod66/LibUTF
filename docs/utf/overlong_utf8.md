# File: docs/utf/overlong_utf8.md

# Overlong UTF-8 in LibUTF (toolkit)

## Purpose and scope

An "overlong UTF-8 encoding" is a byte sequence that decodes to a unicode
code-point value that could have been represented using fewer UTF-8 bytes.

Example idea (not exact bytes here):
- U+0041 ('A') normally fits in 1 byte.
- An overlong form encodes the same value using 2 or more bytes.

LibUTF explicitly supports detecting and intentionally emitting overlong
UTF-8 forms as part of its toolkit goals:
- diagnostics and forensic analysis of malformed or non-conformant data
- controlled repair and transcoding workflows
- optional private signaling in buffers where raw bytes matter

This is a toolkit-only feature. Overlong UTF-8 handling is not exposed by the
std layer and is never required for standards-compliant interchange.

The toolkit API exposes:
- detection helpers (`isOverlongUTF8`, `overlongToIndexUTF8`)
- an indexing model for the overlong space
- explicit-length emission (`encodeUTF8n`, `indexToOverlongUTF8`)

Strict decoders are expected to reject irregular forms, including overlong
forms, when strict decoding is selected. How rejection is reported and how
byte consumption proceeds is defined by the selected `UTF_SUB_TYPE`; see
docs/utf/toolkit_overview.md and docs/utf/variants_utf_sub_type.md.

Notes:
- LibUTF distinguishes Java-style modified UTF-8 (the 2-byte encoding of
  U+0000) from other overlong forms.
- The flags `cp_errors::bits::ModifiedUTF8` and
  `cp_errors::bits::OverlongUTF8` are mutually exclusive. Both must be
  checked to detect all overlong-related cases.

Cross-links:
- docs/utf/toolkit_overview.md
- docs/utf/variants_utf_sub_type.md
- docs/utf/cp_errors.md


## Overlong encodings in LibUTF

LibUTF defines "overlong" in practical, encoding-centric terms.

Given:
- `bytes`: the UTF-8 length being used (1 to 6 supported by the toolkit)
- `unicode`: the decoded code-point value

An encoding is considered overlong when:
- `bytes > 1`, and
- the decoded value fits in fewer bits than are available for that `bytes`
  count under LibUTF's UTF-8 bit-capacity model.

This model is shared across helpers and encode/decode paths:
- `bitCountUTF8(bytes)` computes the number of value bits available for a
  UTF-8 sequence of length `bytes`.
- Overlong detection checks whether the decoded value could have been
  represented using a shorter encoding length.

Special case: Java-style NULL (modified UTF-8)
- If `unicode == 0` and `bytes == 2`, LibUTF flags this as
  `cp_errors::bits::ModifiedUTF8`.
- All other cases where the value is too small for the chosen `bytes`
  length are flagged as `cp_errors::bits::OverlongUTF8`.

On the encode path (`encodeUTF8n`):
- Encoding U+0000 in 2 bytes sets `ModifiedUTF8`.
- Encoding any value using more bytes than required sets
  `OverlongUTF8 | IrregularForm`.
- Modified UTF-8 is treated as irregular unless Java-style behavior is
  explicitly enabled.

In summary:
- "modified UTF-8" is the specific 2-byte encoding of U+0000.
- "overlong UTF-8" covers all other longer-than-minimal encodings.
- Both are classified as irregular forms unless explicitly permitted by
  the selected encoding mode.


## Detection

### isOverlongUTF8(unicode, bytes)

`isOverlongUTF8(unicode, bytes)` is a fast predicate that reports whether a
given `(unicode, bytes)` pair is overlong under LibUTF's bit-capacity model.

Important points:
- The helper tests only the size relationship between the value and the
  specified encoding length.
- It does not special-case Java-style NULL. If that distinction matters,
  callers should explicitly check `(unicode == 0 && bytes == 2)` or rely on
  decode-time flags.

The valid `bytes` range for this helper is 2 to 6.


### Decode-time behavior

Overlong detection during decoding is an internal part of `decodeUTF8`.

Implementation note:
- `decodeUTF8` performs a low-level UTF-8 parse using an internal helper
  that classifies lead bytes, gathers continuation bytes, and detects
  overlong and modified forms.
- This helper is not part of the public API and is documented here only to
  explain the observable behavior of `decodeUTF8`.

During decoding:
- If a multi-byte sequence decodes to a value that fits in fewer bits than
  the encoding length allows, the sequence is flagged as:
  - `ModifiedUTF8` if it is a 2-byte encoding of U+0000
  - otherwise `OverlongUTF8`
- Encodings using 5- or 6-byte forms are flagged with
  `cp_errors::bits::ExtendedUTF8` during decoding, but they otherwise
  participate in the same overlong detection and indexing mechanisms as
  shorter forms.

After the initial parse, `decodeUTF8` applies policy rules:
- Overlong, modified, extended-length, and surrogate-related conditions
  contribute to `cp_errors::bits::IrregularForm`.
- If Java-style behavior is enabled, `ModifiedUTF8` does not make the form
  irregular. Otherwise it does.

The resulting `cp_errors` state determines whether decoding proceeds,
whether a replacement character is suggested, and how many bytes are
consumed on failure. See docs/utf/toolkit_overview.md and
docs/utf/cp_errors.md for the shared decode-failure semantics.


## Indexing model

LibUTF defines an "overlong index" to provide a compact and stable identifier
for overlong UTF-8 encodings.

The index maps a `(unicode, bytes)` pair to a unique integer using
`overlongToIndexUTF8`, and can be inverted using `indexToOverlongUTF8`.

### Conceptual model

LibUTF defines overlong encodings using a generalized UTF-8 model based on
lead-byte classification, continuation-byte structure, and the number of
value bits available for a given encoding length. This model is applied
uniformly to 2- through 6-byte forms, including extended 5- and 6-byte
encodings that follow the same structural rules as UTF-8, even though they
are not part of modern UTF-8 standards.

Encodings that use 5- or 6-byte forms are flagged with
`cp_errors::bits::ExtendedUTF8` during decoding, but they otherwise
participate in the same overlong detection, indexing, and emission
mechanisms as shorter forms.

Support for these extended forms exists for diagnostics, classification,
and controlled emission in the toolkit layer. They are not considered valid
Unicode interchange encodings.

The overlong index space is therefore a complete enumeration of all
`(unicode, bytes)` pairs that fit within this generalized model while using
a longer-than-minimal encoding length. The index reflects LibUTF's internal
representation and diagnostic needs, rather than any external conformance
criteria.

### Indexed ranges

The indexed ranges are grouped by encoding length:

- bytes == 2:
  - unicode in [0x00000000, 0x0000007f]
  - index = unicode
  - count = 0x00000080

- bytes == 3:
  - unicode in [0x00000000, 0x000007ff]
  - index = unicode + 0x00000080
  - count = 0x00000800

- bytes == 4:
  - unicode in [0x00000000, 0x0000ffff]
  - index = unicode + 0x00000880
  - count = 0x00010000

- bytes == 5:
  - unicode in [0x00000000, 0x001fffff]
  - index = unicode + 0x00010880
  - count = 0x00200000

- bytes == 6:
  - unicode in [0x00000000, 0x03ffffff]
  - index = unicode + 0x00210880
  - count = 0x04000000

The total indexed overlong space contains 0x04210880 entries, allowing a
maximum encodable overlong index of 0x0421087f.

### Meaningful vs reserved indices

- Index 0 corresponds to the Java-style 2-byte encoding of U+0000.
- All other indices are unassigned by LibUTF and are available for
  application-defined or diagnostic use.

LibUTF does not attach standardized semantics to indices beyond 0. Any use
of additional indices should be treated as private to the application or
pipeline.


## Emission

### encodeUTF8n(text, unicode, bytes, use_java)

`encodeUTF8n` explicitly encodes a unicode value using a specified UTF-8
byte length.

This function can:
- emit a naturally-sized encoding
- intentionally emit an overlong encoding
- emit the Java-style 2-byte NULL when `(unicode == 0 && bytes == 2)`

Error reporting:
- `ModifiedUTF8` is set for the Java-style NULL.
- `OverlongUTF8 | IrregularForm` is set for other overlong encodings.
- Extended-length encodings set `ExtendedUTF8` and may also set
  `ExtendedUCS4 | IrregularForm` depending on the value.

If `errors.no_error()` is true, the bytes are written. Otherwise the
function reports the error and does not write output.

### indexToOverlongUTF8(index, unicode, bytes)

To emit an overlong form using the index model:
1) Convert `index` to `(unicode, bytes)` using `indexToOverlongUTF8`.
2) Pass the result to `encodeUTF8n`.

This is useful when storing or transporting a compact identifier rather
than raw byte sequences.


## Relationship to strict decoding

`decodeUTF8` classifies overlong and modified forms as irregular inputs.

Permissive decoding:
- returns the decoded unicode value
- reports the consumed byte length
- sets `ModifiedUTF8` or `OverlongUTF8` as appropriate
- sets `IrregularForm` when the form is not permitted by the selected mode

Strict decoding:
- any irregular form triggers `Failed | NotDecodable`
- the returned `unicode` becomes the lead byte value
- `bytes` is forced to 1

This behavior matches the general decode-failure model used throughout the
toolkit layer and allows strict decoders to make forward progress one byte
at a time. See docs/utf/toolkit_overview.md for the shared failure semantics.

Encodings flagged with `ExtendedUTF8` follow the same strictness rules as
other overlong forms: they can be observed in permissive decoding but are
rejected in strict modes.

This behavior supports two complementary workflows:
- permissive decoding for inspection, diagnostics, and repair
- strict decoding for enforcement and normalization


## Use cases

### Diagnostics and forensic analysis

- Identify modified UTF-8 NULLs and other overlong forms.
- Distinguish extended-length encodings via `ExtendedUTF8`.
- Record exact forms using the overlong index for compact logging.

### Legacy or non-conformant data handling

- Decode permissively to classify irregular encodings.
- Decide whether to normalize, preserve, or reject them downstream.

### Controlled signaling or private use

- Use index 0 for Java-style NULL when required.
- Use other indices only with application-defined meaning and care.

### Transcoding and normalization

- Preserve overlong forms by decoding permissively and re-emitting with
  explicit lengths.
- Normalize by re-encoding using natural-length UTF-8 or enforcing strict
  decode rules.


## Examples

1) Detecting an overlong sequence during decode

    utf_text t;
    t.buffer = data;
    t.length = data_len;
    t.offset = 0;

    unicode_t cp = 0;
    uint32_t  n  = 0;

    cp_errors e = unicode::utf::toolkit::decodeUTF8(
        t, cp, n,
        false,   // use_cesu
        false,   // use_java
        false,   // strict
        true     // coalesce
    );

    if (e.any(unicode::utf::toolkit::cp_errors::bits::OverlongUTF8))
    {
        // cp is the decoded value, n is the consumed byte length.
    }

2) Mapping an overlong form to an index

    uint32_t idx = 0;

    if (unicode::utf::toolkit::overlongToIndexUTF8(cp, n, idx))
    {
        // idx identifies the specific (unicode, bytes) overlong form.
    }

3) Emitting an overlong encoding intentionally

    uint32_t  idx = 0;  // Java-style NULL
    unicode_t cp  = 0;
    uint32_t  n   = 0;

    if (unicode::utf::toolkit::indexToOverlongUTF8(idx, cp, n))
    {
        utf_text out;
        out.buffer = dst;
        out.length = dst_len;
        out.offset = 0;

        cp_errors e =
            unicode::utf::toolkit::encodeUTF8n(out, cp, n, true);
    }
