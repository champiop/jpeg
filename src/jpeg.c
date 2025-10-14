#include "netpbm.h"

int main(void) {
  netpbm_image_t *image = netpbm_create(100, 200, NETPBM_MODE_BIT, 1);
  netpbm_save(image, "test.pbm");
  netpbm_destroy(image);

  return 0;
}
