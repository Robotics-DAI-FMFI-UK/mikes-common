#ifndef __XTION_H__
#define __XTION_H__

// module for Asus Xtion
//
// Field of View: 58° H, 45° V, 70° D
// Distance of Use: Between 0.8m and 3.5m
// Dimensions: 18 x 3.5 x 5 (LxWxH)

typedef short xtion_pixel;

#ifdef __cplusplus
extern "C" void init_xtion(int data_width, int data_height);
extern "C" void get_xtion_data(xtion_pixel* buffer);
#else
void init_xtion(int data_width, int data_height);
void get_xtion_data(xtion_pixel* buffer);
#endif


#endif
