# PedalDevKit Example

Basic Seed2 DFM Pedal Dev Kit project with all required libraries, etc.

## Get Started

If cloning this repo from Github, you will want to do so with the submodules with:

```console
git clone https://github.com/electro-smith/PedalDevKit-Example --recurse-submodules
```

if you've already cloned the repo without this you can fill the submodules by running the following:

```console
git submodule update --init
```

Alternatively, you can avoid using git, and building libraries if you download the latest zip compiled zip file in the releases.

## Contents

Includes:

* libDaisy - hardware library for Daisy
* DaisySP - DSP library
* BasicExample - Basic example demonstrating hardware setup code for the dev kit hardware.
* PassthruAndBypass - Example demonstrating toggling between audio passthru and true bypass
* SimplePotReading - Exmaple demonstrating reading two pots, and printing their values to a serial monitor.
* SimpleSwitchReading - Example demonstrating reading the position of a toggle.

When you have the Makefile, or a source file within an examples folder open, you can run the "Build task" with "ctrl-shift-B" on windows, or "cmd-shift-B" on Mac OS, and that will compile the selected example.

For each of these examples, there are the following VS code tasks:

* Build: build the specified example
* Clean: removes the specified example's build folder, and it's contents.
* Program DFU: programs the specified example's binary file via USB
* Build and Program DFU: Rebuilds the specified program and downloads it via USB DFU

In addition, there is a set of tasks that will operate on the entire workspace:

* Build Libraries: builds libDaisy, and DaisySP from scratch. This is only necessary when manually updating, or cloning for the first time.
* Build All: builds all of the individual example projects.
* Clean All: removes all examples' build folders, and their contents.

The following debug configurations are available:

* Debug BasicExample: debugs the BasicExample application

**Note**: debugging an application from VS Code requires an ST-Link or similar probe, as well as the Cortex Debug extension for VS Code.
