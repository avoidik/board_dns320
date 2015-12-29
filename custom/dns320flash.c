
#include <stdio.h>
#include <stdlib.h>

#define UINT32 __uint32_t
#define BYTE unsigned char

#define ALGIN4(_x) (_x + (4 - (_x % 4)))
typedef struct {
    /* 0x00 - 0x03 */ UINT32 uImageOffset;
    /* 0x04 - 0x07 */ UINT32 uImageLenght;
    /* 0x08 - 0x0B */ UINT32 uRamDiskOffset;
    /* 0x0C - 0x0F */ UINT32 uRamDiskLenght;
    /* 0x10 - 0x13 */ UINT32 ImageOffset;
    /* 0x14 - 0x17 */ UINT32 ImageLenght;
    /* 0x18 - 0x1B */ UINT32 DefaultOffset;
    /* 0x1C - 0x1F */ UINT32 DefaultLenght;
    /* 0x20 - 0x23 */ UINT32 uImageChecksum;
    /* 0x24 - 0x27 */ UINT32 uRamDiskChecksum;
    /* 0x28 - 0x2B */ UINT32 ImageChecksum;
    /* 0x2C - 0x2F */ UINT32 DefaultChecksum;
    /* 0x30 - 0x30 */ BYTE magic_0;         // always set 0x55 same as DNS-323
    /* 0x31 - 0x31 */ BYTE magic_1;         // always set 0xAA same as DNS-323
    /* 0x32 - 0x3A */ BYTE strModelName[9]; // always contain string DNS323D1U with no terminateor \0 same as DNS-323
    /* 0x3B - 0x3B */ BYTE magic_2;         // always set 0xAA same as DNS-323
    /* 0x3C - 0x3C */ BYTE productId;       // always set 0x00 same as DNS-323
    /* 0x3D - 0x3D */ BYTE customId;        // always set 0x08
    /* 0x3E - 0x3E */ BYTE modelId;         // always set 0x07
    /* 0x3F - 0x3F */ BYTE hardwareId;      // always set 0x01
    /* 0x40 - 0x40 */ BYTE subId;           // always set 0x00
    /* 0x41 - 0x7B */ BYTE Reseverd[59];    // not in use
    /* 0x7C - 0x7F */ UINT32 NextOffset;    // next offset;
} DLINK_DNS320_FIRMWARE_HEAD_128BYTE;

void showFirmware(char *flashfilename, int extract_frimware, char *kernelfilename, char *ramdiskfilename, char *imagefilename, char * configfilename );
int pack_firmware(char *flashfilename, char *kernelfilename, char *ramdiskfilename, char *imagefilename, char * configfilename );

void firmware_unpack_extract(char *inFileName, char *outFileName, UINT32 inFileOffset, UINT32 dataLenght);
UINT32 firmware_calc_checksum(char *inFileName);
int calc_checksum(char *filename);
int check_size(char *filename, UINT32 *file_size, UINT32 max_size);
char * strtolower(const char *ret);
int compare(char *filename1, char *filename2);


char *strtolower(const char *ret)
{
    char *s = ret;
    for ( s = ret;*s != 0; s++)
    {
        if (*s != 0)
        { *s = tolower(*s); }
    }
    return ret;
 }

int main(int argc, char **argv)
{
    int showHelp = 1;

    if (argc == 3)
    {
        strtolower(argv[1]);
        if ( strcmp("-s",argv[1]) == 0)
        { showFirmware(argv[2], 0, "", "", "", "" ); showHelp--; }

        if ( strcmp("-x",argv[1]) == 0)
        { showFirmware(argv[2], 1, "uImage", "uRamDisk", "image.cfs", "default.tar.gz" ); showHelp--; }

        if ( strcmp("-mf",argv[1]) == 0)
        { pack_firmware(argv[2],"uImage", "uRamDisk", "image.cfs", "default.tar.gz"); showHelp--; }

    }

    if (argc == 4)
    {
        strtolower(argv[1]);
        if ( strcmp("-cmp",argv[1]) == 0)
        { compare(argv[2], argv[3]); showHelp--; }
    }

    if (argc == 8)
    {
        strtolower(argv[1]);
        if ( strcmp("-m",argv[1]) == 0)
        {
            strtolower(argv[6]);
            if ( strcmp("-f",argv[6]) == 0)
            { pack_firmware(argv[7],argv[2], argv[3], argv[4], argv[5]); showHelp--; }
        }

        if ( strcmp("-x",argv[1]) == 0)
        {
            strtolower(argv[3]);
            if ( strcmp("-o",argv[3]) == 0)
            { showFirmware(argv[2], 1, argv[4], argv[5], argv[6], argv[7] ); showHelp--; }
        }
    }

    if (showHelp)
    {
        printf("Copyright 2011 by Magnus Olsen (magnus@greatlord.com)\n");
        printf("This program are under licen GPL 2.0\n\n");
        printf("Merge kernel (uImage) + ramdisk (uRamDisk) + image (image.cfs) and\nconfig (default.tar.gz) to a flash image for dlink DNS320\n\n");
        printf("dns320flash -m kernel ramdisk image config -f dlink320firmware\n\n\n");
        printf("dns320flash -mf dlink320firmware dlink320firmware\n\n\n");
        printf("Extract flash image to kernel (uImage) + ramdisk (uRamDisk) + \nimage (image.cfs) and config (default.tar.gz) for dlink DNS320\n\n");
        printf("dns320flash -x dlink320firmware -o kernel ramdisk image config\n");
        printf("dns320flash -x dlink320firmware \n\n\n");
        printf("Show flash image contains for dlink DNS320\n\n");
        printf("dns320flash -s dlink320firmware\n\n\n");
    }

    return 0;
}

void showFirmware(char *flashfilename, int extract_frimware, char *kernelfilename, char *ramdiskfilename, char *imagefilename, char * configfilename )
{
    int read_byte;
    FILE *fp;
    char str[10];
    DLINK_DNS320_FIRMWARE_HEAD_128BYTE dnsHeader;


    fp = fopen(flashfilename,"rb");
    if (fp)
    {
        memset( &dnsHeader,0,sizeof(DLINK_DNS320_FIRMWARE_HEAD_128BYTE));
        read_byte = fread(&dnsHeader, 1, sizeof(DLINK_DNS320_FIRMWARE_HEAD_128BYTE),fp);
        fclose(fp);

        if (read_byte != sizeof(DLINK_DNS320_FIRMWARE_HEAD_128BYTE))
        {
            printf("not vaild firmware for DNS320");
            exit(0);
        }

        memcpy(&str,dnsHeader.strModelName,9);
        str[9] = 0;

/*
dnsHeader.productId != 0x00  // same as DNS-320
dnsHeader.customId  != 0x08  // same as DNS-320
dnsHeader.modelId   != 0x05  // not same as DNS-320
dnsHeader.hardwareId != 0x01 // same as DNS-320
dnsHeader.subId      != 0x00 // same as DNS-320
dnsHeader.strModelName != "DNS-325 U"
*/
        if  ( (dnsHeader.magic_0 != 0x55) ||
              (dnsHeader.magic_1 != 0xAA) ||
              (dnsHeader.magic_2 != 0xAA) ||
              (dnsHeader.productId != 0x00) ||
              (dnsHeader.customId != 0x08) ||
              (dnsHeader.modelId != 0x07) ||
              (dnsHeader.hardwareId != 0x01) ||
              (dnsHeader.subId != 0x00) ||
              (strcmp(str,"DNS323D1U") != 0) )
        {
            printf("not vaild firmware for DNS320");
            exit(0);
        }

        printf("magic_0     : %02x\n",dnsHeader.magic_0);
        printf("magic_1     : %02x\n",dnsHeader.magic_1);
        printf("string      : %s\n",str);
        printf("magic_2     : %02x\n",dnsHeader.magic_2);
        printf("productId   : %02x\n",dnsHeader.productId);
        printf("customId    : %02x\n",dnsHeader.customId);
        printf("modelId     : %02x\n",dnsHeader.modelId);
        printf("hardwareId  : %02x\n",dnsHeader.hardwareId);
        printf("subId       : %02x\n",dnsHeader.subId);
        printf("Next_offset : %08x\n\n",dnsHeader.NextOffset);

        printf("Kernel (uImage) Offset   head : %08x\n",dnsHeader.uImageOffset);
        printf("Kernel (uImage) Lenght   head : %08x\n",dnsHeader.uImageLenght);
        printf("Kernel (uImage) Checksum head : %08x\n",dnsHeader.uImageChecksum);


        if (extract_frimware)
        {
            firmware_unpack_extract(flashfilename, kernelfilename, dnsHeader.uImageOffset, dnsHeader.uImageLenght);
            printf("Kernel (uImage) Checksum file : %08x\n\n",calc_checksum(kernelfilename));
        }
        else
        { printf("\n"); }

        printf("uRamDisk        Offset   head : %08x\n",dnsHeader.uRamDiskOffset);
        printf("uRamDisk        Lenght   head : %08x\n",dnsHeader.uRamDiskLenght);
        printf("uRamDisk        Checksum head : %08x\n",dnsHeader.uRamDiskChecksum);
        if (extract_frimware)
        {

            firmware_unpack_extract(flashfilename, ramdiskfilename, dnsHeader.uRamDiskOffset, dnsHeader.uRamDiskLenght);
            printf("uRamDisk        Checksum file : %08x\n\n",calc_checksum(ramdiskfilename));
        }
        else
        { printf("\n"); }


        printf("image           Offset   head : %08x\n",dnsHeader.ImageOffset);
        printf("image           Lenght   head : %08x\n",dnsHeader.ImageLenght);
        printf("image           Checksum head : %08x\n",dnsHeader.ImageChecksum);

        if (extract_frimware)
        {
            firmware_unpack_extract(flashfilename, imagefilename, dnsHeader.ImageOffset, dnsHeader.ImageLenght);
            printf("image           Checksum file : %08x\n\n",calc_checksum(imagefilename));
        }
        else
        { printf("\n"); }

        printf("Default         Offset   head : %08x\n",dnsHeader.DefaultOffset);
        printf("Default         Lenght   head : %08x\n",dnsHeader.DefaultLenght);
        printf("Default         Checksum head : %08x\n",dnsHeader.DefaultChecksum);


        if (extract_frimware)
        {
            firmware_unpack_extract(flashfilename, configfilename, dnsHeader.DefaultOffset, dnsHeader.DefaultLenght);
            printf("Default         Checksum file : %08x\n\n",calc_checksum(configfilename));
        }
        else
        { printf("\n"); }




    }

}

void firmware_unpack_extract(char *inFileName, char *outFileName, UINT32 inFileOffset, UINT32 dataLenght)
{
    FILE *read_fp;
    FILE *save_fp;
    BYTE *ch;
    UINT32 lenght = 0;
    printf("Extracting %s from %s\n",outFileName, inFileName);
    read_fp = fopen(inFileName,"rb");
    if (read_fp)
    {
        fseek(read_fp,inFileOffset, SEEK_SET);
        save_fp = fopen(outFileName,"wb+");
        if (save_fp)
        {
            while(lenght<dataLenght)
            {
                fread(&ch,1,1,read_fp);
                fwrite(&ch,1,1,save_fp);
                lenght++;
            }
            printf("writing done\n");
            fclose(save_fp);
        }
        fclose(read_fp);
    }
}

int pack_firmware(char *flashfilename, char *kernelfilename, char *ramdiskfilename, char *imagefilename, char * configfilename )
{
    FILE *fp_read;
    FILE *fp_save;
    int i, kernelSize,ramdiskSize, imageSize, configSize;
    UINT32 byte;
    DLINK_DNS320_FIRMWARE_HEAD_128BYTE dnsHeader;

    memset(&dnsHeader,0,sizeof(DLINK_DNS320_FIRMWARE_HEAD_128BYTE));


    if ( check_size(kernelfilename,         &dnsHeader.uImageLenght,    0x00300000) == -1)
    { return -1; }

    if ( check_size(ramdiskfilename,       &dnsHeader.uRamDiskLenght,  0x00300000) == -1)
    { return -1; }

    if ( check_size(configfilename, &dnsHeader.DefaultLenght,   0x00300000) == -1)
    { return -1; }

    if ( check_size(imagefilename,      &dnsHeader.ImageLenght,     0x04000000) == -1)
    { return -1; }

    // Setup header id
    dnsHeader.magic_0 = 0x55;
    dnsHeader.magic_1 = 0xAA;
    dnsHeader.magic_2 = 0xAA;
    dnsHeader.productId = 0x00;
    dnsHeader.customId = 0x08;
    dnsHeader.modelId = 0x07;
    dnsHeader.hardwareId = 0x01;
    dnsHeader.subId = 0x00;
    memcpy(dnsHeader.strModelName,"DNS323D1U",9);

    // Setup header checksum
    dnsHeader.uImageChecksum = calc_checksum(kernelfilename);
    dnsHeader.uRamDiskChecksum = calc_checksum(ramdiskfilename);
    dnsHeader.DefaultChecksum = calc_checksum(configfilename);
    dnsHeader.ImageChecksum = calc_checksum(imagefilename);

    // Fix Algin issue for header
    kernelSize = dnsHeader.uImageLenght;
    ramdiskSize = dnsHeader.uRamDiskLenght;
    imageSize = dnsHeader.ImageLenght;
    configSize = dnsHeader.DefaultLenght;

    if ((dnsHeader.uImageLenght % 4) != 0)
    { dnsHeader.uImageLenght = ALGIN4(dnsHeader.uImageLenght); }

    if ((dnsHeader.uRamDiskLenght % 4) != 0)
    { dnsHeader.uRamDiskLenght = ALGIN4(dnsHeader.uRamDiskLenght); }

    if ((dnsHeader.ImageLenght % 4) != 0)
    { dnsHeader.ImageLenght = ALGIN4(dnsHeader.ImageLenght); }

    if ((dnsHeader.DefaultLenght % 4) != 0)
    { dnsHeader.DefaultLenght = ALGIN4(dnsHeader.DefaultLenght); }

    // Setup header Offset
    dnsHeader.uImageOffset = sizeof(DLINK_DNS320_FIRMWARE_HEAD_128BYTE);
    dnsHeader.uRamDiskOffset = sizeof(DLINK_DNS320_FIRMWARE_HEAD_128BYTE) + dnsHeader.uImageLenght;
    dnsHeader.ImageOffset = sizeof(DLINK_DNS320_FIRMWARE_HEAD_128BYTE) + dnsHeader.uImageLenght + dnsHeader.uRamDiskLenght ;

    dnsHeader.DefaultOffset = sizeof(DLINK_DNS320_FIRMWARE_HEAD_128BYTE) + dnsHeader.uImageLenght + dnsHeader.uRamDiskLenght + dnsHeader.ImageLenght ;
    // print out info
    printf("Kernel (uImage) Offset    : %08x\n",dnsHeader.uImageOffset);
    printf("Kernel (uImage) Lenght    : %08x\n",dnsHeader.uImageLenght);
    printf("Kernel (uImage) Checksum  : %08x\n\n",dnsHeader.uImageChecksum);
    printf("uRamDisk        Offset    : %08x\n",dnsHeader.uRamDiskOffset);
    printf("uRamDisk        Lenght    : %08x\n",dnsHeader.uRamDiskLenght);
    printf("uRamDisk        Checksum  : %08x\n\n",dnsHeader.uRamDiskChecksum);
    printf("image           Offset    : %08x\n",dnsHeader.ImageOffset);
    printf("image           Lenght    : %08x\n",dnsHeader.ImageLenght);
    printf("image           Checksum  : %08x\n\n",dnsHeader.ImageChecksum);
    printf("Default         Offset    : %08x\n",dnsHeader.DefaultOffset);
    printf("Default         Lenght    : %08x\n",dnsHeader.DefaultLenght);
    printf("Default         Checksum  : %08x\n\n",dnsHeader.DefaultChecksum);

    if ( (fp_save = fopen(flashfilename,"wb+")) == 0)
    {
        printf("Error : Can not merge firmware\n");
        return -1;
    }

    fwrite(&dnsHeader,1,sizeof(DLINK_DNS320_FIRMWARE_HEAD_128BYTE),fp_save);

    if ( (fp_read = fopen(kernelfilename,"rb")) == 0)
    {
        printf("Error : Can not merge firmware\n");
        return -1;
    }
    for(i=0;i<dnsHeader.uImageLenght/4;i++)
    {
        fread(&byte,1,4,fp_read);
        fwrite(&byte,1,4,fp_save);
    }
    fclose(fp_read);

    if ( (fp_read = fopen(ramdiskfilename,"rb")) == 0)
    {
        printf("Error : Can not merge firmware\n");
        return -1;
    }
    for(i=0;i<dnsHeader.uRamDiskLenght/4;i++)
    {
        fread(&byte,1,4,fp_read);
        fwrite(&byte,1,4,fp_save);
    }
    fclose(fp_read);

    if ( (fp_read = fopen(imagefilename,"rb")) == 0)
    {
        printf("Error : Can not merge firmware\n");
        return -1;
    }
    for(i=0;i<dnsHeader.ImageLenght/4;i++)
    {
        fread(&byte,1,4,fp_read);
        fwrite(&byte,1,4,fp_save);
    }

    fclose(fp_read);

    if ( (fp_read = fopen(configfilename,"rb")) == 0)
    {
        printf("Error : Can not merge firmware\n");
        return -1;
    }

    for(i=0;i<dnsHeader.DefaultLenght/4;i++)
    {
        fread(&byte,1,4,fp_read);
        fwrite(&byte,1,4,fp_save);
    }
    fclose(fp_read);


    fclose(fp_save);

    return 0;
}


int check_size(char *filename, UINT32 *file_size, UINT32 max_size)
{
    FILE *fp_read;

    if ((fp_read = fopen(filename,"rb")) )
    {
        fseek(fp_read,0,SEEK_END);
        *file_size = ftell(fp_read);
        if ( *file_size >= max_size )
        {
            printf("\nMake firmware error\n");
            printf("Ramdisk sizes can not more than %d byte",max_size);
            return -1;
        }
        fclose(fp_read);
    }
    else
    {
        printf("Can not open file uImage\n");
    }
    return 0;
}




int calc_checksum(char *filename)
{
    FILE *fp_read;
    UINT32 chk_sum = 0;
    int byte,size,filesize;

    if (!(fp_read = fopen(filename,"rb")))
    {
        printf("Error: Can't open %s file !\n", filename);
        return 0;
    }

    fseek(fp_read, 0, SEEK_END);
    filesize = ftell(fp_read);
    fseek(fp_read, 0, SEEK_SET);

    for(size=0;size<filesize;size+=4)
    {
        byte = 0;
        fread(&byte, 1, 4, fp_read);
        chk_sum ^= byte;
    }

    fclose(fp_read);
    return chk_sum;
}

int compare(char *filename1, char *filename2)
{
    FILE *fp_read1;
    FILE *fp_read2;
    int file1_size;
    int file2_size;
    int i;

    if ((fp_read1 = fopen(filename1,"rb")) == 0)
    { printf("Error : Can not open file %s\n",filename1); return -1; }

    if ((fp_read2 = fopen(filename2,"rb")) == 0)
    { printf("Error : Can not open file %s\n",filename2); fclose(fp_read1); return -1; }

    fseek(fp_read1,0,SEEK_END);
    fseek(fp_read2,0,SEEK_END);

    file1_size = ftell(fp_read1);
    file2_size = ftell(fp_read2);

    fseek(fp_read1,0,SEEK_SET);
    fseek(fp_read2,0,SEEK_SET);

    if (file1_size != file2_size)
    {
        printf("Error : File %s and file %s does not have same filesize\n",filename1,filename2);
        fclose(fp_read1);
        fclose(fp_read2);
        return -1;
    }

    for (i=0;i<file1_size;i++)
    {
        if ( fgetc(fp_read1) != fgetc(fp_read2) )
        {
            printf("Error : Byte are not same\n");
            printf("FileOffset : %08lx\n",ftell(fp_read1));
            fclose(fp_read1);
            fclose(fp_read2);
            return -1;
        }

    }
    fclose(fp_read1);
    fclose(fp_read2);
    return 0;
}


