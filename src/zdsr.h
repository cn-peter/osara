/*
 * OSARA: Open Source Accessibility for the REAPER Application
 * ZDSR screen reader API
 * License: GNU General Public License version 2.0
 */

#pragma once

#include <string>

bool initializeZdsr(bool independentChannel, const std::string& channelName);
bool sendZdsrMessage(const std::string& message, bool interrupt,
	bool independentChannel, const std::string& channelName);
void terminateZdsr();
