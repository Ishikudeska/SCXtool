## What this changes

<!-- What does this pull request do, and why? Link the issue it fixes, if any (for example "Fixes #12"). -->

## Screenshots

<!-- For UI changes, add before and after screenshots. Delete this section otherwise. -->

## Type of change

- [ ] Bug fix
- [ ] New feature or enhancement
- [ ] Translation
- [ ] Game data patch (`resources/patches/`)
- [ ] Build, CI or tooling
- [ ] Documentation

## Checklist

See [CONTRIBUTING.md](https://github.com/Ishikudeska/SCXtool/blob/main/CONTRIBUTING.md) for how to run each check.

- [ ] C++ is formatted with clang-format 22.1.8.
- [ ] It builds with no warnings (`-DSC_WARNINGS_AS_ERRORS=ON`).
- [ ] QML lint passes (`all_qmllint`).
- [ ] Unit tests pass (`ctest --preset mingw-debug -LE parity`), and I added or updated tests for what I changed.
- [ ] If I added UI text, it uses a dotted key and `tools/i18n/check_keys.py` passes.
- [ ] If I changed generated text (enhancements, tags or merging), I ran the parity tests, or I explain below why the output now differs from Smart Citizen on purpose.
- [ ] If I changed what the app writes to disk or connects to over the network, I updated [SECURITY.md](https://github.com/Ishikudeska/SCXtool/blob/main/SECURITY.md).

## Notes for reviewers

<!-- Anything else: tradeoffs, follow-up work, or parts you'd like a close look at. Delete this section if there's nothing to add. -->
