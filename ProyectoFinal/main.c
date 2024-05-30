/*
 * ProyectoFinal.c
 *
 * Created: 24/05/2024 01:19:13 p. m.
 * Author : nunez
 */ 
#define VELOCIDADINICIAL 95
#define F_CPU 16000000L
#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <math.h>
uint8_t nivelVelocidad = 1;
bool modoColision = true;
bool girarDerecha = true;
uint8_t yaGiro = 0;
int adicionIzquierda = 0;
int adicionDerecha = 0;
bool tiempoLeido = false;
bool esDrive = true;
bool esOverflow = false;
uint8_t contadorDeMismaDistancia = 0;
float distanciaAnterior = 0;
void enviarPulso();
float obtenerDistancia();
void dirreccionLllantaIzquierda(char direccion);
void direccionLlantaDerecha(char direccion);
void velocidadLlantaDerecha(uint8_t Velocidad);
void velocidadLlantaIzquierda(uint8_t Velocidad);
void imprimir(float distancia);
void giarHaciaDerecha(float distancia);
void ajustarVelocidades();
void girarALaIzquierda();
void girarALaDerecha();
void mediaVueltaDerechaOverflow();
uint8_t tiempoSinPared(uint8_t,float);
uint8_t girarPorCNY(uint8_t);
int main(void)
{
	uint8_t dejaDeVerPared = 7;
	bool girarDerecha = true;
	int banderaEnviarPulso = 0;
	
	//Salida de direcciones
	DDRD |= (1<<PIND0); //Drive para rueda izquierda
	DDRD |= (1<<PIND1); //Reverse para rueda izquierda
	DDRD |= (1<<PIND2); // Drive para rueda derecha
	DDRD |= (1<<PIND3); // Reverse para rueda derecha

	

	//Naturalmente estan en drive
	direccionLlantaDerecha('d');
	dirreccionLllantaIzquierda('d');
	//Ultrasonico
	DDRB |= (1<<PINB1); //Salida es Trigger
	DDRB &= ~(1<<PINB0); //Entrada es echo
	//PORTB |= (1<<PINB0);
	DDRB |= (1<<PINB5); //Luz led del arduino para indicar menos de 10 cm
	PORTB &= ~(1<<PINB5); //apagado
	//Salida PWM
	DDRD |= (1<<PIND6); //para rueda izquierda usa A
	DDRD |= (1<<PIND5); // rueda derecha usa B
	//Configuracion PWM
	//Timer0
	TCCR0A |= (1<<COM0A1)| (0<<COM0A0); // No inveritdo OC0A
	TCCR0A |= (1<<COM0B1)| (0<<COM0B0); //No invertido 0C0B
	TCCR0A |= (1<<WGM01)|(1<<WGM00); //Fast PWM
	TCCR0B |= (1<<CS02)|(0<<CS01)|(1<<CS00);//Prescaler 1024
	velocidadLlantaDerecha(1); //Marcar la velocidad de cada llanta como inicial
	velocidadLlantaIzquierda(1); // marcar la velocidad de cada 	llanta como inicial
	//Para la lectura del CNY
	ADMUX |= (0<<REFS1)| (0<<REFS0); //exet
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);  //Prescaler 128
	DIDR0 |= (1<<ADC0D); //Apagar ADC0 pin digital
	ADCSRA |= (1<<ADEN); //Habilitar ADC
	ADMUX |= (0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0);// ADC0 PINC0
	
	//Configuracion del timer1 en modo capture
	//cli();
	TCCR1B |= (1<<ICNC1)|(0<<CS12)|(1<<CS11)|(1<<CS10); // Cancelador de ruido activado, preescaler 256
	TCCR1B &= ~(1<<ICES1); // flanco de bajada
	TIFR1 = 1<<ICF1; //Limpiar la bandera de interrupcion
	TIMSK1 |= (1<<ICIE1); //para la interrupcion de la bandera de overflow
	sei();

	float distancia = 13;
	
	float valor=0;
	float distanciaCNY = 0;
	uint8_t tiempoMiradoEnCNY = 0;
	uint8_t dejaDeVerParedCNY = 0;
	_delay_ms(1);
	char cadena[60];
	while (1)
	{
		ADCSRA |= (1<<ADSC); //Comienza la conversion
		while (!(ADCSRA & (1<< ADIF))); // Esperar
		ADCSRA |= (1<<ADIF); // Limpia la bandera
		valor = ((ADC * 5 )/ 1024.0);
		distanciaCNY = (-92.216*valor) + 449.67;
		if(banderaEnviarPulso == 0){
			enviarPulso();
			banderaEnviarPulso = 1;
		}
		if(tiempoLeido){
			distancia = obtenerDistancia();
			banderaEnviarPulso = 0;
			tiempoLeido = false;
		}
		_delay_ms(8);
		
		if(distancia <= 9  & dejaDeVerPared == 1){
			if (distancia<= 7)
			{
				direccionLlantaDerecha('r');
				dirreccionLllantaIzquierda('r');
				_delay_ms(130);
				distancia = distancia+.5; //
			}
			if(distanciaCNY<=150){ // Significa que hay pared en la derecha, por lo que gira
				girarALaIzquierda();
				_delay_ms(20);
				dejaDeVerPared = 0;
			}

		}
		else{
				if (distanciaCNY>=150 & dejaDeVerParedCNY ==1)
				{	_delay_ms(580); // Para que se posicione en una buena zona para girar
					girarALaDerecha();
					_delay_ms(300); //Espera un poquito si no hay pared
					
					dejaDeVerParedCNY = 0;
				}
				if (distanciaCNY<=-0.5)
				{
					adicionDerecha = 3;
					adicionIzquierda = -1;
	
				}
				if (distanciaCNY>=4)
				{
					adicionIzquierda = 3;
					adicionDerecha = -3;
				}
		}
		
		if (distanciaAnterior == distancia)
		{
			contadorDeMismaDistancia++;
		}
		else{
			distanciaAnterior = distancia;
			contadorDeMismaDistancia = 0;
		}
		if (contadorDeMismaDistancia ==5 )
		{
			mediaVueltaDerechaOverflow();
			dejaDeVerParedCNY = 1;
			_delay_ms(100);
		}
		if(distancia>9 ){
			dejaDeVerPared = 1;
		}
		if (distanciaCNY<=80)
		{
			dejaDeVerParedCNY = 1;
		}
		
		ajustarVelocidades();
		imprimir(distancia);
		
	}
}
void imprimir(float distancia){
	
	if (distancia<11)
	{
		PORTB |= (1<<PINB5);
	}else
	{
		PORTB &= ~(1<<PINB5);
	}
}
void ajustarVelocidades(){
	velocidadLlantaDerecha(nivelVelocidad);
	velocidadLlantaIzquierda(nivelVelocidad);
	if(esDrive){
		direccionLlantaDerecha('d');
		dirreccionLllantaIzquierda('d');
	}
	else{
		direccionLlantaDerecha('r');
		dirreccionLllantaIzquierda('r');
	}

}

ISR(TIMER1_CAPT_vect){
	//if(~(PINB&(1<<PINB0))) //Comprueba que sea el flanco de bajada
	//{
	if(!tiempoLeido){
		tiempoLeido = true;
	}
	//}
}	
uint8_t tiempoSinPared(uint8_t tiempo,float dist){
	if(dist>=5){
		tiempo++;
	}
	else
	{
		tiempo = 0;
	}
	return tiempo;
}
uint8_t girarPorCNY(uint8_t tiempo){
	if(tiempo>=80 & yaGiro<=0){
		girarALaDerecha();
		yaGiro = 5;
		tiempo = 0;
	}
	else{
		yaGiro --;
	}
	return tiempo;
}
void velocidadLlantaDerecha(uint8_t Velocidad){
	uint8_t numeroObtenido = (VELOCIDADINICIAL * Velocidad) + adicionDerecha;
	OCR0B = numeroObtenido;
}
void velocidadLlantaIzquierda(uint8_t Velocidad){
	OCR0A = (VELOCIDADINICIAL * Velocidad) +20 +adicionIzquierda;
}
void detenerLlantaDerecha(){
	OCR0B = 0;
}
void detenerLlantaIzquierda(){
	OCR0A = 0;
}
void direccionLlantaDerecha(char direccion){
	//DDRD |= (1<<PIND2); // Drive para rueda derecha
	//DDRD |= (1<<PIND3); // Reverse para rueda derecha
	if(direccion == 'd')
	{
		PORTD |= (1<<PIND2);
		PORTD &= ~(1<<PIND3);
	}
	else{
		if(direccion == 'r')
		{
			PORTD &= ~(1<<PIND2);
			PORTD |= (1<<PIND3);
		}
	}
}
void dirreccionLllantaIzquierda(char direccion){
	//DDRD |= (1<<PIND0); //derecha para rueda izquierda
	//DDRD |= (1<<PIND1); //izquierda para rueda izquierda
	if(direccion == 'd')
	{
		PORTD |= (1<<PIND0);
		PORTD &= ~(1<<PIND1);
	}
	else{
		if(direccion == 'r')
		{
			PORTD &= ~(1<<PIND0);
			PORTD |= (1<<PIND1);
		}
	}

}
void girarALaIzquierda(){

	direccionLlantaDerecha('r');
	dirreccionLllantaIzquierda('r');
	_delay_ms(175);
	direccionLlantaDerecha('d');
	//imprimir(10);
	dirreccionLllantaIzquierda('r');
	OCR0A = 20;
	OCR0B = 100;
	_delay_ms(405);
	ajustarVelocidades();
}
void girarALaDerecha(){
	//imprimir(40);
	direccionLlantaDerecha('r');
	OCR0A = 20;
	OCR0B = 100;
	_delay_ms(460);
	ajustarVelocidades();
}
void mediaVueltaDerechaOverflow(){
	direccionLlantaDerecha('r');
	dirreccionLllantaIzquierda('r');
	_delay_ms(200);
	dirreccionLllantaIzquierda('d');
	OCR0A = 20;
	OCR0B = 100;
	_delay_ms(100);
	ajustarVelocidades();	
}
void enviarPulso(){
	PORTB |=(1<<PINB1);
	_delay_us(15); ;//10 microsegundos de pulso
	PORTB &= ~(1<<PINB1);
	_delay_us(200); //Se espera 200 us porque hay 8 pulsos de 40kHz
	TCNT1 = 0;

	// Se inicio en 0 el contador
}
float obtenerDistancia(){
	int cantidadDeCiclos;
	float distancia;
	float tiempoEnUs;
	cantidadDeCiclos = ICR1;
	//Calculo de distancia para simulacion
	/*
	tiempoEnUs = cantidadDeCiclos*4; //Se le multiplica por 4
	porque cada ciclo equivale a 4us por el preescalador 64
	tiempoEnUs = tiempoEnUs /2; //Solo ocupamos el tiempo que
	tardo en ir
	distancia = tiempoEnUs*0.0334;
	*/
	distancia = (0.071*cantidadDeCiclos)- 35.581; //Calculo para la vida real
	// 1cm/29.2us o 0.0334cm/us es la velocidad del sonido, por lo que D=T*V


	return distancia;
}

