#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"

//define pwm frequency
#define PWM_FREQUENCY 55

/*
 since pwm frequency is 55 and a time time period of 18.2ms and we are gonna devide it by 1000
  so we get the precision of 1.82us..now 1.82*83=1.5ms...since servo is at zero angle when supplided
 */
volatile uint32_t ui32Load;
volatile uint32_t ui32PWMClock;
volatile uint32_t ui8Adjust=83;

/*the following function will set the clock with 40MHz  and the second line of the code will divide the
 * pwm frequency by 64
 * ROM version is used to reduce the code size
 */

void SetClock()
{
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);
}
/*enable port D,F for pwm and smd onboard leds*/

void ConfigPins()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	GPIOPinTypePWM(GPIO_PORTD_BASE,GPIO_PIN_0);
	GPIOPinConfigure(GPIO_PD0_M1PWM0);
}


void UnlockPortF()
{
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY; //addres to unlock
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;   //commit resgister to change
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;		// clear to make changes
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);  //set PF4,0 as input internal pull
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

void SetPwmTimePeriod()
{
	/*pwm clock is system clock/64 divide the pwm clock by 55 so to get the value of the loaad register-1
	 * configure pwm module 1 generator 0 as a down counter and load the count value
	 */

	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
	PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, ui32Load);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_0);

}


void ControlServo()
{
	if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
	{
	ui8Adjust--;
	if (ui8Adjust < 56)
	{
	ui8Adjust = 56;
	}
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
	}
	if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
	{
	ui8Adjust++;
	if (ui8Adjust > 111)
	{
	ui8Adjust = 111;
	}
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
	}
	SysCtlDelay(100000);
}


int main()
{
	 SetClock();
	 ConfigPins();
	 UnlockPortF();
	 SetPwmTimePeriod();

	while(1){
				ControlServo();

	}
}
