#include <avr/io.h>
#include "SD_FAT16.h"

extern MASTER_BOOT_RECODE mbr;
extern BPB_PARAMETER_BLOCK bpb;
extern DIR_ENTRY nowDir;

void wait_ms(int time_ms)	/*0<a<25*/
{
	TCCR2A = 0b00000000;
	TCCR2B = 0b00000111;	/*	1clk=0.1024ms	*/
	TCNT2  = 0xff - time_ms*10;
	TIFR2 |= (1<<TOV2);
    while(!(TIFR2 & (1<<TOV2)));
}

inline uint8_t sd_byte(uint8_t data)
{	
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
} 

void sd_init(void)
{
	char ch;
	
	SPI_DDR |= ((1<<SCK) | (1<<MOSI) | (1<<2));		//SS(PORTB2)は使わないけど設定しないと動かない
	SPI_DDR &= ~(1<<MISO);
	SPI_PORT = 0x00;
	SS_DDR  |= (1<<SS);
	SPCR = ((1<<SPE) | (1<<MSTR));
	SPSR |= (1<<SPI2X);
	wait_ms(20);
	
	/*    74クロック送る      */
	SS_PORT  |= (1<<SS);
	for(char i=0; i<10; i++){
		SPDR = 0xff;
		while(!(SPSR & (1<<SPIF)));
	}
	
	/*	  CMD0      */
	SS_PORT &= ~(1<<SS);
	sd_byte(0x40);
	sd_byte(0x00);
	sd_byte(0x00);
	sd_byte(0x00);
	sd_byte(0x00);
	sd_byte(0x95);
	do{
		ch = sd_byte(0xff);
	}while(ch != 0x01);
	SS_PORT  |= (1<<SS);
	
	/*    CMD1      */
	/*SS_PORT &= ~(1<<SS);
	while(1){
		sd_byte(0x41);
		sd_byte(0x00);
		sd_byte(0x00);
		sd_byte(0x00);
		sd_byte(0x00);
		sd_byte(0x95);
		ch = sd_byte(0xff);
		while(ch == 0xff){
			ch = sd_byte(0xff);
		}
		if(ch == 0x00)	break;
	}
	SS_PORT  |= (1<<SS);*/
	/*     ACMD41     */
	SS_PORT &= ~(1<<SS);
	while(1){
		sd_byte(0x40+55);
		sd_byte(0x00);
		sd_byte(0x00);
		sd_byte(0x00);
		sd_byte(0x00);
		sd_byte(0x00);
		ch = sd_byte(0xff);
		while(ch == 0xff){
			ch = sd_byte(0xff);
		}
		break;
	}
	SS_PORT  |= (1<<SS);
	
	SS_PORT &= ~(1<<SS);
	while(1){
		sd_byte(0x69);
		sd_byte(0x00);
		sd_byte(0x00);
		sd_byte(0x00);
		sd_byte(0x00);
		sd_byte(0x00);
		ch = sd_byte(0xff);
		while(ch == 0xff){
			ch = sd_byte(0xff);
		}
		break;
	}
	SS_PORT  |= (1<<SS);
}

void SD_openSector(uint32_t arg)
{
	arg *= 512;	//arg = arg<<7;
	
	/*    CMD17     */
	SS_PORT &= ~(1<<SS);
	sd_byte(0x51);
	sd_byte((uint8_t)(arg>>24));
	sd_byte((uint8_t)(arg>>16));
	sd_byte((uint8_t)(arg>>8));
	sd_byte((uint8_t)arg);
	sd_byte(0x01);
	
	while(sd_byte(0xff) != 0x00);
	
	while(sd_byte(0xff) != 0xfe);
}

char SD_readMRB(void)
{
	uint16_t i;
	uint32_t a;

	SD_openSector(0x00000000);
	
	for(i=0; i<446; i++)
		sd_byte(0xff);
	
	sd_byte(0xff);	/* 0x80: bootable device, 0x00: non-bootable */
	
	for(i=0; i<3; i++)
		sd_byte(0xff);
	
	mbr.fileSystemDescriptor = sd_byte(0xff);
	if(mbr.fileSystemDescriptor != 0x04 && mbr.fileSystemDescriptor != 0x06){
		return SD_FAILED;
	}	
	
	for(i=0; i<3; i++){
		sd_byte(0xff);
	}
	
	mbr.firstSectorNumbers = 0;
	for(i=0; i<4; i++){
		a = sd_byte(0xff);
		mbr.firstSectorNumbers |= a<<(8*i);
	}
	
	mbr.numberOfSectors = 0;
	for(i=0; i<3; i++){
		a = sd_byte(0xff);
		mbr.numberOfSectors |= a<<8*i;
	}
	
	while(0x55 != sd_byte(0xff));
	while(0xaa != sd_byte(0xff));
	
	SS_PORT  |= (1<<SS);
	return SD_OK;
}

char SD_readBPB(void)
{
	uint16_t i;
	//uint8_t j;
	SD_openSector(mbr.firstSectorNumbers);
	
	for(i=0; i<11; i++)
		sd_byte(0xff);
	
	bpb.bytesPerSector = sd_byte(0xff);
	i = sd_byte(0xff);
	bpb. bytesPerSector |=  i<<8;
	//if(bpb.bytesPerSector == 512)
	// 	;
	
	bpb.sectorsPerCluster = sd_byte(0xff);
	
	for(i=0; i<8; i++)
		sd_byte(0xff);
	
	bpb.sectorsPerFAT = sd_byte(0xff);
	i = sd_byte(0xff);
	bpb. sectorsPerFAT |=  i<<8;
	
	while(0x55 != sd_byte(0xff));
	while(0xaa != sd_byte(0xff));
	
	SS_PORT  |= (1<<SS);
	return SD_OK;
}

uint16_t getnextcluster(uint16_t cluster/*,uint32_t firstSectorNumbers,uint8_t sectorsPerCluster*/)
{
	uint16_t i;
	uint16_t res;
	SD_openSector(mbr.firstSectorNumbers+1+cluster/256);
	
	cluster %= 256;
	for(i=0; i<cluster*2; i++){
		sd_byte(0xff);
	}
	
	res  = sd_byte(0xff);
	res |= sd_byte(0xff)<<8;
	i += 2;
	
	for(i; i<512; i++)
		sd_byte(0xff);
	
	SS_PORT  |= (1<<SS);
	return res;
}

void getNextDir(DIR_ENTRY* dir)
{
	uint16_t i;
	uint8_t j,k;
	
	again:
	SD_openSector(mbr.firstSectorNumbers+bpb.sectorsPerFAT*2+dir->nowFile/16+1);
	k = dir->nowFile%16;
	
	for(i=0; i<k; i++){
		for(j=0; j<32; j++){
			sd_byte(0xff);
		}
	}
	dir->nowFile++;
	if(dir->nowFile == 512){
		dir->nowFile = 0;
		goto again;
	}
	
	for(i=0; i<9; i++){
		dir->name[i] = 0;
	}
	for(i=0; i<8; i++){
		dir->name[i] = sd_byte(0xff);
	}
	//dir->name[8] = 0;
	
	for(i=0; i<4; i++){
		dir->extension[i] = 0;
	}
	for(i=0; i<3; i++){
		dir->extension[i] = sd_byte(0xff);
	}
	//dir->extension[3] = 0;
	
	dir->attribute = sd_byte(0xff);
	
	for(i=0; i<14; i++){
		sd_byte(0xff);
	}
	
	char ch;
	ch = sd_byte(0xff);
	dir->cluster  =  ch;
	ch = sd_byte(0xff);
	dir->cluster |= ch<<8;
	
	dir->fileSize  = (uint32_t)sd_byte(0xff);
	dir->fileSize |= (uint32_t)sd_byte(0xff)<<8;
	dir->fileSize |= (uint32_t)sd_byte(0xff)<<16;
	dir->fileSize |= (uint32_t)sd_byte(0xff)<<24;
	
	for(i=(16-k)*32; 0<i; i--){
		sd_byte(0xff);
	}
	
	SS_PORT  |= (1<<SS);
}

void SD_openNowDir(void)
{
	SD_openSector(mbr.firstSectorNumbers+bpb.sectorsPerFAT*2+33+(nowDir.cluster-2)*bpb.sectorsPerCluster);
	nowDir.nowByte = 0;
	nowDir.nowSector = 0;
}


uint8_t SD_readNowDir(uint8_t *res)
{
	nowDir.nowByte++;
	*res = 1; //1 = true
	if(nowDir.nowByte == 512){
		nowDir.nowByte = 0;
		nowDir.nowSector++;
		if(nowDir.nowSector == bpb.sectorsPerCluster){
			SS_PORT  |= (1<<SS);
			nowDir.cluster = getnextcluster(nowDir.cluster);
			if(0xfff8 <= nowDir.cluster){
				*res = 0; //0=false
				goto BREAK;
			}
			nowDir.nowSector = 0;
		}
		SS_PORT  |= (1<<SS);
		SD_openSector(mbr.firstSectorNumbers+bpb.sectorsPerFAT*2+33+(nowDir.cluster-2)*bpb.sectorsPerCluster+nowDir.nowSector);
	}
	
	return sd_byte(0xff);
	BREAK: return ;
}
