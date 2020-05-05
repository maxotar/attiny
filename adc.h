#ifndef ADC_H_
#define ADC_H_

uint16_t get_internal_temperature_in_K(void)
{
	VREF.CTRLA = VREF_ADC0REFSEL_1V1_gc;
	ADC0.CTRLA = ADC_ENABLE_bm | ADC_RESSEL_10BIT_gc;
	ADC0.CTRLC = (ADC_SAMPCAP_bm | ADC_PRESC_DIV4_gc) & ~ADC_REFSEL_gm;
	ADC0.CTRLD = ADC_INITDLY_DLY32_gc;
	ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc; // sample the temp sensor
	ADC0.SAMPCTRL = ADC_SAMPLEN4_bm;

	ADC0.COMMAND = ADC_STCONV_bm;
	while (ADC0.COMMAND & ADC_STCONV_bm);
	ADC0.CTRLA = 0;

	int8_t sigrow_offset = SIGROW.TEMPSENSE1;
	uint8_t sigrow_gain = SIGROW.TEMPSENSE0;
	uint16_t adc_reading = ADC0.RES;			 // 16 bit register for 10 bits of resolution
	uint32_t temp = adc_reading - sigrow_offset; // 32 bits needed for the following multiplication
	temp *= sigrow_gain;
	temp += 0x80; // Add 1/2 to get correct rounding on division below
	temp >>= 8;	  // Divide result to get Kelvin
	return (uint16_t)temp;
}


uint8_t sample_ADC(PORT_t *port, uint8_t pin, uint8_t ain_num)
{
	port->DIRCLR = (1 << pin);
	*(&port->PIN0CTRL + pin) = PORT_ISC_INPUT_DISABLE_gc;
	ADC0.CTRLA = ADC_ENABLE_bm | ADC_RESSEL_8BIT_gc;
	ADC0.CTRLC = ADC_SAMPCAP_bm | ADC_REFSEL0_bm;
	ADC0.MUXPOS = ain_num;
	ADC0.COMMAND = ADC_STCONV_bm;
	while (ADC0.COMMAND & ADC_STCONV_bm);
	ADC0.CTRLA = 0;
	return ADC0.RESL;
}

#endif /* ADC_H_ */
