#include <avr/io.h>

void UART_put(char c);
char UART_rec(void);
void UART_init(void);
void UART_sendHEX(uint8_t data);
void UART_str(char *str);

void UART_put(char c)
{
	while(!(UCSR0A & 0b00100000)){} //UDREビットが1になるまで待つ
    UDR0 = c;
}

char UART_rec(void){
	char c;
	while(!(UCSR0A & 0b10000000)){} //RXCビットが1になるまで待つ
	c = UDR0;
	return c;
}

void UART_init(void)
{
	UBRR0 = 64;    /* 10MHzにて1200bps設定 */
 	UCSR0B = 0b00011000; /* 送信許可 */
}

void UART_sendHEX(uint8_t data)
{
	uint8_t a = data >>4;
	
	for(uint8_t i=0; i<2; i++){
		if(a<10)
			UART_put(48+a);		/*	ASCIIでは'0'=48	*/
		if(a==10)
			UART_put('A');
		if(a==11)
			UART_put('B');
		if(a==12)
			UART_put('C');
		if(a==13)
			UART_put('D');
		if(a==14)
			UART_put('E');
		if(a==15)
			UART_put('F');
		
		a = data & 0xf;
	}
	
	UART_put(' ');
}

void UART_str(char *str)
{
	while(*str != 0){
		UART_put(*str);
		str++;
	}
}
