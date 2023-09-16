#include "SD_eMMC.h"
#include "ModelIndice.h"
extern stIndice stcG_Index ;
SD_eMMC::SD_eMMC(){

}

bool SD_eMMC::boInicializa_eMMC(){
  if (!SD_MMC.setPins(eMMC_CLK, eMMC_CMD, eMMC_D0, eMMC_D1, eMMC_D2, eMMC_D3)) {
    //Serial.println("Set pin failed!");
    return(ERROR);
  }
  if (!SD_MMC.begin("/sdcard", false, false, 1000, 5)) {
    //Serial.println("Card Mount Failed");
    return(ERROR);
  }
  return(SUCCESS);
}

bool SD_eMMC::boRenameFile(fs::FS &fs, const char *path1, const char *path2) {
  //Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    //Serial.println("File renamed");
    return(SUCCESS);
  } else {
    //Serial.println("Rename failed");
    return(ERROR);
  }
}

bool SD_eMMC::boDeleteFile(fs::FS &fs, const char *path) {
  //Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    return(SUCCESS);
    // Serial.println("File deleted");
  } else {
    return(ERROR);
    // Serial.println("Delete failed");
  }
}

bool SD_eMMC::boAppendFile(fs::FS &fs, const char *path, const char *message) {
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  //file.seek(100, SeekSet);
  if (!file) {
    // Serial.println("Failed to open file for appending");
    return(ERROR);
  }
  int auxT = file.size();
  if(auxT < 0){
    stcG_Index.ui32PosEnArch = 0;
  }else{
    stcG_Index.ui32PosEnArch = file.size();
  }
  if (file.print(message)) {
    // Serial.println("Message appended");

    file.close();
    return(SUCCESS);
  } else {
    // Serial.println("Append failed");
    file.close();
    return(ERROR);
  }
}

bool SD_eMMC::boWriteFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    // Serial.println("Failed to open file for writing");
    return(ERROR);
  }
  if (file.print(message)) {
    // Serial.println("File written");
    file.close();
    return(SUCCESS);
  } else {
    // Serial.println("Write failed");
    file.close();
    return(ERROR);
  }
}

bool SD_eMMC::boWriteFile(fs::FS &fs, const char *path, uint8_t *estructura, uint16_t sizeEstruc) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    // Serial.println("Failed to open file for writing");
    return(ERROR);
  }
  // Serial.println("Imprimeindo datos de la Estructura");
  // for(int i=0; i<20; i++){
  //   Serial.printf("Para el Tipo de Impuesto %d el BI es: %d\n", i+1, estructura[i]);
  // }
  Serial.println("\n\nIniciando Escritura en la eMMC de la Estructura Reporte X");
  file.write((estructura), sizeEstruc);
  // if (file.print(message)) {
  //   // Serial.println("File written");
  //   file.close();
  //   return(SUCCESS);
  // } else {
  //   // Serial.println("Write failed");
  file.close();
  Serial.println("Finalizado Escritura en la eMMC de la Estructura Reporte X");
  return(SUCCESS);
  // }
}

String SD_eMMC::strReadFile(fs::FS &fs, const char *path) {
  Serial.println("Iniciando Rutina strReadFile");
  String fileRead = "";
  // unsigned long timeIni;
  // timeIni = millis();
  // Serial.printf("Reading file: %s\n", path);
  Serial.println("Abriendo Archivo en strReadFile");
  File file = fs.open(path);
  if (!file) {
    // Serial.println("Failed to open file for reading");
    return(fileRead);
  }
  // Serial.print("\nTiempo Open: ");
  // Serial.print(millis() - timeIni);
  // Serial.println(" mSeg");
  // Serial.print("Read from file: \n");
  Serial.println("Leyendo Archivo en strReadFile");
  while (file.available()) {
    // Serial.write(file.read());
    fileRead +=  char(file.read());
  }
  // Serial.print("\nTiempo Total de Lectura: ");
  // Serial.print(millis() - timeIni);
  // Serial.println(" mSeg");
  Serial.println("Cerrando Archivo en strReadFile");
  file.close();
  Serial.println("Finalizando Rutina strReadFile");
  return(fileRead);
}

void SD_eMMC::strReadFileData(fs::FS &fs, const char *path, uint8_t *estructura, uint16_t sizeEstruc) {
  Serial.println("Iniciando Rutina strReadFile");
  //String fileRead = "";
  // unsigned long timeIni;
  // timeIni = millis();
  // Serial.printf("Reading file: %s\n", path);
  Serial.println("Abriendo Archivo en strReadFile");
  File file = fs.open(path);
  if (!file) {
    // Serial.println("Failed to open file for reading");
    return;
  }
  file.read((estructura), (sizeEstruc));
  // Serial.print("\nTiempo Open: ");
  // Serial.print(millis() - timeIni);
  // Serial.println(" mSeg");
  // Serial.print("Read from file: \n");
  // Serial.println("Leyendo Archivo en strReadFile");
  // while (file.available()) {
  //   // Serial.write(file.read());
  //   fileRead +=  char(file.read());
  // }
  // Serial.print("\nTiempo Total de Lectura: ");
  // Serial.print(millis() - timeIni);
  // Serial.println(" mSeg");
  Serial.println("Cerrando Archivo en strReadFile");
  file.close();
  // const uint8_t *p = (const uint8_t *)estructura;
  // for(int i=0; i<sizeof(sizeEstruc); i++){
  //   unsigned char b = *p++;
  //   Serial.print(b >> 4, HEX);
  //   Serial.print(b & 15, HEX);
  //   Serial.print(' ');
  //   if(i%16 == 0){
  //     Serial.println();
  //   }
  // }
  Serial.println("Finalizando Rutina strReadFile");
  // return(fileRead);
}


bool SD_eMMC::boCreateDir(fs::FS &fs, const char *path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    // Serial.println("Dir created");
    return(SUCCESS);
  } else {
    // Serial.println("mkdir failed");
    return(ERROR);
  }
}

bool SD_eMMC::boRemoveDir(fs::FS &fs, const char *path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    // Serial.println("Dir removed");
    return(SUCCESS);
  } else {
    // Serial.println("rmdir failed");
    return(ERROR);
  }
}

bool SD_eMMC::boInfoeMMc(void){
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD_MMC card attached");
    return(ERROR);
  }
  Serial.print("SD_MMC Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD_MMC Card Size: %llu MB\n", cardSize);
  Serial.printf("Total space: %llu MB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %llu MB\n", SD_MMC.usedBytes() / (1024 * 1024));
  return(SUCCESS);
}

bool SD_eMMC::boExistFile(fs::FS &fs, const char *path){
  //File file = fs.open(path);
  if(fs.exists(path)){
    return(SUCCESS);
  }else{
    return(ERROR);
  }
}

bool SD_eMMC::boExistDir(fs::FS &fs, const char *dirname){
  File root = fs.open(dirname);
  if(!root){
    return(ERROR);
  }
  return(SUCCESS);
  // if(fs.exists(path)){
  //   return(SUCCESS);
  // }else{
  //   return(ERROR);
  // }
}
