/*
 *  general.h
 *  
 *
 *  Created by Alex Marshall on 09/01/30.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#define BENZIN_VERSION_MAJOR        1
#define BENZIN_VERSION_MINOR        2
#define BENZIN_VERSION_BUILD        1
#define BENZIN_VERSION_OTHER        "BETA"

#define INFORMATION_TEXT        \
"Benzin %d.%d.%d%s.\n" \
"Written by SquidMan (Alex Marshall), comex, and megazig.\n" \
"(c) 2009 HACKERCHANNEL\n", BENZIN_VERSION_MAJOR, BENZIN_VERSION_MINOR, BENZIN_VERSION_BUILD, BENZIN_VERSION_OTHER

#define fatal(x)    printf(x); exit(1)

