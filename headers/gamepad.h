#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include "BoardConfig.h"
#include <string.h>

#include "enums.pb.h"
#include "gamepad/GamepadDebouncer.h"
#include "gamepad/GamepadState.h"
#include "gamepad/descriptors/HIDDescriptors.h"
#include "gamepad/descriptors/SwitchDescriptors.h"
#include "gamepad/descriptors/XInputDescriptors.h"
#include "gamepad/descriptors/KeyboardDescriptors.h"
#include "gamepad/descriptors/PS4Descriptors.h"

#include "pico/stdlib.h"

#include "config.pb.h"

// MUST BE DEFINED FOR MPG
extern uint32_t getMillis();
extern uint64_t getMicro();

#define GAMEPAD_POLL_MS 1
#define GAMEPAD_POLL_MICRO 100

#define GAMEPAD_FEATURE_REPORT_SIZE 32

struct GamepadButtonMapping
{
	GamepadButtonMapping(uint8_t p, uint16_t bm) :
		pin(p < NUM_BANK0_GPIOS ? p : 0xff),
		pinMask(p < NUM_BANK0_GPIOS? (1 << p) : 0),
		buttonMask(bm)
	{}

	uint8_t pin;
	uint32_t pinMask;
	const uint16_t buttonMask;

	inline void setPin(uint8_t p)
	{
		if (p < NUM_BANK0_GPIOS)
		{
			pin = p;
			pinMask = 1 << p;
		}
		else
		{
			pin = 0xff;
			pinMask = 0;
		}
	}

	bool isAssigned() const { return pin != 0xff; }
};

#define GAMEPAD_DIGITAL_INPUT_COUNT 18 // Total number of buttons, including D-pad

class Gamepad {
public:
	Gamepad(int debounceMS = 5);

	void setup();
	void reassignPinsForProfile(const uint32_t profileNum);
	void process();
	void read();
	void save();
	void debounce();

	void hotkey();

	/**
	 * @brief Flag to indicate analog trigger support.
	 */
	bool hasAnalogTriggers {false};

	/**
	 * @brief Flag to indicate Left analog stick support.
	 */
	bool hasLeftAnalogStick {false};

	/**
	 * @brief Flag to indicate Right analog stick support.
	 */
	bool hasRightAnalogStick {false};

	void *getReport();
	uint16_t getReportSize();
	HIDReport *getHIDReport();
	SwitchReport *getSwitchReport();
	XInputReport *getXInputReport();
	KeyboardReport *getKeyboardReport();
	PS4Report *getPS4Report();

	/**
	 * @brief Check for a button press (current pin profile). Used by `pressed[Button]` helper methods.
	 */
	inline bool __attribute__((always_inline)) pressedButton(const uint16_t mask) {
		return (state.buttons & mask) == mask;
	}

	/**
	 * @brief Check for a button press (as wired). Used by `activeWire[Button]` helper methods.
	 */
	inline bool __attribute__((always_inline)) activeWireB(const uint16_t mask) {
		return (state.bwires & mask) == mask;
	}

	/**
	 * @brief Check for a dpad press (current pin profile). Used by `pressed[Dpad]` helper methods.
	 */
	inline bool __attribute__((always_inline)) pressedDpad(const uint8_t mask) { return (state.dpad & mask) == mask; }

	/**
	 * @brief Check for a dpad press (as wired). Used by `activeWire[Direction]` helper methods.
	 */
	inline bool __attribute__((always_inline)) activeWireD(const uint8_t mask) {
		return (state.dwires & mask) == mask;
	}

	/**
	 * @brief Check for an aux button press. Same idea as `pressedButton`.
	 */
	inline bool __attribute__((always_inline)) pressedAux(const uint16_t mask) {
		return (state.aux & mask) == mask;
	}

	/**
	 * @brief Check for a hotkey combination press. Checks aux, buttons, and dpad.
	 */
	inline bool __attribute__((always_inline)) pressedHotkey(const HotkeyEntry hotkey) {
		return (hotkey.action != 0 && pressedButton(hotkey.buttonsMask) &&
				pressedDpad(hotkey.dpadMask) && pressedAux(hotkey.auxMask));
	}

	/**
	 * @brief Remote hotkey bits from the state bitmask and provide pressed action.
	 */
	inline GamepadHotkey __attribute__((always_inline)) selectHotkey(const HotkeyEntry hotkey) {
		state.buttons &= ~(hotkey.buttonsMask);
		state.dpad &= ~(hotkey.dpadMask);
		return static_cast<GamepadHotkey>(hotkey.action);
	}

	inline bool __attribute__((always_inline)) pressedUp()    { return pressedDpad(GAMEPAD_MASK_UP); }
	inline bool __attribute__((always_inline)) pressedDown()  { return pressedDpad(GAMEPAD_MASK_DOWN); }
	inline bool __attribute__((always_inline)) pressedLeft()  { return pressedDpad(GAMEPAD_MASK_LEFT); }
	inline bool __attribute__((always_inline)) pressedRight() { return pressedDpad(GAMEPAD_MASK_RIGHT); }
	inline bool __attribute__((always_inline)) pressedB1()    { return pressedButton(GAMEPAD_MASK_B1); }
	inline bool __attribute__((always_inline)) pressedB2()    { return pressedButton(GAMEPAD_MASK_B2); }
	inline bool __attribute__((always_inline)) pressedB3()    { return pressedButton(GAMEPAD_MASK_B3); }
	inline bool __attribute__((always_inline)) pressedB4()    { return pressedButton(GAMEPAD_MASK_B4); }
	inline bool __attribute__((always_inline)) pressedL1()    { return pressedButton(GAMEPAD_MASK_L1); }
	inline bool __attribute__((always_inline)) pressedR1()    { return pressedButton(GAMEPAD_MASK_R1); }
	inline bool __attribute__((always_inline)) pressedL2()    { return pressedButton(GAMEPAD_MASK_L2); }
	inline bool __attribute__((always_inline)) pressedR2()    { return pressedButton(GAMEPAD_MASK_R2); }
	inline bool __attribute__((always_inline)) pressedS1()    { return pressedButton(GAMEPAD_MASK_S1); }
	inline bool __attribute__((always_inline)) pressedS2()    { return pressedButton(GAMEPAD_MASK_S2); }
	inline bool __attribute__((always_inline)) pressedL3()    { return pressedButton(GAMEPAD_MASK_L3); }
	inline bool __attribute__((always_inline)) pressedR3()    { return pressedButton(GAMEPAD_MASK_R3); }
	inline bool __attribute__((always_inline)) pressedA1()    { return pressedButton(GAMEPAD_MASK_A1); }
	inline bool __attribute__((always_inline)) pressedA2()    { return pressedButton(GAMEPAD_MASK_A2); }

	// TODO wireUDLR
	inline bool __attribute__((always_inline)) activeWireUp()    	{ return activeWireD(GAMEPAD_MASK_UP); }
	inline bool __attribute__((always_inline)) activeWireDown()  	{ return activeWireD(GAMEPAD_MASK_DOWN); }
	inline bool __attribute__((always_inline)) activeWireLeft()  	{ return activeWireD(GAMEPAD_MASK_LEFT); }
	inline bool __attribute__((always_inline)) activeWireRight() 	{ return activeWireD(GAMEPAD_MASK_RIGHT); }
	inline bool __attribute__((always_inline)) activeWireB1()   	{ return activeWireB(GAMEPAD_MASK_B1); }
	inline bool __attribute__((always_inline)) activeWireB2()   	{ return activeWireB(GAMEPAD_MASK_B2); }
	inline bool __attribute__((always_inline)) activeWireB3()   	{ return activeWireB(GAMEPAD_MASK_B3); }
	inline bool __attribute__((always_inline)) activeWireB4()   	{ return activeWireB(GAMEPAD_MASK_B4); }
	inline bool __attribute__((always_inline)) activeWireL1()   	{ return activeWireB(GAMEPAD_MASK_L1); }
	inline bool __attribute__((always_inline)) activeWireR1()   	{ return activeWireB(GAMEPAD_MASK_R1); }
	inline bool __attribute__((always_inline)) activeWireL2()   	{ return activeWireB(GAMEPAD_MASK_L2); }
	inline bool __attribute__((always_inline)) activeWireR2()   	{ return activeWireB(GAMEPAD_MASK_R2); }
	inline bool __attribute__((always_inline)) activeWireS1()   	{ return activeWireB(GAMEPAD_MASK_S1); }
	inline bool __attribute__((always_inline)) activeWireS2()   	{ return activeWireB(GAMEPAD_MASK_S2); }
	inline bool __attribute__((always_inline)) activeWireL3()   	{ return activeWireB(GAMEPAD_MASK_L3); }
	inline bool __attribute__((always_inline)) activeWireR3()   	{ return activeWireB(GAMEPAD_MASK_R3); }
	inline bool __attribute__((always_inline)) activeWireA1()   	{ return activeWireB(GAMEPAD_MASK_A1); }
	inline bool __attribute__((always_inline)) activeWireA2()   	{ return activeWireB(GAMEPAD_MASK_A2); }

	const GamepadOptions& getOptions() const { return options; }

	void setInputMode(InputMode inputMode) { options.inputMode = inputMode; }
	void setSOCDMode(SOCDMode socdMode) { options.socdMode = socdMode; }
	void setDpadMode(DpadMode dpadMode) { options.dpadMode = dpadMode; }

	GamepadDebouncer debouncer;
	const uint8_t debounceMS;
	GamepadState rawState;
	GamepadState state;
	// Pin Wired Map
	GamepadButtonMapping *mapWireUp;
	GamepadButtonMapping *mapWireDown;
	GamepadButtonMapping *mapWireLeft;
	GamepadButtonMapping *mapWireRight;
	GamepadButtonMapping *mapWireB1;
	GamepadButtonMapping *mapWireB2;
	GamepadButtonMapping *mapWireB3;
	GamepadButtonMapping *mapWireB4;
	GamepadButtonMapping *mapWireL1;
	GamepadButtonMapping *mapWireR1;
	GamepadButtonMapping *mapWireL2;
	GamepadButtonMapping *mapWireR2;
	GamepadButtonMapping *mapWireS1;
	GamepadButtonMapping *mapWireS2;
	GamepadButtonMapping *mapWireL3;
	GamepadButtonMapping *mapWireR3;
	GamepadButtonMapping *mapWireA1;
	GamepadButtonMapping *mapWireA2;
	// Pin Profile Map
	GamepadButtonMapping *mapDpadUp;
	GamepadButtonMapping *mapDpadDown;
	GamepadButtonMapping *mapDpadLeft;
	GamepadButtonMapping *mapDpadRight;
	GamepadButtonMapping *mapButtonB1;
	GamepadButtonMapping *mapButtonB2;
	GamepadButtonMapping *mapButtonB3;
	GamepadButtonMapping *mapButtonB4;
	GamepadButtonMapping *mapButtonL1;
	GamepadButtonMapping *mapButtonR1;
	GamepadButtonMapping *mapButtonL2;
	GamepadButtonMapping *mapButtonR2;
	GamepadButtonMapping *mapButtonS1;
	GamepadButtonMapping *mapButtonS2;
	GamepadButtonMapping *mapButtonL3;
	GamepadButtonMapping *mapButtonR3;
	GamepadButtonMapping *mapButtonA1;
	GamepadButtonMapping *mapButtonA2;
	GamepadButtonMapping **gamepadMappings;

	inline static const SOCDMode resolveSOCDMode(const GamepadOptions& options) {
		 return (options.socdMode == SOCD_MODE_BYPASS &&
				 (options.inputMode == INPUT_MODE_HID ||
				  options.inputMode == INPUT_MODE_SWITCH ||
				  options.inputMode == INPUT_MODE_PS4)) ?
				SOCD_MODE_NEUTRAL : options.socdMode;
	};

private:
	void releaseAllKeys(void);
	void pressKey(uint8_t code);
	uint8_t getModifier(uint8_t code);
	uint8_t getMultimedia(uint8_t code);
	void processHotkeyIfNewAction(GamepadHotkey action);

	GamepadOptions& options;
	const HotkeyOptions& hotkeyOptions;

	GamepadHotkey lastAction = HOTKEY_NONE;
};

#endif
