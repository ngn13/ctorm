#pragma once

/*

 * these options are provided as arguments to GCC in the Makefile
 * however in case the library gets compiled differently
 * the default values are defined here as a backup

*/

#ifndef CTORM_DEBUG
#define CTORM_DEBUG 0
#endif

#ifndef CTORM_JSON_SUPPORT
#define CTORM_JSON_SUPPORT 1
#endif
