# Changelog

All notable changes to this project are documented here.

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
