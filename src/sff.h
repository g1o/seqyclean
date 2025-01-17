#ifndef _SFF_H_
#define _SFF_H_

/*
    Copyright (C) 2009, 2010 Indraniel Das <indraniel@gmail.com>
                             and Washington University in St. Louis

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see <http://www.gnu.org/licenses/>
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <limits.h>
//#include <endian.h>
#include <zlib.h>
#include <arpa/inet.h> // htons(), htonl()
#include "util.h"


#if defined __GENOME__
  #include "endian.h"
#elif defined __linux__
  #include <endian.h>
//#elif defined __LINUX__
//  #include <endian.h>
#elif defined __BSD__
  #include <sys/endian.h>
#elif defined __APPLE__
  #include <sys/_endian.h>
  #include <architecture/byte_order.h>
  #define __bswap_64(x)      NXSwapLongLong(x)
  #define __bswap_32(x)      NXSwapLong(x)
  #define __bswap_16(x)      NXSwapShort(x)

  #if __DARWIN_BYTE_ORDER == __DARWIN_BIG_ENDIAN
    #define htobe16(x) (x)
    #define htole16(x) __bswap_16 (x)
    #define be16toh(x) (x)
    #define le16toh(x) __bswap_16 (x)

    #define htobe32(x) (x)
    #define htole32(x) __bswap_32 (x)
    #define be32toh(x) (x)
    #define le32toh(x) __bswap_32 (x)

    #define htobe64(x) (x)
    #define htole64(x) __bswap_64 (x)
    #define be64toh(x) (x)
    #define le64toh(x) __bswap_64 (x)
  #else
    #define htobe16(x) __bswap_16 (x)
    #define htole16(x) (x)
    #define be16toh(x) __bswap_16 (x)
    #define le16toh(x) (x)

    #define htobe32(x) __bswap_32 (x)
    #define htole32(x) (x)
    #define be32toh(x) __bswap_32 (x)
    #define le32toh(x) (x)

    #define htobe64(x) __bswap_64 (x)
    #define htole64(x) (x)
    #define be64toh(x) __bswap_64 (x)
    #define le64toh(x) (x)
  #endif
#else
  #error Unknown location for endian.h
#endif

//#ifdef __cplusplus
//extern "C" {
//#endif

#define SFF_MAGIC   0x2e736666 /* ".sff" */
#define SFF_VERSION "\0\0\0\1"
#define SFF_VERSION_LENGTH 4
#define PADDING_SIZE 8

//#define min(a,b) ( (a) < (b) ? (a) : (b) )
///#define max(a,b) ( (a) > (b) ? (a) : (b) )

/* This is the overall sff header file */
    
extern bool pcr_flag;
    
typedef struct {
    uint32_t  magic; /*Magic number: identical for all sff files (0x2E736666, the newbler manual explains that this is ‘the uint32_t encoding of the string „.sff“ ‘)*/
    char      version[4]; /*Version: also identical for all sff files (0001)*/
    uint64_t  index_offset; /*Index offset and length: has to do with the index of the binary sff file (points to the location of the index in the file)*/
    uint32_t  index_len;
    uint32_t  nreads; /* # of reads: stored in the sff file*/
    uint16_t  header_len;/*Header length: looks like it is 440 for GS FLX reads, 840 for GS FLX Titanium reads*/
    uint16_t  key_len;/*Key length: the length (in bases) of the key sequence that each read starts with, so far always 4*/
    uint16_t  flow_len;/*# of Flows: each flow consists of a base that is flowed over the plate; for GS20, there were 168 flows (42 cycles of all four nucleotides), 400 for GS FLX (100 cycles) and 800 for Titanium (200 cycles)*/
    uint8_t   flowgram_format;/*Flowgram code: kind of the version of coding the flowgrams (signal strengths); so far, ’1′ for all sff files*/
    char     *flow;/*Flow Chars: a string consisting of’ # of flow’ characters (168, 400 or 800) of the bases in flow order (‘TACG’ up to now)*/
    char     *key; /* Key Sequence: the first four bases of reads are either added during library preparation (they are the last bases of the ‘A’ adaptor) or they are a part of the control beads. For example, Titanium sample beads have key sequence TACG (default library protocol) or GACT (rapid library protocol), control beads have CATG or ATGC. Control reads never make it into sff files*/
} sff_common_header;

/*
 * There is a read_header per "reading" in the SFF archive.
 * It too is padded to an 8-byte boundary.
 */
typedef struct {
    uint16_t  header_len;
    uint16_t  name_len;
    uint32_t  nbases;
    uint16_t  clip_qual_left;
    uint16_t  clip_qual_right;
    uint16_t  clip_adapter_left;
    uint16_t  clip_adapter_right;
    char     *name;
} sff_read_header;

/*
 * The read_data section per reading, following the read_header.
 * It is padded to an 8-byte boundary.
 */
typedef struct {
    uint16_t *flowgram;   /* x 100.0 */
    uint8_t  *flow_index; /* relative to last */
    char     *bases;
    uint8_t  *quality;
} sff_read_data;

/* function to read the sff file */
void read_sff_common_header(FILE *fp, sff_common_header *h);
void free_sff_common_header(sff_common_header *h);
short verify_sff_common_header(sff_common_header *h);
void read_padding(FILE *fp, int header_size);
void read_sff_read_header(FILE *fp, sff_read_header *rh);
void read_sff_read_data(FILE *fp, 
                   sff_read_data *rd, 
                   uint16_t nflows, 
                   uint32_t nbases);
void free_sff_read_header(sff_read_header *rh);
void free_sff_read_data(sff_read_data *d);

void get_clip_values(sff_read_header rh,
                     int trim_flag,
                     int *left_clip,
                     int *right_clip);

char* get_read_bases(sff_read_data rd,
                     int left_clip,
                     int right_clip);

uint8_t* get_read_quality_values(sff_read_data rd,
                                int left_clip,
                                int right_clip);


/*Write functions*/
void write_sff_common_header(FILE *fp, sff_common_header *ch);
void write_sff_read_header(FILE *fp, sff_read_header *rh);

void write_padding(FILE *fp, int header_size) ;
void write_sff_read_data(FILE *fp,  sff_read_data *rd, uint16_t nflows, uint32_t nbases);

void read_manifest(FILE *fp) ;
void write_manifest(FILE *fp);
long get_sff_file_size(FILE *fp);


#endif /* _SFF_H */
