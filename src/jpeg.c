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
void rgb_to_ycbcr(uint8_t *mcu_in, uint8_t *mcu_out) {
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

void display(uint16_t *mcu, int stride) {
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      printf(" ");
      printf("%04x ", mcu[3 * 8 * j + 3 * i + stride]);
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

  uint16_t *data = malloc(3 * w * h * sizeof(uint16_t));

  netpbm_clone_data(image, data);
  printf("---- Initial data -----------------------------------------\n");
  printf("Channel R:\n");
  display(data, 0);
  printf("Channel G:\n");
  display(data, 1);
  printf("Channel B:\n");
  display(data, 2);
  printf("-----------------------------------------------------------\n");

  rgb_to_ycbcr(data);
  printf("---- Conversion -------------------------------------------\n");
  printf("Channel Y:\n");
  display(data, 0);
  printf("Channel Cb:\n");
  display(data, 1);
  printf("Channel Cr:\n");
  display(data, 2);
  printf("-----------------------------------------------------------\n");

  dct(data);
  printf("---- DCT --------------------------------------------------\n");
  printf("Channel Y:\n");
  display(data, 0);
  printf("Channel Cb:\n");
  display(data, 1);
  printf("Channel Cr:\n");
  display(data, 2);
  printf("-----------------------------------------------------------\n");

  netpbm_destroy(image);
  return EXIT_SUCCESS;
}
