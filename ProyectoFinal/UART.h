/*
 * UART.h
 *
 * Created: 28/05/2024 12:04:59 a. m.
 *  Author: nunez
 */ 



#ifndef UART_H_
#define UART_H_
void UART_init(int baund, uint8_t dataSize, uint8_t UPMn1, uint8_t UPMn0,uint8_t stopBit, uint8_t TXInt, uint8_t RXInt, uint8_t dataInt);//Inicializa la conexion serial
unsigned char UART_read();
void UART_write(unsigned char);
void UART_string(char*);

void UART_init(int baundRate, uint8_t dataSize, uint8_t UPMn1, uint8_t UPMn0,uint8_t stopBit, uint8_t TXInt, uint8_t RXInt, uint8_t dataInt){
	
	UCSR0A|= (0<<U2X0);
	UCSR0A|= (0<MPCM0);
	UCSR0B|= (1<<TXEN0);
	UCSR0B|= (1<<RXEN0);
	UCSR0B|= (dataInt<<UDRIE0); // normalmente 0
	UCSR0B|= (TXInt<<TXCIE0); //Complete interrup EN, normalmente 0
	UCSR0B|= (RXInt<<RXCIE0); // complete interrup EN, normalmente 0
	UCSR0C|= (0<<UMSEL01)|(0<<UMSEL00);
	UCSR0C|= (UPMn1<<UPM01)|(UPMn0<<UPM00); // normalmente 0 0
	UCSR0C|= (stopBit<<USBS0);
	switch(dataSize){
		case 5:
		UCSR0C|= (0<<UCSZ01)|(0<<UCSZ00);
		break;
		case 6:
		UCSR0C|= (0<<UCSZ01)|(1<<UCSZ00);
		break;
		case 7:
		UCSR0C|= (1<<UCSZ01)|(0<<UCSZ00);
		break;
		case 8:
		UCSR0C|= (1<<UCSZ01)|(1<<UCSZ00);
		break;
		default:
		UCSR0C|= (1<<UCSZ01)|(1<<UCSZ00);
		
	}
	
	
	UCSR0C|= (0<<UCPOL0);
	UBRR0 = baundRate;//baud rate : 103 = 9600
}
unsigned char UART_read(){
	if(UCSR0A & (1<<RXC0)){
		return UDR0;
	}
	return 0;
}

void UART_write(unsigned char data){
	while(!(UCSR0A&(1<<UDRE0)));
	UDR0 = data;
}
void UART_string(char* string){
	while(*string !=0)
	{
		UART_write(*string);
		string++;
	}
}

#endif /* UART_H_ */