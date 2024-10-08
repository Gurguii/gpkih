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

extern string GPKIH_BASEDIR;
extern string CONF_DIRPATH;
extern string DB_DIRPATH;

extern GpkihBuffer *gpkihBuffer;

// TODO - Give each profile their own logger instance and don't only log info (warnings and errors)
extern gurgui::logging::Logger *gpkihLogger;