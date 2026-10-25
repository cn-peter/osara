/*
 * OSARA: Open Source Accessibility for the REAPER Application
 * ZDSR screen reader API
 * License: GNU General Public License version 2.0
 */

#pragma once

#include <string>

bool sendZdsrMessage(const std::string& message, bool interrupt = true);
void terminateZdsr();
