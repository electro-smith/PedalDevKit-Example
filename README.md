# DaisyBlinkProject

Basic Blink Project with all required libraries, etc.

## Get Started

If cloning this repo from Github, you will want to do so with the submodules with:

```console
git clone https://github.com/electro-smith/DaisyBlinkProject --recurse-submodules
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
* Blink - Basic example demonstrating a blink

This also contains the following VS code tasks:

* Build Libraries: builds libDaisy, and DaisySP from scratch. This is only necessary when manually updating, or cloning for the first time.
* Build: build the blink example
* Clean: removes compiled Blink code
* Program DFU: programs the Blink.bin file via USB
* Build and Program DFU: Rebuilds the program and downloads it via USB DFU

In addition, the following debug configurations are available:

* Debug Blink: debugs the Blink application

**Note**: debugging an application from VS Code requires an ST-Link or similar probe, as well as the Cortex Debug extension for VS Code.
