# Headturner Hat

Headturner Hat is a focused techno hi-hat instrument built with JUCE for Ableton Live and other VST3/AU hosts.

It is designed around a noise-first analog-techno character:

- `Closed/Open` mode
- `Decay`
- `Tone`
- `Air`
- `Metal`
- `Drive`
- named preset save/load inside the plugin

## Project layout

- `src/` plugin processor and editor code
- `scripts/` local packaging scripts for macOS and Windows
- `.github/workflows/` GitHub Actions pipeline for hosted macOS and Windows installer builds
- `packaging/` release notes and Windows installer assets
- `JUCE/` vendored JUCE source used by this project

## Local macOS build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_COPY_PLUGIN_AFTER_BUILD=OFF
cmake --build build --config Release --target HeadturnerHat_VST3
```

## GitHub installers

The repo includes a GitHub Actions workflow that builds:

- a macOS installer package
- a Windows installer executable
- a combined release zip

For a public repository, the simplest release flow is:

1. Push the repo to GitHub.
2. Make the repository public.
3. Push a tag like `v0.1.0`.
4. Let GitHub Actions build both installers and attach them to the GitHub release.

See [PUBLISHING.md](/Users/rahu/Documents/Playground/PUBLISHING.md) for the exact setup flow.
