# imglib

`imglib` is a minimal and portable C library for image manipulation, designed to be simple and efficient while adhering to the **Suckless C coding style**. It is intended to be **portable across all Unix systems** and comes with a **portable Makefile** that works with different `make` implementations.

## ⚠️ Warning

> ⚠️  Expect dirty, spaghetti, smelly code! <br>
>  This project is a learning exercise, and the code may not be pretty or optimized. I'm still learning, so bear with me as I improve over time. Feedbacks are welcome!

## Features
- Lightweight and dependency-free (where possible)
- Written in clean and minimal C code
- Compatible with all Unix-like systems
- Portable `Makefile`
- Supports only PPM format for now (future support for other formats may be added)

## Usage
- Include the library in your C project by adding `#include "image.h"` to your source code. 


Compile with:
```sh
gcc -o main.c main.c -L./build -limglib
export LD_LIBRARY_PATH="./build/:$LD_LIBRARY_PATH"
./main

```
- For a complete example of how to use the library, refer to the main.c file in the repository. It demonstrates loading an image, manipulating pixel data, saving the modified image, and displaying it using an external viewer.

## Makefile
The `Makefile` is written to be portable across different Unix systems, avoiding GNU-specific extensions. It provides the following targets:

- `make lib`: Compiles the shared object (`.so`) for the library.
- `make example`: Compiles `main.c` as an example program using the library. The example program demonstrates loading an image and accessing pixel data:
- `make clean`: Removes compiled objects and binaries.
