## Description

<!-- What does this PR do? Link any related issues. -->

Closes #

## Changes

<!-- Bullet list of key changes -->

-

## Test checklist

- [ ] All existing tests pass: `ctest --output-on-failure --test-dir build`
- [ ] New tests added for new functionality (if applicable)
- [ ] Integration test passes: `bash tests/run_integration.sh`

## Build verification

- [ ] CMake build succeeds with `BUILD_TESTS=ON`
- [ ] Nix headless build succeeds: `nix build .`
- [ ] Nix UI plugin build succeeds: `nix build .#ui-plugin`

## Additional notes

<!-- Anything reviewers should know: breaking changes, migration steps, screenshots, etc. -->
