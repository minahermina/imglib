## ⚠️ Warning

> - This project is a learning exercise, and the code may not be pretty or optimized. I'm still learning, so bear with me as I improve over time. Feedbacks are welcome! <br>
> - Expect dirty, spaghetti, smelly, and buggy code! <br>

# imglib

`imglib` is a minimal and portable C library for image manipulation, designed to be simple and efficient while adhering to the **Suckless C coding style**. It is intended to be **portable across all Unix systems** and comes with a **portable Makefile** that works with different `make` implementations.


## Features
- Lightweight and dependency-free (where possible)
- Written in ~~clean~~ and minimal C code
- Compatible with all Unix-like systems
- Portable `Makefile`
- Supports only [PNM](https://netpbm.sourceforge.net/doc/pnm.html) formats (partially), with possible future support for other formats
- Image processing functions including convolution and filtering.

## Current Version: v0.2.0-beta (2025-03-13)

### Latest Features
- **Image Processing Utilities**
  - Image convolution with multiple border handling options (`img_convolve`, `img_filter2D`)
  - Image resizing (`img_resize`)
  - Grayscale conversion (`img_rgb2gray`)
  - Image addition (`img_add`)

- **Kernel Management**
  - Support for various kernel types:
    - Identity
    - Box blur
    - Sharpen
    - Sobel (X and Y)
    - Laplacian
  - Kernel management functions:
    - `img_get_kernel`
    - `img_print_kernel`
    - `img_free_kernel`

## Usage

1. **Include the library** in your C project:  
   ```c
   #include "image.h"
   ```
   ```sh
   # Step 1: Build the shared library (creates build/libimglib.so)
    make  

    # Step 2: Compile your program, linking against imglib
    gcc -o main main.c -L./build -limglib  

    # Step 3: Set the library path and run the program
    export LD_LIBRARY_PATH="./build/:$LD_LIBRARY_PATH"
    ./main
    ```

- For a complete example of how to use the library, refer to the main.c file in the repository. It demonstrates loading an image, manipulating pixel data, saving the modified image, and displaying it using an external viewer.

## Makefile
The `Makefile` is written to be portable across different Unix systems, avoiding GNU-specific extensions. It provides the following targets:

- `make release`: Compiles the shared object (`.so`) for the library for release build.
- **`make debug`**  
  - Compiles the shared object (`.so`) for the library in debug mode.
  - By default, it uses Clang's sanitizer feature, which may not be supported on some Unix-like systems.
  - If encountering issues, consider changing the compiler (`CC=gcc` or another supported compiler).

- `make example`: Compiles `main.c` as an example program using the library. The example program demonstrates loading an image and accessing pixel data.
- `make clean`: Removes compiled objects and binaries.


## Future Plans

For a detailed list of planned features and enhancements, check out the [`TODO.md`](TODO.md) file.
