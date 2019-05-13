/*
 * libilda.c
 *
 *  Created on: May 13, 2019
 *      Author: mendicantbias
 */


#include "libilda.h"


struct ILDA_color_header_t * ilda_parse_color_header(char * f, uint32_t cfp){

    struct ILDA_color_header_t * ch = malloc(sizeof(struct ILDA_color_header_t));
    char buf [32];
    memcpy(buf, &f[cfp], 32);
    cfp += 32;

    if (strncmp(buf, ILDA_FORMAT_HEADER, 7) != 0){
        ch->valid = ILDA_INVALID;
    }else{
        ch->valid = ILDA_VALID;
    }

    ch->format_code = buf[7];
    memcpy(&ch->palette_name, &buf[8], 8);
    memcpy(&ch->company_name, &buf[16], 8);
    memcpy(&ch->total_colors, &buf[24], 2);
    memcpy(&ch->palette_number, &buf[26], 2);
    memcpy(&ch->scanner_head, &buf[30], 1);

    return ch;

}

struct ILDA_coordinate_header_t * ilda_parse_coor_header(char * f, uint32_t cfp){

    struct ILDA_coordinate_header_t * ch = malloc(sizeof(struct ILDA_coordinate_header_t));
    char buf [32];
    memcpy(buf, &f[cfp], 32);
    cfp += 32;

    if (strncmp(buf, ILDA_FORMAT_HEADER, 7) != 0){
        ch->valid = ILDA_INVALID;
    }else{
        ch->valid = ILDA_VALID;
    }

    ch->format_code = buf[7];
    memcpy(&ch->frame_name, &buf[8], 8);
    memcpy(&ch->company_name, &buf[16], 8);
    memcpy(&ch->total_points, &buf[24], 2);
    memcpy(&ch->frame_number, &buf[26], 2);
    memcpy(&ch->total_frames, &buf[28], 2);
    memcpy(&ch->scanner_head, &buf[30], 1);

    return ch;

}

struct ILDA_frame_t * ilda_parse_animation_frame(char * f, uint32_t cfp, struct ILDA_coordinate_header_t * ch, struct ILDA_color_table_t * ct){
    // Assumes that FILE * f is properly positioned at the beginning of a header.

    struct ILDA_frame_t * frame = malloc(sizeof(struct ILDA_frame_t));
    frame->pts = malloc(ch->total_points * sizeof(struct ILDA_point_t));
    frame->num_pts = ch->total_points;

    switch (ch->format_code){
        case ILDA_2D_COORDINATE:
            for(int i = 0; i < ch->total_points; i++){
                char buf[6];
                memcpy(buf, &f[cfp], 6);
                cfp += 6;

                struct ILDA_point_t pt = frame->pts[i];
                memcpy(&pt.x, &buf[0], 2);
                memcpy(&pt.y, &buf[2], 2);
                pt.z = 0;

                if (ct == NULL){
                    pt.r = 0;
                    pt.g = 255;
                    pt.b = 0;
                }else{
                    uint8_t color_index = buf[4];
                    pt.r = ct->colors[color_index].r;
                    pt.g = ct->colors[color_index].g;
                    pt.b = ct->colors[color_index].b;
                }

                char blanking = buf[5];
                blanking = (blanking << 6) & 0b00000010;
                pt.blanking = blanking;

            }
            return frame;
        case ILDA_3D_COORDINATE:
            for(int i = 0; i < ch->total_points; i++){
                char buf[8];
                memcpy(buf, &f[cfp], 8);
                cfp += 8;

                struct ILDA_point_t pt = frame->pts[i];
                memcpy(&pt.x, &buf[0], 2);
                memcpy(&pt.y, &buf[2], 2);
                memcpy(&pt.y, &buf[4], 2);

                if (ct == NULL){
                    pt.r = 0;
                    pt.g = 255;
                    pt.b = 0;
                }else{
                    uint8_t color_index = buf[6];
                    pt.r = ct->colors[color_index].r;
                    pt.g = ct->colors[color_index].g;
                    pt.b = ct->colors[color_index].b;
                }

                char blanking = buf[7];
                blanking = (blanking << 6) & 0b00000010;
                pt.blanking = blanking;

            }
            return frame;
        default:
            return NULL;
    }
}

struct ILDA_color_table_t * ilda_parse_color_table(char * f, uint32_t cfp, struct ILDA_color_header_t * ch){

    struct ILDA_color_table_t * color_table = malloc(sizeof(struct ILDA_color_table_t));
    color_table->colors = malloc(ch->total_colors * sizeof(struct ILDA_color_t));
    color_table->num_colors = ch->total_colors;

    for(int i = 0; i < ch->total_colors; i++){
        char buf[3];
        memcpy(buf, &f[cfp], 3);
        cfp += 3;

        struct ILDA_color_t c = color_table->colors[i];
        c.r = buf[0];
        c.g = buf[1];
        c.b = buf[2];

    }

    return color_table;

}

enum ILDA_header_type_t ilda_get_next_header_type(char * f, uint32_t cfp){
    // Assume we are at the start of a header

    // Read 8 bytes
    char buf[8];
    memcpy(buf, &f[cfp], 8);

    // Check it is a valid header
    if(strncmp(buf, ILDA_FORMAT_HEADER, 7) != 0) return ILDA_UNKNOWN_HEADER_OR_FAIL;

    // Check the header type
    switch (buf[7]){
        case 0:
            return ILDA_3D_COORDINATE;
        case 1:
            return ILDA_2D_COORDINATE;
        case 2:
            return ILDA_COLOR_TABLE;
        default:
            return ILDA_UNKNOWN_HEADER_OR_FAIL;
    }
}

struct ILDA_animation_t * ilda_parse_animation(char * f, uint32_t cfp, uint32_t filesize){

    int num_frames = 0;

    struct ILDA_animation_t * anim = malloc(sizeof(struct ILDA_animation_t));

    struct ILDA_color_table_t * current_color_table = NULL;

    if (f == NULL){
        return NULL;
    }

    while(cfp < filesize){
        // Read a header
        enum ILDA_header_type_t nht = ilda_get_next_header_type(f, cfp);


        if(nht == ILDA_3D_COORDINATE || nht == ILDA_2D_COORDINATE){
            struct ILDA_coordinate_header_t * ch = ilda_parse_coor_header(f, cfp);
            if(ch->total_points == 0){
                break; // End of file
            }

            struct ILDA_frame_t * frame = ilda_parse_animation_frame(f, cfp, ch, current_color_table);

            if(num_frames == 0){
                num_frames = ch->total_frames;
                anim->num_frames = ch->total_frames;
                anim->animation = malloc(num_frames * sizeof(struct ILDA_frame_t));
            }

            memcpy(&anim->animation[ch->frame_number], frame, sizeof(struct ILDA_frame_t)); // Copy in new frame
            free(frame);

        }else if(nht == ILDA_COLOR_TABLE){

            struct ILDA_color_header_t * ch = ilda_parse_color_header(f, cfp);
            if(ch->total_colors == 0){
                break; // End of file
            }

            current_color_table = ilda_parse_color_table(f, cfp, ch);
        }else{
            break;
        }

    }

    return anim;

}

