# Discrete Fourier Transform Tool (DFTT)
![badge](https://badgen.net/badge/Coverage/100/blue) 
<a href="https://scan.coverity.com/projects/ymich9963-dftt">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/32163/badge.svg"/>
</a>

Input real-valued data and get the DFT output. Supports audio files and CSV file or string inputs with a variety of output formats. Also supports FFT algorithms. 

## Features
- Input types,
    - Audio files.
    - CSV file or string.
- FFT algorithms,
    - Radix-2 Decimation In Time (DIT).
    - Radix-2 Decimation In Frequency (DIF).
- Timer to time the process just for fun.
- Output formats,
    - Standard output to terminal.
    - Text file.
    - CSV.
    - Hex dump.
    - C-style array.
- High precision output with ability to change precision.
- Output frequency bins alongside the DFT result.
- Output the power spectrum of the result.
- Choose to only output half of the result.
- Choose to shift the result (fftshift() function in popular tools like Matlab or Octave).

## Motivation
I always wondered how the DFT and its FFT algorithms are implemented in C code. So I decided to create a project that tries to implement various different algorithms so that maybe other people can be fascinated with me. The main point is that the code is commented and as easy to understand as possible, while using as few dependencies as possible. Therefore, speed and memory efficiency are not a priority in this project.

## Installing
Currently an automatic installation exists only for Windows, and binaries are built only for Windows. For other Operating Systems you need to build from source.

### Windows
To install automatically, use the install script located in `install/` by executing the command below in a PowerShell terminal with Admin rights,

```
irm "https://raw.githubusercontent.com/ymich9963/dftt/refs/heads/main/install/dftt-windows-install.ps1" | iex
```

The script downloads the executable, moves it to `C:\Program Files\dftt\`, and adds that path to the system environment variable. If you do not want the automated script feel free to download the executable or build from source. In case your organisation doesn't allow you to install from the script due to protected paths, download the script and change the `$DFTT_install_path` variable to a location that suits you.

### macOS & Linux
Please the Building section. Use `make` to build from source.

## Usage
The `--help` option which provides a list of the available commands is listed below, followed by example uses.

```
Discrete Fourier Transform Tool (DFTT) help page.

Basic usage 'dftt <Input audio file or CSV file or CSV string> [options]. For list of options see below.

                --fft <Algo>                    = Use an FFT algorithm to compute the DFT. Selecte between 'radix2-dit'.
                --dft                           = Regular DFT calculation using Euler's formula to expand the summation. Default behaviour, included for completion.
                --timer                         = Start a timer to see how long the calculation takes.
                --info                          = Output to stdout some info about the input file.
        -i,     --input <File/String>   = Accepts audio files and CSV files or strings. Make sure to separate string with commas, e.g. 1,0,0,1. Use the options below if you want to specify but DFTT implements auto-detection.
                --input-audio
                --input-csv
        -o,     --output <File Name>            = Path or name of the output file.
        -f,     --output-format <Format>        = Format of the output file. Select between: 'stdout', 'stdout-csv', 'columns', 'csv', 'hex-dump', and 'c-array'.
        -N,     --total-samples <Number>        = Set total number of samples to use when calculating. If using the FFT, it rounds up to the next power of 2 samples, zero-padding the signal if necessary.
        -p,     --precision <Number>            = Decimal number to define how many decimal places to output.
        -s,     --sampling-frequency <Number>   = Specify sampling frequency, only used when showing the frequency bins.
        -w,     --window <Window>               = Select a windowing function. Choose between 'rectangular', 'hann', 'hamming', and 'blackman'.
        -q,     --quiet                         = Silence all status messages to stdout. Overwrites '--timer' and '--info'.
        -b,     --bins                          = Show the frequency bins in the output.
        --pow,  --power-spectrum                = Output the power spectrum instead of the DFT itself.
        --norm, --normalise             = Normalise the data. Only works wit --pow.
        --half, --output-half                   = Output only half of the result.
        --shift,--fft-shift                     = Shift the result between -N/2 and N/2.
        --no-headers                    = No headers in the output.
```
Automatically converts a multi-channel audio file to single-channel (mono) and executes the DFT. To convert your file before using the tool, see [`mix2mono`](https://github.com/ymich9963/mix2mono).

### Example Uses
To calculate a DFT for a real-valued CSV string use it as shown below
```
dftt 1,0,0,1
```
Can also use audio files,
```
dftt smells-like-teen-spirit.wav
```
Output to a CSV file with headers and frequency bins,
```
dftt smells-like-teen-spirit.wav --headers --bins -f csv
```

## Building
Simply use the `make` command to build the executable.

## Tests and Coverage
Running the tests or coverage can be done by running,

```
make test
```
```
make coverage
```
Testing suite used is [Unity](https://github.com/ThrowTheSwitch/Unity) and LLVM-COV for coverage.
