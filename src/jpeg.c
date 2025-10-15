#include "netpbm.h"

#include <stdint.h>

// Converts every sample value of a 8x8 mcu from RGB to YCbCr color format
void rgb_to_ycbcr(uint8_t *mcu) {
  for (int i = 0; i < 8; i++) {
    uint8_t r = mcu[3 * i];
    uint8_t g = mcu[3 * i + 1];
    uint8_t b = mcu[3 * i + 2];

    mcu[3 * i] = 0.299 * r + 0.587 * g + 0.114 * b;
    mcu[3 * i + 1] = -0.167 * r - 0.3313 * g - 0.5 * b + 128;
    mcu[3 * i + 2] = 0.5 * r - 0.4187 * g - 0.0813 * b + 128;
  }
}

int main(void) {
  netpbm_image_t *image = netpbm_create(100, 200, NETPBM_MODE_BIT, 1);
  netpbm_save(image, "test.pbm");
  netpbm_destroy(image);

  return 0;
}
