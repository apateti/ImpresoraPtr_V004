#ifndef ModelIndice_h
#define ModelIndice_h


#include "SD_MMC.h"
#include "Definiciones.h"



struct stIndice {
  char chNumCedRif[1];
  uint32_t ui32CedRif ;
  uint32_t ui32NumCon ;
  uint32_t ui32TimeStamp ;
  uint32_t ui32SizeFile;
  uint32_t ui32PosEnArch ;
  char pszNameIndex[32] ;
  bool bAddIndex(fs::FS &fs) {
    Serial.printf("Nombre del Archivo: %s\n", pszNameIndex);
    File FIndex ; 
    if(!fs.exists(pszNameIndex)){
      FIndex = fs.open(pszNameIndex,FILE_WRITE) ;
    }else{
      FIndex = fs.open(pszNameIndex,FILE_APPEND) ;
    }
    
    if(!FIndex){
      Serial.println(" Error Abriendo Archivo");
    }
    FIndex.write((uint8_t*)&ui32NumCon,4) ;
    FIndex.write((uint8_t*)&ui32TimeStamp,4) ;
    FIndex.write((uint8_t*)&ui32SizeFile,4) ;
    FIndex.write((uint8_t*)&ui32PosEnArch,4) ;
    FIndex.close() ;
    FIndex = fs.open(pszNameIndex) ;
    Serial.printf("El tamano del archivo en Escritura es: %d\n", FIndex.size());
    FIndex.close() ;
    return(1) ;
  }
  bool bReBuid(const char *  szFile){

  } 
  bool bFindIndex(fs::FS &fs ) 
  {
    File FIndex ;
    uint32_t ui32Aux  ;
    boolean bFlag = 0 ;

    if( ui32NumCon != 0)
    {
      ui32Aux = ui32NumCon ;
      bFlag = 0 ;
    }
    else
    {
      ui32Aux = ui32TimeStamp ;
      bFlag = 1 ;
    }

    FIndex = fs.open(pszNameIndex,FILE_READ);
    if(!FIndex || FIndex.size() == 0){
      FIndex.close() ;
      return (false);
    }
    do{
      FIndex.read((uint8_t*)&ui32NumCon, sizeof(ui32NumCon)) ;
      FIndex.read((uint8_t*)&ui32TimeStamp, sizeof(ui32TimeStamp)) ;
      FIndex.read((uint8_t*)&ui32SizeFile, sizeof(ui32TimeStamp)) ;
      FIndex.read((uint8_t*)&ui32PosEnArch,sizeof(ui32PosEnArch)) ;
      if(ui32Aux == ui32NumCon && bFlag == 0)
      {
        FIndex.close() ;
        return(true) ;
      }
      if(ui32Aux == ui32TimeStamp && bFlag == 1)
      {
        FIndex.close() ;
        return(true) ;
      }
    }
    while(FIndex.available()) ;
    FIndex.close() ;
    return(false) ;
  }
};

extern struct stIndice stcG_Index ;





















#endif ModelIndice_h