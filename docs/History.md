# SuiteUTF - Project History

This document records the development history of the SuiteUTF library and related
components, including later refactoring and documentation work. It is intended
as a factual record rather than a retrospective justification.

## Early development

SuiteUTF started life as just a couple of files with a few functions to read basic
UTF-8 files which were the foundation for a number of parsers in a much larger
game engine codebase. Over time it evolved to become more comprehensive and add
error checking and diagnostics as some of the files I had produced had encoding
errors and could not be read with some editors. It also expanded to handle some
legacy documentation from the internet that contained invalid encodings.

It continued expanding over a number of years becoming close to what it is now.

SuiteUTF was never intended to be a higher level library with heavy Unicode
manipulation, text processing, font awareness, locale awareness and the like,
and it still isn't.

The core design principles were established early and remained largely stable:

- explicit handling of encoded byte streams
- clear separation between encoding and decoding
- avoidance of dynamic allocation
- predictable and inspectable behaviour in the presence of malformed input

The utf_toolkit layer was written first and reflected these principles in a
fairly direct way. It provided explicit error reporting through a structured
error return type, allowing detailed diagnostics when dealing with malformed or
non-standard encodings.

The utf_std layer was created later by editing down the toolkit implementation
to provide a lighter-weight interface for standards-compliant UTF handling. In
this process, explicit error codes were removed and replaced with the use of
sentinel unicode values to signal failure or exceptional conditions where no
separate error channel was available.

At the time, this was a pragmatic trade-off intended to keep the standard layer
simple and easy to integrate. Basically I'd found that the toolkit layer was
overkill for most of the use I was making of it and I wanted something simpler
that didn't carry all the mental noise of the error reporting and choosing
between all the possible encoding types when I was mostly using it for simple
well formed utf.

## Period of stability

From about mid-2016 the library saw little active development. The code remained
in use, but largely unchanged, and accumulated alongside other utilities in a
larger codebase. Over time, some functionality accreted into SuiteUTF that was not
strictly part of its original remit.

Although the code continued to work, familiarity with the original design
decisions inevitably faded.

## Reassessment and re-orientation

Before any use of AI tools, I was already reassessing the whole game engine
codebase manually, to update it and resuscitate an incomplete game I'd been
working on before consulting and contracting took up much of my life. The
process involved re-reading old code, checking assumptions, and rebuilding a
mental model of the design as it had existed at the time it was written.

Around October 2025, this process was supplemented by the use of Codex and later
ChatGPT. The initial goal was simple: to catalogue a large, old codebase and help
re-establish the mindset and intent that existed when the library was originally
developed.

Early use focused on orientation rather than change.

By the way, I still do consulting, contracting and development work. If anyone
is interested, more details can be found at www.morphicvoid.com.

## Refactoring and validation with AI assistance

As understanding improved, attention narrowed to the SuiteUTF directory itself.
AI tools were used to help identify code that had accreted into the library over
time but no longer belonged there. This led to a small refactor that moved such
code to more appropriate locations and left a cleaner, more coherent SuiteUTF
structure.

During this phase, AI was also used as a reviewer to help reason about original
design decisions and to sanity-check assumptions that were no longer fresh.

Once the directory structure was cleaned up, AI-assisted validation began. This
took the form of reviewer-style passes at varying levels of granularity,
including line-by-line inspection, function-level reasoning, and whole-file
ingestion.

Only one genuine logic issue was identified, on a code path that was almost
certainly never used. No other significant defects were found.

Much of the value came instead from repeated consistency passes. These included
clarifying signed versus unsigned intent through consistent use of trailing `u`
markers, gradual modernisation toward more modern C++ expectations, and the
systematic introduction of `noexcept`, `[[nodiscard]]`, and `constexpr` where
appropriate. This work eventually led to a refactor of error handling toward the
current `cp_errors` class.

These early validation passes also included a basic check of the encode, decode,
and error-reporting behaviour against the UTF and Unicode standards. This was
used as a consistency and sanity check and should not be considered a formal
standards compliance claim. No obvious discrepancies were identified during the
process, and the exercise helped confirm assumptions and clarify edge cases
already baked into the design.

AI was not used to write any code during this process. All changes were
implemented manually, with AI acting as a reviewer and guide, not a creator.

## Prompt refinement and false positives

Early AI validation passes produced a number of false positives. At some point
in the process the effort needed to check these reports started to outweigh the
benefit of using the AI at all.

Rather than giving up on the AI, I raised the issue directly with the AI, using
actual examples of incorrect reports. The AI was able to explain why the false
positives were occurring, which made sense for a rough, conservative review
pass, and suggested changes in prompt wording and expressiveness that would
enforce stricter internal validation before reporting issues.

With refined prompts, false positives based on simple logic and basic
expressions were eliminated entirely (or at least I stopped noticing them). The
remaining false positives were largely contextual, for example where a function
was flagged as unsafe in the abstract despite being called only with safe
literal parameters in the actual code.

This led to a collaborative prompt refinement process that seriously improved
the signal-to-noise ratio and allowed the AI to function as a genuine
accelerator, not a distraction.

## Documentation and further clarification

Documentation began with the expectation that the AI would act mainly as a
reviewer. In practice it became far more central to the process than expected.

By this point a solid understanding of the code and the original design intent
had been re-established, but it was clear that this knowledge existed largely in
my head and was not accessible to anyone else.

Given time constraints and limitations in writing capacity, AI was used heavily
to help structure and organise the documentation. This involved proposing
outlines, drafting explanatory text, and acting as a reviewer of wording and
clarity.

The documentation process itself surfaced ambiguities, inconsistencies, and edge
cases that had previously gone unnoticed. Several small changes and fixes
emerged directly from this work as assumptions were forced to be written down.

Throughout this phase, style guidance was provided, misunderstandings caused by
outdated or incomplete comments were corrected, additional code context was
supplied as needed, and all output was proof-read, corrected, or redirected as
it was produced.

The result was documentation that exceeded initial expectations, at least from
my perspective.

## Use of AI tools

AI tools were used throughout the later phases of this work as support tools.
Their role evolved over time, beginning with codebase orientation, expanding
into reviewer-style validation and refactoring assistance, and eventually
playing a significant role in structuring and refining the documentation.

All design decisions and final changes were deliberated by me. AI was used as a
tool to provide feedback, alternative viewpoints, and review passes, but not as
an author or a source of design intent.

The library remains a human-designed system. AI assistance served to accelerate
analysis and feedback, not to originate behaviour or policy.
