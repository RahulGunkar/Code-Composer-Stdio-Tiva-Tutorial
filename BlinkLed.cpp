#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"




void setClock()
{
	SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

}

void setPin()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,GPIO_PIN_1);


}

void blinkLed()
{
	GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,0x02);
	SysCtlDelay(20000000);
	GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,0x00);
	SysCtlDelay(20000000);
}

int main(void)
{

	setClock();
	setPin();
	while(1)
	{

		blinkLed();

	}
}

