/* L I C E N S E *************************************************************/

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

/* I N C L U D E S ***********************************************************/
#include "sff.h"

char *manifest = NULL;
long sff_file_size = 0;
long manifest_size = 0;

/* F U N C T I O N S *********************************************************/
/* convert 64 bit Big Endian integer to Native Endian(means current machine) */
// As far as I know, there is no standard function for 64 bit conversion on OSX
uint64_t BE64toNA(uint64_t bigEndian)
{
    uint64_t littleEndian = ((bigEndian & (0x00000000000000FF)) << 56) |
                            ((bigEndian & (0x000000000000FF00)) << 40) |
                            ((bigEndian & (0x0000000000FF0000)) << 24) |
                            ((bigEndian & (0x00000000FF000000)) << 8) |
                            ((bigEndian & (0x000000FF00000000)) >> 8) |
                            ((bigEndian & (0x0000FF0000000000)) >> 24) |
                            ((bigEndian & (0x00FF000000000000)) >> 40) |
                            ((bigEndian & (0xFF00000000000000)) >> 56);
    return littleEndian;
}

void  read_sff_common_header(FILE *fp, sff_common_header *h) {
    char *flow;
    char *key;
    int header_size;

    size_t bytes = fread(&(h->magic)          , sizeof(uint32_t), 1, fp);
    bytes = fread(&(h->version)        , sizeof(char)    , 4, fp);
    bytes = fread(&(h->index_offset)   , sizeof(uint64_t), 1, fp);
    bytes = fread(&(h->index_len)      , sizeof(uint32_t), 1, fp);
    bytes = fread(&(h->nreads)         , sizeof(uint32_t), 1, fp);
    bytes = fread(&(h->header_len)     , sizeof(uint16_t), 1, fp);
    bytes = fread(&(h->key_len)        , sizeof(uint16_t), 1, fp);
    bytes = fread(&(h->flow_len)       , sizeof(uint16_t), 1, fp);
    bytes = fread(&(h->flowgram_format), sizeof(uint8_t) , 1, fp);

    /* sff files are in big endian notation so adjust appropriately */
    /* Linux: not in use any more
    h->magic        = htobe32(h->magic);
    h->index_offset = htobe64(h->index_offset);
    h->index_len    = htobe32(h->index_len);
    h->nreads       = htobe32(h->nreads);
    h->header_len   = htobe16(h->header_len);
    h->key_len      = htobe16(h->key_len);
    h->flow_len     = htobe16(h->flow_len);
     */
    
    h->magic        = htonl(h->magic);
    h->index_offset = BE64toNA(h->index_offset);
    h->index_len    = htonl(h->index_len);
    h->nreads       = htonl(h->nreads);
    h->header_len   = htons(h->header_len);
    h->key_len      = htons(h->key_len);
    h->flow_len     = htons(h->flow_len);

    /* finally appropriately allocate and read the flow and key strings */
    flow = (char *) malloc( h->flow_len * sizeof(char) );
    if (!flow) {
        fprintf(stderr,
                "Out of memory! Could not allocate header flow string!\n");
        exit(1);
    }

    key  = (char *) malloc( h->key_len * sizeof(char) );
    if (!key) {
        fprintf(stderr,
                "Out of memory! Could not allocate header key string!\n");
        exit(1);
    }

    bytes = fread(flow, sizeof(char), h->flow_len, fp);
    h->flow = flow;

    bytes = fread(key , sizeof(char), h->key_len , fp);
    
    h->key = key;
    
    /* the common header section should be a multiple of 8-bytes 
       if the header is not, it is zero-byte padded to make it so */

    header_size = sizeof(h->magic)
                  + sizeof( h->version )
                  + sizeof(h->index_offset)
                  + sizeof(h->index_len)
                  + sizeof(h->nreads)
                  + sizeof(h->header_len)
                  + sizeof(h->key_len)
                  + sizeof(h->flow_len)
                  + sizeof(h->flowgram_format)
                  + (sizeof(char) * h->flow_len)
                  + (sizeof(char) * h->key_len);

    if ( !( (header_size  % PADDING_SIZE) == 0) ) {
        read_padding(fp, header_size);
    }
    
  
}

void write_sff_common_header(FILE *fp, sff_common_header *ch) {
    
    fwrite(&(ch->magic)          , sizeof(uint32_t), 1, fp);
    fwrite(&(ch->version)        , sizeof(char) , 4, fp);
    fwrite(&(ch->index_offset)   , sizeof(uint64_t), 1, fp);
    fwrite(&(ch->index_len)      , sizeof(uint32_t), 1, fp);
    fwrite(&(ch->nreads)         , sizeof(uint32_t), 1, fp);
    fwrite(&(ch->header_len)     , sizeof(uint16_t), 1, fp);
    fwrite(&(ch->key_len)        , sizeof(uint16_t), 1, fp);
    fwrite(&(ch->flow_len)       , sizeof(uint16_t), 1, fp);
    fwrite(&(ch->flowgram_format), sizeof(uint8_t) , 1, fp);
    fwrite(ch->flow             , sizeof(char), htons(ch->flow_len)  , fp);
    fwrite(ch->key               , sizeof(char) , htons(ch->key_len)  , fp);
    
    int header_size = sizeof(ch->magic)
                  + sizeof(*(ch->version))*4
                  + sizeof(ch->index_offset) 
                  + sizeof(ch->index_len) 
                  + sizeof(ch->nreads) 
                  + sizeof(ch->header_len) 
                  + sizeof(ch->key_len)  
                  + sizeof(ch->flow_len) 
                  + sizeof(ch->flowgram_format)
                  + (sizeof(char) * htons(ch->flow_len) )
                  + (sizeof(char) * htons(ch->key_len) ) ;
    
    if ( !(header_size % PADDING_SIZE == 0) ) {
        write_padding(fp, header_size);
    }
    
}

void write_sff_read_header(FILE *fp, sff_read_header *rh) {
   
   rh->header_len=ntohs(rh->header_len); 
   rh->name_len=ntohs(rh->name_len);
   rh->nbases=ntohl(rh->nbases);
   rh->clip_qual_left=ntohs(rh->clip_qual_left); 
   rh->clip_qual_right=ntohs(rh->clip_qual_right); 
   rh->clip_adapter_left=ntohs(rh->clip_adapter_left);
   rh->clip_adapter_right=ntohs(rh->clip_adapter_right);
   
   fwrite( &(rh->header_len)      , sizeof(uint16_t), 1, fp);
   fwrite(&(rh->name_len)        , sizeof(uint16_t), 1, fp);
   fwrite(&(rh->nbases)            , sizeof(uint32_t), 1, fp);
   fwrite(&(rh->clip_qual_left)     , sizeof(uint16_t), 1, fp);
   fwrite(&(rh->clip_qual_right)    , sizeof(uint16_t), 1, fp);
   fwrite(&(rh->clip_adapter_left)  , sizeof(uint16_t), 1, fp);
   fwrite(&(rh->clip_adapter_right) , sizeof(uint16_t), 1, fp);
   fwrite(rh->name               , sizeof(char), htons(rh->name_len), fp);
   
   int header_size = sizeof(rh->header_len) 
                  + sizeof(rh->name_len)
                  + sizeof(rh->nbases) 
                  + sizeof(rh->clip_qual_left) 
                  + sizeof(rh->clip_qual_right) 
                  + sizeof(rh->clip_adapter_left) 
                  + sizeof(rh->clip_adapter_right) 
                  + (sizeof(char) * htons(rh->name_len));
   
   
   if ( !(header_size % PADDING_SIZE == 0) ) {
        write_padding(fp, header_size);
   }
 
}

void write_sff_read_data(FILE *fp,  sff_read_data *rd, uint16_t nflows, uint32_t nbases) {
    
    /* sff files are in big endian notation so adjust appropriately */
    for (int i = 0; i < nflows; i++) {
        rd->flowgram[i] = ntohs(rd->flowgram[i]);
    }
    
    fwrite(rd->flowgram, sizeof(uint16_t), (size_t) nflows, fp);
    fwrite(rd->flow_index, sizeof(uint8_t), (size_t) nbases, fp);
    fwrite(rd->bases, sizeof(char), (size_t) nbases, fp);
    fwrite(rd->quality, sizeof(uint8_t), (size_t) nbases, fp);
    /* the section should be a multiple of 8-bytes, if not,
       it is zero-byte padded to make it so */

    int data_size = (sizeof(uint16_t) * nflows)    // flowgram size
                + (sizeof(uint8_t) * nbases)   // flow_index size
              +   (sizeof(char) * nbases)      // bases size
              +   (sizeof(uint8_t) * nbases);  // quality size
                  
    if ( !(data_size % PADDING_SIZE == 0) ) {
        write_padding(fp, data_size);
    }
}


void write_padding(FILE *fp, int header_size) {
    int remainder = PADDING_SIZE - (header_size % PADDING_SIZE);
    uint8_t padding[remainder];
    int i;
    for(i=0; i< remainder; ++i)
      padding[i] = 0;
    
    
    fwrite(padding, sizeof(uint8_t), remainder, fp);
}

void read_padding(FILE *fp, int header_size) {
    int remainder = PADDING_SIZE - (header_size % PADDING_SIZE);
    uint8_t padding[remainder];
    //size_t bytes = fread(padding, sizeof(uint8_t), remainder, fp);
    fread(padding, sizeof(uint8_t), remainder, fp);
}

void
free_sff_common_header(sff_common_header *h) {
    free(h->flow);
    free(h->key);
}

short  verify_sff_common_header(sff_common_header *h) {

    /* ensure that the magic file type is valid */
    if (h->magic != SFF_MAGIC) {
        fprintf(stderr, "The SFF header has magic value '%d' \n", h->magic);
        fprintf(stderr,
                "[err] : '%d' \n Program only knows how to deal an SFF magic value of type",
                SFF_MAGIC);
        free_sff_common_header(h);
        return -1;
        //exit(2);
    }

    /* ensure that the version header is valid */
    if ( memcmp(h->version, SFF_VERSION, SFF_VERSION_LENGTH) ) {
        //fprintf(stderr, "The SFF file has header version: ");
        int i;
        char *sff_header_version = h->version;
        for (i=0; i < SFF_VERSION_LENGTH; i++) {
            printf("0x%02x ", sff_header_version[i]);
        }
        //printf("\n");
        //fprintf(stderr,
        //        "[err] : ", 
        //        "only knows how to deal an SFF header version: ");
        //char valid_header_version[/*SFF_VERSION_LENGTH*/4] = "0001";//\0\0\0\1";/*SFF_VERSION*/;
        char* valid_header_version = (char*)SFF_VERSION;
        for (i=0; i < SFF_VERSION_LENGTH; i++) {
            printf("0x%02x ", valid_header_version[i]);
            //printf("0x%02x ", valid_header[i]);
        }
        free_sff_common_header(h);
        return -1;//exit(2);
    }
    
    return 0;
}

void read_sff_read_header(FILE *fp, sff_read_header *rh) {
    char *name;
    int header_size;

    size_t bytes = fread(&(rh->header_len)        , sizeof(uint16_t), 1, fp);
    bytes = fread(&(rh->name_len)          , sizeof(uint16_t), 1, fp);
    bytes = fread(&(rh->nbases)            , sizeof(uint32_t), 1, fp);
    bytes = fread(&(rh->clip_qual_left)    , sizeof(uint16_t), 1, fp);
    bytes = fread(&(rh->clip_qual_right)   , sizeof(uint16_t), 1, fp);
    bytes = fread(&(rh->clip_adapter_left) , sizeof(uint16_t), 1, fp);
    bytes = fread(&(rh->clip_adapter_right), sizeof(uint16_t), 1, fp);

    /* sff files are in big endian notation so adjust appropriately */
    /* Linux version, not in use any more
    rh->header_len         = htobe16(rh->header_len);
    rh->name_len           = htobe16(rh->name_len);
    rh->nbases             = htobe32(rh->nbases);
    rh->clip_qual_left     = htobe16(rh->clip_qual_left);
    rh->clip_qual_right    = htobe16(rh->clip_qual_right);
    rh->clip_adapter_left  = htobe16(rh->clip_adapter_left);
    rh->clip_adapter_right = htobe16(rh->clip_adapter_right);
     */
    
    rh->header_len         = htons(rh->header_len);
    rh->name_len           = htons(rh->name_len);
    rh->nbases             = htonl(rh->nbases);
    rh->clip_qual_left     = htons(rh->clip_qual_left);
    rh->clip_qual_right    = htons(rh->clip_qual_right);
    rh->clip_adapter_left  = htons(rh->clip_adapter_left);
    rh->clip_adapter_right = htons(rh->clip_adapter_right);
     
    /* finally appropriately allocate and read the read_name string */
    name = (char *) malloc( rh->name_len * sizeof(char) );
    if (!name) {
        fprintf(stderr,
                "Out of memory! Could not allocate header flow string!\n");
        exit(1);
    }

    bytes = fread(name, sizeof(char), rh->name_len, fp);
    rh->name = name;

    /* the section should be a multiple of 8-bytes, if not,
       it is zero-byte padded to make it so */

    header_size = sizeof(rh->header_len)
                  + sizeof(rh->name_len)
                  + sizeof(rh->nbases)
                  + sizeof(rh->clip_qual_left)
                  + sizeof(rh->clip_qual_right)
                  + sizeof(rh->clip_adapter_left)
                  + sizeof(rh->clip_adapter_right)
                  + (sizeof(char) * rh->name_len);

    if ( !(header_size % PADDING_SIZE == 0) ) {
        read_padding(fp, header_size);
    }
}

void
free_sff_read_header(sff_read_header *h) {
    free(h->name);
}

void read_sff_read_data(FILE *fp, 
                   sff_read_data *rd, 
                   uint16_t nflows, 
                   uint32_t nbases) {
    uint16_t *flowgram;
    uint8_t *flow_index;
    uint8_t *quality;
    char *bases;
    int data_size;
    int i;

    /* allocate the appropriate arrays */
    flowgram   = (uint16_t *) malloc( nflows * sizeof(uint16_t) );
    if (!flowgram) {
        fprintf(stderr,
                "Out of memory! Could not allocate for a read flowgram!\n");
        exit(1);
    }

    flow_index = (uint8_t *) malloc( nbases * sizeof(uint8_t)  );
    if (!flow_index) {
        fprintf(stderr,
                "Out of memory! Could not allocate for a read flow index!\n");
        exit(1);
    }

    quality = (uint8_t *) malloc( nbases * sizeof(uint8_t)  );
    if (!quality) {
        fprintf(stderr,
                "Out of memory! Could not allocate for a read quality string!\n");
        exit(1);
    }

    bases = (char * ) malloc( nbases * sizeof(char) );
    if (!bases) {
        fprintf(stderr,
                "Out of memory! Could not allocate for a read base string!\n");
        exit(1);
    }

    /* read from the sff file and assign to sff_read_data */
    size_t bytes = fread(flowgram, sizeof(uint16_t), (size_t) nflows, fp);

    /* sff files are in big endian notation so adjust appropriately */
    for (i = 0; i < nflows; i++) {
        flowgram[i] = htons(flowgram[i]);
    }
    rd->flowgram = flowgram;

    bytes = fread(flow_index, sizeof(uint8_t), (size_t) nbases, fp);
    rd->flow_index = flow_index;

    bytes = fread(bases, sizeof(char), (size_t) nbases, fp);
    rd->bases = bases;

    bytes = fread(quality, sizeof(uint8_t), (size_t) nbases, fp);
    rd->quality = quality;
    
    
    /* the section should be a multiple of 8-bytes, if not,
       it is zero-byte padded to make it so */

    data_size = (sizeof(uint16_t) * nflows)    // flowgram size
                + (sizeof(uint8_t) * nbases)   // flow_index size
                + (sizeof(char) * nbases)      // bases size
                + (sizeof(uint8_t) * nbases);  // quality size
                  
    if ( !(data_size % PADDING_SIZE == 0) ) {
        read_padding(fp, data_size);
    }
}

void free_sff_read_data(sff_read_data *d) {
    free(d->flowgram);
    free(d->flow_index);
    free(d->bases);
    free(d->quality);
}

/* as described in section 13.3.8.2 "Read Header Section" in the
   454 Genome Sequencer Data Analysis Software Manual
   see (http://sequence.otago.ac.nz/download/GS_FLX_Software_Manual.pdf) */

void get_clip_values(sff_read_header rh,
                int trim_flag,
                int *left_clip,
                int *right_clip) {
    if (trim_flag) {
        (*left_clip)  = (int) max(1, max(rh.clip_qual_left, rh.clip_adapter_left));

        // account for the 1-based index value
        *left_clip = *left_clip - 1;

        (*right_clip) = (int) min( (rh.clip_qual_right    == 0 ? rh.nbases : rh.clip_qual_right   ), (rh.clip_adapter_right == 0 ? rh.nbases : rh.clip_adapter_right) );
    }
    else 
    {
        (*left_clip)  = 0;
        (*right_clip) = (int) rh.nbases;
    }
}

char* get_read_bases(sff_read_data rd, int left_clip, int right_clip) {
    char *bases;

    // account for NULL termination
    int bases_length = (right_clip - left_clip) + 1;

    // inititalize the bases string/array
    bases = (char *) malloc( bases_length * sizeof(char) );
    if (!bases) {
        fprintf(stderr, "Out of memory! For read bases string!\n");
        exit(1);
    }
    memset(bases, '\0', (size_t) bases_length);

    // copy the relative substring
    int start = left_clip;
    int stop  = right_clip;
    int i, j = 0;

    for (i = start; i < stop; i++) {
        *(bases + j) = *(rd.bases + i);
        j++;
    }

    return bases;
}

uint8_t*
get_read_quality_values(sff_read_data rd,
                        int left_clip,
                        int right_clip) {
    uint8_t *quality;

    // account for NULL termination
    int quality_length = (right_clip - left_clip) + 1;

    // inititalize the quality array
    quality = (uint8_t *) malloc( quality_length * sizeof(uint8_t) );
    if (!quality) {
        fprintf(stderr, "Out of memory! For read quality array!\n");
        exit(1);
    }
    memset(quality, '\0', (size_t) quality_length);

    // copy the relative substring
    int start = left_clip;
    int stop  = right_clip;
    int i, j = 0;

    for (i = start; i < stop; i++) {
        *(quality + j) = *(rd.quality + i);
        j++;
    }

    return quality;
}

void read_manifest(FILE *fp) 
{
    manifest_size =  sff_file_size - ftell( fp );
    manifest = (char*)malloc( manifest_size  * sizeof(char) );
    if (!manifest) 
    {
        fprintf(stderr,
                "Out of memory! Could not allocate header flow string!\n");
        exit(1);
    }

   //int i = fread(manifest, sizeof(char), manifest_size, fp);
   fread(manifest, sizeof(char), manifest_size, fp);
   
}

void write_manifest(FILE *fp) 
{
    
    fwrite(manifest, sizeof(char), manifest_size, fp);
    
}

long get_sff_file_size(FILE *fp)
{
  fseek(fp, 0, SEEK_END); // seek to end of file
  //long sff_file_size = ftell(fp); // get current file pointer
  sff_file_size = ftell(fp); // get current file pointer
  fseek(fp, 0, SEEK_SET); // seek back to beginning of file
// proceed with allocating memory and reading the file
  
  return sff_file_size;
}
