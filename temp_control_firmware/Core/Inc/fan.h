/**
 * @file fan.h
 * @brief Cooling fan control module.
 *
 * This module provides an interface for controlling an additional
 * control output device in the temperature control system.
 * The fan is used as an auxiliary actuator to reduce temperature
 * when the measured value exceeds the reference value.
 *
 * The module is intended to be used by the main application logic,
 * while the decision logic (when to turn the fan on or off) is kept
 * outside of this module.
 */

#ifndef INC_FAN_H_
#define INC_FAN_H_

#include <stdbool.h>

void Fan_Set(bool on);

#endif /* INC_FAN_H_ */
