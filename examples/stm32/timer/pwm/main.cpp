
#include <xpcc/architecture.hpp>

using namespace xpcc::stm32;

GPIO__OUTPUT(Led1, A, 1);

MAIN_FUNCTION
{
	Led1::setOutput(xpcc::stm32::ALTERNATE, xpcc::stm32::PUSH_PULL);
	
	Timer2::enable();
	Timer2::setMode(Timer2::UP_COUNTER);

	// 72 MHz / 2 / 2^16 ~ 550 Hz
	Timer2::setPrescaler(2);
	Timer2::setOverflow(65535);
	
	Timer2::configureOutputChannel(2, Timer2::OUTPUT_PWM, 40000);
	Timer2::applyAndReset();
	
	Timer2::start();

	uint16_t pwm = 0;
	bool up = true;
	while (1)
	{
		// Let the LED fade up and down
		if (up) {
			pwm += 10;
			if (pwm >= 65000) {
				up = false;
			}
		}
		else {
			pwm -= 10;
			if (pwm <= 100) {
				up = true;
			}
		}
		Timer2::setCompareValue(2, pwm);

		xpcc::delay_us(80);
	}
}

