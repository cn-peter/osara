/*
 * OSARA: Open Source Accessibility for the REAPER Application
 * ZDSR screen reader API
 * License: GNU General Public License version 2.0
 */

#include <windows.h>
#include <string>
#include "zdsr.h"

namespace {

using InitTtsFn = int (WINAPI*)(int, const WCHAR*, BOOL);
using SpeakFn = int (WINAPI*)(const WCHAR*, BOOL);

#ifdef _WIN64
constexpr WCHAR ZDSR_DLL_PATH[] =
	L"C:\\Program Files (x86)\\zdsr\\zdsr\\ZDSRAPI_x64.dll";
#else
constexpr WCHAR ZDSR_DLL_PATH[] =
	L"C:\\Program Files (x86)\\zdsr\\zdsr\\ZDSRAPI.dll";
#endif

HMODULE zdsrDll = nullptr;
InitTtsFn initTts = nullptr;
SpeakFn speak = nullptr;
bool loadAttempted = false;
bool ttsInitialized = false;
bool ttsUsesIndependentChannel = false;
std::string ttsChannelName;

std::wstring widen(const std::string& text) {
	if (text.empty()) {
		return {};
	}
	UINT codePage = CP_UTF8;
	DWORD flags = MB_ERR_INVALID_CHARS;
	int length = MultiByteToWideChar(codePage, flags, text.data(),
		static_cast<int>(text.size()), nullptr, 0);
	if (length == 0) {
		// Preserve OSARA's fallback for legacy ANSI strings.
		codePage = CP_ACP;
		flags = 0;
		length = MultiByteToWideChar(codePage, flags, text.data(),
			static_cast<int>(text.size()), nullptr, 0);
	}
	std::wstring result(length, L'\0');
	MultiByteToWideChar(codePage, flags, text.data(),
		static_cast<int>(text.size()), result.data(), length);
	return result;
}

void loadZdsr() {
	if (loadAttempted) {
		return;
	}
	loadAttempted = true;
	zdsrDll = LoadLibraryW(ZDSR_DLL_PATH);
	if (!zdsrDll) {
		return;
	}
	initTts = reinterpret_cast<InitTtsFn>(
		GetProcAddress(zdsrDll, "InitTTS"));
	speak = reinterpret_cast<SpeakFn>(GetProcAddress(zdsrDll, "Speak"));
	if (!initTts || !speak) {
		FreeLibrary(zdsrDll);
		zdsrDll = nullptr;
		initTts = nullptr;
		speak = nullptr;
	}
}

bool initializeTts(bool independentChannel, const std::string& channelName) {
	loadZdsr();
	if (!initTts) {
		return false;
	}
	if (ttsInitialized && ttsUsesIndependentChannel == independentChannel &&
		(!independentChannel || ttsChannelName == channelName)) {
		return true;
	}
	std::wstring wideChannelName;
	if (independentChannel && !channelName.empty()) {
		wideChannelName = widen(channelName);
	}
	if (initTts(independentChannel ? 1 : 0,
		wideChannelName.empty() ? nullptr : wideChannelName.c_str(), TRUE) != 0) {
		ttsInitialized = false;
		return false;
	}
	ttsInitialized = true;
	ttsUsesIndependentChannel = independentChannel;
	ttsChannelName = channelName;
	return true;
}

} // namespace

bool initializeZdsr(bool independentChannel, const std::string& channelName) {
	return initializeTts(independentChannel, channelName);
}

bool sendZdsrMessage(const std::string& message, bool interrupt,
	bool independentChannel, const std::string& channelName
) {
	if (message.empty() || !initializeTts(independentChannel, channelName)) {
		return false;
	}
	return speak(widen(message).c_str(), interrupt ? TRUE : FALSE) == 0;
}

void terminateZdsr() {
	if (zdsrDll) {
		FreeLibrary(zdsrDll);
	}
	zdsrDll = nullptr;
	initTts = nullptr;
	speak = nullptr;
	ttsInitialized = false;
	ttsChannelName.clear();
}
