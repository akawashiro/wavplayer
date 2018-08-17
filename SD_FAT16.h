#include <avr/io.h>

#define SD_OK 1
#define SD_FAILED 0
#define SD_EOF 2

#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define SCK 5
#define MOSI 3
#define MISO 4
#define SS_DDR DDRC
#define SS_PORT PORTC
#define SS 0

typedef struct _MASTER_BOOT_RECODE{
   uint8_t    fileSystemDescriptor;       /* 1:FAT12, 4:FAT16(less than 32MB), 5:拡張 DOS パーティション,
                                           6:FAT16(more 32MB), 0xb:FAT32(more 2GB),
                                           0xc:FAT32 Int32h 拡張, 0xe:FAT16 Int32h 拡張,
                                           0xf:拡張 DOS パーティションの Int32h 拡張 */
   uint32_t    firstSectorNumbers;      /* first sector number (link to BPB sector) */
   uint32_t    numberOfSectors;
} MASTER_BOOT_RECODE;


typedef struct _BPB_PARAMETER_BLOCK{
	//uint8_t    jmpOpeCode[3];          /* 0xeb ?? 0x90 */
    //*uint8_t    OEMName[8];
    //* FAT16 */
    uint16_t    bytesPerSector;      /* bytes/sector */
    uint8_t    sectorsPerCluster;      /* sectors/cluster */
    //uint8_t    reservedSectors[2];     /* reserved sector, beginning with sector 0 */
    //uint8_t    numberOfFATs;           /* file allocation table */
    //uint8_t    rootEntries[2];         /* root entry (512) */
    //uint8_t    totalSectors[2];        /* partion total secter */
    //uint8_t    mediaDescriptor;        /* 0xf8: Hard Disk */
    uint16_t    sectorsPerFAT;       /* sector/FAT (FAT32 always zero: see bigSectorsPerFAT) */
    //uint8_t    sectorsPerTrack[2];     /* sector/track (not use) */
    //uint8_t    heads[2];               /* heads number (not use) */
    //uint8_t    hiddenSectors[4];       /* hidden sector number */
    //uint8_t    bigTotalSectors[4];     /* total sector number */
    /* info */
    //uint8_t    driveNumber;
    //uint8_t    unused;
    //uint8_t    extBootSignature;
    //uint8_t    serialNumber[4];
    //uint8_t    volumeLabel[11];
    //uint8_t    fileSystemType[8];      /* "FAT16   " */
    //uint8_t    loadProgramCode[448];
    //uint8_t    sig[2];                 /* 0x55, 0xaa */

} BPB_PARAMETER_BLOCK;

typedef struct _DIR_ENTRY{
    uint8_t    name[9];            /* file name */
    uint8_t    extension[4];       /* file name extension */
    uint8_t    attribute;          /* file attribute
                                     bit 4    directory flag
                                     bit 3    volume flag
                                     bit 2    hidden flag
                                     bit 1    system flag
                                     bit 0    read only flag */
    //uint8_t    reserved;           /* use NT or same OS */
    //uint8_t    createTimeMs;       /* VFAT で使用するファイル作成時刻の10ミリ秒 (0 〜 199) */
    //uint8_t    createTime[2];      /* VFAT で使用するファイル作成時間 */
    //uint8_t    createDate[2];      /* VFAT で使用するファイル作成日付 */
    //uint8_t    accessDate[2];      /* VFAT で使用するファイル・アクセス日付 */
    //uint8_t    clusterHighWord[2]; /* FAT32 で使用するクラスタ番号の上位 16 bits */
    //uint8_t    updateTime[2];
    //uint8_t    updateDate[2];
    uint16_t    cluster;         /* start cluster number */
    uint32_t    fileSize;        /* file size in bytes (directory is always zero) */
	
	uint16_t 	nowFile;    /*   must initialized  */
	uint16_t	nowByte;
	uint16_t	nowSector;		/*  this parameter 0-19   */
} DIR_ENTRY;

void wait_ms(int time_ms);
uint8_t sd_byte(uint8_t data);
char SD_readMRB(void);
char SD_readBPB(void);
uint16_t getnextcluster(uint16_t sector/*,uint32_t firstSectorNumbers,uint8_t sectorsPerCluster*/);
void SD_openSector(uint32_t);
void getNextDir(DIR_ENTRY*);
void sd_init(void);
void SD_openNowDir(void);
uint8_t SD_readNowDir(uint8_t *res);