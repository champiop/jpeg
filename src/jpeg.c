#include "netpbm.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif
#include <string.h>

int spatial_to_zigzag[8][8] = {
    {0, 1, 5, 6, 14, 15, 27, 28},     {2, 4, 7, 13, 16, 26, 29, 42},
    {3, 8, 12, 17, 25, 30, 41, 43},   {9, 11, 18, 24, 31, 40, 44, 53},
    {10, 19, 23, 32, 39, 45, 52, 54}, {20, 22, 33, 38, 46, 51, 55, 60},
    {21, 34, 37, 47, 50, 56, 59, 61}, {35, 36, 48, 49, 57, 58, 62, 63}};

uint16_t quantization_table_y[64] = {
    0x05, 0x03, 0x03, 0x05, 0x07, 0x0c, 0x0f, 0x12, 0x04, 0x04, 0x04,
    0x06, 0x08, 0x11, 0x12, 0x11, 0x04, 0x04, 0x05, 0x07, 0x0c, 0x11,
    0x15, 0x11, 0x04, 0x05, 0x07, 0x09, 0x0f, 0x1a, 0x18, 0x13, 0x05,
    0x07, 0x0b, 0x11, 0x14, 0x21, 0x1f, 0x17, 0x07, 0x0b, 0x11, 0x13,
    0x18, 0x1f, 0x22, 0x1c, 0x0f, 0x13, 0x17, 0x1a, 0x1f, 0x24, 0x24,
    0x1e, 0x16, 0x1c, 0x1d, 0x1d, 0x22, 0x1e, 0x1f, 0x1e,
};

uint16_t quantization_table_cbcr[64] = {
    0x05, 0x05, 0x07, 0x0e, 0x1e, 0x1e, 0x1e, 0x1e, 0x05, 0x06, 0x08,
    0x14, 0x1e, 0x1e, 0x1e, 0x1e, 0x07, 0x08, 0x11, 0x1e, 0x1e, 0x1e,
    0x1e, 0x1e, 0x0e, 0x14, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
    0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
    0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
    0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
};

// Converts every sample value of a 8x8 mcu from RGB to YCbCr color format
void rgb_to_ycbcr(uint16_t *mcu_in, uint8_t *mcu_out) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      uint8_t r = mcu_in[3 * 8 * j + 3 * i + 0];
      uint8_t g = mcu_in[3 * 8 * j + 3 * i + 1];
      uint8_t b = mcu_in[3 * 8 * j + 3 * i + 2];

      mcu_out[3 * 8 * j + 3 * i + 0] = 0.299 * r + 0.587 * g + 0.114 * b;
      mcu_out[3 * 8 * j + 3 * i + 1] = -0.1687 * r - 0.3313 * g + 0.5 * b + 128;
      mcu_out[3 * 8 * j + 3 * i + 2] = 0.5 * r - 0.4187 * g - 0.0813 * b + 128;
    }
  }
}

// Substracts 128 from each sample value of a 8x8 mcu
void offset(uint8_t *mcu_in, int8_t *mcu_out) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      uint8_t r = mcu_in[3 * 8 * j + 3 * i + 0];
      uint8_t g = mcu_in[3 * 8 * j + 3 * i + 1];
      uint8_t b = mcu_in[3 * 8 * j + 3 * i + 2];

      mcu_out[3 * 8 * j + 3 * i + 0] = r - 128;
      mcu_out[3 * 8 * j + 3 * i + 1] = g - 128;
      mcu_out[3 * 8 * j + 3 * i + 2] = b - 128;
    }
  }
}

// Naive implementation of a 2D DCT
void dct(int8_t *mcu_in, int16_t *mcu_out) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      for (int c = 0; c < 3; c++) {
        double tmp = 0;

        for (int y = 0; y < 8; y++) {
          for (int x = 0; x < 8; x++) {
            tmp += mcu_in[3 * 8 * y + 3 * x + c] *
                   cos(((2 * x + 1) * i * M_PI) / 16) *
                   cos(((2 * y + 1) * j * M_PI) / 16);
          }
        }

        tmp *= 1.0 / 4.0;
        tmp *= (i == 0 ? 1.0 / sqrt(2) : 1);
        tmp *= (i == 0 ? 1.0 / sqrt(2) : 1);

        mcu_out[3 * 8 * j + 3 * i + c] = (int16_t)tmp;
      }
    }
  }
}

// Reorders data samples in a "zig-zag" way
void zigzag(int16_t *mcu_in, int16_t *mcu_out) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      mcu_out[3 * spatial_to_zigzag[j][i] + 0] = mcu_in[3 * 8 * j + 3 * i + 0];
      mcu_out[3 * spatial_to_zigzag[j][i] + 1] = mcu_in[3 * 8 * j + 3 * i + 1];
      mcu_out[3 * spatial_to_zigzag[j][i] + 2] = mcu_in[3 * 8 * j + 3 * i + 2];
    }
  }
}

// Quantize data samples
void quantize(int16_t *mcu_in, int16_t *mcu_out) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      mcu_out[3 * 8 * j + 3 * i + 0] =
          mcu_in[3 * 8 * j + 3 * i + 0] /
          quantization_table_y[spatial_to_zigzag[j][i]];
      mcu_out[3 * 8 * j + 3 * i + 1] =
          mcu_in[3 * 8 * j + 3 * i + 1] /
          quantization_table_cbcr[spatial_to_zigzag[j][i]];
      mcu_out[3 * 8 * j + 3 * i + 2] =
          mcu_in[3 * 8 * j + 3 * i + 2] /
          quantization_table_cbcr[spatial_to_zigzag[j][i]];
    }
  }
}

// Write appx section to file
void write_appx(FILE *file) {
  // APP0
  fputc(0xff, file);
  fputc(0xe0, file);

  // Length
  fputc(0x00, file);
  fputc(0x10, file);

  // "JFIF"
  fputc('J', file);
  fputc('F', file);
  fputc('I', file);
  fputc('F', file);
  fputc('\0', file);

  // Version
  fputc(0x01, file);
  fputc(0x01, file);

  // JFIF metadata (zero)
  fputc(0x00, file);
  fputc(0x00, file);
  fputc(0x00, file);
  fputc(0x00, file);
  fputc(0x00, file);
  fputc(0x00, file);
  fputc(0x00, file);
}

// Write dqt section to file
void write_dqt(FILE *file, uint16_t *quantization_table) {
  static int id = 0;

  // DQT
  fputc(0xff, file);
  fputc(0xdb, file);

  // Length
  fputc(0x00, file);
  fputc(0x43, file);

  // Precision + table id
  fputc(id++, file);

  // Values
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      fputc(quantization_table[spatial_to_zigzag[j][i]], file);
    }
  }
}

// Write sofx section to file
void write_sofx(FILE *file, size_t width, size_t height) {
  // SOF0
  fputc(0xff, file);
  fputc(0xc0, file);

  // Length
  fputc(0x00, file);
  fputc(0x11, file);

  // Precision
  fputc(0x08, file);

  // Image height
  fputc((height >> 8) & 0xff, file);
  fputc(height & 0xff, file);

  // Image width
  fputc((width >> 8) & 0xff, file);
  fputc(width & 0xff, file);

  // Components
  fputc(0x03, file);

  // Component Y
  fputc(0x00, file); // Component id
  fputc(0x11, file); // Sampling factors
  fputc(0x00, file); // Quantification table id

  // Component Cb
  fputc(0x01, file); // Component id
  fputc(0x11, file); // Sampling factors
  fputc(0x01, file); // Quantification table id

  // Component Cr
  fputc(0x02, file); // Component id
  fputc(0x11, file); // Sampling factors
  fputc(0x01, file); // Quantification table id
}

// Write the JFIF image into the file
void write_image(FILE *file) {
  // Start of image
  fputc(0xff, file);
  fputc(0xd8, file);

  // Application data
  write_appx(file);

  // Define quantization table
  write_dqt(file, quantization_table_y);
  write_dqt(file, quantization_table_cbcr);

  // End of image
  fputc(0xff, file);
  fputc(0xd9, file);
}

void display_uint8(uint8_t *mcu, int stride) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      printf(" ");
      printf("%02x ", mcu[3 * 8 * j + 3 * i + stride]);
    }
    printf("\n\n");
  }
}

void display_int8(int8_t *mcu, int stride) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      printf(" ");
      printf("%02x ", mcu[3 * 8 * j + 3 * i + stride]);
    }
    printf("\n\n");
  }
}

void display_uint16(uint16_t *mcu, int stride) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      printf(" ");
      printf("%04x ", mcu[3 * 8 * j + 3 * i + stride]);
    }
    printf("\n\n");
  }
}

void display_int16(int16_t *mcu, int stride) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      printf(" ");
      printf("%04x ", 0xffff & mcu[3 * 8 * j + 3 * i + stride]);
    }
    printf("\n\n");
  }
}

int main(void) {
  netpbm_image_t *image = netpbm_open("images/invader.ppm");
  if (image == NULL) {
    printf("Error: Could not open input image\n");
    return EXIT_SUCCESS;
  }

  if (netpbm_torgb(image) == -1) {
    printf("Error: Could not convert image to RGB color format");
    netpbm_destroy(image);
    return EXIT_SUCCESS;
  }

  size_t w = netpbm_get_width(image);
  size_t h = netpbm_get_height(image);

  uint16_t *mcu_rgb = malloc(3 * 8 * 8 * sizeof(uint16_t));
  netpbm_clone_data(image, mcu_rgb);
  netpbm_destroy(image);

  printf("---- Initial data -----------------------------------------\n");
  printf("Channel R:\n");
  display_uint16(mcu_rgb, 0);
  printf("Channel G:\n");
  display_uint16(mcu_rgb, 1);
  printf("Channel B:\n");
  display_uint16(mcu_rgb, 2);
  printf("-----------------------------------------------------------\n");

  uint8_t *mcu_ycbcr = malloc(3 * 8 * 8 * sizeof(uint8_t));
  rgb_to_ycbcr(mcu_rgb, mcu_ycbcr);
  free(mcu_rgb);

  printf("---- Conversion -------------------------------------------\n");
  printf("Channel Y:\n");
  display_uint8(mcu_ycbcr, 0);
  printf("Channel Cb:\n");
  display_uint8(mcu_ycbcr, 1);
  printf("Channel Cr:\n");
  display_uint8(mcu_ycbcr, 2);
  printf("-----------------------------------------------------------\n");

  int8_t *mcu_offset = malloc(3 * 8 * 8 * sizeof(int8_t));
  offset(mcu_ycbcr, mcu_offset);
  free(mcu_ycbcr);

  printf("---- Offset -----------------------------------------------\n");
  printf("Channel Y:\n");
  display_int8(mcu_offset, 0);
  printf("Channel Cb:\n");
  display_int8(mcu_offset, 1);
  printf("Channel Cr:\n");
  display_int8(mcu_offset, 2);
  printf("-----------------------------------------------------------\n");

  int16_t *mcu_dct = malloc(3 * 8 * 8 * sizeof(int16_t));
  dct(mcu_offset, mcu_dct);
  free(mcu_offset);

  printf("---- DCT --------------------------------------------------\n");
  printf("Channel Y:\n");
  display_int16(mcu_dct, 0);
  printf("Channel Cb:\n");
  display_int16(mcu_dct, 1);
  printf("Channel Cr:\n");
  display_int16(mcu_dct, 2);
  printf("-----------------------------------------------------------\n");

  int16_t *mcu_zigzag = malloc(3 * 8 * 8 * sizeof(int16_t));
  zigzag(mcu_dct, mcu_zigzag);
  free(mcu_dct);

  printf("---- Zig-Zag ----------------------------------------------\n");
  printf("Channel Y:\n");
  display_int16(mcu_zigzag, 0);
  printf("Channel Cb:\n");
  display_int16(mcu_zigzag, 1);
  printf("Channel Cr:\n");
  display_int16(mcu_zigzag, 2);
  printf("-----------------------------------------------------------\n");

  int16_t *mcu_quantize = malloc(3 * 8 * 8 * sizeof(int16_t));
  quantize(mcu_zigzag, mcu_quantize);
  free(mcu_zigzag);

  printf("---- Quantization -----------------------------------------\n");
  printf("Channel Y:\n");
  display_int16(mcu_quantize, 0);
  printf("Channel Cb:\n");
  display_int16(mcu_quantize, 1);
  printf("Channel Cr:\n");
  display_int16(mcu_quantize, 2);
  printf("-----------------------------------------------------------\n");

  return EXIT_SUCCESS;
}
