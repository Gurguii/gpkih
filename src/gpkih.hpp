#pragma once

#include "libs/logger/logger.hpp" // Logger gpkihLogger
#include "buffer.hpp"

#include "aliases.hpp"
#include "macros.hpp"
#include "consts.hpp"

extern bool DRY_RUN;
extern bool SHOW_HEADER;

extern string GPKIH_BASEDIR;
extern string CONF_DIRPATH;
extern string DB_DIRPATH;

// Buffer instance to manage dynamically allocated memory, used by any part of the program
// that would require allocating dynamic memory (malloc()) started by main() to 'MAYBE, NOT YET'
// allow modification of the buffer size from configuration file 'gpkih.conf'
extern GpkihBuffer *gpkihBuffer;

// TODO - Give each profile their own logger instance and don't only log info (warnings and errors)
extern gurgui::logging::Logger *gpkihLogger;