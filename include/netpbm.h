#ifndef NETPBM_H
#define NETPBM_H

#include <stdint.h>
#include <stdlib.h>

// Specifies what type of netpbm image is handled
// BIT      =>    Portable Bit  Map
// GRAY     =>    Portable Gray Map
// RGB      =>    Portable Pix  Map
typedef enum {
  NETPBM_MODE_UNKNOWN = 0,
  NETPBM_MODE_BIT,
  NETPBM_MODE_GRAY,
  NETPBM_MODE_RGB,
} netpbm_mode_t;

typedef enum {
  NETPBM_CHANNEL_R = 0,
  NETPBM_CHANNEL_G,
  NETPBM_CHANNEL_B,
  NETPBM_CHANNEL_GRAY,
} netpbm_channel_t;

// Netpbm image type handler
typedef struct netpbm_image netpbm_image_t;

// ==== Public API ====

// Resource management

// Creates an image of size w x h with indicated color mode and max sample
// value Returns NULL on error, or the image on success
netpbm_image_t *netpbm_create(size_t w, size_t h, netpbm_mode_t mode,
                              uint16_t max_val);

// Frees data allocated by image
void netpbm_destroy(netpbm_image_t *image);

// File handling

// Opens an existing netpbm image saved under filename
// Returns NULL on error, or the image on success
netpbm_image_t *netpbm_open(const char *filename);

// Saves the given image under filename
// Returns -1 on error, 0 on success
int netpbm_save(netpbm_image_t *image, const char *filename);

// Accessors

// Get the image width
size_t netpbm_get_width(netpbm_image_t *image);

// Get the image height
size_t netpbm_get_height(netpbm_image_t *image);

// Get the image color mode
netpbm_mode_t netpbm_get_mode(netpbm_image_t *image);

// Get the image max sample value
uint16_t netpbm_get_max_val(netpbm_image_t *image);

// Get the value at raw index idx
// Returns value of 0 on error
uint16_t netpbm_get_raw(netpbm_image_t *image, size_t idx);

// Get the value of channel c at coordinates (x, y)
// Returns value of 0 on error
uint16_t netpbm_get(netpbm_image_t *image, size_t x, size_t y,
                    netpbm_channel_t c);

// Copy image data into dst
// User must allocate the buffer first
void netpbm_clone_data(netpbm_image_t *image, uint16_t *dst);

// Modifiers

// Set the value at raw index idx
// Returns -1 on error, 0 on success
int netpbm_set_raw(netpbm_image_t *image, size_t idx, uint16_t value);

// Set the value of channel c at coordinates (x, y)
// Returns -1 on error, 0 on success
int netpbm_set(netpbm_image_t *image, size_t x, size_t y, netpbm_channel_t c,
               uint16_t value);

// Changes image mode to gray mode
// Returns -1 on error, 0 on success
int netpbm_togray(netpbm_image_t *image);

// Changes image mode to color mode
// Returns -1 on error, 0 on success
int netpbm_torgb(netpbm_image_t *image);

#endif // NETPBM_H
