#include "FS.h"
#include "SD_MMC.h"


//String indexOfFilename(String filename);
String formatBytes(size_t bytes);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
char deleteFile(fs::FS &fs, const char * path);
char writeFile(fs::FS &fs, const char * path, const char * message);
