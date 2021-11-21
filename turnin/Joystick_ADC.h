#ifndef JOYSTICK_ADC
#define JOYSTICK_ADC

void ADC_init() {
	ADCSRA = 0x87;	ADMUX = 0x40;
}

int Get_ADC() {
	ADCSRA |= (1 << ADSC);
	while((ADCSRA &(1 << ADIF)) == 0);
	ADCSRA |= (1 << ADIF);
	return (int)ADC;
}

#endif
