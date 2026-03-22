Headturner Hat Windows packaging assets

This folder contains an Inno Setup installer script for a Windows release:

- HeadturnerHat.iss

What is missing in this macOS-built bundle:

- A Windows-built Headturner Hat.vst3 binary
- A compiled Windows installer .exe

To produce the Windows installer:

1. Build the VST3 on Windows with JUCE/CMake and Visual Studio.
2. Place the resulting "Headturner Hat.vst3" folder next to HeadturnerHat.iss.
3. Open HeadturnerHat.iss in Inno Setup and compile it.

The installer script will place the plugin in:

C:\Program Files\Common Files\VST3
