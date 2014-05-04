
#ifndef TRANSFORM_COLOR_FORMAT_H
#define TRANSFORM_COLOR_FORMAT_H

#include "cores/allwinner/libcedarv.h"

#ifdef __cplusplus
extern "C" {
#endif

void TransformToYUVPlaner(cedarv_picture_t* pict, void* ybuf, int display_height_align, int display_width_align,
int dst_c_stride,
int dst_y_size, int dst_c_size);

void TransformToGPUBuffer (cedarv_picture_t* pict, void* ybuf);

#ifdef __cplusplus
}
#endif

#endif

