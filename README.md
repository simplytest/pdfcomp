<div align="center">
<img src="https://avatars.githubusercontent.com/u/49808127?s=200&v=4" width="200">

### PDFComp

</div>

<br/>

## 👋 Introduction

PDFComp is a tool to compare two given PDFs (or images) for equality. 
Equality is determined by the absolute pixel difference.

## 🖥️ Usage

```
Usage: pdfcomp [OPTION]... <first> <second> 

A utility to compare PDF files

OPTIONS

  -t, --tol[=<value>]     Absolute tolerance
  -d, --diff[=<path>]     Folder to save a difference image(s) to
      --help              show help
      --version           show version
```

If the given PDFs are equal the return code is `0`, in case they aren't it is `2`.

## 🧰 Compilation

Make sure you have all required dependencies:

* ImageMagick
* C++23 Compiler

Then clone the project and compile:

```sh
git clone https://github.com/simplytest/pdfcomp
cd pdfcomp

cmake -B build && cmake --build build
```
