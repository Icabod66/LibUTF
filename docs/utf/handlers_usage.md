# File: docs/utf/handlers_usage.md

# SuiteUTF Handler-Based APIs (IUTF and IUTFTK)

This document describes how to use the handler-based APIs in SuiteUTF:

- unicode::utf::std::IUTF
- unicode::utf::toolkit::IUTFTK

These abstractions are the primary real-world interfaces for working with
encoded text buffers. They wrap the low-level encode and decode functions in
utf_std and utf_toolkit and provide a consistent way to read, write, validate,
and iterate over text stored in a utf_text buffer.


## 1. utf_text: the common buffer view

Both handler interfaces operate on the utf_text structure defined in utf_std.h.

utf_text is a non-owning view of an encoded buffer with an explicit cursor.
Operations advance the cursor on success and leave it unchanged on failure.
This guarantees that decode and encode operations are atomic with respect to
stream position.


## 2. What handlers are and why use them

Handlers encapsulate:

- The selected encoding.
- Encoding-specific validation rules.
- Sequential processing semantics.

Using handlers allows encoding-agnostic code and avoids manual selection of
encoding-specific functions. For most applications, handlers are the preferred
interface.


## 3. Getting standard handlers (IUTF)

IUTF provides strict, standards-compliant behavior with simple success or
failure reporting.

IUTF handlers are obtained via static factory functions and are returned as
const references. Callers do not construct or destroy handler objects.


## 4. Getting toolkit handlers (IUTFTK)

IUTFTK extends IUTF with:

- A richer set of encoding variants.
- Control over strictness and skipping behavior.
- Detailed diagnostics via cp_errors.

The diagnostic model is described in docs/utf/cp_errors.md.


## 5. UTF_SUB_TYPE and variants

UTF_SUB_TYPE enumerates UTF encoding variants and legacy forms.

Most users do not need to understand all variants. Common starting points are
the strict variants corresponding to UTF8, UTF16, and UTF32. Detailed behavior
is described in docs/utf/variants_utf_sub_type.md.

The term "strict" in UTF_SUB_TYPE refers to variant-level validation rules and
is distinct from the strict behavior of the utf_std layer.

The strict variants in UTF_SUB_TYPE are derived from the same acceptance
rules as the utf_std layer. They differ in reporting detail, available
controls, and edge-case interpretation.


## 6. When to use IUTF vs IUTFTK

Use IUTF when you need:

- Strict, standards-compliant UTF processing.
- Simple success or failure results.

Use IUTFTK when you need:

- Detailed diagnostics.
- Control over permissive or non-skipping behavior.
- Support for legacy or CESU-style encodings.
