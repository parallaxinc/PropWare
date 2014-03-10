#ifndef LIBPROPELLER_BOARD_H_
#define LIBPROPELLER_BOARD_H_
/**
This nameplace defines the standard unit testing board.
*/
namespace Board{

	const int kPinI2CSCL = 0;
	const int kPinI2CSDA = 1;
	
	const int kPinMCP3208Data = 2;
	const int kPinMCP3208Clock = 3;
	const int kPinMCP3208Select = 4;
	
	// 32.768KHz square wave
	const int kPinSqw = 5;
	
	// DAC output
	const int kPinDac = 6;
	
	//SD Card
	const int kPinSDDo = 10;
	const int kPinSDClk = 11;
	const int kPinSDDi = 12;
	const int kPinSDCs = 13;
	
	// Pins are tied together with a 10kOhm resistor.
	const int kPinTie1a = 18;
	const int kPinTie1b = 19;
	
	// Pins are tied together with a 10kOhm resistor.
	const int kPinTie2a = 20;
	const int kPinTie2b = 21;
	
	
	const int kPinEepromScl = 28;
	const int kPinEepromSda = 29;
	const int kPinUsbTx = 30;
	const int kPinUsbRx = 31;
}

#endif // libpropeller_board_h_
