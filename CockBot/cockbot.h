// Guards and includes
// *********************

#ifndef COCKBOT_H_INCLUDED
#define COCKBOT_H_INCLUDED

#include "ficl/ficl.h"
// *********************
// Global Forth
// *********************

extern ficlSystem *forth_system;
extern time_t uptime;
extern long long polloemotes[31];

// *********************
// Values
// *********************

#define MUTE_ROLE 1342319605925937232

#define GUILD_ID 953769673634246757

#define BOT_STATUS "with FORTH" // Will display next to name
#define BOT_DETAILS "AAAAH"

#define ADMIN_ROLE 1235943635888111637

#define POLLO_ID 958592178026852352 // Emoji ID are fairly annoying to get.

// Convert regular rgb hex to decimal.
#define COLOR_SUCCESS 4835913  // #49CA49
#define COLOR_FAILURE 16077157 // #F55165

#define WATCHCAT_TIME                                                          \
  10 // If a given command takes longer than the said time in seconds it will
     // be automatically killed!

#define LINE_BUFFERSIZE 512 // Pretty big but I have a habbit of one liners.
#define ADMIN_SCRIPT "fth-scripts/admin.fth"

#endif
