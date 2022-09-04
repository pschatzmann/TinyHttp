#pragma once
#include "Extensions/ExtensionStream.h"
#include "Extensions/ExtensionLoggingStream.h"

#ifndef TINYHTTP_NO_SD
#include "Extensions/ExtensionSD.h"
#include "Extensions/ExtensionSDStreamed.h"
#include "Extensions/ExtensionMusicFileStream.h"
#endif

#ifdef ESP32
#include "Extensions/ExtensionESPTask.h"
#endif