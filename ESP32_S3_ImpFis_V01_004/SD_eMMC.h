#ifndef SD_eMMC_h
#define SD_eMMC_h
#include "FS.h"
#include "SD_MMC.h"
//#include <SD.h>
//#include "SdFat.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include <stdio.h>
#include <string.h>
#include "Definiciones.h"

#define eMMC_CMD 36
#define eMMC_CLK 35
#define eMMC_D0 39
#define eMMC_D1 40
#define eMMC_D2 38
#define eMMC_D3 41


class SD_eMMC{
  public:
  SD_eMMC();
  bool boInicializa_eMMC();
  bool boRenameFile(fs::FS &fs, const char *path1, const char *path2);
  bool boDeleteFile(fs::FS &fs, const char *path);
  bool boAppendFile(fs::FS &fs, const char *path, const char *message);
  bool boWriteFile(fs::FS &fs, const char *path, const char *message);
  bool boWriteFile(fs::FS &fs, const char *path, uint8_t *estructura, uint16_t sizeEstruc);
  String strReadFile(fs::FS &fs, const char *path);
  void strReadFileData(fs::FS &fs, const char *path, uint8_t *estructura, uint16_t sizeEstruc);
  bool boCreateDir(fs::FS &fs, const char *path);
  bool boRemoveDir(fs::FS &fs, const char *path);
  bool boInfoeMMc(void);
  bool boExistFile(fs::FS &fs, const char *path);
  bool boExistDir(fs::FS &fs, const char *dirname);
  
};

#endif
