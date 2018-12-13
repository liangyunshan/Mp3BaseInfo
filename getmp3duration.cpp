#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFSIZE 256

#include "getmp3duration.h"

static int ExtractI4(unsigned char *buf)
{
    int x;
    // big endian extract

    x = buf[0];
    x <<= 8;
    x |= buf[1];
    x <<= 8;
    x |= buf[2];
    x <<= 8;
    x |= buf[3];

    return x;
}


int GetXingHeader(XHEADDATA *X,  unsigned char *buf)
{
    int i, head_flags;
    int h_id, h_mode, h_sr_index;
    static int sr_table[4] = { 44100, 48000, 32000, 99999 };

    // get Xing header data
    X->flags = 0;     // clear to null incase fail

    // get selected MPEG header data
    h_id       = (buf[1] >> 3) & 1;
    h_sr_index = (buf[2] >> 2) & 3;
    h_mode     = (buf[3] >> 6) & 3;

    // determine offset of header
    if( h_id ) {        // mpeg1
        if( h_mode != 3 ) buf+=(32+4);
        else              buf+=(17+4);
    }
    else {      // mpeg2
        if( h_mode != 3 ) buf+=(17+4);
        else              buf+=(9+4);
    }

    if( buf[0] != 'X' ) return 0;    // fail
    if( buf[1] != 'i' ) return 0;    // header not found
    if( buf[2] != 'n' ) return 0;
    if( buf[3] != 'g' ) return 0;
    buf+=4;

    X->h_id = h_id;
    X->samprate = sr_table[h_sr_index];
    if( h_id == 0 ) X->samprate >>= 1;

    head_flags = X->flags = ExtractI4(buf); buf+=4;      // get flags

    if( head_flags & FRAMES_FLAG ) {X->frames = ExtractI4(buf); buf+=4;}
    if( head_flags & BYTES_FLAG )  {X->bytes = ExtractI4(buf); buf+=4;}


    if( head_flags & TOC_FLAG ) {
        if( X->toc != NULL ) {
            for(i=0;i<100;i++) X->toc[i] = buf[i];
        }
        buf+=100;
    }

    X->vbr_scale = -1;
    if( head_flags & VBR_SCALE_FLAG )  {X->vbr_scale = ExtractI4(buf); buf+=4;}

    if( X->toc != NULL ) {
        for(i=0;i<100;i++) {
            if( (i%10) == 0 ) printf("\n");
            printf(" %3d", (int)(X->toc[i]));
        }
    }

    if (!X->frames && X->bytes)
        return -1;
    else
        return 1;       // success
}

int get_mp3header(MPEG_HEAD * mh,off_t size,int fd)
{
    unsigned char bytes[8];
    unsigned char bits[33];
    unsigned char buf[BUFSIZE];
    register int i;
    int times=0;
    int version=0,layer=0,crc_protected=0,bitrate=0;
    int length=0,mins,secs;

    XHEADDATA xing_header;


    int v1l1[16] ={-1,32,64,96,128,160,192,224,256,288,320,352,384,416,448,-1};
    int v1l2[16] ={-1,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,-1};
    int v1l3[16] ={-1,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,-1};
    int v2l1[16] ={-1,32,48,56, 64, 80, 96,112,128,144,160,176,192,224,256,-1};
    int v2l23[16]={-1,8 ,16,24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,-1};
    
    mh->vbr=0;
    mh->skiped=0;
    mh->id3v2skiped=0;
    xing_header.frames=0;
    xing_header.samprate=1;
    xing_header.toc=0;
    

    do {

        if (read(fd,buf,BUFSIZE)>0)
            times++;
        else break;


        mh->valid=0;
        mh->enctype=1;
        // Detect VBR (header on first frame's data) //
        if ( times==1 ) {
            if(buf[0] == 'I' && buf[1]=='D' && buf[2]=='3' ) { // id3v2, must skip it
                int num=0;
                //skip id3v2
                //synchronize synchsafe integers
                num=buf[6];
                num=num*128+buf[7];
                num=num*128+buf[8];
                num=num*128+buf[9];

                num+=10; //we ignore the footer (would be +20)
                //printf("H:%d\n",num);
                lseek(fd,num,SEEK_SET);
                mh->skiped+=num;
                mh->id3v2skiped=num;

                if (read(fd,buf,BUFSIZE)>0); //times++;
                else break;

            }

            if ((i=GetXingHeader(&xing_header,buf))) { //vbr
                mh->vbr=1;
                if (i<0) mh->valid=0;else mh->valid=1;
            }

        }

        i=0;
        while ((i < BUFSIZE) && !((buf[i]==0xFF) && ((buf[i+1] & 0xF0) == 0xF0)))
            i++;

        memcpy(&bytes[1],&buf[i],4);

        mh->skiped+=i;

        for (i=1;i<33;i++) {
            bits[i] = (bytes[ ((i - 1)/ 8) +1] >> ((7 - (i -1) % 8))) & 0x01;
        }

        if (bits[12] && bits[13]) version = 1;
        else if (bits[12] && !bits[13]) version =2 ;
        else if (!bits[12] && !bits[13]) version = 25;
        else version =0;

        mh->version=version;

        if (!bits[14] && bits[15]) layer = 3;
        else if(bits[14] && !bits[15]) layer = 2;
        else if(bits[14] && bits[15]) layer = 1;
        else layer=0;

        mh->layer=layer;

        if(bits[16]) crc_protected=1;
        else crc_protected=0;

        i = (bits[17] << 3) + (bits[18] << 2) + (bits[19] << 1) + bits[20];

        if (version == 1) {
            if (layer == 3)
                bitrate = v1l3[i];
            else if (layer == 2)
                bitrate = v1l2[i];
            else if (layer == 1)
                bitrate = v1l1[i];
            else bitrate=-1 ;
        }
        else if ((version == 2) || (version == 25)) {
            if (layer == 1)
                bitrate = v2l1[i];
            else if ((layer == 2) || (layer == 3))
                bitrate = v2l23[i];
            else bitrate=-1 ;
        }
        else bitrate=-1  ;

        mh->bitrate=bitrate;

        if(!bits[25] && !bits[26]) strcpy(mh->mode,"stereo");
        else if(!bits[25] && bits[26]) strcpy(mh->mode,"joint-stereo");
        else if(bits[25] && !bits[26]) strcpy(mh->mode,"dual channel");
        else if(bits[25] && bits[26])  strcpy(mh->mode,"Mono");
        else mh->mode[0]=0;

        if (!mh->vbr)
            length=(size*8)/(bitrate*1000);
        else if (mh->valid)
            length=xing_header.frames*1152/xing_header.samprate;
        else
            mh->length=mh->mins=mh->secs=length=0;


        if (bitrate>0 && length>0) {
            mins=length/60;
            secs=(length%60);
            mh->length=length;
            mh->mins=mins;
            mh->secs=secs;
            mh->valid=1;
        }

    } while(!mh->valid && times<50 &&!mh->vbr);

    return (mh->valid);
}