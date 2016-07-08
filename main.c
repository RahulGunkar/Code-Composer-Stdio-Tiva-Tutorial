#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"

/*To store the data read from the adc fifo It should be equal to the sequencer used
 * since we are using sequencer 1 with the depth of 4, the next few variables are
 * for avg adc temp, celcious,farenhiet value*/
uint32_t ui32ADC0Value[4];
volatile uint32_t ui32TempAvg;
volatile uint32_t ui32TempValueC;
volatile uint32_t ui32TempValueF;

void ClockConfigure()
{
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
}

void Adc0Seq1Configure()
{
	//enable periphera
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	//configure adc 0 ,sequencer 1,trigger the processor with highset priority
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	//sample sequencer 4 times
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
	/*The final sequencer step requires a couple of extra settings. Sample the temperature
	sensor (ADC_CTL_TS) and configure the interrupt flag (ADC_CTL_IE) to be set
	when the sample is done. Tell the ADC logic that this is the last conversion on
	sequencer	1 (ADC_CTL_END).*/
	ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
	//enable adc sequencer one
	ADCSequenceEnable(ADC0_BASE, 1);
	
}

void StartAdc()
{
	//clear the interrupt flag in the beginning
	ADCIntClear(ADC0_BASE, 1);
	//Trigger the adc directly from the software as it can be triggered from many other divies
	ADCProcessorTrigger(ADC0_BASE, 1);
	//wait for the conversion to complete
	while(!ADCIntStatus(ADC0_BASE, 1, false))
	{
	}
}


void TempCalculations()
{
	//read the adc value from the fifo
	ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
	//take the average of the four samples
	ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
	//calculate the temperature in celcius by the following formula
	//TEMP = 147.5 – ((75 * (VREFP – VREFN) * ADCVALUE) / 4096)
	//now VREFP-VRFEN=Vdd or 3.3v,we will multiple by 10 to get precision  and then multyply by 75
	ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
	ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;

}


int main()
{
	Adc0Seq1Configure();
	while(1)
	{
		StartAdc();
		TempCalculations();

	}
}

