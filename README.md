# Discrete Fourier Transform Tool (DFTT)
Input 1-dimensional data and get the DFT output. Supports audio and CSV file inputs with a variety of output formats. Also supports FFT algorithms. 

## Features
- Input types,
    - Audio files.
    - CSV, either file or a string parameter.
- FFT algorithms,
    - Radix-2 Decimation In Time (DIT).
- Timer to check how long the process takes.
- Output formats,
    - Standard output to terminal.
    - Text file.
    - CSV.
    - Hex dump.
    - C-style array.
- Change precision of output.
- Output frequency bins alongside the DFT result.
- Output the power spectrum of the result.
- Choose to only output half of the result.
- Choose to shift the result.

## Motivation
I always wondered how the DFT and its FFT algorithms are implemented in code. So I decided to create a project that tries to implement various different algorithms to shed light on how certain FFT implementations can be coded in C. The main point is that the code is commented and as easy to understand as possible, while using the standard libraries for compatibility. Therefore, speed and memory efficiency are not a priority in this project.

