unsigned short rle(unsigned short len,unsigned char *in_buffer,
        unsigned char *out_buffer);

unsigned short unrle(unsigned short len,unsigned char *in_buffer,
        unsigned char *out_buffer);

/****************************************************************************/
/*                   Encode (compress) the input string.                    */
/*   The routine looks for groups of identical characters and replaces them */
/*   with the character  0xBB, a word denoting the number of characters to  */
/*   duplicate, followed by the character to duplicate.                     */
/*                                                                          */
/****************************************************************************/

unsigned short rle(unsigned short len,unsigned char *in_buffer,
        unsigned char *out_buffer)
{
    unsigned short count=0,how_many=0;
    unsigned char dupl;

    while(len) {
        if ((*in_buffer==0xBB) || ((*in_buffer==*(in_buffer+1)) &&
            (*in_buffer==*(in_buffer+2)) && (*in_buffer==*(in_buffer+3)) &&
            (*in_buffer==*(in_buffer+3)) && (*in_buffer==*(in_buffer+4)))) {
        dupl=*in_buffer;
        how_many=0;
        while((*in_buffer++==dupl) && (len)) {
            how_many++;
            len--;
        }
        *out_buffer++=0xBB;
        *out_buffer++=(unsigned char)how_many;
        *out_buffer++=(unsigned char)(how_many>>8);
        *out_buffer++=dupl;
        count+=4;
        in_buffer--;
        } else {
            *out_buffer++ = *in_buffer++;
            count++;
            len--;
        }
    }
    return count;
}

unsigned short unrle(unsigned short len,unsigned char *in_buffer,
        unsigned char *out_buffer)
{
    unsigned short i=0,count=0;
    unsigned char c;
    unsigned char *limit=in_buffer+len;

    while (in_buffer<limit) {
        if (*in_buffer==0xBB) {
            in_buffer++;
            i=(unsigned short)(*in_buffer++);
            i=i|(unsigned short)(*in_buffer++<<8);
            c=*in_buffer++;
            for (; i >0; i--,*out_buffer++=c,count++)
            ;
        } else {
            *out_buffer++=*in_buffer++;
            count++;
        }
    }
    return count;
}
