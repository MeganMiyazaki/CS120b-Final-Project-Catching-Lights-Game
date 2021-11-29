#ifndef SHIFT_H
#define SHIFT_H

#define HC595_PORT   PORTB
#define HC595_DDR    DDRB
#define HC595_DS_POS PB0
#define HC595_SH_CP_POS PB1
#define HC595_ST_CP_POS PB2

void shift_init() {
   HC595_DDR |= ((1 << HC595_SH_CP_POS) | (1 << HC595_ST_CP_POS) | (1 << HC595_DS_POS));
}

#define HC595DataHigh() (HC595_PORT |= (1 << HC595_DS_POS))
#define HC595DataLow() (HC595_PORT &= ~(1 << HC595_DS_POS))

void shift_Pulse() {
   HC595_PORT |= (1 << HC595_SH_CP_POS);
   HC595_PORT &= ~(1 << HC595_SH_CP_POS);
}

void shift_Echo() {
   HC595_PORT |= (1 << HC595_ST_CP_POS);
   HC595_PORT &= ~(1 << HC595_ST_CP_POS);
}

void shift_WriteData(uint8_t input) {

	for(uint8_t i = 0; i < 8; i++) {

		if(input & 0b10000000) HC595DataHigh();

     		else { HC595DataLow(); }

      		shift_Pulse();
      		input = input << 1;

	}

   	shift_Echo();

}

#endif
