# Changelog

All notable changes to this project are documented here.

## [v0.3.0] - 2025-07-12

### Added
- `img_subtract` function for image subtraction.
- Comprehensive error handling system using the `ImgError` enum and `img_strerror` for detailed error messages.

### Changed
- **API Overhaul & Refactoring**
  - Functions that can fail now return an `ImgError` code.
  - Image manipulation functions (`img_add`, `img_subtract`, `img_rgb2gray`, `img_resize`, `img_cpy`) now accept a destination image pointer as a parameter instead of returning a new image. This provides better memory management and clearer API semantics.
  - Removed the `ImagePtr` typedef; the API now consistently uses `Image *`.
  - Improved function signatures for better consistency and clarity.

---

## [v0.2.0-beta] - 2025-03-13

### Added
- **Image Processing Utilities**
  - Image convolution with multiple border handling options (`img_convolve`, `img_filter2D`)

- **Kernel Management**
  - Support for various kernel types (Identity, Box blur, Sharpen, Sobel, Laplacian)
  - Kernel management functions:
    - `img_get_kernel`
    - `img_print_kernel`
    - `img_free_kernel`

---

## [v0.1.0-beta] - 2025-03-04
- Partial support for PNM formats (PPM, PGM only).
- **Basic Image Processing Utilities**
  - `img_resize`
  - `img_rgb2gray`
  - `img_add`
