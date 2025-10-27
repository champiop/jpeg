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

// Converts every sample value of a 8x8 mcu from RGB to YCbCr color format
void rgb_to_ycbcr(uint16_t *mcu_in, uint8_t *mcu_out) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      uint8_t r = mcu_in[3 * 8 * j + 3 * i + 0];
      uint8_t g = mcu_in[3 * 8 * j + 3 * i + 1];
      uint8_t b = mcu_in[3 * 8 * j + 3 * i + 2];

      mcu_out[3 * 8 * j + 3 * i + 0] = 0.299 * r + 0.587 * g + 0.114 * b;
      mcu_out[3 * 8 * j + 3 * i + 1] = -0.167 * r - 0.3313 * g - 0.5 * b + 128;
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
void dct(int8_t *mcu_in, double *mcu_out) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      for (int c = 0; c < 3; c++) {
        mcu_out[3 * 8 * j + 3 * i + c] = 0;

        for (int y = 0; y < 8; y++) {
          for (int x = 0; x < 8; x++) {
            mcu_out[3 * 8 * j + 3 * i + c] +=
                mcu_in[3 * 8 * y + 3 * x + c] *
                cos(((2 * x + 1) * i * M_PI) / 16) *
                cos(((2 * y + 1) * j * M_PI) / 16);
          }
        }

        mcu_out[3 * 8 * j + 3 * i + c] *= 1.0 / 4.0;
        mcu_out[3 * 8 * j + 3 * i + c] *= (i == 0 ? 1.0 / sqrt(2) : 1);
        mcu_out[3 * 8 * j + 3 * i + c] *= (i == 0 ? 1.0 / sqrt(2) : 1);
      }
    }
  }
}

// Reorders data samples in a "zig-zag" way
void zigzag(double *mcu_in, double *mcu_out) {
  int spatial_to_zigzag[8][8] = {
      {0, 1, 5, 6, 14, 15, 27, 28},     {2, 4, 7, 13, 16, 26, 29, 42},
      {3, 8, 12, 17, 25, 30, 41, 43},   {9, 11, 18, 24, 31, 40, 44, 53},
      {10, 19, 23, 32, 39, 45, 52, 54}, {20, 22, 33, 38, 46, 51, 55, 60},
      {21, 34, 37, 47, 50, 56, 59, 61}, {35, 36, 48, 49, 57, 58, 62, 63}};

  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      mcu_out[3 * spatial_to_zigzag[j][i] + 0] = mcu_in[3 * 8 * j + 3 * i + 0];
      mcu_out[3 * spatial_to_zigzag[j][i] + 1] = mcu_in[3 * 8 * j + 3 * i + 1];
      mcu_out[3 * spatial_to_zigzag[j][i] + 2] = mcu_in[3 * 8 * j + 3 * i + 2];
    }
  }
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

void display_double(double *mcu, int stride) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      printf(" ");
      printf("%02e ", mcu[3 * 8 * j + 3 * i + stride]);
    }
    printf("\n\n");
  }
}

int main(void) {
  netpbm_image_t *image = netpbm_open("images/invader.pgm");
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

  double *mcu_dct = malloc(3 * 8 * 8 * sizeof(double));
  dct(mcu_offset, mcu_dct);
  free(mcu_offset);

  printf("---- DCT --------------------------------------------------\n");
  printf("Channel Y:\n");
  display_double(mcu_dct, 0);
  printf("Channel Cb:\n");
  display_double(mcu_dct, 1);
  printf("Channel Cr:\n");
  display_double(mcu_dct, 2);
  printf("-----------------------------------------------------------\n");

  double *mcu_zigzag = malloc(3 * 8 * 8 * sizeof(double));
  zigzag(mcu_dct, mcu_zigzag);
  free(mcu_dct);

  printf("---- Zig-Zag ----------------------------------------------\n");
  printf("Channel Y:\n");
  display_double(mcu_zigzag, 0);
  printf("Channel Cb:\n");
  display_double(mcu_zigzag, 1);
  printf("Channel Cr:\n");
  display_double(mcu_zigzag, 2);
  printf("-----------------------------------------------------------\n");

  return EXIT_SUCCESS;
}
