/*
 * Laboving10_SPI.c
 *
 * Created: 3/19/2019 8:38:48 AM
 * Author : Hans-Robert
 */ 

#define SPI_Master 1	//Setter master eller slave
#define F_CPU 16000000UL // System clock
#define USART_BAUDRATE 9600 // desired baud rate
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) // UBRR value

#include <avr/io.h>
#include <stdio.h>

void uart_init(void)
{
	UBRR0 = UBRR_VALUE;
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0); /* Enable RX and TX */
}

int uart_putchar(char c, FILE *stream)
{
	// if (c == '\n') {
	// uart_putchar('\r', stream);
	// }
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

int uart_getchar(FILE *stream)
{
	loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
	return UDR0;
}

void SPI_MasterInit(void)
{
	/* Set MOSI, SS and SCK output, all others input */
	DDRB = (1<<DDB2)|(1<<DDB3)|(1<<DDB5);
	/*Enable pull-up on MISO*/
	PORTB = (1<<PORTB4);
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}
void SPI_MasterTransmit(char cData)
{
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
	/*Prints SPDR*/
	printf("%c",SPDR);
}

void SPI_SlaveInit(void)
{
	/* Set MISO output, all others input */
	DDRB = (1<<DDB4);
	PORTB = (1<<PB5)|(1<<PB3)|(1<<PB2);
	/* Enable SPI */
	SPCR = (1<<SPE);
}
char SPI_SlaveReceive(char cData)
{
	/*Sends input char into SPDR*/
	SPDR = cData;
	/* Wait for reception complete */
	while(!(SPSR & (1<<SPIF)))
	;
	/*Prints SPDR*/
	printf("%c",SPDR);
	/* Return Data Register */
	return SPDR;
}

int main(void)
{
	/*If SPI_Master == true we init as master, else init as slave*/
	#if SPI_Master
		SPI_MasterInit();
	#else
		SPI_SlaveInit();
	#endif
	
	uart_init();
	FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
	FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
	stdout = &uart_output;
	stdin = &uart_input;
	
	/*Start message*/
	puts("Start transmit: \r\n");
	
	char input;
	
	while(1) {
		if(SPI_Master)
		{
			/*Receives char from uart*/
			input = getchar();
			//printf("%c", input);
			/*Sends char to transmit function*/
			SPI_MasterTransmit(input);	
		}
		else
		{
			/*Receives char from uart*/
			input = getchar();
			/*Sends char to receive function*/
			SPI_SlaveReceive(input);	
		}	
	}
	// return 0;
}