/*=====================================================*/
/*   mega168   10MHz     AVRマイコン活用ブックから     */
/*        SDカード            mega168                  */
/*        CS/CD/DAT3          PB2/SS                   */
/*        DI/CMD              PB4/MISO                 */
/*        SCLK/CLK            PB5/SCK                  */
/*        DO/DAT0             PB3/MOSI                 */
/*=====================================================*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "SD_FAT16.h"

MASTER_BOOT_RECODE mbr;
BPB_PARAMETER_BLOCK bpb;
DIR_ENTRY nowDir;
uint8_t wave = 0x80;

void UART_init(void);
void UART_sendHEX(uint8_t data);
void UART_str(char *str);

ISR( INT0_vect )
{
	UART_str("\n\rINT0_vect\n\r");
	
	uint16_t i;
	
	for(i=0; i<40; i++){
		wait_ms(20);
	}
	
	UART_str("\n\rINT0_vect 1\n\r");
	
	for(i=nowDir.nowByte; i<512; i++){
		sd_byte(0xff);
	}
	SS_PORT  |= (1<<SS);
	
	UART_str("\n\rINT0_vect 2\n\r");
	
	do{
			getNextDir(&nowDir);
	}while(!(nowDir.name[0] != 0xe5 && nowDir.name[0] != 0x05 && 
		nowDir.extension[0] == 'W' && nowDir.extension[1] == 'A' && nowDir.extension[2] == 'V'));
	
	UART_str("\n\rINT0_vect 3\n\r");
	
	SD_openNowDir();
	
	UART_str("\n\rINT0_vect end\n\r");
}

ISR(TIMER0_COMPA_vect)
{
	uint8_t res;
	
	OCR1A = wave;
	OCR1B = wave;
	wave = SD_readNowDir(&res);
	
	if(res == 0){
		do{
			getNextDir(&nowDir);
		}while(!(nowDir.name[0] != 0xe5 && nowDir.name[0] != 0x05 && 
			nowDir.extension[0] == 'W' && nowDir.extension[1] == 'A' && nowDir.extension[2] == 'V'));
		SD_openNowDir();
	}
}

void timer_init(void)
{
	DDRB  |= 0b00000110;
	
	TCCR1A = 0b10110001;
	TCCR1B = 0b00001001;
	OCR1A  = 0x80;
	OCR1B  = 0x80;
	
	TCCR0A = 0b00000010;
	TCCR0B = 0b00000010;
	OCR0A  = 78; //78 = 128bps 
	TIMSK0 = (1<<OCIE0A);
}

void button_init(void)
{
	DDRD  &= 0b11111011;
	PORTD |= 0b00000100;
	
	EICRA = 0b00000000;
	EIMSK = 0b00000001;
}

int main(void)
{
	sd_init();
	
	SD_readMRB();
	
	SD_readBPB();
	
	nowDir.nowFile = 0;
	
	while(!(nowDir.name[0] != 0xe5 && nowDir.name[0] != 0x05 && 
		nowDir.extension[0] == 'W' && nowDir.extension[1] == 'A' && nowDir.extension[2] == 'V'))
		getNextDir(&nowDir);
	
	SD_openNowDir();
	
	timer_init();
	
	button_init();
	
	UART_init();
	
	sei();
	
	while(1){}
	
	return 0;
}