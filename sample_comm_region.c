/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "sample_comm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "loadbmp.h"
#include <termios.h>
#include<sys/select.h>
#include <fcntl.h>
#include <unistd.h>


#define OVERLAY_MIN_HANDLE 0
#define OVERLAYEX_MIN_HANDLE 20
#define COVER_MIN_HANDLE 40
#define COVEREX_MIN_HANDLE 60
#define LINE_MIN_HANDLE 80
#define MOSAIC_MIN_HANDLE 100
#define MOSAICEX_MIN_HANDLE 120
#define CORNER_RECT_MIN_HANDLE 140
#define CORNER_RECTEX_MIN_HANDLE 160
#define FONT_WIDTH 16
#define FONT_HEIGHT 32
#define SAMPLE_SVP_VGS_BYTE_BIT 8

#define DEFAULT_CANVAS_NUM 2

#define BITS_NUM_PER_BYTE 8
#define BYTE_PER_PIX_1555 2
#define BYTE_PER_PIX_8888 4
#define PIX_PER_BYTE_CLUT2 4
#define PIX_PER_BYTE_CLUT4 2
#define OFFSET_NUM0 200
#define OFFSET_NUM1 160
#define OFFSET_NUM2 100
#define OFFSET_NUM3 50
#define OFFSET_NUM4 16

#define MAX_BIT_COUNT 32

#define RGN_DEFAULT_WIDTH 1920
#define RGN_DEFAULT_HEIGHT 1080
#define RGN_ALPHA 128
#define RGN_ARGB8888_BLUE 0xff0000ff
#define RGN_ARGB8888_WHITE 0xffffffff
#define RGN_RGB888_BLUE 0x0000ff
#define RGN_RGB888_RED  0xff0000

#define UART_INPUT_TTL_LEN 8


static hi_u8 sample_common_svp_vgs_font_matrixs[896] = { /* 640: 10 digits * 32 bit * 16 bit = 640 B */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x0c, 0x30, 0x18, 0x30, 0x18, 0x18,
    0x38, 0x18, 0x30, 0x1c, 0x30, 0x0c, 0x30, 0x0c, 0x70, 0x0c, 0x70, 0x0c, 0x70, 0x0c, 0x70, 0x0c,
    0x70, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x1c, 0x30, 0x18, 0x18, 0x18, 0x18, 0x18, 0x0c, 0x30,
    0x06, 0x60, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x80, 0x07, 0x80, 0x01, 0x80,
    0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
    0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
    0x07, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 1 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x18, 0x30, 0x10, 0x18, 0x30, 0x18,
    0x30, 0x1c, 0x38, 0x1c, 0x38, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x30, 0x00, 0x60, 0x00, 0x40,
    0x00, 0x80, 0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x08, 0x04, 0x10, 0x04, 0x30, 0x0c, 0x20, 0x38,
    0x3f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 2 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x18, 0x30, 0x30, 0x30, 0x30, 0x18,
    0x38, 0x18, 0x10, 0x18, 0x00, 0x18, 0x00, 0x30, 0x00, 0x60, 0x03, 0xc0, 0x01, 0xe0, 0x00, 0x30,
    0x00, 0x18, 0x00, 0x18, 0x00, 0x0c, 0x00, 0x0c, 0x30, 0x0c, 0x38, 0x1c, 0x30, 0x18, 0x30, 0x30,
    0x0c, 0x60, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 3 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x60, 0x00, 0xe0, 0x00, 0xe0,
    0x01, 0x60, 0x03, 0x60, 0x02, 0x60, 0x04, 0x60, 0x04, 0x60, 0x08, 0x60, 0x10, 0x60, 0x10, 0x60,
    0x20, 0x60, 0x60, 0x60, 0x7f, 0xfe, 0x00, 0x60, 0x00, 0x60, 0x00, 0x60, 0x00, 0x60, 0x00, 0x60,
    0x01, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 4 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x1f, 0xf8, 0x10, 0x00, 0x10, 0x00,
    0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x17, 0xe0, 0x18, 0x30, 0x10, 0x18, 0x00, 0x18,
    0x00, 0x0c, 0x00, 0x0c, 0x00, 0x0c, 0x10, 0x0c, 0x38, 0x0c, 0x30, 0x18, 0x30, 0x18, 0x10, 0x30,
    0x0c, 0x60, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 5 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x04, 0x18, 0x08, 0x18, 0x18, 0x18,
    0x10, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x33, 0xf0, 0x74, 0x38, 0x78, 0x18, 0x70, 0x0c,
    0x70, 0x0c, 0x70, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x18, 0x08, 0x1c, 0x18,
    0x0e, 0x70, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 6 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x3f, 0xfc, 0x30, 0x08, 0x20, 0x10,
    0x20, 0x10, 0x00, 0x20, 0x00, 0x20, 0x00, 0x60, 0x00, 0x40, 0x00, 0xc0, 0x00, 0x80, 0x01, 0x80,
    0x01, 0x80, 0x01, 0x80, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x07, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 7 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x18, 0x10, 0x30, 0x18, 0x30, 0x0c,
    0x30, 0x0c, 0x30, 0x0c, 0x30, 0x08, 0x38, 0x18, 0x1e, 0x30, 0x0f, 0xe0, 0x07, 0xe0, 0x18, 0xf0,
    0x30, 0x38, 0x30, 0x18, 0x60, 0x0c, 0x60, 0x0c, 0x60, 0x0c, 0x60, 0x0c, 0x30, 0x08, 0x10, 0x18,
    0x0c, 0x70, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x18, 0x30, 0x30, 0x18, 0x30, 0x18,
    0x70, 0x18, 0x60, 0x0c, 0x60, 0x0c, 0x60, 0x0c, 0x70, 0x0c, 0x30, 0x1c, 0x30, 0x2c, 0x38, 0x6c,
    0x0f, 0xcc, 0x00, 0x1c, 0x00, 0x1c, 0x00, 0x18, 0x00, 0x18, 0x10, 0x30, 0x38, 0x30, 0x38, 0x60,
    0x18, 0xc0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 9 */ 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,  /* . */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00,
    0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* : */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x06, 0x00, 0x06, 0x00, 0x04, 0x00,
    0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x08, 0x00, 0x08, 0x00, 0x18, 0x00,
    0x18, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x30, 0x00, 0x30, 0x00, 0x20, 0x00,
    0x20, 0x00, 0x60, 0x00, 0x60, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x00, 0x00, /* / */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /*   */
}; 


typedef struct {
    const hi_char *filename;
    hi_bmp bmp;
    hi_bool fil;
    hi_u32 fil_color;
    hi_pixel_format pixel_fmt;
} rgn_load_bmp_info;

static osd_color_format region_mst_get_color_format_by_pixel_format(hi_pixel_format pixel_format)
{
    switch (pixel_format) {
        case HI_PIXEL_FORMAT_ARGB_CLUT2:
        /* is the same with argb1555 */
        case HI_PIXEL_FORMAT_ARGB_CLUT4:
        /* is the same with argb1555 */
        case HI_PIXEL_FORMAT_ARGB_1555:
            return OSD_COLOR_FORMAT_RGB1555;
        case HI_PIXEL_FORMAT_ARGB_4444:
            return OSD_COLOR_FORMAT_RGB4444;
        case HI_PIXEL_FORMAT_ARGB_8888:
            return OSD_COLOR_FORMAT_RGB8888;
        default:
            printf("pixel format is not support!\n");
            return OSD_COLOR_FORMAT_BUTT;
    }
}

static hi_u8 *region_mst_get_clut2_data_from_bmp_data(hi_bmp *bmp)
{
    hi_u8 *clut_data = NULL;
    hi_u16 *temp = HI_NULL;
    hi_u8 *c_temp = HI_NULL;
    hi_u32 i, j, k;
    hi_s32 value;
    hi_u8 value_temp;

    temp = (hi_u16 *)bmp->data;

    clut_data = malloc(bmp->height * bmp->width / PIX_PER_BYTE_CLUT2);
    if (clut_data == NULL) {
        printf("malloc osd memory err!\n");
        return HI_NULL;
    }

    c_temp = (hi_u8 *)clut_data;
    for (i = 0; i < bmp->height; i++) {
        for (j = 0; j < bmp->width / PIX_PER_BYTE_CLUT2; j++) {
            value = 0;
            for (k = 0; k < PIX_PER_BYTE_CLUT2; k++) {
                value_temp = ((*temp & 0x001f) * 28 + ((*temp >> 5) & 0x001f) * 58 + /* 0x001f:28:5:58:color modulus */
                    ((*temp >> 10) & 0x001f) * 14) / 800; /* 10:0x001f:14:800:color modulus */
                value_temp = value_temp << (2 * (PIX_PER_BYTE_CLUT2 - k - 1)); /* 2:color modulus */
                value += value_temp;
                temp++;
            }
            *c_temp = value;
            c_temp++;
        }
    }

    return clut_data;
}

static hi_u8 *region_mst_get_clut4_data_from_bmp_data(hi_bmp *bmp)
{
    hi_u8 *clut_data = NULL;
    hi_u16 *temp = HI_NULL;
    hi_u8 *c_temp = HI_NULL;
    hi_u32 i, j, k;
    hi_s32 value;
    hi_u8 value_temp;

    temp = (hi_u16 *)bmp->data;

    clut_data = malloc(bmp->height * bmp->width / PIX_PER_BYTE_CLUT4);
    if (clut_data == NULL) {
        printf("malloc osd memory err!\n");
        return HI_NULL;
    }

    c_temp = (hi_u8 *)clut_data;
    for (i = 0; i < bmp->height; i++) {
        for (j = 0; j < bmp->width / PIX_PER_BYTE_CLUT4; j++) {
            value = 0;
            for (k = j; k < j + PIX_PER_BYTE_CLUT4; k++) {
                value_temp = ((*temp & 0x001f) * 28 + ((*temp >> 5) & 0x001f) * 58 + /* 0x001f:28:5:58:color modulus */
                    ((*temp >> 10) & 0x001f) * 14) / 200; /* 10:0x001f:14:200:color modulus */
                temp++;
                value = (value_temp << 4) + value_temp; /* 4:color modulus */
            }
            *c_temp = value;
            c_temp++;
        }
    }

    return clut_data;
}

hi_void fil_bmp(hi_bmp *bmp, hi_bool fil, hi_u32 fil_color)
{
    hi_u16 *temp = HI_NULL;
    hi_u32 i, j;

    if (!fil) {
        return;
    }

    temp = (hi_u16 *)bmp->data;
    for (i = 0; i < bmp->height; i++) {
        for (j = 0; j < bmp->width; j++) {
            if (*temp == fil_color) {
                *temp &= 0x7FFF;
            }
            temp++;
        }
    }
    return;
}

hi_s32 region_mst_load_bmp(rgn_load_bmp_info *load_bmp_info)
{
    osd_surface surface;
    osd_bit_map_file_header bmp_file_header;
    osd_bit_map_info bmp_info;
    hi_u32 bpp;
    hi_u8 *clut_data = NULL;
    if (get_bmp_info(load_bmp_info->filename, &bmp_file_header, &bmp_info) < 0) {
        printf("get_bmp_info err!\n");
        return HI_FAILURE;
    }

    if (bmp_info.bmp_header.bi_bit_count > MAX_BIT_COUNT || bmp_info.bmp_header.bi_width > HI_RGN_OVERLAY_MAX_WIDTH ||
        bmp_info.bmp_header.bi_height > HI_RGN_OVERLAY_MAX_HEIGHT) {
        printf("bmp info error!");
        return HI_FAILURE;
    }
    surface.color_format = region_mst_get_color_format_by_pixel_format(load_bmp_info->pixel_fmt);
    if (surface.color_format == OSD_COLOR_FORMAT_BUTT) {
        return HI_FAILURE;
    }
    bpp = bmp_info.bmp_header.bi_bit_count / BITS_NUM_PER_BYTE;
    load_bmp_info->bmp.data = malloc(bmp_info.bmp_header.bi_width * bpp * abs(bmp_info.bmp_header.bi_height));
    if (load_bmp_info->bmp.data == NULL) {
        printf("malloc osd memory err!\n");
        return HI_FAILURE;
    }

    create_surface_by_bit_map(load_bmp_info->filename, &surface, (hi_u8 *)(load_bmp_info->bmp.data));
    load_bmp_info->bmp.width = surface.width;
    load_bmp_info->bmp.height = surface.height;
    load_bmp_info->bmp.pixel_format = load_bmp_info->pixel_fmt;

    if (load_bmp_info->pixel_fmt == HI_PIXEL_FORMAT_ARGB_CLUT2) {
        clut_data = region_mst_get_clut2_data_from_bmp_data(&load_bmp_info->bmp);
    } else if (load_bmp_info->pixel_fmt == HI_PIXEL_FORMAT_ARGB_CLUT4) {
        clut_data = region_mst_get_clut4_data_from_bmp_data(&load_bmp_info->bmp);
    } else {
        fil_bmp(&load_bmp_info->bmp, load_bmp_info->fil, load_bmp_info->fil_color);
    }
    if (clut_data != NULL) {
        free(load_bmp_info->bmp.data);
        load_bmp_info->bmp.data = clut_data;
    }
    return HI_SUCCESS;
}

hi_s32 region_mst_update_canvas(rgn_load_bmp_info *load_bmp_info, hi_size *size, hi_u32 stride)
{
    osd_surface surface;
    osd_bit_map_file_header bmp_file_header;
    osd_bit_map_info bmp_info;
    canvas_size_info canvas_size;

    if (get_bmp_info(load_bmp_info->filename, &bmp_file_header, &bmp_info) < 0) {
        printf("get_bmp_info err!\n");
        return HI_FAILURE;
    }

    if (HI_PIXEL_FORMAT_ARGB_1555 == load_bmp_info->pixel_fmt) {
        surface.color_format = OSD_COLOR_FORMAT_RGB1555;
    } else if (HI_PIXEL_FORMAT_ARGB_4444 == load_bmp_info->pixel_fmt) {
        surface.color_format = OSD_COLOR_FORMAT_RGB4444;
    } else if (HI_PIXEL_FORMAT_ARGB_8888 == load_bmp_info->pixel_fmt) {
        surface.color_format = OSD_COLOR_FORMAT_RGB8888;
    } else if (HI_PIXEL_FORMAT_ARGB_CLUT2 == load_bmp_info->pixel_fmt) {
        surface.color_format = OSD_COLOR_FORMAT_CLUT4;
    } else if (HI_PIXEL_FORMAT_ARGB_CLUT4 == load_bmp_info->pixel_fmt) {
        surface.color_format = OSD_COLOR_FORMAT_CLUT4;
    } else {
        printf("pixel format is not support!\n");
        return HI_FAILURE;
    }

    if (load_bmp_info->bmp.data == NULL) {
        printf("malloc osd memory err!\n");
        return HI_FAILURE;
    }

    canvas_size.width = size->width;
    canvas_size.height = size->height;
    canvas_size.stride = stride;
    create_surface_by_canvas(load_bmp_info->filename, &surface, (hi_u8 *)(load_bmp_info->bmp.data), &canvas_size);

    load_bmp_info->bmp.width = surface.width;
    load_bmp_info->bmp.height = surface.height;

    if (HI_PIXEL_FORMAT_ARGB_1555 == load_bmp_info->pixel_fmt) {
        load_bmp_info->bmp.pixel_format = HI_PIXEL_FORMAT_ARGB_1555;
    } else if (HI_PIXEL_FORMAT_ARGB_4444 == load_bmp_info->pixel_fmt) {
        load_bmp_info->bmp.pixel_format = HI_PIXEL_FORMAT_ARGB_4444;
    } else if (HI_PIXEL_FORMAT_ARGB_8888 == load_bmp_info->pixel_fmt) {
        load_bmp_info->bmp.pixel_format = HI_PIXEL_FORMAT_ARGB_8888;
    } else if (HI_PIXEL_FORMAT_ARGB_CLUT2 == load_bmp_info->pixel_fmt) {
        load_bmp_info->bmp.pixel_format = HI_PIXEL_FORMAT_ARGB_CLUT2;
    } else if (HI_PIXEL_FORMAT_ARGB_CLUT4 == load_bmp_info->pixel_fmt) {
        load_bmp_info->bmp.pixel_format = HI_PIXEL_FORMAT_ARGB_CLUT4;
    }

    return HI_SUCCESS;
}

hi_s32 sample_comm_region_get_min_handle(hi_rgn_type type)
{
    hi_s32 min_handle;
    switch (type) {
        case HI_RGN_OVERLAY:
            min_handle = OVERLAY_MIN_HANDLE;
            break;
        case HI_RGN_OVERLAYEX:
            min_handle = OVERLAYEX_MIN_HANDLE;
            break;
        case HI_RGN_COVER:
            min_handle = COVER_MIN_HANDLE;
            break;
        case HI_RGN_COVEREX:
            min_handle = COVEREX_MIN_HANDLE;
            break;
        case HI_RGN_LINEEX:
            min_handle = LINE_MIN_HANDLE;
            break;
        case HI_RGN_MOSAIC:
            min_handle = MOSAIC_MIN_HANDLE;
            break;
        case HI_RGN_MOSAICEX:
            min_handle = MOSAICEX_MIN_HANDLE;
            break;
        case HI_RGN_CORNER_RECT:
            min_handle = CORNER_RECT_MIN_HANDLE;
            break;
        case HI_RGN_CORNER_RECTEX:
            min_handle = CORNER_RECTEX_MIN_HANDLE;
            break;
        default:
            min_handle = -1;
            break;
    }
    return min_handle;
}

hi_s32 sample_region_create_overlay(hi_s32 handle_num)
{
    hi_s32 ret;
    hi_s32 i;
    hi_rgn_attr region;

    region.type = HI_RGN_OVERLAY;
    region.attr.overlay.pixel_format = HI_PIXEL_FORMAT_ARGB_4444;
    // region.attr.overlay.pixel_format = HI_PIXEL_FORMAT_ARGB_CLUT2;
    // region.attr.overlay.pixel_format = HI_PIXEL_FORMAT_ARGB_CLUT4;
    // region.attr.overlay.pixel_format = HI_PIXEL_FORMAT_ARGB_4444;

    // region.attr.overlay.size.height = 1080;
    // region.attr.overlay.size.width = 1920;
    region.attr.overlay.size.height = RGN_DEFAULT_HEIGHT;
    region.attr.overlay.size.width = RGN_DEFAULT_WIDTH;
    region.attr.overlay.bg_color = 0x000000000000000;
    
    region.attr.overlay.canvas_num = DEFAULT_CANVAS_NUM;
    for (i = 0; i < HI_RGN_CLUT_NUM; i++) {
        region.attr.overlay.clut[i] = 0x000fff0f * (i + 1) * (i + 1);
        region.attr.overlay.clut[i] |= 0xff000000;
    }

    if (handle_num <= 8) { /* 2 :two region */
        for (i = OVERLAY_MIN_HANDLE; i < OVERLAY_MIN_HANDLE + handle_num; i++) {
            ret = hi_mpi_rgn_create(i, &region);
        }
    }
    else {
        ret = HI_FALSE;
    }
    if (ret != HI_SUCCESS) {
        sample_print("hi_mpi_rgn_create failed with %#x!\n", ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

hi_s32 sample_region_create_overlayex(hi_s32 handle_num)
{
    hi_s32 ret;
    hi_s32 i;
    hi_rgn_attr region;

    region.type = HI_RGN_OVERLAYEX;
    region.attr.overlayex.pixel_format = HI_PIXEL_FORMAT_ARGB_8888;
    region.attr.overlayex.size.height = RGN_DEFAULT_HEIGHT;
    region.attr.overlayex.size.width = RGN_DEFAULT_WIDTH;
    region.attr.overlayex.bg_color = 0;
    region.attr.overlayex.canvas_num = DEFAULT_CANVAS_NUM;
    for (i = OVERLAYEX_MIN_HANDLE; i < OVERLAYEX_MIN_HANDLE + handle_num; i++) {
        ret = hi_mpi_rgn_create(i, &region);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_create failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

hi_s32 sample_region_create_cover(hi_s32 handle_num)
{
    hi_s32 ret;
    hi_s32 i;
    hi_rgn_attr region;

    region.type = HI_RGN_COVER;

    for (i = COVER_MIN_HANDLE; i < COVER_MIN_HANDLE + handle_num; i++) {
        ret = hi_mpi_rgn_create(i, &region);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_create failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

hi_s32 sample_region_create_coverex(hi_s32 handle_num)
{
    hi_s32 ret;
    hi_s32 i;
    hi_rgn_attr region;

    region.type = HI_RGN_COVEREX;

    for (i = COVEREX_MIN_HANDLE; i < COVEREX_MIN_HANDLE + handle_num; i++) {
        ret = hi_mpi_rgn_create(i, &region);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_create failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

hi_s32 sample_region_create_line(hi_s32 handle_num)
{
    hi_s32 ret;
    hi_s32 i;
    hi_rgn_attr region;

    region.type = HI_RGN_LINEEX;

    for (i = LINE_MIN_HANDLE; i < LINE_MIN_HANDLE + handle_num; i++) {
        ret = hi_mpi_rgn_create(i, &region);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_create failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}


hi_s32 sample_region_create_mosaic(hi_s32 handle_num)
{
    hi_s32 ret;
    hi_s32 i;
    hi_rgn_attr region;

    region.type = HI_RGN_MOSAIC;

    for (i = MOSAIC_MIN_HANDLE; i < MOSAIC_MIN_HANDLE + handle_num; i++) {
        ret = hi_mpi_rgn_create(i, &region);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_create failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

hi_s32 sample_region_create_mosaicex(hi_s32 handle_num)
{
    hi_s32 ret;
    hi_s32 i;
    hi_rgn_attr region;

    region.type = HI_RGN_MOSAICEX;

    for (i = MOSAICEX_MIN_HANDLE; i < MOSAICEX_MIN_HANDLE + handle_num; i++) {
        ret = hi_mpi_rgn_create(i, &region);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_create failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

hi_s32 sample_region_create_corner_rect(hi_s32 handle_num)
{
    hi_s32 ret;
    hi_s32 i;
    hi_rgn_attr region;

    region.type = HI_RGN_CORNER_RECT;

    for (i = CORNER_RECT_MIN_HANDLE; i < CORNER_RECT_MIN_HANDLE + handle_num; i++) {
        ret = hi_mpi_rgn_create(i, &region);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_create failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

hi_s32 sample_region_create_corner_rectex(hi_s32 handle_num)
{
    hi_s32 ret;
    hi_s32 i;
    hi_rgn_attr region;

    region.type = HI_RGN_CORNER_RECTEX;

    for (i = CORNER_RECTEX_MIN_HANDLE; i < CORNER_RECTEX_MIN_HANDLE + handle_num; i++) {
        ret = hi_mpi_rgn_create(i, &region);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_create failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

hi_s32 sample_region_destroy(hi_rgn_handle handle)
{
    hi_s32 ret;

    ret = hi_mpi_rgn_destroy(handle);
    if (ret != HI_SUCCESS) {
        sample_print("hi_mpi_rgn_destroy failed with %#x!\n", ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 sample_region_attach(hi_rgn_handle handle, hi_mpp_chn *chn, hi_rgn_chn_attr *chn_attr,
    region_op_flag op_flag)
{
    hi_s32 ret;

    if (op_flag & REGION_OP_CHN) {
        ret = hi_mpi_rgn_attach_to_chn(handle, chn, chn_attr);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_attach_to_chn failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    } else if (op_flag & REGION_OP_DEV) {
        ret = hi_mpi_rgn_attach_to_dev(handle, chn, chn_attr);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_attach_to_dev failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

hi_s32 sample_region_detach(hi_rgn_handle handle, hi_mpp_chn *chn, region_op_flag op_flag)
{
    hi_s32 ret;

    if (op_flag & REGION_OP_CHN) {
        ret = hi_mpi_rgn_detach_from_chn(handle, chn);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_detach_from_chn failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    } else if (op_flag & REGION_OP_DEV) {
        ret = hi_mpi_rgn_detach_from_dev(handle, chn);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_detach_from_dev failed with %#x!\n", ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

hi_s32 sample_region_set_bit_map(hi_rgn_handle handle, hi_bmp *bitmap)
{
    hi_s32 ret;
    ret = hi_mpi_rgn_set_bmp(handle, bitmap);
    if (ret != HI_SUCCESS) {
        sample_print("hi_mpi_rgn_set_bit_map failed with %#x!\n", ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

hi_s32 sample_region_get_up_canvas_info(hi_rgn_handle handle, hi_rgn_canvas_info *canvas_info)
{
    hi_s32 ret;
    ret = hi_mpi_rgn_get_canvas_info(handle, canvas_info);
    if (ret != HI_SUCCESS) {
        sample_print("hi_mpi_rgn_get_canvas_info failed with %#x!\n", ret);
        return HI_FAILURE;
    }

    ret = hi_mpi_rgn_update_canvas(handle);
    if (ret != HI_SUCCESS) {
        sample_print("hi_mpi_rgn_update_canvas failed with %#x!\n", ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

hi_s32 sample_comm_region_create(hi_s32 handle_num, hi_rgn_type type)
{
    hi_s32 ret = HI_SUCCESS;
    if (handle_num <= 0 || handle_num > 16) { /* 16:max_num */
        sample_print("handle_num is illegal %d!\n", handle_num);
        return HI_FAILURE;
    }
    if (type < 0 || type >= HI_RGN_BUTT) {
        sample_print("type is illegal %d!\n", type);
        return HI_FAILURE;
    }
    switch (type) {
        case HI_RGN_OVERLAY:
            ret = sample_region_create_overlay(handle_num);
            break;
        case HI_RGN_OVERLAYEX:
            ret = sample_region_create_overlayex(handle_num);
            break;
        case HI_RGN_COVER:
            ret = sample_region_create_cover(handle_num);
            break;
        case HI_RGN_COVEREX:
            ret = sample_region_create_coverex(handle_num);
            break;
        case HI_RGN_LINEEX:
            ret = sample_region_create_line(handle_num);
            break;
        case HI_RGN_MOSAIC:
            ret = sample_region_create_mosaic(handle_num);
            break;
        case HI_RGN_MOSAICEX:
            ret = sample_region_create_mosaicex(handle_num);
            break;
        case HI_RGN_CORNER_RECT:
            ret = sample_region_create_corner_rect(handle_num);
            break;
        case HI_RGN_CORNER_RECTEX:
            ret = sample_region_create_corner_rectex(handle_num);
            break;
        default:
            break;
    }
    if (ret != HI_SUCCESS) {
        sample_print("sample_comm_region_create failed! handle_num%d,type:%d!\n", handle_num, type);
        return HI_FAILURE;
    }
    return ret;
}

hi_s32 sample_comm_region_destroy(hi_s32 handle_num, hi_rgn_type type)
{
    hi_s32 i;
    hi_s32 ret;
    hi_s32 min_handle;

    if (handle_num <= 0 || handle_num > 16) { /* 16 max_num */
        sample_print("handle_num is illegal %d!\n", handle_num);
        return HI_FAILURE;
    }
    if (type < 0 || type >= HI_RGN_BUTT) {
        sample_print("type is illegal %d!\n", type);
        return HI_FAILURE;
    }
    min_handle = sample_comm_region_get_min_handle(type);
    for (i = min_handle; i < min_handle + handle_num; i++) {
        ret = sample_region_destroy(i);
        if (ret != HI_SUCCESS) {
            sample_print("sample_comm_region_destroy failed!\n");
        }
    }
    return HI_SUCCESS;
}

hi_s32 sample_comm_region_attach_check(hi_s32 handle_num, hi_rgn_type type, hi_mpp_chn *mpp_chn)
{
    if (handle_num <= 0 || handle_num > 16) { /* 16 max_num */
        sample_print("handle_num is illegal %d!\n", handle_num);
        return HI_FAILURE;
    }
    if (type < 0 || type >= HI_RGN_BUTT) {
        sample_print("type is illegal %d!\n", type);
        return HI_FAILURE;
    }
    if (mpp_chn == HI_NULL) {
        sample_print("mpp_chn is NULL !\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

#define rgn_check_handle_min_ret(handle, min_value) \
    do { \
        if ((handle) < (min_value)) { \
            sample_print("handle(%d) invalid\n", (handle)); \
            return; \
        } \
    } while (0)

static hi_void sample_region_get_overlay_chn_attr(hi_s32 handle, hi_rgn_overlay_chn_attr *overlay_chn)
{
    rgn_check_handle_min_ret(handle, OVERLAY_MIN_HANDLE);

    overlay_chn->bg_alpha = 0;
    overlay_chn->fg_alpha = 0;
    overlay_chn->qp_info.enable = HI_TRUE;
    overlay_chn->qp_info.is_abs_qp = HI_TRUE;
    overlay_chn->qp_info.qp_val = 0;
    overlay_chn->dst = HI_RGN_ATTACH_JPEG_MAIN;
    overlay_chn->point.x = 0;
    overlay_chn->point.y = 0;
    overlay_chn->layer = handle - OVERLAY_MIN_HANDLE;
}

hi_s32 sample_region_set_overlay_chn_attr(hi_s32 handle_num,
    hi_rgn_chn_attr *chn_attr, hi_mpp_chn *mpp_chn, region_op_flag op_flag)
{
    hi_s32 i;
    hi_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = HI_RGN_OVERLAY;

    for (i = OVERLAY_MIN_HANDLE; i < OVERLAY_MIN_HANDLE + handle_num; i++) {
        sample_region_get_overlay_chn_attr(i, &chn_attr->attr.overlay_chn);
        ret = sample_region_attach(i, mpp_chn, chn_attr, op_flag);
        if (ret != HI_SUCCESS) {
            sample_print("sample_region_attach failed!\n");
            sample_comm_region_detach(i - OVERLAY_MIN_HANDLE + 1, HI_RGN_OVERLAY, mpp_chn, op_flag);
            return ret;
        }
    }
    return HI_SUCCESS;
}

static hi_void sample_region_get_overlayex_chn_attr(hi_s32 handle,
    hi_rgn_overlayex_chn_attr *overlayex_chn)
{
    rgn_check_handle_min_ret(handle, OVERLAYEX_MIN_HANDLE);

    overlayex_chn->bg_alpha = RGN_ALPHA;
    overlayex_chn->fg_alpha = RGN_ALPHA;
    overlayex_chn->point.x = 0;
    overlayex_chn->point.y = 0;
    overlayex_chn->layer = handle - OVERLAYEX_MIN_HANDLE;
}

hi_s32 sample_region_set_overlayex_chn_attr(hi_s32 handle_num,
    hi_rgn_chn_attr *chn_attr, hi_mpp_chn *mpp_chn, region_op_flag op_flag)
{
    hi_s32 i;
    hi_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = HI_RGN_OVERLAYEX;

    for (i = OVERLAYEX_MIN_HANDLE; i < OVERLAYEX_MIN_HANDLE + handle_num; i++) {
        sample_region_get_overlayex_chn_attr(i, &chn_attr->attr.overlayex_chn);
        ret = sample_region_attach(i, mpp_chn, chn_attr, op_flag);
        if (ret != HI_SUCCESS) {
            sample_print("sample_region_attach failed!\n");
            sample_comm_region_detach(i - OVERLAYEX_MIN_HANDLE + 1, HI_RGN_OVERLAYEX, mpp_chn, op_flag);
            return ret;
        }
    }
    return HI_SUCCESS;
}

static hi_void sample_region_get_rect_cover_chn_attr(hi_s32 handle, hi_rgn_cover_chn_attr *cover_chn)
{
    rgn_check_handle_min_ret(handle, COVER_MIN_HANDLE);

    cover_chn->coord = HI_COORD_ABS;
    cover_chn->layer = handle - COVER_MIN_HANDLE;

    cover_chn->cover.type = HI_COVER_RECT;
    cover_chn->cover.color = RGN_RGB888_BLUE;
    cover_chn->cover.rect_attr.rect.height = RGN_DEFAULT_HEIGHT;
    cover_chn->cover.rect_attr.rect.width = RGN_DEFAULT_WIDTH;
    cover_chn->cover.rect_attr.rect.x = OFFSET_NUM0 * (handle - COVER_MIN_HANDLE) + OFFSET_NUM0;
    cover_chn->cover.rect_attr.rect.y = OFFSET_NUM0;
    cover_chn->cover.rect_attr.is_solid = HI_TRUE;
}

hi_s32 sample_region_set_cover_chn_attr(hi_s32 handle_num,
    hi_rgn_chn_attr *chn_attr, hi_mpp_chn *mpp_chn, region_op_flag op_flag)
{
    hi_s32 i;
    hi_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = HI_RGN_COVER;

    for (i = COVER_MIN_HANDLE; i < COVER_MIN_HANDLE + handle_num; i++) {
        sample_region_get_rect_cover_chn_attr(i, &chn_attr->attr.cover_chn);
        ret = sample_region_attach(i, mpp_chn, chn_attr, op_flag);
        if (ret != HI_SUCCESS) {
            sample_print("sample_region_attach failed!\n");
            sample_comm_region_detach(i - COVER_MIN_HANDLE + 1, HI_RGN_COVER, mpp_chn, op_flag);
            return ret;
        }
    }
    return HI_SUCCESS;
}

static hi_void sample_region_get_quad_coverex_chn_attr(hi_s32 handle,
    hi_rgn_coverex_chn_attr *coverex_chn)
{
    rgn_check_handle_min_ret(handle, COVEREX_MIN_HANDLE);

    coverex_chn->coord = HI_COORD_ABS;
    coverex_chn->layer = handle - COVEREX_MIN_HANDLE;
    coverex_chn->coverex.type = HI_COVER_QUAD;
    coverex_chn->coverex.color = RGN_RGB888_RED;
    coverex_chn->coverex.quad_attr.is_solid = handle % 2; /* 2:solid or not */
    coverex_chn->coverex.quad_attr.thick = HI_RGN_COVEREX_MIN_THICK + (handle - COVEREX_MIN_HANDLE) %
        (HI_RGN_COVEREX_MAX_THICK - HI_RGN_COVEREX_MIN_THICK + 1);
    coverex_chn->coverex.quad_attr.point[0].x = OFFSET_NUM0 * (handle - COVEREX_MIN_HANDLE);
    coverex_chn->coverex.quad_attr.point[0].y = OFFSET_NUM3 + OFFSET_NUM3;
    coverex_chn->coverex.quad_attr.point[1].x = OFFSET_NUM0 * (handle - COVEREX_MIN_HANDLE) + OFFSET_NUM2;
    coverex_chn->coverex.quad_attr.point[1].y = OFFSET_NUM3;
    coverex_chn->coverex.quad_attr.point[2].x = /* 2:point num */
        OFFSET_NUM0 * (handle - COVEREX_MIN_HANDLE) + OFFSET_NUM0;
    coverex_chn->coverex.quad_attr.point[2].y = OFFSET_NUM3 + OFFSET_NUM3; /* 2:point num */
    coverex_chn->coverex.quad_attr.point[3].x =                            /* 3:point num */
        OFFSET_NUM0 * (handle - COVEREX_MIN_HANDLE) + OFFSET_NUM2;
    coverex_chn->coverex.quad_attr.point[3].y = OFFSET_NUM3 + OFFSET_NUM2; /* 3:point num */
}

static hi_void sample_region_get_rect_coverex_chn_attr(hi_s32 handle,
    hi_rgn_coverex_chn_attr *coverex_chn)
{
    rgn_check_handle_min_ret(handle, COVEREX_MIN_HANDLE);

    coverex_chn->coord = HI_COORD_ABS;
    coverex_chn->layer = handle - COVEREX_MIN_HANDLE;
    coverex_chn->coverex.type = HI_COVER_RECT;
    coverex_chn->coverex.color = RGN_RGB888_BLUE;
    coverex_chn->coverex.rect_attr.rect.height = RGN_DEFAULT_HEIGHT;
    coverex_chn->coverex.rect_attr.rect.width = RGN_DEFAULT_WIDTH;
    coverex_chn->coverex.rect_attr.rect.x = OFFSET_NUM0 * (handle - COVEREX_MIN_HANDLE) + OFFSET_NUM0;
    coverex_chn->coverex.rect_attr.rect.y = OFFSET_NUM0;
}

hi_s32 sample_region_set_coverex_chn_attr(hi_s32 handle_num,
    hi_rgn_chn_attr *chn_attr, hi_mpp_chn *mpp_chn, region_op_flag op_flag)
{
    hi_s32 i;
    hi_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = HI_RGN_COVEREX;

    for (i = COVEREX_MIN_HANDLE; i < COVEREX_MIN_HANDLE + handle_num; i++) {
        if (i - COVEREX_MIN_HANDLE < 4) { /* 4:quad num */
            sample_region_get_quad_coverex_chn_attr(i, &chn_attr->attr.coverex_chn);
        } else {
            sample_region_get_rect_coverex_chn_attr(i, &chn_attr->attr.coverex_chn);
        }
        ret = sample_region_attach(i, mpp_chn, chn_attr, op_flag);
        if (ret != HI_SUCCESS) {
            sample_print("sample_region_attach failed!\n");
            sample_comm_region_detach(i - COVEREX_MIN_HANDLE + 1, HI_RGN_COVEREX, mpp_chn, op_flag);
            return ret;
        }
    }
    return HI_SUCCESS;
}

static hi_void sample_region_get_corner_rect_chn_attr(hi_s32 handle,
    hi_rgn_corner_rect_chn_attr *corner_rect_chn)
{
    rgn_check_handle_min_ret(handle, CORNER_RECT_MIN_HANDLE);

    corner_rect_chn->corner_rect.rect.height = RGN_DEFAULT_HEIGHT;
    corner_rect_chn->corner_rect.rect.width = RGN_DEFAULT_WIDTH;
    corner_rect_chn->corner_rect.thick = HI_RGN_CORNER_RECT_MIN_THICK;
    corner_rect_chn->corner_rect.hor_len = OFFSET_NUM4;
    corner_rect_chn->corner_rect.ver_len = OFFSET_NUM4;
    corner_rect_chn->corner_rect_attr.color = RGN_RGB888_RED;
    corner_rect_chn->corner_rect_attr.corner_rect_type = HI_CORNER_RECT_TYPE_CORNER;
    corner_rect_chn->layer = handle - CORNER_RECT_MIN_HANDLE;
    corner_rect_chn->corner_rect.rect.x = OFFSET_NUM0 * (handle - CORNER_RECT_MIN_HANDLE) + OFFSET_NUM3;
    corner_rect_chn->corner_rect.rect.y = OFFSET_NUM2 * (handle - CORNER_RECT_MIN_HANDLE) + OFFSET_NUM3;
}

static hi_void sample_region_get_corner_rectex_chn_attr(hi_s32 handle,
    hi_rgn_corner_rectex_chn_attr *corner_rectex_chn)
{
    rgn_check_handle_min_ret(handle, CORNER_RECTEX_MIN_HANDLE);

    corner_rectex_chn->corner_rect.rect.height = RGN_DEFAULT_HEIGHT;
    corner_rectex_chn->corner_rect.rect.width = RGN_DEFAULT_WIDTH;
    corner_rectex_chn->corner_rect.thick = HI_RGN_CORNER_RECTEX_MIN_THICK;
    corner_rectex_chn->corner_rect.hor_len = OFFSET_NUM4;
    corner_rectex_chn->corner_rect.ver_len = OFFSET_NUM4;
    corner_rectex_chn->corner_rect_attr.color = RGN_RGB888_RED;
    corner_rectex_chn->corner_rect_attr.corner_rect_type = HI_CORNER_RECT_TYPE_CORNER;
    corner_rectex_chn->layer = handle - CORNER_RECTEX_MIN_HANDLE;
    corner_rectex_chn->corner_rect.rect.x = OFFSET_NUM0 * (handle - CORNER_RECTEX_MIN_HANDLE) + OFFSET_NUM3;
    corner_rectex_chn->corner_rect.rect.y = OFFSET_NUM2 * (handle - CORNER_RECTEX_MIN_HANDLE) + OFFSET_NUM3;
}

hi_s32 sample_region_set_corner_rect_chn_attr(hi_s32 handle_num,
    hi_rgn_chn_attr *chn_attr, hi_mpp_chn *mpp_chn, region_op_flag op_flag)
{
    hi_s32 i;
    hi_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = HI_RGN_CORNER_RECT;

    for (i = CORNER_RECT_MIN_HANDLE; i < CORNER_RECT_MIN_HANDLE + handle_num; i++) {
        sample_region_get_corner_rect_chn_attr(i, &chn_attr->attr.corner_rect_chn);
        if (i - CORNER_RECT_MIN_HANDLE < 4) { /* 4:corner type number */
            chn_attr->attr.corner_rect_chn.corner_rect_attr.corner_rect_type = HI_CORNER_RECT_TYPE_CORNER;
        } else {
            chn_attr->attr.corner_rect_chn.corner_rect_attr.corner_rect_type = HI_CORNER_RECT_TYPE_FULL_LINE;
        }
        ret = sample_region_attach(i, mpp_chn, chn_attr, op_flag);
        if (ret != HI_SUCCESS) {
            sample_print("sample_region_attach failed!\n");
            sample_comm_region_detach(i - CORNER_RECT_MIN_HANDLE + 1, HI_RGN_CORNER_RECT, mpp_chn, op_flag);
            return ret;
        }
    }
    return HI_SUCCESS;
}

hi_s32 sample_region_set_corner_rectex_chn_attr(hi_s32 handle_num,
    hi_rgn_chn_attr *chn_attr, hi_mpp_chn *mpp_chn, region_op_flag op_flag)
{
    hi_s32 i;
    hi_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = HI_RGN_CORNER_RECTEX;

    for (i = CORNER_RECTEX_MIN_HANDLE; i < CORNER_RECTEX_MIN_HANDLE + handle_num; i++) {
        sample_region_get_corner_rectex_chn_attr(i, &chn_attr->attr.corner_rectex_chn);
        if (i - CORNER_RECTEX_MIN_HANDLE < 4) { /* 4:corner type number */
            chn_attr->attr.corner_rectex_chn.corner_rect_attr.corner_rect_type = HI_CORNER_RECT_TYPE_CORNER;
        } else {
            chn_attr->attr.corner_rectex_chn.corner_rect_attr.corner_rect_type = HI_CORNER_RECT_TYPE_FULL_LINE;
        }
        ret = sample_region_attach(i, mpp_chn, chn_attr, op_flag);
        if (ret != HI_SUCCESS) {
            sample_print("sample_region_attach failed!\n");
            sample_comm_region_detach(i - CORNER_RECTEX_MIN_HANDLE + 1, HI_RGN_CORNER_RECTEX, mpp_chn, op_flag);
            return ret;
        }
    }
    return HI_SUCCESS;
}

static hi_void sample_region_get_line_chn_attr(hi_s32 handle, hi_rgn_lineex_chn_attr *line_chn)
{
    rgn_check_handle_min_ret(handle, LINE_MIN_HANDLE);

    line_chn->color = RGN_RGB888_BLUE;
    line_chn->points[0].x = OFFSET_NUM0 + OFFSET_NUM2 * (handle - LINE_MIN_HANDLE);
    line_chn->points[0].y = OFFSET_NUM0 + OFFSET_NUM0 * (handle - LINE_MIN_HANDLE);
    line_chn->points[1].x = OFFSET_NUM3 + OFFSET_NUM0 * (handle - LINE_MIN_HANDLE);
    line_chn->points[1].y = OFFSET_NUM3 + OFFSET_NUM2 * (handle - LINE_MIN_HANDLE);
    line_chn->thick = ((handle - LINE_MIN_HANDLE + HI_RGN_LINE_MIN_THICK) * 2) % HI_RGN_LINE_MAX_THICK; /* align 2 */
}

static hi_s32 sample_region_set_lineex_chn_attr(hi_s32 handle_num,
    hi_rgn_chn_attr *chn_attr, hi_mpp_chn *mpp_chn, region_op_flag op_flag)
{
    hi_s32 i;
    hi_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = HI_RGN_LINEEX;

    for (i = LINE_MIN_HANDLE; i < LINE_MIN_HANDLE + handle_num; i++) {
        sample_region_get_line_chn_attr(i, &chn_attr->attr.lineex_chn);
        ret = sample_region_attach(i, mpp_chn, chn_attr, op_flag);
        if (ret != HI_SUCCESS) {
            sample_print("sample_region_attach failed!\n");
            sample_comm_region_detach(i - LINE_MIN_HANDLE + 1, HI_RGN_LINEEX, mpp_chn, op_flag);
            return ret;
        }
    }
    return HI_SUCCESS;
}

static hi_void sample_region_get_mosaic_chn_attr(hi_s32 handle, hi_rgn_mosaic_chn_attr *mosaic_chn)
{
    rgn_check_handle_min_ret(handle, MOSAIC_MIN_HANDLE);

    mosaic_chn->blk_size = HI_MOSAIC_BLK_SIZE_32;
    mosaic_chn->rect.height = OFFSET_NUM0;
    mosaic_chn->rect.width = OFFSET_NUM0;
    mosaic_chn->rect.x = OFFSET_NUM0 * (handle - MOSAIC_MIN_HANDLE);
    mosaic_chn->rect.y = OFFSET_NUM0 * (handle - MOSAIC_MIN_HANDLE);
    mosaic_chn->layer = handle - MOSAIC_MIN_HANDLE;
}

static hi_s32 sample_region_set_mosaic_chn_attr(hi_s32 handle_num,
    hi_rgn_chn_attr *chn_attr, hi_mpp_chn *mpp_chn, region_op_flag op_flag)
{
    hi_s32 i;
    hi_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = HI_RGN_MOSAIC;

    for (i = MOSAIC_MIN_HANDLE; i < MOSAIC_MIN_HANDLE + handle_num; i++) {
        sample_region_get_mosaic_chn_attr(i, &chn_attr->attr.mosaic_chn);
        ret = sample_region_attach(i, mpp_chn, chn_attr, op_flag);
        if (ret != HI_SUCCESS) {
            sample_print("sample_region_attach failed!\n");
            sample_comm_region_detach(i - MOSAIC_MIN_HANDLE + 1, HI_RGN_MOSAIC, mpp_chn, op_flag);
            return ret;
        }
    }
    return HI_SUCCESS;
}

static hi_void sample_region_get_mosaicex_chn_attr(hi_s32 handle, hi_rgn_mosaicex_chn_attr *mosaicex_chn)
{
    rgn_check_handle_min_ret(handle, MOSAICEX_MIN_HANDLE);

    mosaicex_chn->blk_size = HI_MOSAIC_BLK_SIZE_16 + (handle - MOSAICEX_MIN_HANDLE) %
        (HI_MOSAIC_BLK_SIZE_64 - HI_MOSAIC_BLK_SIZE_16 + 1);
    mosaicex_chn->rect.height = OFFSET_NUM0;
    mosaicex_chn->rect.width = OFFSET_NUM0;
    mosaicex_chn->rect.x = OFFSET_NUM0 * (handle - MOSAICEX_MIN_HANDLE);
    mosaicex_chn->rect.y = OFFSET_NUM0 * (handle - MOSAICEX_MIN_HANDLE);
    mosaicex_chn->layer = handle - MOSAICEX_MIN_HANDLE;
}

hi_s32 sample_region_set_mosaicex_chn_attr(hi_s32 handle_num,
    hi_rgn_chn_attr *chn_attr, hi_mpp_chn *mpp_chn, region_op_flag op_flag)
{
    hi_s32 i;
    hi_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = HI_RGN_MOSAICEX;

    for (i = MOSAICEX_MIN_HANDLE; i < MOSAICEX_MIN_HANDLE + handle_num; i++) {
        sample_region_get_mosaicex_chn_attr(i, &chn_attr->attr.mosaicex_chn);
        ret = sample_region_attach(i, mpp_chn, chn_attr, op_flag);
        if (ret != HI_SUCCESS) {
            sample_print("sample_region_attach failed!\n");
            sample_comm_region_detach(i - MOSAICEX_MIN_HANDLE + 1, MOSAICEX_MIN_HANDLE, mpp_chn, op_flag);
            return ret;
        }
    }
    return HI_SUCCESS;
}

hi_s32 sample_region_set_chn_attr(hi_s32 handle_num, hi_rgn_type type, hi_rgn_chn_attr *chn_attr,
    hi_mpp_chn *chn, region_op_flag op_flag)
{
    hi_s32 ret = HI_SUCCESS;
    switch (type) {
        case HI_RGN_OVERLAY:
            ret = sample_region_set_overlay_chn_attr(handle_num, chn_attr, chn, op_flag);
            break;
        case HI_RGN_OVERLAYEX:
            ret = sample_region_set_overlayex_chn_attr(handle_num, chn_attr, chn, op_flag);
            break;
        case HI_RGN_COVER:
            ret = sample_region_set_cover_chn_attr(handle_num, chn_attr, chn, op_flag);
            break;
        case HI_RGN_COVEREX:
            ret = sample_region_set_coverex_chn_attr(handle_num, chn_attr, chn, op_flag);
            break;
        case HI_RGN_LINEEX:
            ret = sample_region_set_lineex_chn_attr(handle_num, chn_attr, chn, op_flag);
            break;
        case HI_RGN_MOSAIC:
            ret = sample_region_set_mosaic_chn_attr(handle_num, chn_attr, chn, op_flag);
            break;
        case HI_RGN_MOSAICEX:
            ret = sample_region_set_mosaicex_chn_attr(handle_num, chn_attr, chn, op_flag);
            break;
        case HI_RGN_CORNER_RECT:
            ret = sample_region_set_corner_rect_chn_attr(handle_num, chn_attr, chn, op_flag);
            break;
        case HI_RGN_CORNER_RECTEX:
            ret = sample_region_set_corner_rectex_chn_attr(handle_num, chn_attr, chn, op_flag);
            break;
        default:
            break;
    }
    return ret;
}

hi_s32 sample_comm_region_attach(hi_s32 handle_num, hi_rgn_type type, hi_mpp_chn *mpp_chn,
    region_op_flag op_flag)
{
    hi_s32 ret;
    hi_rgn_chn_attr chn_attr;
    ret = sample_comm_region_attach_check(handle_num, type, mpp_chn);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    /* set the chn config */
    chn_attr.is_show = HI_TRUE;
    ret = sample_region_set_chn_attr(handle_num, type, &chn_attr, mpp_chn, op_flag);
    if (ret != HI_SUCCESS) {
        sample_print("sample_region_attach failed!\n");
    }
    return ret;
}

hi_s32 sample_comm_check_min(hi_s32 min_handle)
{
    if ((min_handle <= HI_INVALID_HANDLE) || (min_handle > HI_RGN_HANDLE_MAX)) {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 sample_comm_region_detach(hi_s32 handle_num, hi_rgn_type type, hi_mpp_chn *mpp_chn,
    region_op_flag op_flag)
{
    hi_s32 i;
    hi_s32 ret;
    hi_s32 min_handle;

    if (handle_num <= 0 || handle_num > 16) { /* 16:max region num */
        sample_print("handle_num is illegal %d!\n", handle_num);
        return HI_FAILURE;
    }
    if (type < 0 || type >= HI_RGN_BUTT) {
        sample_print("type is illegal %d!\n", type);
        return HI_FAILURE;
    }
    if (mpp_chn == HI_NULL) {
        sample_print("mpp_chn is NULL !\n");
        return HI_FAILURE;
    }
    min_handle = sample_comm_region_get_min_handle(type);
    if (sample_comm_check_min(min_handle) != HI_SUCCESS) {
        sample_print("min_handle(%d) should be in [0, %d).\n", min_handle, HI_RGN_HANDLE_MAX);
        return HI_FAILURE;
    }
    for (i = min_handle; i < min_handle + handle_num; i++) {
        ret = sample_region_detach(i, mpp_chn, op_flag);
        if (ret != HI_SUCCESS) {
            sample_print("sample_region_detach failed! handle:%d\n", i);
        }
    }
    return HI_SUCCESS;
}

hi_s32 sample_comm_region_set_bit_map(hi_rgn_handle handle, const hi_char *bmp_path)
{
    hi_s32 ret;
    rgn_load_bmp_info load_bmp_info = {0};

    load_bmp_info.filename = bmp_path; //设置路径
    load_bmp_info.fil = HI_FALSE; // 其他设置
    load_bmp_info.fil_color = 0; // 其他设置

    load_bmp_info.pixel_fmt = HI_PIXEL_FORMAT_ARGB_8888; //设置像素格式
    ret = region_mst_load_bmp(&load_bmp_info); //读取bmp图片放入info中
    if (ret != HI_SUCCESS) {
        sample_print("region_mst_load_bmp failed!handle\n");
    }
    ret = sample_region_set_bit_map(handle, &load_bmp_info.bmp);//通过句柄将bmp放入region通道中

    if (ret != HI_SUCCESS) {
        sample_print("sample_region_set_bit_map failed!handle:%u\n", handle);
    }
    free(load_bmp_info.bmp.data);//释放
    return ret;
}

static hi_bool sample_common_region_font_fill_in_bit(hi_u32 bitmap_row, hi_u32 bitmap_col, hi_s32 id)
{
    if (id < 14) {
        int bit = bitmap_row * FONT_WIDTH + bitmap_col;
        hi_s32 matrix_pos = bit / SAMPLE_SVP_VGS_BYTE_BIT;
        hi_s32 matrix_bit = SAMPLE_SVP_VGS_BYTE_BIT - 1 - (bit % SAMPLE_SVP_VGS_BYTE_BIT);
        if (sample_common_svp_vgs_font_matrixs[id * (FONT_WIDTH * FONT_HEIGHT /
            SAMPLE_SVP_VGS_BYTE_BIT) + matrix_pos] & (hi_u8)(1 << matrix_bit)) {
            return HI_TRUE;
        } else {
            return HI_FALSE;
        }
    }
    return HI_FALSE;
}


volatile int g_exit_flag = 0;
void handle_exit_signal(int signal)
{
    g_exit_flag = 1;  // 设置退出标志
}

#define FALSE  -1    
#define TRUE   0
#define UART_INPUT_TTL_LEN 8


struct parameter {
    int date[8];       // 年月日
    int time[8];        // 时分秒
    int distance[7];        // 距离
    int battery[3];        //电池电量
};

struct parameter data_references = {0};

int UART4_Set(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity) {

    int i;
    int status;
    int speed_arr[] = {B115200, B19200, B9600, B4800, B2400, B1200, B300};
    int name_arr[] = {115200, 19200, 9600, 4800, 2400, 1200, 300};

    struct termios options;
     if (tcgetattr(fd, &options) != 0) {
//        perror("SetupSerial 1");
#ifndef USING_GLOG_PRINT_FILE
        printf("SetupSerial ERROR\n");
#else
        printf("SetupSerial ERROR\n");
#endif
        return (FALSE);
    }

    //设置串口输入波特率和输出波特率      
    for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
        if (speed == name_arr[i]) {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
        }
    }
     //修改控制模式，保证程序不会占用串口    
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据      
    options.c_cflag |= CREAD;

    //设置数据流控制    
    switch (flow_ctrl) {

        case 0 ://不使用流控制    
            options.c_cflag &= ~CRTSCTS;
            break;

        case 1 ://使用硬件流控制    
            options.c_cflag |= CRTSCTS;
            break;
        case 2 ://使用软件流控制     
            options.c_cflag |= IXON | IXOFF | IXANY;
            break;
    }
    //设置数据位    
    //屏蔽其他标志位    
    options.c_cflag &= ~CSIZE;
    switch (databits) {
        case 5:
            options.c_cflag |= CS5;
            break;
        case 6    :
            options.c_cflag |= CS6;
            break;
        case 7    :
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
#ifndef USING_GLOG_PRINT_FILE
        printf("Unsupported data size!\n") ;
#else
        printf("Unsupported data size!\n") ;
#endif
//          fprintf(stderr, "Unsupported data size\n");
            return (FALSE);
    }
        switch (parity) {
        case 'n':
        case 'N': //无奇偶校验位。     
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 'o':
        case 'O'://设置为奇校验        
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;
        case 'e':
        case 'E'://设置为偶校验     
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        case 's':
        case 'S': //设置为空格      
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
            default:
#ifndef USING_GLOG_PRINT_FILE
            printf("Unsupported parity!\n") ;
#else
             printf(""Unsupported parity"!\n") ;
#endif
//            fprintf(stderr, "Unsupported parity\n");
            return (FALSE);
    }
        switch (stopbits) {
            case 1:
                options.c_cflag &= ~CSTOPB;
                break;
            case 2:
                options.c_cflag |= CSTOPB;
                break;
            default:
#ifndef USING_GLOG_PRINT_FILE
        printf("Unsupported stop bits\n") ;
#else
        printf("Unsupported stop bits\n") ;
#endif
//            fprintf(stderr, "Unsupported stop bits\n");
            return (FALSE);
    }

    //修改输出模式，原始数据输出     
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(ICRNL | IGNCR | IXON | IXOFF);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //options.c_lflag &= ~(ISIG | ICANON);    

    //设置等待时间和最小接收字符     
    options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */
    options.c_cc[VMIN] = 0; /* 读取字符的最少个数为1 */

    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读   
    tcflush(fd, TCIFLUSH);

    //激活配置 (将修改后的termios数据设置到串口中）    
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
//        perror("com set error!\n");
#ifndef USING_GLOG_PRINT_FILE
        printf("com set error!\n");
#else
        printf("com set error!\n") ;
#endif
        return (FALSE);
    }
    return (TRUE);
}


int UART4_Init(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity) {
    int err;
    //设置串口数据帧格式     
    if (UART4_Set(fd, speed, flow_ctrl, databits, stopbits, parity) == FALSE) {
        return FALSE;
    } else {
        return TRUE;
    }
}
int UART4_Recv(int fd, char *rcv_buf, int data_len) {
    int len, fs_sel;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);

    time.tv_sec = 10;
    time.tv_usec = 0;

    len = read(fd, rcv_buf, data_len);
    //printf("len = %d\n", len);
    return len;
}

int hex_to_decimal(const char *hex_string) {
    // 使用 strtol 函数将十六进制字符串转换为十进制整数
    int decimal_value = strtol(hex_string, NULL, 16);
    return decimal_value;
}


void Encode_Data_Analyzing(char *recv_buf)
{
    switch(recv_buf[3])
    {
        case 0x01:
            printf("02 b3 = %02X\n",recv_buf[4]);
            break;
        case 0x03:
            printf("02 b3 = %02X\n",recv_buf[4]);
            break;
        case 0x04:
            printf("02 b3 = %02X\n",recv_buf[4]);
            break;
        default:
            break;
    }

}

void Menu_Data_Analyzing(char *recv_buf)
{
    switch(recv_buf[4])
    {
        case 0x01:
            printf("02 b4 = %02X\n",recv_buf[4]);
            break;

        case 0x02:
            printf("02 b4 = %02X\n",recv_buf[4]);
            break;
        default:
            break;
    }

}
void Rng_Data_Analyzing(char *recv_buf)
{
    switch(recv_buf[5])
    {
        case 0x01://无用，测距按钮，客户这边操作，只是发出来
            printf("02 b5 = %02X\n",recv_buf[5]);
            break;
        default:
            break;
    }

}
void Distance_Analyzing(char *recv_buf)
{
    // printf("Raw data received111:"); 
    // for (int i = 0; i < 8; ++i) {
    //     printf("%02X ", (unsigned char)recv_buf[i]);
    // }
    // printf("\n");
    char result[20] = {0};
    int i =0 ;
    unsigned char hihg = recv_buf[3];
    unsigned char low = recv_buf[4];
    sprintf(result, "%02X%02X", hihg,low);
    //printf("result1 = %s",result);
    int Integer_number = hex_to_decimal(&result);
    memset(result, 0, 20);
    sprintf(result, "%02X", recv_buf[5]);
    //printf("result2 = %s",result);
    int decimal_fraction = hex_to_decimal(&result);
    memset(result, 0, 20);
    sprintf(result, "%d%d", Integer_number, decimal_fraction);
    int size = strlen(result);
     for( i = 0; i < size; i++) {
        data_references.distance[i] = result[i] - '0';
    }
    data_references.distance[i] = data_references.distance[i-1];
    data_references.distance[i-1] = 10;
    printf("测距值 %s\n 分解后: ", result);
    for(int i = 0; i < size+1; i++) {
        printf("%d ", data_references.distance[i]);
    }
    printf("\n");
}

void Date_Analyzing(char *recv_buf)
{
    char date[6] = {0};
    sprintf(date, "%02d%02d%02d", recv_buf[3],recv_buf[4],recv_buf[5]);
    int size = strlen(date);
    for(int i = 0; i < size; i++) {
        data_references.date[i] = date[i] - '0';
    }
    printf("日期 %s\n 分解后: ", date);
    for(int i = 0; i < size; i++) {
        printf("%d ", data_references.date[i]);
    }
    printf("\n");
    //printf("日期: %s\n", data_references.year);
}

void Time_Analyzing(char *recv_buf)
{
    char time[6] = {0};
    sprintf(time, "%02d%02d%02d", recv_buf[3],recv_buf[4],recv_buf[5]);
    int size = strlen(time);
    for(int i = 0; i < size; i++) {
        data_references.time[i] = time[i] - '0';
    }
    printf("时间%s\n 分解后: ",time);
    for(int i = 0; i < size; i++) {
        printf("%d ", data_references.time[i]);
    }
    printf("\n");
    // strcpy(data_references.time, time);
    // printf("时间: %s\n", data_references.time);
}
void Battery_Analyzing(char *recv_buf)
{
    char result[3] = {0};
    sprintf(result, "%02X", recv_buf[3]);
    int Integer_number = hex_to_decimal(result);
    memset(result, 0, 3);
    sprintf(result, "%d", Integer_number);
    int size = strlen(result);
    for(int i = 0; i < size; i++) {
        data_references.battery[i] = result[i] - '0';
    }
    printf("电量 %s\n 分解后: ", result);
    for(int i = 0; i < size; i++) {
        printf("%d ", data_references.battery[i]);
    }
    printf("\n");
    //data_references.battery = Integer_number;
    //printf("电量: %d%%\n", data_references.battery);

}
void State_judgment(char *recv_buf)
{
    switch(recv_buf[6])
    {
         case 0x01:
            Distance_Analyzing(recv_buf);
            break;

        case 0x02:
            // printf("超程\n");
            data_references.distance[0]= 0;
            // printf("测距值: %d\n", (int)data_references.grade);
            break;
        case 0x03:
            data_references.distance[0]= 0;
            // printf("欠光\n");
            // printf("测距值: %d\n", (int)data_references.grade);
            break;
            default:
            break;


    }


}
void Data_Analyzing(char *recv_buf)
{

    switch(recv_buf[2])
    {
        case 0x01:
            State_judgment(recv_buf);
            break;

        case 0x02:
            Encode_Data_Analyzing(recv_buf);
            Menu_Data_Analyzing(recv_buf);
            Rng_Data_Analyzing(recv_buf);
            break;

        case 0x03:
            Battery_Analyzing(recv_buf);
            break;

        case 0x04:
            Date_Analyzing(recv_buf);
            break;
            
        case 0x05:
            Time_Analyzing(recv_buf);
            break;

        default:
            break;


    }


}
int fd ;
int open_port(void)
{ 
    fd = open("/dev/ttyAMA4", O_RDWR|O_NOCTTY|O_NDELAY);
    if (-1 == fd)
    {
        perror("Open:");
        return(-1);
    }
    /*恢复串口为阻塞状态*/
    if(fcntl(fd, F_SETFL, 0) <0)
    {
        perror("fcntl");
        //return -1;
    }

    /*测试是否为终端设备*/
    if(isatty(STDIN_FILENO) == 0)
    {
        perror("isatty");
        return -1;
    }
    return fd;
}
hi_void* sample_comm_region_draw_number(void* arg) 
{
    

    hi_s32 ret;
    hi_rgn_canvas_info canvas_info;

    // 将传入的 arg 转换为 handle 类型
    hi_rgn_handle handle = *((hi_rgn_handle*)arg);

 //   sample_print("Region handle: %p\n", handle);  // 打印句柄的值

    hi_u32 stride;

    // 设置初始位置
    hi_s32 x_date = 1550;     // 日期起始位置
    hi_s32 y_date = 50;      // 日期起始位置
    hi_s32 x_time = 1550;     // 时间起始位置
    hi_s32 y_time = 100;      // 时间起始位置
    hi_s32 x_battery = 650;  // 电量起始位置
    hi_s32 y_battery = 50;   // 电量起始位置
    hi_s32 x_distance = 200; // 测距值起始位置
    hi_s32 y_distance = 50;   // 测距值起始位置

    open_port();
    if (fd < 0)
    {
        perror("Can't Open Serial Port");
    }
    printf("fd= %d \n", fd);
    
    if (UART4_Init(fd, 115200, 0, 8, 1, 'n') == FALSE) {
        printf("Failed to initialize UART\n");
        return -1;
    }
    const int buffer_size = UART_INPUT_TTL_LEN;
    unsigned char recv_buf[buffer_size];  // 接收缓冲区
    int len;
    while (!g_exit_flag) {

        // 清空接收缓冲区
        memset(recv_buf, 0, buffer_size);
        // 接收数据
        len = UART4_Recv(fd, recv_buf, buffer_size);
        // 打印每次接收到的数据
        if (len > 0) {


            printf("Raw data received:"); 
            for (int i = 0; i < len; ++i) {
                printf("%02X ", (unsigned char)recv_buf[i]);
            }
            printf("\n");
            if(recv_buf[0] != 0xAA && recv_buf[1] != 0x55)
            {
                memset(recv_buf, 0, buffer_size);
                continue;
            }
            unsigned char sun = recv_buf[0]+recv_buf[1]+recv_buf[2]+recv_buf[3]+recv_buf[4]+recv_buf[5]+recv_buf[6];
            printf("B7 = %02X\n",recv_buf[7]);
            printf("sun = %02X\n",sun);
            if(sun != recv_buf[7])
            {
                memset(recv_buf, 0, buffer_size);
                continue;  
            }
            // printf("Raw data received:"); 
            // for (int i = 0; i < len; ++i) {
            //     printf("%02X ", (unsigned char)recv_buf[i]);
            // }
            // printf("\n");
            Data_Analyzing(recv_buf);
            memset(recv_buf, 0, buffer_size);
        } 
        usleep(10); //1ms

      
        // 获取画布信息
        ret = hi_mpi_rgn_get_canvas_info(handle, &canvas_info);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_get_canvas_info failed with %#x!\n", ret);
            return (void*)HI_FAILURE;
        }

        // 获取画布数据指针和步幅
        hi_u32* bitmap_data = (hi_u32 *)(canvas_info.virt_addr);
        stride = canvas_info.stride;

        // 清除数字区域
        clear_number_area(bitmap_data, stride, x_date, y_date, 6);  // 假设日期长度为 6 位
        clear_number_area(bitmap_data, stride, x_time, y_time, 6);  // 假设时间长度为 6 位
        clear_number_area(bitmap_data, stride, x_battery, y_battery, 3);  // 假设电量长度为 3 位
        clear_number_area(bitmap_data, stride, x_distance, y_distance, 7);  // 假设测距值长度为 7 位

    
        // 绘制日期
        ret = sample_comm_region_draw_number_at_position(bitmap_data, stride, x_date, y_date, sizeof(data_references.date) / sizeof(data_references.date[0]), data_references.date);
        if (ret != HI_SUCCESS) {
            sample_print("sample_comm_region_draw_number_at_position (date) failed with %#x!\n", ret);
            return (void*)HI_FAILURE;
        }
        // 绘制时间
        ret = sample_comm_region_draw_number_at_position(bitmap_data, stride, x_time, y_time, sizeof(data_references.time) / sizeof(data_references.time[0]), data_references.time);
        if (ret != HI_SUCCESS) {
            sample_print("sample_comm_region_draw_number_at_position (time) failed with %#x!\n", ret);
            return (void*)HI_FAILURE;
        }

        // 绘制电量
        ret = sample_comm_region_draw_number_at_position(bitmap_data, stride, x_battery, y_battery, sizeof(data_references.battery) / sizeof(data_references.battery[0]), data_references.battery);
        if (ret != HI_SUCCESS) {
            sample_print("sample_comm_region_draw_number_at_position (battery) failed with %#x!\n", ret);
            return (void*)HI_FAILURE;
        }

        // 绘制测距值
        ret = sample_comm_region_draw_number_at_position(bitmap_data, stride, x_distance, y_distance, sizeof(data_references.distance) / sizeof(data_references.distance[0]), data_references.distance);
        if (ret != HI_SUCCESS) {
            sample_print("sample_comm_region_draw_number_at_position (distance) failed with %#x!\n", ret);
            return (void*)HI_FAILURE;
        }
        // 更新画布
        ret = hi_mpi_rgn_update_canvas(handle);
        if (ret != HI_SUCCESS) {
            sample_print("hi_mpi_rgn_update_canvas failed with %#x!\n", ret);
            return (void*)HI_FAILURE;
        }

        // 打印调试信息
     //   sample_print("Region handle: %p, counter: %d\n", handle, counter);

    }
    close(fd);
    return (void*)HI_SUCCESS;
}


hi_s32 sample_comm_region_draw_number_at_position(hi_u32 *bitmap_data, hi_u32 stride, hi_s32 x, hi_s32 y, int len, const int *number)
{
    hi_s32 ret;
    hi_u32 font_color = RGN_ARGB8888_WHITE;
    int row, col, i;

    // 遍历数字串
    for (i = 0; i < len; ++i) {
        int current_number = number[i];  // 当前要绘制的数字

        // 遍历每个数字的字体图案
        for (row = 0; row < FONT_HEIGHT; ++row) {
            for (col = 0; col < FONT_WIDTH; ++col) {
                // 计算当前数字对应的画布像素索引
                hi_u32 data_idx = (y + row) * (stride / 4) + (x + col + 16 * i);

                // 判断是否要填充当前像素
                if (sample_common_region_font_fill_in_bit(row, col, current_number) == HI_TRUE) {
                    bitmap_data[data_idx] = font_color;
                }
            }
        }
    }

    return HI_SUCCESS;
}

void clear_number_area(hi_u32 *bitmap_data, hi_u32 stride, hi_s32 x, hi_s32 y, int len) {
    int row, col, i;

    // 遍历数字串
    for (i = 0; i < len; ++i) {
        // 遍历每个数字的字体图案
        for (row = 0; row < FONT_HEIGHT; ++row) {
            for (col = 0; col < FONT_WIDTH; ++col) {
                // 计算当前数字对应的画布像素索引
                hi_u32 data_idx = (y + row) * (stride / 4) + (x + col + 16 * i);

                // 将像素清除为背景色
                bitmap_data[data_idx] = 0;  // 假设背景色为 0
            }
        }
    }
}

hi_s32 sample_comm_region_get_up_canvas(hi_rgn_handle handle, const hi_char *bmp_path)
{
    hi_s32 ret;
    hi_size size;
    hi_rgn_canvas_info canvas_info;
    rgn_load_bmp_info load_bmp_info = {0};

    ret = hi_mpi_rgn_get_canvas_info(handle, &canvas_info);
    if (ret != HI_SUCCESS) {
        sample_print("hi_mpi_rgn_get_canvas_info failed with %#x!\n", ret);
        return HI_FAILURE;
    }

    size.width = canvas_info.size.width;
    size.height = canvas_info.size.height;
    load_bmp_info.bmp.data = canvas_info.virt_addr;
    load_bmp_info.pixel_fmt = canvas_info.pixel_format;
    load_bmp_info.fil = HI_FALSE;
    load_bmp_info.fil_color = 0;
    load_bmp_info.filename = bmp_path;
    ret = region_mst_update_canvas(&load_bmp_info, &size, canvas_info.stride);
    if (ret != HI_SUCCESS) {
        sample_print("region_mst_update_canvas failed with %#x!\n", ret);
        return HI_FAILURE;
    }

    ret = hi_mpi_rgn_update_canvas(handle);
    if (ret != HI_SUCCESS) {
        sample_print("hi_mpi_rgn_update_canvas failed with %#x!\n", ret);
        return HI_FAILURE;
    }

    return ret;
}
