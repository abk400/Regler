#ifndef LOGGER_H
#define LOGGER_H

#define _DEBUG_

// Define where debug output will be printed.
#define D_PRINTER Serial

// Setup debug printing macros.
#ifdef _DEBUG_
#define D_PRINT(...) { D_PRINTER.print(__VA_ARGS__); }
#define D_PRINTLN(...) { D_PRINTER.println(__VA_ARGS__); }
#define D_PRINT_F(...) { D_PRINTER.printf(__VA_ARGS__); }
#else
#define D_PRINT(...) {}
#define D_PRINTLN(...) {}
#define D_PRINT_F(...) {}
#endif

#endif
