/*
 * libilda.h
 *
 *  Created on: May 13, 2019
 *      Author: mendicantbias
 */

#ifndef LIBILDA_H_
#define LIBILDA_H_

// NOTE ignore the colors right now

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define RETURN_FAIL 0
#define RETURN_SUCCESS 1

#define ILDA_VALID 1
#define ILDA_INVALID 0

#define ILDA_FORMAT_HEADER "ILDA\x00\x00\x00"

uint32_t ilda_cfp = 0;

struct ILDA_point_t {
    int16_t x;
    int16_t y;
    int16_t z;

    uint8_t r;
    uint8_t g;
    uint8_t b;

    uint8_t blanking;
};

enum ILDA_header_type_t {
    ILDA_3D_COORDINATE = 0,
    ILDA_2D_COORDINATE = 1,
    ILDA_COLOR_TABLE = 2,
    ILDA_UNKNOWN_HEADER_OR_FAIL = -1,
};

struct ILDA_color_t {
    uint8_t r, g, b;
};

struct ILDA_frame_t {
    struct ILDA_point_t * pts;
    int num_pts;
};

struct ILDA_animation_t {
    struct ILDA_frame_t * animation;
    int num_frames;
};

struct ILDA_color_table_t {
    struct ILDA_color_t * colors;
    int num_colors;
};

struct ILDA_coordinate_header_t {
    uint8_t valid;
    enum ILDA_header_type_t format_code;
    char frame_name[8];
    char company_name[8];
    uint16_t total_points;
    uint16_t frame_number;
    uint16_t total_frames;
    uint8_t scanner_head;

};

struct ILDA_color_header_t {
    uint8_t valid;
    uint8_t format_code;
    char palette_name[8];
    char company_name[8];
    uint16_t total_colors;
    uint16_t palette_number;
    uint8_t scanner_head;

};




struct ILDA_frame_t * ilda_parse_animation_frame(char * f, uint32_t cfp, struct ILDA_coordinate_header_t * ch, struct ILDA_color_table_t * ct);

struct ILDA_color_table_t * ilda_parse_color_table(char * f, uint32_t cfp, struct ILDA_color_header_t * ch);

enum ILDA_header_type_t ilda_get_next_header_type(char * f, uint32_t cfp);

struct ILDA_color_header_t * ilda_parse_color_header(char * f, uint32_t cfp);

struct ILDA_coordinate_header_t * ilda_parse_coor_header(char * f, uint32_t cfp);

struct ILDA_animation_t * ilda_parse_animation(char * f, uint32_t cfp, uint32_t filesize);


#endif /* LIBILDA_H_ */
