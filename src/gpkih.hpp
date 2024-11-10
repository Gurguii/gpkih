#pragma once

#include "libs/logger/logger.hpp" // Logger gpkihLogger
#include "buffer.hpp"

#include "aliases.hpp"
#include "macros.hpp"
#include "consts.hpp"

// TODO - Think about adding these within new Config class. Maybe add as properties? maybe as a custom struct 
// like 'struct RuntimeParams'
extern bool DRY_RUN;
extern bool SHOW_HEADER;

extern string GPKIH_DIR_ROOT;   // ~/.config/gpkih
extern string GPKIH_DIR_CONFIG; // ~/.config/gpkih/config
extern string GPKIH_DIR_DB;     // ~/.config/gpkih/db

extern GpkihBuffer *gpkihBuffer;

// TODO - Give each profile their own logger instance and don't only log info (warnings and errors)
extern gurgui::logging::Logger *gpkihLogger;