#include "Definiciones.h"
#include "Model.h"
#include "stdio.h"
#include "SD_eMMC.h"
#define ARDUINOJSON_USE_LONG_LONG 1
#define ARDUINOJSON_ENABLE_NAN 1
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "Funciones.h"
#include "FS.h"
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <SerialFlash.h>
#include "ModelIndice.h"
//****************************************************************************
//RTC1302
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <Wire.h>
#include <time.h>
#include <TimeLib.h>
#define DS1302_CLK_PIN 7
#define DS1302_IO_PIN 6
#define DS1302_CE_PIN 4
ThreeWire myWire(DS1302_IO_PIN, DS1302_CLK_PIN, DS1302_CE_PIN);
RtcDS1302<ThreeWire> rtc1302(myWire);  //OBJETO De TIPO RtcDS1302
//******************************************************************
//** Se Definen los pines del bus SPI ESP32-S3
//******************************************************************
#define SCKPIN 12   // SCK Pin
#define MISOPIN 11  // MISO Pin
#define MOSIPIN 13  // MOSI Pin
#define CSPIN 10    // CS Pin
SPIClass spiFlash(HSPI);
//****************************************************************************
#ifdef useSerialOutPtr
HardwareSerial SerialOutPtr(1);
#endif
SD_eMMC mySD_eMMC;
//Variables a ser Almacenadas los Productos y el
Productos* stcG_Productos;
char* chG_ImpPtr;
Encabezado stcG_Encabezado[CantMaxEncab];
LineaAdPostF stcG_LineaAdPostF[CantMaxLinAdPF];
PieTicket stcG_PieTicket[CantMaxPieTick];
Cliente stcG_Cliente;
//Comentario
Comentario stcG_Comentario;
char chG_SerialPtr[CantMaxSerialPtr];
char chG_SimMoneda[CantMaxCarConf];
//cantidad de Impuestos de Articulos a calcular
uint8_t ui8G_CountImpArt;
//Estructura para Almacenar los Impuestos. Se Leen desde el archivo infoPtr.txt
ImpuestoArticulos stcG_ImpuestoArticulos[CantMaxImpArt];
TotalImpuestoArtiulos stcG_TotalImpuestoArtiulos[CantMaxImpArt];
//Estructura para campos obligatorios de la Nota de Credito
NotaCreditoObli stcG_NotaCreditoObli;
//ContadoDocumentos stcG_ContadoDocumentos;     //Estructura de Contadores de Documentos
Contadores stcG_Contadores;  //Estructura de Contadores de Documentos
//cantidad de Impuestos para Las formas de Pagos de la Factura
uint8_t ui8G_CountFormPago;
//cantidad de Formas de Pago Totales a imprimir de la Factura
uint8_t ui8G_CountFormPagoTotal;
//Estructura para Almacenar los Impuestos. Se Leen desde el archivo infoPtr.txt
FormasPagoImp stcG_FormasPagoImp[CantMaxImpPago];
FormasPagoRxCMD stcG_FormasPagoRxCMD[CantMaxImpPago];
TotalImpuestoFormPago stcG_TotalImpuestoFormPago[CantMaxImpPago];
//Estructura para los Reportes X y Z
XyZ stcG_XyZ;
//Estructura para los Acumuladoos diarios de las BI por Venta, Desc, etc.
TotalAcumuladoBI stcG_TotalAcumuladoBI;
//Estructura para los Acumuladoos diarios de las FP por ejm Efectivo T.Debito etc
TotalAcumuladoFP stcG_TotalAcumuladoFP;
//Mis Funciones para Utilidades   -> Funciones.h
Funciones myFunciones;
//Variables de Estructuras utilizadas para la Factura
State stcG_currentState;        //Se define la Variable currentState para la Maquina de Estado
Input stcG_currentInput;        //Estado Actual
DynamicJsonDocument doc(2048);  //Para Recibir los Datos Json
//Flag que Indican Error
FlagErr stcG_FlagErr;
//Flag que Indican Estatus de la Factura
FlagFact stcG_FlagFact;
//Numero de Producto que se a Recibido
uint16_t ui16G_NroProducto;  //Cuenta la Cantidad de Productos Recibidos
byte byG_NroLinAdCli;        // Cantidad de Lineas adicionales en la Trama Clientes
byte byG_NroLinAdicionales;  // Cantidad de Lineas adicionales post Productos
byte byG_NroLinAdPieTick;    // Cantidad de Lineas adicionales de Pie de Tickets
byte byG_NroEncab;           // Cantidad de Encabezados Rx
//*************************************************************************
//Variable *** ui8G_CountFormPagoRx *** de tipo uint8
//  En esta variable se lancantidad de Formas de Pago recibidas por el comando
//  Si no llega la Forma de Pago se Inicializa en el Tipo 1
//*************************************************************************
uint8_t ui8G_CountFormPagoRx;
//*************************************************************************
//Variable *** ui8G_CountFormPagoImp *** de tipo uint8
//  En esta variable es la cantidad de Formas de Pago a Ser Impresas en Factura
//  Si no llega la Forma de Pago se Imprime la FP Tipo 1
//*************************************************************************
uint8_t ui8G_CountFormPagoImp;
//*************************************************************************
//Variable *** ui64L_SubTotalFact *** de tipo uint64_t
//  En esta variable se Coloca el Sup Total de la Factura
//  Es la Suma la Suma de los montos de los Productos + El impuesto (si aplica)
//*************************************************************************
uint64_t ui64L_SubTotalFact;
//*************************************************************************
//Variable *** ui64L_TotalMontoFactura *** de tipo uint64_t
//  En esta variable que Almacena el Monto Total a cancelar en la
//  Factuar
//*************************************************************************
uint64_t ui64L_TotalMontoFactura;
//Variable *** chG_TituloFormPago *** de tipo char
//  En esta variable que Almacena el Titulo que se Muestra en la
//  Forma de Pago a Cancelar
//*************************************************************************
char chG_TituloFormPago[CantMaxCarLin];
//*************************************************************************
//Variable *** ui32G_Time24H *** de tipo unit32
//  En esta variable llevara el conteo de 1 Hora em milisegundos, cada vez que se active
//  revisara si a pasado el tiempo de 24H para hacer el Reporte Z
//*************************************************************************
#define Constante1Hora 3600000  //Constante = a 3600000 uSeg, equivale a 1H
uint32_t ui32G_Time1H;


stIndice stcG_Index;
//*************************************************************************
//Variable *** ui64L_SubTotalImpFormPago *** de tipo uint64_t
//  En esta variable se Coloca el Sup Total de la Suma de los Impuestos
//  por la forma de Pago (si aplica)
//*************************************************************************
uint64_t ui64L_SubTotalImpFormPago;

//uint32_t ui32G_CountFact;
//
//
char chG_tituloFactura[CantMaxCarLin];  //Se coloca el Titulo de la Factura SENIAT y RIF (desde archivo configParam.json)
char chG_rifEmpresa[CantMaxCarLin];     //Se coloca el RIF de la Empresa en la Factura (desde archivo configParam.json)

//byte byG_CodeTx;                //Variable que tendra el Code que se Tx
//char chG_CmdRx[CaracMaxCmdTx];  //Variable para almacenar el Comando Json a Tx
String strG_JsonRx;           //Se almacena el JSON Rx
uint32_t inCantMaxPsramProd;  //Cantidad de Memoria PSRAM reservada para las Variables
char chG_Util[CantMaxDirFile];
//Encabezado *stcG_Encabezado = (Encabezado *)ps_malloc(CantMaxCarLin * CantMaxEncab);
//*************************************************************************
//Declaracion de Funciones
//  Las Funciones utilizadas se declaran a continuacion
//
//*************************************************************************
bool bValidoSecuencia_cmd(void);
void updateStateMachine(void);
void listDir(fs::FS& fs, const char* dirname, uint8_t levels);
//Estructura de Respuesta a Comandos
CmdTxJson stcG_CmdTxJson;

void setup() {
  fInicializaFactura();
  inicializaAcumuladores();
  if (boInicializarHardware()) {
    Serial.println("❌ERROR, No puede continuar por error en Hardware");
    return;
  }

  if (boCompruebaCambio_Memoria()) {
    Serial.println("ERROR, memoria eMMC o FLASH cambiada");
  }
  bfLeerReporteX();
  RtcDateTime timeRTC;
  timeRTC = rtc1302.GetDateTime();
  uint32_t timeNowUnix = myFunciones.timeToUnix(timeRTC);
  uint32_t auxTime = timeNowUnix - stcG_XyZ.stContadores.ui32FechaIniRep;
  // Si ui32FechaIniRep = 0 Y ui32UltFact = 0 -> Impresora de Fabrica
  if (stcG_XyZ.stContadores.ui32FechaIniRep != 0 && stcG_XyZ.stContadores.ui32UltFact != 0) {
    Serial.println("Comprobando si hay que generar Reporte Z **Se paso de 24H**");
    if (auxTime > Segundos24H) {
      Serial.println("Inicializando Reportes de Z");
      //generaReporteZ();
      outputGenImpRepZ();
    } else {
      Serial.println("Se Reviso generar Reporte Z **No han pasado 24H**");
    }
  } else {
    Serial.println("No se Revisa el Reporte Z por estar la Ptr de Fabrica");
  }
  ui32G_Time1H = millis();
}

void loop() {
  if (bReadUART()) {
    if (bValidoJson()) {
      if (bValidoDatos_cmd()) {
        if (bValidoSecuencia_cmd()) {
          //byG_CodeTx = ErrorNo;
          stcG_CmdTxJson.ui8CodeTx = ErrorNo;
          updateStateMachine();
        }
        stcG_FlagFact.flagTxRespuesta = 1;
      }
    }
  }
  if (stcG_FlagFact.flagTxRespuesta) {
#ifdef useDebug
    Serial.println("\n12");
#endif
    stcG_FlagFact.flagTxRespuesta = 0;
    sTxHost();
    /////Prueba
  }
  //Se Revisa cada Hora si han pasado las 24H desde que se recibio la primera Factura
  //  del Reporte X
  if (!stcG_FlagFact.flagDocProc) {
    if (millis() - ui32G_Time1H > Constante1Hora) {
      ui32G_Time1H = millis();
      Serial.println("Paso 1H. Se revisa Tiempo de Reporte Z...");
      RtcDateTime timeRTC;
      timeRTC = rtc1302.GetDateTime();
      uint32_t timeNowUnix = myFunciones.timeToUnix(timeRTC);
      char dirFileRepX[MaxCarNameFile];  //Nombre Archivo para Almacenar el Reporte X
      if (bfLeerReporteX() == SUCCESS) {
        if (stcG_XyZ.stContadores.ui32FechaIniRep != 0 && stcG_XyZ.stContadores.ui32UltFact != 0) {
          uint32_t auxTime = timeNowUnix - stcG_XyZ.stContadores.ui32FechaIniRep;
          if (auxTime > Segundos24H) {
            Serial.println("Inicializando Reportes de Z");
            //generaReporteZ();
            outputGenImpRepZ();
          }
        }
      }
    }
  }
}

//*****************************************************************************
// Funcion sTxHost Tx Respuesta al Host con la Estrcutura JSON
//    { "cmd":"cmdRx"
//      "code": (Codigo)
//    }
//    Los Codigos  Son:
//      errorNo 0x00          **No Hay Errores
//      errorFrame 0x01 (01)      **Error en la Trama, JSON Incompleto
//      errorData 0x02  (02)     **Error en el JSON, un dato Incompleto o no adecuado (int mal Rx)
//      errorBlank 0x03 (03)      **Error en el JSON, JSON vacio
//      errorCmdUnk 0x05 (05)     **Error Comando que se Rx no es Reconocido
//      errorUnk 0x06    (06)     **Error Desconocido
//      errorTimeO 0x07  (07)     **Error por Time Out, sucede cuando el JSON Rx Muy extenso > timeOutUAR
//Codigos de Errores de Respuesta al Host (Errores de Logica)
//      errorCmNa 0x10   (16)     **Error Logica, no se Rx en el JSON "cmd"
//      errorCmIn 0x11   (17)     **Error Logica, Comando Rx esta fuuera de Secuancia
//      errorTotal 0x12   (18)    **Error Logica, Comando Rx incompleto
//      errorDatBlank 0x13 (19)   **Error se Recibio la Palabra TOTAL
//      errorDatBlank 0x14 (20)   **Error se Rx datos Vacios o np se Rx datod Validos
//*****************************************************************************
void sTxHost() {
  String output;
  StaticJsonDocument<2048> docSer;
  //docSer["cmd"] = chG_CmdRx;
  docSer["cmd"] = stcG_CmdTxJson.chrG_CmdTx;
  //docSer["code"] = byG_CodeTx;
  docSer["code"] = stcG_CmdTxJson.ui8CodeTx;
  if (strlen(stcG_CmdTxJson.chrG_DataTx) > 0) {
    //JsonObject data = doc.createNestedObject("data");
    //docSer["data"] = stcG_CmdTxJson.chrG_DataTx;
    if (stcG_CmdTxJson.chrG_CmdTx == CMD_GetTime) {
      docSer["data"] = stcG_CmdTxJson.chrG_DataTx;
    } else {
      docSer["data"] = serialized(stcG_CmdTxJson.chrG_DataTx);
    }

  } else if (stcG_CmdTxJson.ui32G_DataTx > 0) {
    //JsonObject data = doc.createNestedObject("data");
    docSer["data"] = stcG_CmdTxJson.ui32G_DataTx;
  }
  serializeJson(docSer, output);
  Serial.print(output);
  ///

  /////DEBUG
  if (strcmp(stcG_CmdTxJson.chrG_CmdTx, CMD_EndFactura) == 0) {
    stcG_Index.ui32NumCon = 24;
    stcG_Index.ui32TimeStamp = 0;
    strncpy(stcG_Index.pszNameIndex, fileIndiceNroFac, 64);
    if (stcG_Index.bFindIndex(SD_MMC)) {
      Serial.println("\n🎈Primera Busqueda...");
      Serial.printf("Numero de Factura: %d\n",stcG_Index.ui32NumCon);
      Serial.printf("Timestamp: %d\n",stcG_Index.ui32TimeStamp);
      Serial.printf("Tamaño del Registro: %d\n",stcG_Index.ui32SizeFile);
      Serial.printf("Posicion de Inicio del Registro: %d\n",stcG_Index.ui32PosEnArch);
      File archivo;
      char readFile[1500];
      memset(readFile, 0, 1500);
      // // memset(readFile,0,sizeof(readFile));
      // archivo = SD_MMC.open("/Report/ReporteX.txt", FILE_READ);
      ///Seniat/DF_20230913.txt
      archivo = SD_MMC.open("/Seniat/DF_20230913.txt");
      if (archivo) {
        Serial.println("Leyendo Archivo de la Busqueda Encontrada...");
        archivo.seek(stcG_Index.ui32PosEnArch);
        for (int j = 0; j < stcG_Index.ui32SizeFile; j++) {
          readFile[j] = archivo.read();
        }
      }else{
        Serial.println("No se pudo abrir el archivo");
      }
      archivo.close();
      Serial.printf("El tamano de la variable leida es: %d\n", strlen(readFile));
      Serial.println("El Registro de Factura Encontrado es:");
      Serial.print(readFile);
    } else {
      Serial.println("No Hay Coincidencias");
    }
  }

  if (strcmp(stcG_CmdTxJson.chrG_CmdTx, CMD_setTimer) == 0 || strcmp(stcG_CmdTxJson.chrG_CmdTx, CMD_GetTime) == 0 || strcmp(stcG_CmdTxJson.chrG_CmdTx, CMD_EndFactura) == 0 || strcmp(stcG_CmdTxJson.chrG_CmdTx, CMD_WriteFileSPIFF) == 0 || strcmp(stcG_CmdTxJson.chrG_CmdTx, CMD_ReadFileSPIFF) == 0 || strcmp(stcG_CmdTxJson.chrG_CmdTx, CMD_WriteFileeMMC) == 0 || strcmp(stcG_CmdTxJson.chrG_CmdTx, CMD_ReadFileeMMC) == 0 || strcmp(stcG_CmdTxJson.chrG_CmdTx, CMD_ImpRepX) == 0 || strcmp(stcG_CmdTxJson.chrG_CmdTx, CMD_GenImpRepZ) == 0 || strcmp(stcG_CmdTxJson.chrG_CmdTx, CMD_InicializaPtr) == 0) {
    Serial.println("\nSe debe ir al Estado Inicio");
    changeState(State::stateInicial);
  }
  //memset(&stcG_CmdTxJson, 0 , sizeof(stcG_CmdTxJson));
  return;
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: bool boInicializarHardware(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que Inicializa y verifica si el Hardware de la Impresora Funciona
              Correctamente
RETORNO		: uint8_t
            0x00 -> No hay Errores
            0x01 -> No existe MT eMMC
            0x02 -> No existe MF FLASH
            0x03 -> Error por RTC
            0x10 -> Error en la Configuracion desde la MT archivo infoPtr.txt
*****************************************************************************************/
uint8_t boInicializarHardware(void) {
  //Inicializa UART
  Serial.begin(115200);
#ifdef useSerialOutPtr
  SerialOutPtr.begin(115200, SERIAL_8N1, ptrRx, ptrTx);
#endif
  uint8_t verHw = verificarPSRAM();
  if (verHw) {
    stcG_FlagErr.fErrorPSRAM = 1;
    return (verHw);
  }
  verHw = verifica_eMMC();
  if (verHw) {
    stcG_FlagErr.fErroreMMC = 1;
    return (verHw);
  }
  verHw = verifica_SPIFFS();
  if (verHw) {
    stcG_FlagErr.fErrorSPIFFS = 1;
    return (verHw);
  }
  verHw = verificar_RTC();
  if (verHw) {
    stcG_FlagErr.fErrorRTC = 1;
    return (verHw);
  }
  verHw = verificar_FLASH();
  if (verHw) {
    stcG_FlagErr.fErrorFlash = 1;
    return (verHw);
  }

  return (SUCCESS);
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: uint8_t verificarPSRAM(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que inicializa La memoria externa PSRAM
RETORNO		: uint8_t
*****************************************************************************************/
uint8_t verificarPSRAM(void) {
  Serial.print("Espacio Libre de la PSRAM: ");
  Serial.println(ESP.getFreePsram());
  if (boInicializaPSRAM()) {
    Serial.println("\nError Inicializando Datos en la PSRAM");
    return (ERROR_HW);
  }
  Serial.println("\Inicializando Datos en la PSRAM Inicializada con Exito");
  Serial.print("Tamano de la PSRAM: ");
  Serial.println(ESP.getPsramSize());
  Serial.print("Espacio Libre de la PSRAM: ");
  Serial.println(ESP.getFreePsram());
  Serial.print("Espacio Usado de la PSRAM: ");
  Serial.println(ESP.getPsramSize() - ESP.getFreePsram());
  return (SUCCESS);
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: void voInicializaPSRAM(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que inicializa La memoria externa PSRAM
RETORNO		: bool
*****************************************************************************************/
bool boInicializaPSRAM(void) {
  if (!psramInit()) {
    //Serial.println("PSRAM not available");
    return (ERROR);
  }
  int available_PSRAM_size = ESP.getFreePsram();
  //Se Determina la cantidad Maxima de RAM que se reservara para los Datos de la Estructura Productos en la PSRAM
  //    Se multiplica la Cantidad Maxima de Productos por el tamano de la Estructura Producto
  //inCantMaxPsramProd = CantMaxProd * (sizeof(uint8_t) + sizeof(uint16_t) + (sizeof(uint64_t) * 9) + (sizeof(char) * CantMaxCarArt * 3));
  inCantMaxPsramProd = sizeof(Productos);
  Serial.printf("Tamano de la Estructura Productos: %d\n", inCantMaxPsramProd);
  Serial.printf("Tamano de la Estructura stcG_Productos: %d\n", inCantMaxPsramProd * CantMaxProd);
  if (available_PSRAM_size < inCantMaxPsramProd * 2) {
    return (ERROR);  //No existe la cantidad de RAM en PSRAM para los Productos
  }
  stcG_Productos = (Productos*)ps_malloc(inCantMaxPsramProd * CantMaxProd);
  chG_ImpPtr = (char*)ps_malloc(inCantMaxPsramProd * CantMaxProd);
  //chG_ImpPtr = (char *)ps_malloc(ESP.getFreePsram());
  //Serial.println((String) "PSRAM Size available (bytes): " + available_PSRAM_size);
  return (SUCCESS);
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: uint8_t verifica_eMMC(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que Verifica se encuentre la Memoria de Trabajo eMMC y que el archivo
              de configuracion contenga los datos de configuracion
RETORNO		: uint8_t
*****************************************************************************************/
uint8_t verifica_eMMC(void) {
  if (mySD_eMMC.boInicializa_eMMC()) {
    Serial.print("eMMC No Esta Iniciaizada Correctamente");
    return (ERROR_eMMC);

  } else {
    strncpy(chG_Util, DirConfig, CantMaxDirFile);
    strncat(chG_Util, NameFileConfig, CantMaxDirFile);
    Serial.println("eMMC Iniciaizada Correctamente");
    if (mySD_eMMC.boExistFile(SD_MMC, chG_Util)) {
      Serial.println("No existe Archivo de Configuracion");
      return (ERROR_CONF_MT);
    } else {
      Serial.println("Existe Archivo de Configuracion");
      Serial.print("Nombre del Archivo: ");
      Serial.println(chG_Util);
      if (initParametrosPtr(chG_Util)) {
        Serial.println("Error Iniciando Configuracion del Sistema en archivo infoPtr.txt");
        return (ERROR_CONF_MT);
      } else {
        Serial.println("Configuracion del Sistema realizado con Exito del archivo infoPtr.txt");
      }
    }
  }
  return (SUCCESS);
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: uint8_t verifica_SPIFFS(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que Verifica se encuentre la Memoria de archivos interna SPIFFs
              en esta contiene de configuracion  los datos de configuracion
RETORNO		: uint8_t
*****************************************************************************************/
uint8_t verifica_SPIFFS(void) {

  Serial.println("Iniciando Rutina verifica_SPIFFS");
  if (!SPIFFS.begin(true)) {
    Serial.println("Error Inicializando la SPIFFS");
    return (ERROR_SPIFF);
  }
  //size_t Info SPIFFS.totalBytes();
  strncpy(chG_Util, NameFileConfigSPIFFS, CantMaxDirFile);
  File file = SPIFFS.open(chG_Util);
  if (!file) {
    Serial.printf("No se puede Abrir el Archivo de Configuracion de SPIFFS: %s", chG_Util);
    return (ERROR_SPIFF);
  }
  if (initParametrosPtrSPIFFS(chG_Util)) {
    Serial.printf("Error en el Archivo de Configuracion de SPIFFS: %s", chG_Util);
    return (ERROR_SPIFF);
  }
  strncpy(chG_Util, NameFileContDocSPIFFS, CantMaxDirFile);
  file = SPIFFS.open(chG_Util);
  if (!file) {
    Serial.printf("No se puede Abrir el Archivo de Contadores de SPIFFS: %s", chG_Util);
    return (ERROR_SPIFF);
  }
  if (readContadoresPtrSPIFFS(chG_Util)) {
    Serial.printf("Error en el Archivo de Configuracion de SPIFFS: %s", chG_Util);
    return (ERROR_SPIFF);
  }
  strncpy(chG_Util, NameFileParaFacSPIFF, CantMaxDirFile);
  file = SPIFFS.open(chG_Util);
  if (file) {
    //Serial.printf("Leyendo Parametros de Factura de SPIFFS: %s", chG_Util);
    readParametrosFacturaSPIFF(chG_Util);
  }

  Serial.println("Finalizando Rutina verifica_SPIFFS");
  return (SUCCESS);
}

/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: uint8_t verificar_FLASH(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que Verifica se encuentre la Memoria FLASH
              en esta contiene de Datos de Verificacion
RETORNO		: uint8_t
*****************************************************************************************/
uint8_t verificar_FLASH(void) {
  uint8_t id[5];
  spiFlash.begin(SCKPIN, MISOPIN, MOSIPIN, CSPIN);  // Inicializa Bus SPI
  if (!SerialFlash.begin(spiFlash, CSPIN)) {
    Serial.println(F("\n❌ERROR to access SPI Flash chip"));
    return (ERROR_FLASH);
  } else {
    Serial.println(F("✅Se pudo access SPI Flash chip"));
    uint8_t id[5];
    SerialFlash.readID(id);
    Serial.printf("ID: %02X %02X %02X\n", id[0], id[1], id[2]);
    //SerialFlash.blockSize();
  }
  Serial.print(F("  Memory Size:  "));
  int chipsize = SerialFlash.capacity(id);
  Serial.print(chipsize);
  return (SUCCESS);
}



/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: uint8_t verifica_RTC(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que Verifica el funcionamiento del RTC
              de configuracion contenga los datos de configuracion
RETORNO		: uint8_t
*****************************************************************************************/
uint8_t initParametrosPtrSPIFFS(char* fileC) {
  Serial.println("Iniciando Rutina initParametrosPtrSPIFFS");
  //Inicio de Prueba
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("No existe Directorio");
    //return;
  }
  //listDir(SPIFFS, "/", 0);
  //Fin de Prueba




  String fileConfig = "";
  File file = SPIFFS.open(fileC);
  while (file.available()) {
    fileConfig += char(file.read());
  }
  file.close();
  Serial.println("Archivo Leido desde SPIFFS: ");
  Serial.println(fileConfig);
  doc.clear();
  DeserializationError error = deserializeJson(doc, fileConfig);
  if (error) {
    Serial.print("Error convirtiendo JSON desde: ");
    Serial.println(error.c_str());
    return (ERROR_SPIFF);
  }
  if (doc["simMonL"]) {
    const char* simboloMonL = doc["simMonL"];
    strncpy(chG_SimMoneda, simboloMonL, CantMaxCarConf);
  } else {
    memset(chG_SimMoneda, 0, CantMaxCarConf);
    return (ERROR_SPIFF);
  }
  if (doc["impArt"]) {
    JsonObject impArt = doc["impArt"];
    if (impArt["desc"] && impArt["abrev"] && impArt["valor"] && impArt["impMontoPtr"] && impArt["impMontoImp"]) {
      JsonArray impArt_desc = impArt["desc"];
      JsonArray impArt_abrev = impArt["abrev"];
      JsonArray impArt_valor = impArt["valor"];
      JsonArray impArt_impMontoPtr = impArt["impMontoPtr"];
      JsonArray impArt_impMontoImp = impArt["impMontoImp"];
      int sizeDesc, sizeAdve, sizeValor, sizeImpF1, sizeImpF2;
      sizeDesc = impArt["desc"].size();
      sizeAdve = impArt["abrev"].size();
      sizeValor = impArt["valor"].size();
      sizeImpF1 = impArt["impMontoPtr"].size();
      sizeImpF2 = impArt["impMontoImp"].size();
      ui8G_CountImpArt = sizeDesc;
      if (ui8G_CountImpArt > 0) {
        for (int i = 0; i < ui8G_CountImpArt; i++) {
          memset(stcG_ImpuestoArticulos[i].chG_descripcion, 0, CantMaxCarConf);
          memset(stcG_ImpuestoArticulos[i].chG_Abreviatura, 0, CantMaxCarConf);
          memset(stcG_ImpuestoArticulos[i].impMontoPtr, 0, CantMaxCarConf);
          memset(stcG_ImpuestoArticulos[i].impMontoImp, 0, CantMaxCarConf);
          strncpy(stcG_ImpuestoArticulos[i].chG_descripcion, impArt_desc[i], CantMaxCarConf);
          strncpy(stcG_ImpuestoArticulos[i].chG_Abreviatura, impArt_abrev[i], CantMaxCarConf);
          strncpy(stcG_ImpuestoArticulos[i].impMontoPtr, impArt_impMontoPtr[i], CantMaxCarConf);
          strncpy(stcG_ImpuestoArticulos[i].impMontoImp, impArt_impMontoImp[i], CantMaxCarConf);
          stcG_ImpuestoArticulos[i].valorImp = impArt_valor[i];
        }
      } else {
        for (int i = 0; i < CantMaxImpArt; i++) {
          memset(stcG_ImpuestoArticulos[i].chG_descripcion, 0, CantMaxCarConf);
          memset(stcG_ImpuestoArticulos[i].chG_Abreviatura, 0, CantMaxCarConf);
          memset(stcG_ImpuestoArticulos[i].impMontoPtr, 0, CantMaxCarConf);
          memset(stcG_ImpuestoArticulos[i].impMontoImp, 0, CantMaxCarConf);
          stcG_ImpuestoArticulos[i].valorImp = 0;
        }
        return (ERROR_SPIFF);
      }
    }
  } else {
    return (ERROR_CONF_MT);
  }
  if (doc["formPago"]) {
    JsonObject formPago = doc["formPago"];
    if (formPago["tituloFormPag"]) {
      const char* tituloFormaPago = formPago["tituloFormPag"];
      strncpy(chG_TituloFormPago, tituloFormaPago, CantMaxCarLin);
    }
    if (formPago["desc"] && formPago["impG"] && formPago["impMontoPtr"] && formPago["impMontoImp"]) {
      JsonArray formPago_desc = formPago["desc"];
      JsonArray formPago_impG = formPago["impG"];
      JsonArray formPago_impMontoPtr = formPago["impMontoPtr"];
      JsonArray formPago_impMontoImp = formPago["impMontoImp"];
      int sizeDesc, sizeImpG, sizeMontoPtr, sizeMontoImp;
      sizeDesc = formPago_desc.size();
      sizeImpG = formPago_impG.size();
      sizeMontoPtr = formPago_impMontoPtr.size();
      sizeMontoImp = formPago_impMontoImp.size();
      ui8G_CountFormPago = sizeDesc;
      for (int i = 0; i < ui8G_CountFormPago; i++) {
        memset(stcG_FormasPagoImp[i].chG_descripcion, 0, CantMaxCarConf);
        memset(stcG_FormasPagoImp[i].chG_impBI_Monto, 0, CantMaxCarConf);
        memset(stcG_FormasPagoImp[i].chG_impIVA_Porc, 0, CantMaxCarConf);
        stcG_FormasPagoImp[i].ui16G_ivaPorc = 0;
        strncpy(stcG_FormasPagoImp[i].chG_descripcion, formPago_desc[i], CantMaxCarConf);
        strncpy(stcG_FormasPagoImp[i].chG_impBI_Monto, formPago_impMontoPtr[i], CantMaxCarConf);
        strncpy(stcG_FormasPagoImp[i].chG_impIVA_Porc, formPago_impMontoImp[i], CantMaxCarConf);
        stcG_FormasPagoImp[i].ui16G_ivaPorc = formPago_impG[i];
      }
    }
  } else {
    return (ERROR_SPIFF);
  }
  Serial.println("Finalizando Rutina initParametrosPtrSPIFFS");
  return (SUCCESS);
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: uint8_t readContadoresPtrSPIFFS(char* fileC)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que lee los contadores de Reportes x,z, fact, ND y NC
              desde la memoria SPIFFS
RETORNO		: uint8_t
*****************************************************************************************/
uint8_t readContadoresPtrSPIFFS(char* fileC) {
  String fileConfig = "";
  File file = SPIFFS.open(fileC);
  while (file.available()) {
    fileConfig += char(file.read());
  }
  file.close();
  Serial.println("Archivo Leido desde SPIFFS: ");
  Serial.println(fileConfig);
  doc.clear();
  DeserializationError error = deserializeJson(doc, fileConfig);
  if (error) {
    Serial.print("Error convirtiendo JSON desde: ");
    Serial.println(error.c_str());
    return (ERROR_SPIFF);
  }
  if (doc["ContadorFactDia"]) {
    stcG_Contadores.ui32CountFacDia = doc["ContadorFactDia"];
  }
  if (doc["ultZ"]) {
    stcG_Contadores.ui32UltRepZ = doc["ultZ"];
  }
  if (doc["fechaUZ"]) {
    stcG_Contadores.ui32FechaRepZ = doc["fechaUZ"];
  }
  if (doc["ultFac"]) {
    stcG_Contadores.ui32UltFact = doc["ultFac"];
  }
  if (doc["fechaUF"]) {
    stcG_Contadores.ui32FechaUF = doc["fechaUF"];
  }
  if (doc["ultND"]) {
    stcG_Contadores.ui32UltNDeb = doc["ultND"];
  }
  if (doc["ultNC"]) {
    stcG_Contadores.ui32UltNCre = doc["ultNC"];
  }
  if (doc["ultDNF"]) {
    stcG_Contadores.ui32UltDNF = doc["ultDNF"];
  }
  if (doc["fechaUDNF"]) {
    stcG_Contadores.ui32FechaDNF = doc["fechaUDNF"];
  }
  if (doc["ultRMF"]) {
    stcG_Contadores.ui32UltRMF = doc["ultRMF"];
  }
  if (doc["fechaURMF"]) {
    stcG_Contadores.ui32FechaUltRMF = doc["fechaURMF"];
  }
  if (doc["fechaIniRep"]) {
    stcG_Contadores.ui32FechaIniRep = doc["fechaIniRep"];
  }
  Serial.println("Finalizando Rutina initContadoresPtrSPIFFS");
  return (SUCCESS);
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: void readParametrosFacturaSPIFF(char* fileC)
PARAMETROS	: nombre Archivo
DESCRIPCION	: Funcion que lee los Parametros de la Factura, Encabezado y Pie de Factuar
              desde la memoria SPIFFS
RETORNO		: uint8_t
*****************************************************************************************/
void readParametrosFacturaSPIFF(char* fileC) {
  String fileConfig = "";
  File file = SPIFFS.open(fileC);
  while (file.available()) {
    fileConfig += char(file.read());
  }
  file.close();
  // Serial.println("Archivo Leido desde SPIFFS: ");
  // Serial.println(fileConfig);
  doc.clear();
  memset(&stcG_Encabezado, 0, sizeof(stcG_Encabezado));
  DeserializationError error = deserializeJson(doc, fileConfig);
  if (error) {
    Serial.print("Error convirtiendo JSON desde: ");
    Serial.println(error.c_str());
    return;
  }
  if (doc["encFacFijo"]) {
    JsonArray encFacFijo = doc["encFacFijo"];
    byG_NroEncab = doc["encFacFijo"].size();
    if (byG_NroEncab > CantMaxEncab) {
      byG_NroEncab = CantMaxEncab;
    }
    for (int i = 0; i < byG_NroEncab; i++) {
      const char* dataRx = encFacFijo[i];
      strncpy(stcG_Encabezado[i].Encabezado, dataRx, CantMaxCarLin - 1);
      //Serial.printf("⏭El Encabezado Fijo %d, es %s\n", i, stcG_Encabezado[i].Encabezado);
    }
  }
  if (doc["pieFacFijo"]) {
    JsonArray pieFacFijo = doc["pieFacFijo"];
    byG_NroLinAdPieTick = doc["pieFacFijo"].size();
    if (byG_NroLinAdPieTick > CantMaxPieTick)
      byG_NroLinAdPieTick = CantMaxPieTick;
    for (int i = 0; i < byG_NroLinAdPieTick; i++) {
      const char* dataRx = pieFacFijo[i];
      strncpy(stcG_PieTicket[i].pieTicket, dataRx, CantMaxCarLin - 1);
      //Serial.printf("⏭El Pie de Ticket Fijo %d, es %s\n", i, stcG_PieTicket[i].pieTicket);
    }
  }
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: bool initParametrosPtr(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que inicializa la Configuracion de Parametros para la Impresora Fiscal
              Dicha configuracion esta en la memoria de Trabajo (eMMC), en el archivo
              /Configuration/infoPtr.txt
              Se Configura los siguientes Parametros:
              General
              ✅ Serial de la Impresora
              ✅ Simbolo de la Moneda Local
              Impuesto de Articulos
              ✅ Impuestos que poseen los Articulos, ej. Exonerado, IVA, Reducido, Lujo, Percibido
              ✅ Abreviatura dsimbolica de los impuestos antes descritos, ej, (E), (G), (R), (L), (P)
              ✅ Valor del % para el calcualo ej, 16.00 %
              ✅ Cadena que se Imprime en la Factura especificando el Monto, ej. "BI G (16.00%)"
              ✅ Cadena que se Imprime en la Factura especificando el % Calculado, ej. "IVA G (16.00%)"
              Sobre las Formas de Pago
              ✅ Descripcion del tipo de Pago, ej. "EFECTIVO", "T. DEBITO", "T. CREDITO",
              ✅ Cadena que se Imprime en la Factura describiendo el Impuesto del Monto, ej. ""BI IGTF (3.00%)"
              ✅ Cadena que se Imprime en la Factura especificando el % Calculado, ej. "IGTF (3.00%)",
              ✅ Impuesto que graba la Forma de Pago, ej. 3.00 % 
RETORNO		: bool
*****************************************************************************************/
bool initParametrosPtr(char* fileC) {
  //StaticJsonDocument<1536> doc;
  Serial.println("Inicializa Rutina initParametrosPtr");
  String fileConfig = mySD_eMMC.strReadFile(SD_MMC, fileC);
  Serial.println("Archivo Leido:");
  Serial.println(fileConfig);
  if (fileConfig.length() == 0) {
    return (ERROR_CONF_MT);
  }
  Serial.println("Inicializa DeserializationError initParametrosPtr");
  DeserializationError error = deserializeJson(doc, fileConfig);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return (ERROR_CONF_MT);
  }
  Serial.println("Verificando serialPtr");
  if (doc["serialPtr"]) {
    const char* chL_serialPtr = doc["serialPtr"];
    //memset(cG_SerialImp, 0, sizeLineConf);
    strncpy(chG_SerialPtr, chL_serialPtr, CantMaxCarConf);
  } else {
    memset(chG_SerialPtr, 0, CantMaxCarConf);
    return (ERROR_CONF_MT);
  }
  Serial.println("Verificando tituloSeniat");
  if (doc["tituloSeniat"]) {
    const char* tituloSeniat = doc["tituloSeniat"];
    strncpy(chG_tituloFactura, tituloSeniat, CantMaxCarLin);
  } else {
    memset(chG_tituloFactura, 0, CantMaxCarConf);
    return (ERROR_CONF_MT);
  }
  Serial.println("Verificando rifEmp");
  if (doc["rifEmp"]) {
    const char* rifE = doc["rifEmp"];
    strncpy(chG_rifEmpresa, rifE, CantMaxCarLin);
  } else {
    memset(chG_rifEmpresa, 0, CantMaxCarConf);
    return (ERROR_CONF_MT);
  }
  Serial.println("Finalizada Rutina initParametrosPtr");
  return (SUCCESS);
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: uint8_t verificarPSRAM(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que inicializa La memoria externa PSRAM
RETORNO		: uint8_t
*****************************************************************************************/
uint8_t verificar_RTC(void) {
  rtc1302.Begin();
  RtcDateTime compiled;
  Serial.println();
  if (!rtc1302.IsDateTimeValid()) {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing
    // Serial.println("RTC lost confidence in the DateTime!");
    rtc1302.SetDateTime(compiled);
    return (ERROR_RTC);
  }
  if (rtc1302.GetIsWriteProtected()) {
    Serial.println("RTC was write protected, enabling writing now");
    rtc1302.SetIsWriteProtected(false);
    return (ERROR_RTC);
  }

  if (!rtc1302.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    rtc1302.SetIsRunning(true);
    return (ERROR_RTC);
  }
  RtcDateTime now = rtc1302.GetDateTime();
  if (now < compiled) {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    rtc1302.SetDateTime(compiled);
    return (ERROR_RTC);
  } else if (now > compiled) {
    Serial.println("RTC is newer than compile time. (this is expected)");
  } else if (now == compiled) {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }
  return (SUCCESS);
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: void fInicializaFactura(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que inicializa las variables que se utilizan para la Factura
RETORNO		: void
*****************************************************************************************/
void fInicializaFactura(void) {
  //Se Inicializa los Campos de Encabezado
  int i;
  //ui32G_CountFact++;  //Se incrementa el # de Factura
  // stcG_XyZ.stContadores.ui32CountFacDia++;
  // stcG_XyZ.stContadores.ui32UltFact++;
  // for (i = 0; i < CantMaxEncab; i++) {
  //   stcG_Encabezado[i].Encabezado[0] = '\0';
  // }
  memset(&stcG_FlagFact, 0, sizeof(stcG_FlagFact));
  strncpy(chG_Util, NameFileParaFacSPIFF, CantMaxDirFile);
  File file;
  file = SPIFFS.open(chG_Util);
  if (file) {
    Serial.printf("Leyendo Parametros de Factura de SPIFFS: %s", chG_Util);
    readParametrosFacturaSPIFF(chG_Util);
  }
  stcG_FlagFact.flagIindEncab = 0;
  //Se Inicializa los Campos de Cliente
  stcG_Cliente.RazSoc[0] = '\0';
  stcG_Cliente.RifCi[0] = '\0';
  for (i = 0; i < CantMaxLinAd; i++) {
    stcG_Cliente.LineaAd[0][i] = '\0';
  }
  memset(&stcG_CmdTxJson.chrG_DataTx, 0, sizeof(stcG_CmdTxJson));
  //Se inicializa Contador de Productos
  ui16G_NroProducto = 0;
  //Se inicializa Numero Lineas Adicionales
  byG_NroLinAdCli = 0;
  //Se inicializan los Impuestos de la Factura

  // for (i = 0; i < CantMaxImpArt; i++) {
  //   stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto = 0;
  //   stcG_TotalImpuestoArtiulos[i].ui64G_IVA_Monto = 0;
  // }
  memset(&stcG_TotalImpuestoArtiulos, 0, sizeof(stcG_TotalImpuestoArtiulos));
  /*
  for (i = 0; i < CantMaxImpPago; i++) {
    stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo = 0;
    stcG_TotalImpuestoFormPago[i].ui64G_MontoImpEx = 0;
    stcG_TotalImpuestoFormPago[i].ui64G_IVA_Monto = 0;
    stcG_TotalImpuestoFormPago[i].ui32G_TasaCambio = 0;
  }
  */
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    stcG_TotalImpuestoArtiulos[i].ui64G_IVA_Monto = 0;
    stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto = 0;
    strncpy(stcG_TotalImpuestoArtiulos[i].impMontoPtr, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarConf);
    strncpy(stcG_TotalImpuestoArtiulos[i].impMontoImp, stcG_ImpuestoArticulos[i].impMontoImp, CantMaxCarConf);
  }
  memset(&stcG_TotalImpuestoFormPago, 0, sizeof(stcG_TotalImpuestoFormPago));
  //
  //Se Inicializa los Campos de Lineas Adicionales Post Productos
  //
  for (i = 0; i < CantMaxLinAdPF; i++) {
    stcG_LineaAdPostF[i].lineaAd[0] = '\0';
  }
  //Se Inicializa los Campos de Lineas Pie de Factura o Ticket
  //
  // for (i = 0; i < CantMaxPieTick; i++) {
  //   stcG_PieTicket[i].pieTicket[0] = '\0';
  // }
  memset(&stcG_FlagErr, 0, sizeof(stcG_FlagErr));
  // stcG_FlagErr.flag = 0;
  // stcG_FlagFact.flagFact = 0;
  //Inicialicza Contador de Formas de Pagos Rx
  ui8G_CountFormPagoRx = 0;
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: void inicializaAcumuladores(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que inicializa los Acumuladores para los Reportes Z y X
RETORNO		: void
//***************************************************************************************/
void inicializaAcumuladores(void) {
  //Se inicializan los Acumuladores de Impuestos de Articulos
  for (int i = 0; i < CantMaxImpArt; i++) {
    stcG_TotalAcumuladoBI.ui64G_BI_Ventas[i] = 0;
    stcG_TotalAcumuladoBI.ui64G_BI_Descuento[i] = 0;
    stcG_TotalAcumuladoBI.ui64G_BI_Recargo[i] = 0;
    stcG_TotalAcumuladoBI.ui64G_BI_Correcciones[i] = 0;
    stcG_TotalAcumuladoBI.ui64G_BI_Anulaciones[i] = 0;
    stcG_TotalAcumuladoBI.ui64G_BI_NotaDebito[i] = 0;
    stcG_TotalAcumuladoBI.ui64G_BI_NotaCredito[i] = 0;
  }
  //Se inicializan los Acumuladores por Formas de Pagos
  for (int i = 0; i < CantMaxImpPago; i++) {
    stcG_TotalAcumuladoFP.ui64G_FP_Ventas[i] = 0;
    stcG_TotalAcumuladoFP.ui64G_FP_NotaDebito[i] = 0;
    stcG_TotalAcumuladoFP.ui64G_FP_NotaCredito[i] = 0;
  }
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: void fInicializaFactura(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion que inicializa las variables que se utilizan para la Factura
RETORNO		: void
//***************************************************************************************/
bool bReadUART(void) {
#ifdef timeDebug
  long iniTime, finTime;
  iniTime = millis();
#endif
  long timeOutIni;
  uint16_t ui16L_Cont;                    //Contador de {} del Json Rx
  uint16_t ui16L_Index;                   //Contador de Caracteres Rx
  char chL_CarRx;                         //Se Almacena el ultimo Caracter Rx
  char chL_JsonRx[CantMaxCaracRx];        //Se Almacena El JSON Rx
  memset(chL_JsonRx, 0, CantMaxCaracRx);  //Se inicializa el Arreglo de Rx
  ui16L_Index = 0;                        //Se inicializa el Contador de Carac Rx
  ui16L_Cont = 0;                         //Se inicializa el Contador de {}
  stcG_FlagErr.fErrorTimeO = 0;           //Bandera que Indica si hay Timer OUT
  if (Serial.available()) {
    do {                         //Ciclo para Rx la Trama JSON
      if (Serial.available()) {  //    Se Inicia con el caracter '{'
        chL_CarRx = Serial.read();
        if (ui16L_Index == 0) {  //Si es el Primer Caracter se inicializ el contador Time Out
          timeOutIni = millis();
        }
        if (chL_CarRx == '{') {  //El contador sCont se Incrementa cada vez que se Rx '{'
          ui16L_Cont++;
#ifdef useDebug
          Serial.println("\n1");
#endif
        }
        if (ui16L_Cont > 0) {
          chL_JsonRx[ui16L_Index++] = chL_CarRx;
          if (chL_CarRx == '}') {  //Si se Rx '}' se decrementa sCont
            ui16L_Cont--;          //Al final del JSON sCont=0
#ifdef useDebug
            Serial.println("\n2");
#endif
          }
        }  //Si se Empezo a Rx el Json se Almacena en el Arreglo
      }
      if (millis() - timeOutIni > timeOutUART) {  //Se verifica si hay Time OUT
        stcG_FlagErr.fErrorTimeO = 1;
#ifdef useDebug
        Serial.println("\n3");
#endif
        break;
      }

    } while (ui16L_Cont > 0);  //El ciclo se Repite mientras no se a finalizado de Rx el JSON
#ifdef timeDebug
    finTime = millis();
    Serial.print("Time Rx: ");
    Serial.print(finTime - iniTime);
    Serial.println(" mS");
#endif
    if (stcG_FlagErr.fErrorTimeO) {  //Se verifica si se salio de ciclo por Time Out
      //strncpy(chG_CmdRx, CMD_Desconocido, CaracMaxCmdTx);
      strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_Desconocido, CaracMaxCmdTx);
      //byG_CodeTx = ErrorTimeO;
      stcG_CmdTxJson.ui8CodeTx = ErrorTimeO;
      stcG_FlagFact.flagTxRespuesta = 1;
      return ErrorYes;
    }
    //Se verifica si no se empezo a Rx la Trama Json
    if (ui16L_Index == 0) {
      stcG_FlagFact.flagTxRespuesta = 0;  //Como no se Rx Datos no se Tx Respuesta la Host
#ifdef useDebug
      Serial.println("\n0");
#endif
      return ErrorYes;
    }
    //Si se termino de Rx la Trama JSON se almacena en la variable strJsonRx y se retorna sin errores
    strG_JsonRx = String(chL_JsonRx);
    return ErrorNoF;
  } else {                              //Si no se a Rx ni un dato se ejecuta este ELSE
    stcG_FlagFact.flagTxRespuesta = 0;  //Como no se Rx Datos no se Tx Respuesta la Host
    return ErrorYes;                    //retorna Error, No se Rx datos
  }
}
/****************************************************************************************
AUTOR		: Antonio Pateti
FECHA		: 2007/08/202318
MODIFICADO	: **
ACTUALIZACION	: **.
NOMBRE		: bool bValidoJson(void)
PARAMETROS	: NINGUNO
DESCRIPCION	: Funcion bValidoJson Verifica si El JSON Rx es Valido
RETORNO		: Si JSON Rx  Valido retorna TRUE
//          Si JSON Rx  No es Valido retorna FALSE + flagTxRespuesta -> 1
//            en byG_CodeTx se almacena el Codigo a Tx al Host
//***************************************************************************************/
bool bValidoJson(void) {
#ifdef useDebug
  Serial.println("\n4");
#endif
  String strL_SinEspacio;
  if (strG_JsonRx.length() < 10) {  //Se verifica si la cadena JSON tiene un minimo de 10 caracteres
    //strncpy(chG_CmdRx, CMD_Desconocido, CaracMaxCmdTx);  //De no ser Asi se considera que no es un comando valido
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_Desconocido, CaracMaxCmdTx);
    //byG_CodeTx = ErrorFrame;                             //Se inicializa los comandos a Tx al HOST
    stcG_CmdTxJson.ui8CodeTx = ErrorFrame;
    stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
    return ErrorYes;
  }
  strL_SinEspacio = strG_JsonRx;
  //Se Deserializ en JSON
  DeserializationError error = deserializeJson(doc, strG_JsonRx);
#ifdef useDebug
  Serial.println("\n5");
#endif
  //Si el JSON tieme errores se determna cual es y se retorna para enviar al HOST la respuesta de ERROR
  if (error) {
    //strncpy(chG_CmdRx, CMD_Desconocido, CaracMaxCmdTx);
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_Desconocido, CaracMaxCmdTx);
    if (error == DeserializationError::IncompleteInput) {  //Error por JSON Incompleto?
      stcG_CmdTxJson.ui8CodeTx = ErrorFrame;
    } else if (error == DeserializationError::InvalidInput) {  //Error por Dato Invalido?
      stcG_CmdTxJson.ui8CodeTx = ErrorData;
    } else if (error == DeserializationError::EmptyInput) {  //Error por datos Vacios?
      stcG_CmdTxJson.ui8CodeTx = ErrorBlank;
    } else if (error == DeserializationError::NoMemory) {  //Error por falta de MEmoria
      stcG_CmdTxJson.ui8CodeTx = ErrorMemory;
    } else {  //Error Desconocido
      stcG_CmdTxJson.ui8CodeTx = ErrorUnk;
    }
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
#ifdef useDebug
    Serial.println("\n6");
#endif
    return ErrorYes;
  }
  strL_SinEspacio.replace(" ", "");
  if (myFunciones.funcionlTotalString(strL_SinEspacio)) {
    const char* cmdRx = doc["cmd"];
    //chG_CmdRx = String(cmdRx);
    //strncpy(chG_CmdRx, cmdRx, CaracMaxCmdTx);
    strncpy(stcG_CmdTxJson.chrG_CmdTx, cmdRx, CaracMaxCmdTx);
    //byG_CodeTx = ErrorTotal;
    stcG_CmdTxJson.ui8CodeTx = ErrorTotal;
    ErrorFrame;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return ErrorYes;
  }
  if (!doc["cmd"]) {  //Se verifica si se Rx el comando "cmd"
    //chG_CmdRx = "UNK";
    //strncpy(chG_CmdRx, CMD_Desconocido, CaracMaxCmdTx);
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_Desconocido, CaracMaxCmdTx);
    //byG_CodeTx = ErrorCmd;
    stcG_CmdTxJson.ui8CodeTx = ErrorCmd;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return ErrorYes;
  }
#ifdef useDebug
  Serial.println("\n7");
#endif
  return ErrorNoF;
}
//*****************************************************************************
// Funcion bValidoDatos_cmd Verifica sel comando "cmd" recibe los datos
//    completos y son validos, las respuestas son las siguientes
//    Si los datos estan completos y son Validos retorna TRUE
//    Si los datos no estan completos o No es Valido retorna FALSE + flagTxRespuesta -> 1
//      en bG_CodeTx se almacena el Codigo a Tx al Host
//*****************************************************************************
bool bValidoDatos_cmd() {
  //**************************************
  //Se Validan los Datos de La Factura
  //**************************************
  if (doc["cmd"] == CMD_EncabezadoF) {  //El comando "cmd" es Encabezado?
    //strncpy(chG_CmdRx, CMD_Encabezado, CaracMaxCmdTx);
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_EncabezadoF, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    JsonArray data = doc["data"];
    //byG_NroEncab = doc["data"].size();
    byte auxD = doc["data"].size();
    if (auxD == 0) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_currentInput = Input::InpEnc;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_ClienteF) {  //El comando "cmd" es Cliente?
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_ClienteF, CaracMaxCmdTx);
    if (!doc["data"]) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_Cliente.RifCi[0] = '\0';
    stcG_Cliente.RazSoc[0] = '\0';
    stcG_currentInput = Input::InpCli;  //Comando Cliente
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_ComentarioF) {  //El comando "cmd" es Comoentarios?
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_ComentarioF, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_currentInput = Input::InpLinCom;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_ProductoF) {  //El comando "cmd" es Productos?
#ifdef timeDebug
    long iniTime;
    iniTime = millis();
#endif
    //chG_CmdRx = "pro";
    //strncpy(chG_CmdRx, CMD_Producto, CaracMaxCmdTx);
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_ProductoF, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    JsonObject data = doc["data"];
    if (!data["imp"]) {
#ifdef useDebug
      Serial.println("Sin data[\"imp\"]");
#endif
      // byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    if (!data["pre"]) {
#ifdef useDebug
      Serial.println("Sin data[\"pre\"]");
#endif
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    if (!data["cant"]) {
#ifdef useDebug
      Serial.println("Sin data[\"cant\"]");
#endif
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    if (!data["des01"]) {
      Serial.println("Sin data[\"des01\"]");
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    } else {
      memset(stcG_Productos[ui16G_NroProducto].Descripcion1, 0, CantMaxCarArt);
      memset(stcG_Productos[ui16G_NroProducto].Descripcion2, 0, CantMaxCarArt);
      memset(stcG_Productos[ui16G_NroProducto].Descripcion3, 0, CantMaxCarArt);
      const char* carProRx = data["des01"];
#ifdef useDebug
      Serial.println("Nro Prod: " + String(ui16G_NroProducto));
      Serial.println(stcG_Productos[ui16G_NroProducto].Descripcion1);
#endif
      stcG_currentInput = Input::InpProd;
#ifdef timeDebug
      Serial.print("Time Ejec: ");
      Serial.print(String(millis() - iniTime));
      Serial.println(" mS");
#endif
    }
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_DescPorcF) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_DescPorcF, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpDesPor;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_DescMontF) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_DescMontF, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpDesMon;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_RecPorcF) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_RecPorcF, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpRecPor;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_RecMontF) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_RecMontF, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpRecMon;
  } else if (doc["cmd"] == CMD_CorrProF) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_CorrProF, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpCorrProd;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_AnulaProF) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_AnulaProF, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    JsonObject data = doc["data"];
    if (!data["pre"]) {
#ifdef useDebug
      Serial.println("Anula Sin data[\"pre\"]");
#endif
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    if (!data["cant"]) {
#ifdef useDebug
      Serial.println("Anula Sin data[\"cant\"]");
#endif
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    if (!data["des01"]) {
      Serial.println("Anula Sin data[\"des01\"]");
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_currentInput = Input::InpAnulProd;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_FormPagoF) {  //Entrada Forma de Pago
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_FormPagoF, CaracMaxCmdTx);
    if (!doc["data"]) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    JsonArray data_Array = doc["data"];
    uint8_t sizeArrayJseon = data_Array.size();
    if (sizeArrayJseon == 0) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    uint8_t noData = 1;
    for (int i = 0; i < sizeArrayJseon; i++) {
      uint8_t tipoFP = data_Array[i]["tipo"];
      uint64_t montoFP = data_Array[i]["monto"];
      if (data_Array[i]["tipo"] && data_Array[i]["monto"]) {
        noData = 0;
      }
    }
    if (noData) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_currentInput = Input::InpFormPa;
  } else if (doc["cmd"] == CMD_EndProductoF) {
    //strncpy(chG_CmdRx, CMD_EndProducto, CaracMaxCmdTx);
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_EndProductoF, CaracMaxCmdTx);
    stcG_currentInput = Input::InpFinProd;
  } else if (doc["cmd"] == CMD_LineAdicionalesF) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_LineAdicionalesF, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    JsonArray data = doc["data"];
    //byG_NroEncab = doc["data"].size();
    byte auxD = doc["data"].size();
    if (auxD == 0) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_currentInput = Input::InpLinAdi;
  } else if (doc["cmd"] == CMD_PieTicketF) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_PieTicketF, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    JsonArray data = doc["data"];
    byte auxD = doc["data"].size();
    if (auxD == 0) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_currentInput = Input::InpPieTic;
    // return ErrorNoF;
  } else if (doc["cmd"] == CMD_EndFactura) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_EndFactura, CaracMaxCmdTx);
    stcG_currentInput = Input::InpFinF;
  } 
  //**************************************
  //Se Validan los Datos de La Nota de Credito
  //**************************************
  else if (doc["cmd"] == CMD_EncabezadoNC) {  //El comando "cmd" es Encabezado?
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_EncabezadoNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    JsonArray data = doc["data"];
    //byG_NroEncab = doc["data"].size();
    byte auxD = doc["data"].size();
    if (auxD == 0) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_currentInput = Input::InpEncNC;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_NroFacNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_NroFacNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpNroFactAfNC;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_FechaFacNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_FechaFacNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpFechaFacAfNC;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_RirCiNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_RirCiNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpRifCiAfNC;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_RazSocNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_RazSocNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpRazSocAfNC;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_ComNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_ComNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpLinComNC;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_ProdNC) {  //El comando "cmd" es Productos?
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_ProdNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    JsonObject data = doc["data"];
    if (!data["imp"]) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    if (!data["pre"]) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    if (!data["cant"]) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    if (!data["des01"]) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    } else {
      memset(stcG_Productos[ui16G_NroProducto].Descripcion1, 0, CantMaxCarArt);
      memset(stcG_Productos[ui16G_NroProducto].Descripcion2, 0, CantMaxCarArt);
      memset(stcG_Productos[ui16G_NroProducto].Descripcion3, 0, CantMaxCarArt);
      const char* carProRx = data["des01"];
      stcG_currentInput = Input::InpProdNC;
    }
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_DescPorcNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_DescPorcNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpDesPorNC;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_DescMontNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_DescMontNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpDesMonNC;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_RecPorcNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_RecPorcNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpRecPorNC;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_RecMontNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_RecMontNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpRecMonNC;
  } else if (doc["cmd"] == CMD_CorrProNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_CorrProNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpCorrProdNC;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_AnulaProNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_AnulaProNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    JsonObject data = doc["data"];
    if (!data["pre"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    if (!data["cant"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    if (!data["des01"]) {
      Serial.println("Anula Sin data[\"des01\"]");
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_currentInput = Input::InpAnulProdNC;
    return ErrorNoF;
  } else if (doc["cmd"] == CMD_EndProductoNC) {
    //strncpy(chG_CmdRx, CMD_EndProducto, CaracMaxCmdTx);
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_EndProductoNC, CaracMaxCmdTx);
    stcG_currentInput = Input::InpFinProdNC;
  } else if (doc["cmd"] == CMD_FormPagoNC) {  //Entrada Forma de Pago
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_FormPagoNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    JsonArray data_Array = doc["data"];
    uint8_t sizeArrayJseon = data_Array.size();
    if (sizeArrayJseon == 0) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    uint8_t noData = 1;
    for (int i = 0; i < sizeArrayJseon; i++) {
      uint8_t tipoFP = data_Array[i]["tipo"];
      uint64_t montoFP = data_Array[i]["monto"];
      if (data_Array[i]["tipo"] && data_Array[i]["monto"]) {
        noData = 0;
      }
    }
    if (noData) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_currentInput = Input::InpFormPaNC;
  } else if (doc["cmd"] == CMD_LineAdicionalesNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_LineAdicionalesNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    JsonArray data = doc["data"];
    //byG_NroEncab = doc["data"].size();
    byte auxD = doc["data"].size();
    if (auxD == 0) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_currentInput = Input::InpLinAdiNC;
  } else if (doc["cmd"] == CMD_PieTicketNC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_PieTicketNC, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    JsonArray data = doc["data"];
    byte auxD = doc["data"].size();
    if (auxD == 0) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return ErrorYes;
    }
    stcG_currentInput = Input::InpPieTicNC;
    // return ErrorNoF;
  } else if (doc["cmd"] == CMD_EndNotaCredito) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_EndNotaCredito, CaracMaxCmdTx);
    stcG_currentInput = Input::InpFinNC;
  }
  //**************************************
  //Se Validan los Datos de Los CMD de Configuracion
  //**************************************
  else if (doc["cmd"] == CMD_setTimer) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_setTimer, CaracMaxCmdTx);
    if (!doc["data"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_CmdTxJson.ui8CodeTx = ErrorNo;
    stcG_currentInput = Input::InpSetTimer;
    return ErrorNoF;

  } else if (doc["cmd"] == CMD_GetTime) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_GetTime, CaracMaxCmdTx);
    if (!doc["data"]) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpGetTimer;
  } else if (doc["cmd"] == CMD_WriteFileSPIFF) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_WriteFileSPIFF, CaracMaxCmdTx);
    if (!doc["data"]["nameFile"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    if (!doc["data"]["contenido"]) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpWriteSPIFF;

  } else if (doc["cmd"] == CMD_ReadFileSPIFF) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_ReadFileSPIFF, CaracMaxCmdTx);
    Serial.println("Evaluzando Comando CMD_ReadFileSPIFFS");
    if (!doc["data"]["nameFile"]) {
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpReadSPIFF;
  } else if (doc["cmd"] == CMD_WriteFileeMMC) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_WriteFileeMMC, CaracMaxCmdTx);
    char chL_NameFile[32];
    String strL_Contenido;
    strncpy(chL_NameFile, "/", 32);
    if (doc["data"]["nameFile"]) {
      const char* nameFile = doc["data"]["nameFile"];
      strncat(chL_NameFile, nameFile, 32);
    } else {
      //byG_CodeTx = ErrorData;
      stcG_CmdTxJson.ui8CodeTx = ErrorData;
      stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
      return ErrorYes;
    }
    if (doc["data"]["contenido"]) {

      // serializeJson(data_contenido, strL_Contenido);
      Serial.println("El Json del Contenido es:");
      Serial.println(strL_Contenido);
    } else {
      //byG_CodeTx = ErrorData;
      stcG_CmdTxJson.ui8CodeTx = ErrorData;
      stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
      return ErrorYes;
    }
    JsonObject data_contenido = doc["data"]["contenido"];
    memset(&stcG_XyZ, 0, sizeof(stcG_XyZ));
    if (data_contenido["ContadorFactDia"]) {
      stcG_XyZ.stContadores.ui32CountFacDia = data_contenido["ContadorFactDia"];
    }
    if (data_contenido["fechaIniRep"]) {
      stcG_XyZ.stContadores.ui32FechaIniRep = data_contenido["fechaIniRep"];
    }
    if (data_contenido["ultFac"]) {
      stcG_XyZ.stContadores.ui32UltFact = data_contenido["ultFac"];
    }
    if (data_contenido["fechaUF"]) {
      stcG_XyZ.stContadores.ui32FechaUF = data_contenido["fechaUF"];
    }
    if (data_contenido["ultND"]) {
      stcG_XyZ.stContadores.ui32UltNDeb = data_contenido["ultND"];
    }
    if (data_contenido["ultNC"]) {
      stcG_XyZ.stContadores.ui32UltNCre = data_contenido["ultNC"];
    }
    if (data_contenido["ultDNF"]) {
      stcG_XyZ.stContadores.ui32UltDNF = data_contenido["ultDNF"];
    }
    if (data_contenido["fechaUDNF"]) {
      stcG_XyZ.stContadores.ui32FechaDNF = data_contenido["fechaUDNF"];
    }
    if (data_contenido["ultZ"]) {
      stcG_XyZ.stContadores.ui32UltRepZ = data_contenido["ultZ"];
    }
    if (data_contenido["fechaUZ"]) {
      stcG_XyZ.stContadores.ui32FechaRepZ = data_contenido["fechaUZ"];
    }
    if (data_contenido["ultRMF"]) {
      stcG_XyZ.stContadores.ui32UltRMF = data_contenido["ultRMF"];
    }
    if (data_contenido["fechaURMF"]) {
      stcG_XyZ.stContadores.ui32FechaUltRMF = data_contenido["fechaURMF"];
    }
    if (mySD_eMMC.boWriteFile(SD_MMC, chL_NameFile, (uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ)) == ERROR) {
      Serial.print("ERROR abriendo Archvo en la SD");
      ///Falta ver que sehace con este tipo de error
      stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
      // stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
      // return ErrorYes;
    } else {
      stcG_CmdTxJson.ui8CodeTx = ErrorNo;
    }
    stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
    return ErrorYes;
  } else if (doc["cmd"] == CMD_ImpRepX) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_ImpRepX, CaracMaxCmdTx);
    stcG_currentInput = Input::InpImpRepX;
  } else if (doc["cmd"] == CMD_GenImpRepZ) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_GenImpRepZ, CaracMaxCmdTx);
    stcG_currentInput = Input::InpGenImpRepZ;
  } else if (doc["cmd"] == CMD_InicializaPtr) {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_InicializaPtr, CaracMaxCmdTx);
    if (!doc["data"]["SerialPtr"]) {
      //byG_CodeTx = ErrorDatBlank;
      stcG_CmdTxJson.ui8CodeTx = ErrorDatBlank;
      return ErrorYes;
    }
    stcG_currentInput = Input::InpInitPtr;
  } else {
    strncpy(stcG_CmdTxJson.chrG_CmdTx, CMD_Desconocido, CaracMaxCmdTx);
    stcG_CmdTxJson.ui8CodeTx = ErrorCmdUnk;
    stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
    return ErrorYes;
  }
  return ErrorNoF;
}
//*****************************************************************************
// Funcion bValidoSecuencia_cmd Verifica si la secuencia del comando "cmd"
//    es valida, las respuestas son las siguientes
//    Si secuencia "cmd" es  Valida retorna TRUE
//    Si secuencia "cmd"  No es Valido retorna FALSE + flagTxRespuesta -> 1
//      en bG_CodeTx se almacena el Codigo a Tx al Host
//*****************************************************************************
bool bValidoSecuencia_cmd(void) {
  bool boL_Respuesta = ErrorNoF;
  const char* cmdRx = doc["cmd"];
  //strncpy(chG_CmdRx, cmdRx, CaracMaxCmdTx);
  strncpy(stcG_CmdTxJson.chrG_CmdTx, cmdRx, CaracMaxCmdTx);
  bool boL_envError = 0;
  switch (stcG_currentState) {
    case stateInicial:                               //Se verifica si Hay error De Secuencia de Estados
      //Secuencia Invalida para la Factura
      if (stcG_currentInput == Input::InpFinProd) {  //Del Estado Inicial no puede ir a Estado FinProd
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpPieTic) {  //Del Estado Inicial no puede ir a Estado PieTic
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinF) {  //Del Estado Inicial no puede ir a Estado FinFactura
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPa) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinAdi) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpPieTic) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProd) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } 
      //Secuencia Invalida para la Nota de Credito
      else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateEncabezado:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado Encabezado no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpPieTic) {  //Del Estado Encabezado no puede ir a Estado PieTic
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinF) {  //Del Estado Encabezado no puede ir a Estado FinF
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPa) {  //Del Estado Inicial no puede ir a Estado FinF
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinAdi) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpPieTic) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateCliente:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado Cliente no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpPieTic) {  //Del Estado Cliente no puede ir a Estado PieTic
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinF) {  //Del Estado Cliente no puede ir a Estado FinF
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPa) {  //Del Estado Inicial no puede ir a Estado FinF
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinAdi) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpPieTic) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateLinea_Com:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado LineaComentario no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpPieTic) {  //Del Estado LineaComentario no puede ir a Estado PieTic
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinF) {  //Del Estado LineaComentario no puede ir a Estado FinF
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPa) {  //Del Estado Inicial no puede ir a Estado FinF
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinAdi) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpPieTic) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateProductos:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado Producto no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado Producto no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado Producto no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateDescPor:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado Producto no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado Producto no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado Producto no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateDesMonto:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado Producto no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado Producto no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado Producto no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateRecarPor:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado Producto no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado Producto no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado Producto no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateRecarMonto:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado Producto no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado Producto no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado Producto no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateCorrecProd:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado Producto no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado Producto no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado Producto no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateAnulaProd:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado Producto no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado Producto no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado Producto no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateFinProducto:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado FinProducto no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado FinProducto no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado FinProducto no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProd) {  //Del Estado FinProducto no puede ir a Estado Producto
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateFormaPago:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado FinProducto no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado FinProducto no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado FinProducto no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProd) {  //Del Estado FinProducto no puede ir a Estado Producto
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case statePieTicket:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado PieTicket no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado PieTicket no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado PieTicket no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProd) {  //Del Estado PieTicket no puede ir a Estado Producto
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateLineaAdic:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado PieTicket no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado PieTicket no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado PieTicket no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProd) {  //Del Estado PieTicket no puede ir a Estado Producto
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    case stateFin_Factura:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado Fin_Factura no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado Fin_Factura no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado Fin_Factura no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProd) {  //Del Estado Fin_Factura no puede ir a Estado Producto
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Secuencia Invalida para NC
      else if (stcG_currentInput == Input::InpNroFactAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      if(boL_envError){
        stcG_CmdTxJson.ui8CodeTx = ErrorCmNa;
        stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
        boL_Respuesta = ErrorYes;
      }
      break;
    //Para Input de las Notas de Creditos  
    case stateEncabezadoNC:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado PieTicket no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado PieTicket no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado PieTicket no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProd) {  //Del Estado PieTicket no puede ir a Estado Producto
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPor) {  //Del Estado PieTicket no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMon) {  //Del Estado PieTicket no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPor) {  //Del Estado PieTicket no puede ir a Estado Producto
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMon) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProd) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProd) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPa) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProd) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinAdi) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpPieTic) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } 
      //Para Input de NC
      else if (stcG_currentInput == Input::InpFechaFacAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Comandos Especiales
      else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      break;
    case stateLinComNC:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado PieTicket no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado PieTicket no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado PieTicket no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProd) {  //Del Estado PieTicket no puede ir a Estado Producto
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPor) {  //Del Estado PieTicket no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMon) {  //Del Estado PieTicket no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPor) {  //Del Estado PieTicket no puede ir a Estado Producto
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMon) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProd) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProd) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPa) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProd) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinAdi) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpPieTic) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } 
      //Para Input de NC
      else if (stcG_currentInput == Input::InpRifCiAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Comandos Especiales
      else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      break;
    case stateNroFactAfNC:
      if (stcG_currentInput == Input::InpEnc) {  //Del Estado PieTicket no puede ir a Estado Encabezado
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCli) {  //Del Estado PieTicket no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinCom) {  //Del Estado PieTicket no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProd) {  //Del Estado PieTicket no puede ir a Estado Producto
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesPor) {  //Del Estado PieTicket no puede ir a Estado Ciente
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMon) {  //Del Estado PieTicket no puede ir a Estado LineaComentario
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecPor) {  //Del Estado PieTicket no puede ir a Estado Producto
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMon) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpCorrProd) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProd) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFormPa) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProd) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpLinAdi) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpPieTic) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } 
      //Para Input de NC del  estado stateNroFactNC
      else if (stcG_currentInput == Input::InpEncNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRazSocAfNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpDesPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpDesMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpRecPorNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpRecMonNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpCorrProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpAnulProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpFormPaNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpFinProdNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }else if (stcG_currentInput == Input::InpFinNC) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      //Comandos Especiales
      else if (stcG_currentInput == Input::InpImpRepX) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGenImpRepZ) {
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpGetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpSetTimer) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpWriteSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpReadSPIFF) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      } else if (stcG_currentInput == Input::InpInitPtr) {  //Del Estado Inicial no puede ir a Estado Forma Pago
        boL_envError = 1;
      }
      break;
    case stateImpRepX:
    case stateGenImpRepZ:
      break;
  }
  return boL_Respuesta;
}
//*****************************************************************************
// Funciones Para la Maquina de Estado Finita
//    Los Estados son:
//      stateInicial,       **Estado Inicial
//      stateEncabezado,    **Encabezados
//      stateCliente,       **RIF o CI del Cliente y Razon Social
//      stateLinea_Com,     **Lineas Adicionales Comentarios
//      stateProductos,     **Productos para la Factura
//      stateFinProducto,   **Fin de guardar Productos
//      statePieTicket,     **Informacion del Pie de Ticket
//      stateFin_Factura    **Fin de la Factura
//    Si JSON Rx  No es Valido retorna FALSE + flagTxRespuesta -> 1
//      en bCodeTx se almacena el Codigo a Tx al Host
//*****************************************************************************
//*****************************************************************************
// Funcion para  Actualiza el Estado de la Maquina de Estados
//*****************************************************************************
// Actualiza el estado de la maquina
void updateStateMachine(void) {
  stcG_FlagFact.flagDocProc = 1;  //Bandera que Indica Documento en Proceso, No se puede Hacer nada Alterno
  switch (stcG_currentState) {
    //Estados Inicial
    case stateInicial:
      stateIni();
      break;
    //Estados para la Factura
    case stateEncabezado:
      stateEnc();
      break;
    case stateCliente:
      stateCli();
      break;
    case stateLinea_Com:
      stateLinC();
      break;
    case stateProductos:
      statePro();
      break;
    case stateDescPor:
      stateDesPor();
      break;
    case stateDesMonto:
      stateDesMon();
      break;
    case stateRecarPor:
      stateRecarPorcen();
      break;
    case stateRecarMonto:
      stateRecargaMonto();
      break;
    case stateCorrecProd:
      stateCorreccionPro();
      break;
    case stateAnulaProd:
      stateAnulaProducto();
      break;
    case stateFinProducto:
      stateFinPro();
      break;
    case stateFormaPago:
      stateFormaPag();
      break;
    case stateLineaAdic:
      stateLineasAdicionales();
      break;
    case statePieTicket:
      statePieTickets();
      break;
    case stateFin_Factura:
      stateFinF();
      break;
    //Estados para na Nota de Credito
    case stateEncabezadoNC:
      fStateEncNC();
      break;
    case stateNroFactAfNC:
      fStateNroFactNC();
      break;
    case stateFechaFacAfNC:
      fStateNroRegAfNC();
      break;
    case stateRifCiAfNC:
      fStateRifCiAfNC();
      break;
    case stateRazSocAfNC:
      fStateRazSocAfNC();
      break;
    case stateLinComNC:
      fStateLinComNC();
      break;
    case stateProductoNC:
      fStateProNC();
      break;
    case stateDescPorNC:
      fStateDesPorNC();
      break;
    case stateDesMontoNC:
      fStateDesMonNC();
      break;
    case stateRecarPorNC:
      fStateRecarPorcenNC();
      break;
    case stateRecarMontoNC:
      fStateRecargaMontoNC();
      break;
    case stateCorrecProdNC:
      fStateCorreccionProNC();
      break;
    case stateAnulaProdNC:
      fStateAnulaProductoNC();
      break;
    case stateFinProductoNC:
      fStateFinProNC();
      break;
    case stateFormaPagoNC:
      fStateFormaPagNC();
      break;
    case stateLineaAdicNC:
      fStateLineasAdicionalesNC();
      break;
    case statePieTicketNC:
      fStatePieTicketsNC();
      break;
    case stateFin_NC:
      fStateFinFNC();
      break;
    //Estados para Comandos de Control
    case stateImpRepX:
      statImprimirReporteX();
      break;
    case stateGenImpRepZ:
      stateGenerarImpReporteZ();
      break;
  }
}
//*****************************************************************************
//********************* Funcion de la Maquina de Estados **********************
//*****************************************************************************
//*****************************************************************************
// Funcion del Estado *Inicial* de la Maquina de Estados
//*****************************************************************************
void stateIni(void) {
  //Cambio de Estados para la Factura
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  } 
  //Cambio de Estados para la Nota de Credito
  else if (stcG_currentInput == Input::InpEncNC) {
    changeState(State::stateEncabezadoNC);
  } else if (stcG_currentInput == Input::InpLinComNC) {
    changeState(State::stateLinComNC);
  } else if (stcG_currentInput == Input::InpNroFactAfNC) {
    changeState(State::stateNroFactAfNC);
  }
  //Cambio de Estados para Comandos de Control
  else if (stcG_currentInput == Input::InpImpRepX) {
    changeState(State::stateImpRepX);
  } else if (stcG_currentInput == Input::InpGenImpRepZ) {
    changeState(State::stateGenImpRepZ);
  } else if (stcG_currentInput == Input::InpGetTimer) {
    changeState(State::stateGetTimer);
  } else if (stcG_currentInput == Input::InpSetTimer) {
    changeState(State::stateSetTimer);
  } else if (stcG_currentInput == Input::InpWriteSPIFF) {
    changeState(State::stateWrSPIFF);
  } else if (stcG_currentInput == Input::InpReadSPIFF) {
    changeState(State::stateReSPIFF);
  } else if (stcG_currentInput == Input::InpInitPtr) {
    changeState(State::stateInitPtr);
  } 

}
//*****************************************************************************
// Funcion del Estado *Encabezado* de la Maquina de Estados
//*****************************************************************************
void stateEnc(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  }
}
//*****************************************************************************
// Funcion del Estado *Cliente* de la Maquina de Estados
//*****************************************************************************
void stateCli(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  }
}
//*****************************************************************************
// Funcion del Estado *Lineas Adicionales* de la Maquina de Estados
//*****************************************************************************
void stateLinC(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  }
}
//*****************************************************************************
// Funcion del Estado *Producto* de la Maquina de Estados
//*****************************************************************************
void statePro(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  } else if (stcG_currentInput == Input::InpDesPor) {
    changeState(State::stateDescPor);
  } else if (stcG_currentInput == Input::InpDesMon) {
    changeState(State::stateDesMonto);
  } else if (stcG_currentInput == Input::InpRecPor) {
    changeState(State::stateRecarPor);
  } else if (stcG_currentInput == Input::InpRecMon) {
    changeState(State::stateRecarMonto);
  } else if (stcG_currentInput == Input::InpCorrProd) {
    changeState(State::stateCorrecProd);
  } else if (stcG_currentInput == Input::InpAnulProd) {
    changeState(State::stateAnulaProd);
  } else if (stcG_currentInput == Input::InpFinProd) {
    changeState(State::stateFinProducto);
  } else if (stcG_currentInput == Input::InpFinF) {
    changeState(State::stateFin_Factura);
  } else if (stcG_currentInput == Input::InpPieTic) {
    changeState(State::statePieTicket);
  }
}
//*****************************************************************************
// Funcion del Estado *Descuento Por Porcentaje* de la Maquina de Estados
//*****************************************************************************
void stateDesPor(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  } else if (stcG_currentInput == Input::InpFinProd) {
    changeState(State::stateFinProducto);
  } else if (stcG_currentInput == Input::InpFinF) {
    changeState(State::stateFin_Factura);
  } else if (stcG_currentInput == Input::InpPieTic) {
    changeState(State::statePieTicket);
  }
}
//*****************************************************************************
// Funcion del Estado *Descuento Por Monto* de la Maquina de Estados
//*****************************************************************************
void stateDesMon(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  } else if (stcG_currentInput == Input::InpFinProd) {
    changeState(State::stateFinProducto);
  } else if (stcG_currentInput == Input::InpFinF) {
    changeState(State::stateFin_Factura);
  } else if (stcG_currentInput == Input::InpPieTic) {
    changeState(State::statePieTicket);
  }
}

//*****************************************************************************
// Funcion del Estado *Recarga Por Porcentaje* de la Maquina de Estados
//*****************************************************************************
void stateRecarPorcen(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  } else if (stcG_currentInput == Input::InpFinProd) {
    changeState(State::stateFinProducto);
  } else if (stcG_currentInput == Input::InpFinF) {
    changeState(State::stateFin_Factura);
  } else if (stcG_currentInput == Input::InpPieTic) {
    changeState(State::statePieTicket);
  }
}

//*****************************************************************************
// Funcion del Estado *Recarga Por Monto* de la Maquina de Estados
//*****************************************************************************
void stateRecargaMonto(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  } else if (stcG_currentInput == Input::InpFinProd) {
    changeState(State::stateFinProducto);
  } else if (stcG_currentInput == Input::InpFinF) {
    changeState(State::stateFin_Factura);
  } else if (stcG_currentInput == Input::InpPieTic) {
    changeState(State::statePieTicket);
  }
}
//*****************************************************************************
// Funcion del Estado *Correcccion de Producto* de la Maquina de Estados
//*****************************************************************************
void stateCorreccionPro(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  } else if (stcG_currentInput == Input::InpFinProd) {
    changeState(State::stateFinProducto);
  } else if (stcG_currentInput == Input::InpFinF) {
    changeState(State::stateFin_Factura);
  } else if (stcG_currentInput == Input::InpPieTic) {
    changeState(State::statePieTicket);
  }
}

//*****************************************************************************
// Funcion del Estado *Anulacion de Producto* de la Maquina de Estados
//*****************************************************************************
void stateAnulaProducto(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  } else if (stcG_currentInput == Input::InpAnulProd) {
    changeState(State::stateAnulaProd);
  } else if (stcG_currentInput == Input::InpFinProd) {
    changeState(State::stateFinProducto);
  } else if (stcG_currentInput == Input::InpFinF) {
    changeState(State::stateFin_Factura);
  } else if (stcG_currentInput == Input::InpPieTic) {
    changeState(State::statePieTicket);
  }
}
//*****************************************************************************
// Funcion del Estado *Producto* de la Maquina de Estados
//*****************************************************************************
void stateFinPro(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  } else if (stcG_currentInput == Input::InpFinProd) {
    changeState(State::stateFinProducto);
  } else if (stcG_currentInput == Input::InpFinF) {
    changeState(State::stateFin_Factura);
  } else if (stcG_currentInput == Input::InpFormPa) {
    changeState(State::stateFormaPago);
  } else if (stcG_currentInput == Input::InpPieTic) {
    changeState(State::statePieTicket);
  } else if (stcG_currentInput == Input::InpLinAdi) {
    changeState(State::stateLineaAdic);
  }
}
//*****************************************************************************
// Funcion del Estado *FormaPago* de la Maquina de Estados
//*****************************************************************************
void stateFormaPag(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  } else if (stcG_currentInput == Input::InpFinProd) {
    changeState(State::stateFinProducto);
  } else if (stcG_currentInput == Input::InpFinF) {
    changeState(State::stateFin_Factura);
  } else if (stcG_currentInput == Input::InpFormPa) {
    changeState(State::stateFormaPago);
  } else if (stcG_currentInput == Input::InpPieTic) {
    changeState(State::statePieTicket);
  } else if (stcG_currentInput == Input::InpLinAdi) {
    changeState(State::stateLineaAdic);
  }
}
//*****************************************************************************
// Funcion del Estado *Pie de Ticket* de la Maquina de Estados
//*****************************************************************************
void stateLineasAdicionales(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::statePieTicket);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::statePieTicket);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::statePieTicket);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::statePieTicket);
  } else if (stcG_currentInput == Input::InpFinF) {
    changeState(State::stateFin_Factura);
  } else if (stcG_currentInput == Input::InpPieTic) {
    changeState(State::statePieTicket);
  }
  return;
}
//*****************************************************************************
// Funcion del Estado *Pie de Ticket* de la Maquina de Estados
//*****************************************************************************
void statePieTickets(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::statePieTicket);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::statePieTicket);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::statePieTicket);
  } else if (stcG_currentInput == Input::InpLinAdi) {
    changeState(State::stateLineaAdic);
  } else if (stcG_currentInput == Input::InpFinF) {
    changeState(State::stateFin_Factura);
  }
  return;
}
//*****************************************************************************
// Funcion del Estado *FinFactura* de la Maquina de Estados
//*****************************************************************************
void stateFinF(void) {
  return;
}


//NOTA CREDITO
//*****************************************************************************
// Funcion del Estado *Encabezado* de la Maquina de Estados de la Nota Crdito (NC)
//*****************************************************************************
void fStateEncNC(void) {
  if (stcG_currentInput == Input::InpLinComNC) {
    changeState(State::stateLinComNC);
  } else if (stcG_currentInput == Input::InpNroFactAfNC) {
    changeState(State::stateNroFactAfNC);
  } 
}
//*****************************************************************************
// Funcion del Estado *Nro Factura* de la Maquina de Estados de la Nota Crdito (NC)
//*****************************************************************************
void fStateNroFactNC(void) {
  if (stcG_currentInput == Input::InpFechaFacAfNC) {
    changeState(State::stateFechaFacAfNC);
  } 
}
//*****************************************************************************
// Funcion del Estado *Fecha de la Factura* de la Maquina de Estados de la Nota Crdito (NC)
//*****************************************************************************
void fStateFechaFacAfNC(void) {
  if (stcG_currentInput == Input::InpNroRegAfNC) {
    changeState(State::stateNroRegAfNC);
  } 
}
//*****************************************************************************
// Funcion del Estado *Fecha de la Factura* de la Maquina de Estados de la Nota Crdito (NC)
//*****************************************************************************
void fStateNroRegAfNC(void) {
  if (stcG_currentInput == Input::InpRifCiAfNC) {
    changeState(State::stateRifCiAfNC);
  } 
}
//*****************************************************************************
// Funcion del Estado *Fecha de la Factura* de la Maquina de Estados de la Nota Crdito (NC)
//*****************************************************************************
void fStateRifCiAfNC(void) {
  if (stcG_currentInput == Input::InpRazSocAfNC) {
    changeState(State::stateRazSocAfNC);
  } 
}
//*****************************************************************************
// Funcion del Estado *Fecha de la Factura* de la Maquina de Estados de la Nota Crdito (NC)
//*****************************************************************************
void fStateNroRegNC(void) {
  if (stcG_currentInput == Input::InpRazSocAfNC) {
    changeState(State::stateRazSocAfNC);
  } 
}
//*****************************************************************************
// Funcion del Estado *Fecha de la Factura* de la Maquina de Estados de la Nota Crdito (NC)
//*****************************************************************************
void fStateRazSocAfNC(void) {
  if (stcG_currentInput == Input::InpLinAdi1NC) {
    changeState(State::stateLinAdi1NC);
  } else if (stcG_currentInput == Input::InpLinComNC) {
    changeState(State::stateLinComNC);
  } else if (stcG_currentInput == Input::InpProdNC) {
    changeState(State::stateProductoNC);
  }
}
//*****************************************************************************
// Funcion del Estado *Lineas Adicionales* de la Maquina de Estados
//*****************************************************************************
void fStateLinComNC(void) {
  if (stcG_currentInput == Input::InpEnc) {
    changeState(State::stateEncabezado);
  } else if (stcG_currentInput == Input::InpCli) {
    changeState(State::stateCliente);
  } else if (stcG_currentInput == Input::InpLinCom) {
    changeState(State::stateLinea_Com);
  } else if (stcG_currentInput == Input::InpProd) {
    changeState(State::stateProductos);
  }
}
//*****************************************************************************
// Funcion del Estado *Producto* de la Maquina de Estados de la NC
//*****************************************************************************
void fStateProNC(void) {
  if (stcG_currentInput == Input::InpProdNC) {
    changeState(State::stateProductoNC);
  } else if (stcG_currentInput == Input::InpFinProdNC) {
    changeState(State::stateFinProductoNC);
  // } else if (stcG_currentInput == Input::) {
  //   changeState(State::);
  // } else if (stcG_currentInput == Input::) {
  //   changeState(State::);
  } else if (stcG_currentInput == Input::InpDesPorNC) {
    changeState(State::stateDescPorNC);
  } else if (stcG_currentInput == Input::InpDesMonNC) {
    changeState(State::stateDesMontoNC);
  } else if (stcG_currentInput == Input::InpRecPorNC) {
    changeState(State::stateRecarPorNC);
  } else if (stcG_currentInput == Input::InpRecMonNC) {
    changeState(State::stateRecarMontoNC);
  } else if (stcG_currentInput == Input::InpCorrProdNC) {
    changeState(State::stateCorrecProdNC);
  } else if (stcG_currentInput == Input::InpAnulProdNC) {
    changeState(State::stateAnulaProdNC);
  } else if (stcG_currentInput == Input::InpFinProdNC) {
    changeState(State::stateFinProductoNC);
  } else if (stcG_currentInput == Input::InpFinNC) {
    changeState(State::stateFin_NC);
  } else if (stcG_currentInput == Input::InpPieTicNC) {
    changeState(State::statePieTicketNC);
  }
}
//*****************************************************************************
// Funcion del Estado *Descuento Por Porcentaje* de la Maquina de Estados
//*****************************************************************************
void fStateDesPorNC(void) {
  if (stcG_currentInput == Input::InpProdNC) {
    changeState(State::stateProductoNC);
  } else if (stcG_currentInput == Input::InpCorrProdNC) {
    changeState(State::stateCorrecProdNC);
  } else if (stcG_currentInput == Input::InpAnulProdNC) {
    changeState(State::stateAnulaProdNC);
  } else if (stcG_currentInput == Input::InpFinProdNC) {
    changeState(State::stateFinProductoNC);
  } 
}
//*****************************************************************************
// Funcion del Estado *Descuento Por Monto* de la Maquina de Estados
//*****************************************************************************
void fStateDesMonNC(void) {
  if (stcG_currentInput == Input::InpProdNC) {
    changeState(State::stateProductoNC);
  } else if (stcG_currentInput == Input::InpCorrProdNC) {
    changeState(State::stateCorrecProdNC);
  } else if (stcG_currentInput == Input::InpAnulProdNC) {
    changeState(State::stateAnulaProdNC);
  } else if (stcG_currentInput == Input::InpFinProdNC) {
    changeState(State::stateFinProductoNC);
  } 
}

//*****************************************************************************
// Funcion del Estado *Recarga Por Porcentaje* de la Maquina de Estados
//*****************************************************************************
void fStateRecarPorcenNC(void) {
  if (stcG_currentInput == Input::InpProdNC) {
    changeState(State::stateProductoNC);
  } else if (stcG_currentInput == Input::InpCorrProdNC) {
    changeState(State::stateCorrecProdNC);
  } else if (stcG_currentInput == Input::InpAnulProdNC) {
    changeState(State::stateAnulaProdNC);
  } else if (stcG_currentInput == Input::InpFinProdNC) {
    changeState(State::stateFinProductoNC);
  } 
}

//*****************************************************************************
// Funcion del Estado *Recarga Por Monto* de la Maquina de Estados
//*****************************************************************************
void fStateRecargaMontoNC(void) {
  if (stcG_currentInput == Input::InpProdNC) {
    changeState(State::stateProductoNC);
  } else if (stcG_currentInput == Input::InpCorrProdNC) {
    changeState(State::stateCorrecProdNC);
  } else if (stcG_currentInput == Input::InpAnulProdNC) {
    changeState(State::stateAnulaProdNC);
  } else if (stcG_currentInput == Input::InpFinProdNC) {
    changeState(State::stateFinProductoNC);
  } 
}
//*****************************************************************************
// Funcion del Estado *Correcccion de Producto* de la Maquina de Estados
//*****************************************************************************
void fStateCorreccionProNC(void) {
  if (stcG_currentInput == Input::InpProdNC) {
    changeState(State::stateProductoNC);
  } else if (stcG_currentInput == Input::InpAnulProdNC) {
    changeState(State::stateAnulaProdNC);
  } else if (stcG_currentInput == Input::InpFinProdNC) {
    changeState(State::stateFinProductoNC);
  } 
}

//*****************************************************************************
// Funcion del Estado *Anulacion de Producto* de la Maquina de Estados
//*****************************************************************************
void fStateAnulaProductoNC(void) {
  if (stcG_currentInput == Input::InpProdNC) {
    changeState(State::stateProductoNC);
  } else if (stcG_currentInput == Input::InpCorrProdNC) {
    changeState(State::stateCorrecProdNC);
  } else if (stcG_currentInput == Input::InpFinProdNC) {
    changeState(State::stateFinProductoNC);
  } 
}
//*****************************************************************************
// Funcion del Estado *Producto* de la Maquina de Estados
//*****************************************************************************
void fStateFinProNC(void) {
  if (stcG_currentInput == Input::InpFinNC) {
    changeState(State::stateFin_NC);
  } else if (stcG_currentInput == Input::InpFormPaNC) {
    changeState(State::stateFormaPagoNC);
  } else if (stcG_currentInput == Input::InpPieTicNC) {
    changeState(State::statePieTicketNC);
  } else if (stcG_currentInput == Input::InpLinAdiNC) {
    changeState(State::stateLineaAdicNC);
  }
}
//*****************************************************************************
// Funcion del Estado *FormaPago* de la Maquina de Estados
//*****************************************************************************
void fStateFormaPagNC(void) {
  if (stcG_currentInput == Input::InpFinNC) {
    changeState(State::stateFin_NC);
  } else if (stcG_currentInput == Input::InpPieTicNC) {
    changeState(State::statePieTicketNC);
  } else if (stcG_currentInput == Input::InpLinAdiNC) {
    changeState(State::stateLineaAdicNC);
  }
}
//*****************************************************************************
// Funcion del Estado *Pie de Ticket* de la Maquina de Estados
//*****************************************************************************
void fStateLineasAdicionalesNC(void) {
  if (stcG_currentInput == Input::InpPieTic) {
    changeState(State::statePieTicket);
  }  else if (stcG_currentInput == Input::InpFinNC) {
    changeState(State::stateFin_NC);
  }
  return;
}
//*****************************************************************************
// Funcion del Estado *Pie de Ticket* de la Maquina de Estados
//*****************************************************************************
void fStatePieTicketsNC(void) {
  if (stcG_currentInput == Input::InpFinNC) {
    changeState(State::stateFin_NC);
  }
  return;
}
//*****************************************************************************
// Funcion del Estado *FinFactura* de la Maquina de Estados
//*****************************************************************************
void fStateFinFNC(void) {
  return;
}
//*****************************************************************************
// Funcion del Estado *statImprimirReporteX* de la Maquina de Estados
//*****************************************************************************
void statImprimirReporteX(void) {
  return;
}
//*****************************************************************************
// Funcion del Estado *stateGenerarImpReporteZ* de la Maquina de Estados
//*****************************************************************************
void stateGenerarImpReporteZ(void) {
  return;
}
//*****************************************************************************
// Funcion para  Acambia el estado y dispara las transiciones de la
//         Maquina de Estados
//*****************************************************************************
void changeState(State newState) {
  stcG_currentState = newState;

  switch (stcG_currentState) {
    //Ejecuta Estado Inicial
    case State::stateInicial: outputSIni(); break;
    //Estados para Ejecutar los Comandos de Facturas
    case State::stateEncabezado: outputSEnc(); break;
    case State::stateCliente: outputSCli(); break;
    case State::stateLinea_Com: outputSLCom(); break;
    case State::stateProductos: outputSPro(); break;
    case State::stateDescPor: outputSDesPor(); break;
    case State::stateDesMonto: outputDesMon(); break;
    case State::stateRecarPor: outputRecargPor(); break;
    case State::stateRecarMonto: outputRecargMon(); break;
    case State::stateCorrecProd: outputCorrPro(); break;
    case State::stateAnulaProd: outputAnulaProducto(); break;
    case State::stateFinProducto: outputSFinPro(); break;
    case State::stateFormaPago: outputSFormPago(); break;
    case State::statePieTicket: outputSPieTick(); break;
    case State::stateLineaAdic: outputLinAdic(); break;
    case State::stateFin_Factura: outputSFinF(); break;
    //Estados para Ejecutar los Comandos de Notas de Creditos
    case State::stateEncabezadoNC: outputSEnc(); break;
    case State::stateNroFactAfNC: outputSNroFacAfNC(); break;
    case State::stateFechaFacAfNC: outputSFechaFacAfNC(); break;
    case State::stateNroRegAfNC: outputSNroRegAfNC(); break;
    case State::stateRifCiAfNC: outputSRifCiAfNC(); break;
    case State::stateRazSocAfNC: outputSRazSocAfNC(); break;
    case State::stateLinAdi1NC: outputLinAdic(); break;
    case State::stateLinComNC: outputSLCom(); break;
    case State::stateProductoNC: outputSPro(); break;
    case State::stateDescPorNC: outputSDesPor(); break;
    case State::stateDesMontoNC: outputDesMon(); break;
    case State::stateRecarPorNC: outputRecargPor(); break;
    case State::stateRecarMontoNC: outputRecargMon(); break;
    case State::stateCorrecProdNC: outputCorrPro(); break;
    case State::stateAnulaProdNC: outputAnulaProducto(); break;
    case State::stateFinProductoNC: outputSFinPro(); break;
    case State::stateFormaPagoNC: outputSFormPago(); break;
    case State::stateLineaAdicNC: outputSPieTick(); break;
    case State::statePieTicketNC: outputLinAdic(); break;
    case State::stateFin_NC: outputSFinNC(); break;






    //Estados para Ejecutar los Comandos de Configuracion y Reportes
    case State::stateImpRepX: outputImpRepX(); break;
    case State::stateGenImpRepZ: outputGenImpRepZ(); break;
    case State::stateGetTimer: outputGetTimer(); break;
    case State::stateSetTimer: outputSetTimer(); break;
    case State::stateWrSPIFF: outputWrSPIFF(); break;
    case State::stateReSPIFF: outputReSPIFF(); break;
    case State::stateInitPtr: outputInitPtr(); break;
    default: break;
  }
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado INICIAL de la
//         Maquina de Estados
//*****************************************************************************
void outputSIni(void) {
#ifdef useDebug
  Serial.println("\nOutput Estate Inicial");
#endif
  stcG_FlagErr.flag = 0;
  stcG_FlagFact.flagFact = 0;
  stcG_FlagFact.flagIindEncab = 0;
  ui16G_NroProducto = 0;
  fInicializaFactura();
  //strG_FacturaOut = "";
  //strG_FacturaOut = "\n**************************************************************\n";
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado ENCABEZADO de la
//         Maquina de Estados
//*****************************************************************************
void outputSEnc(void) {
  if (stcG_FlagFact.flagIindEncab) {
    return;
  }
  stcG_FlagFact.flagIindEncab = 1;
  JsonArray data = doc["data"];
  //byG_NroEncab = doc["data"].size();
  byte byL_Aux = doc["data"].size();

  if (byG_NroEncab + byL_Aux > CantMaxEncab) {
    //byG_NroEncab = CantMaxEncab;
    byL_Aux = CantMaxEncab - byG_NroEncab;
  }
  for (int i = 0; i < byL_Aux; i++) {
    const char* dataRx = data[i];
    strncpy(stcG_Encabezado[byG_NroEncab + i].Encabezado, dataRx, CantMaxCarLin - 1);
  }
  byG_NroEncab += byL_Aux;
#ifdef useDebug
  Serial.println("\nOutput Estate Encabezado");
  Serial.print("Nro de Encabezados: ");
  Serial.println(byG_NroEncab);
  for (int i = 0; i < byG_NroEncab; i++) {
    Serial.println(stcG_Encabezado[i].Encabezado);
  }
#endif
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado CLIENTE de la
//         Maquina de Estados
//*****************************************************************************
void outputSCli(void) {
  if (stcG_FlagFact.flagIndiClien) {
    return;
  }
  JsonObject data = doc["data"];
  if (data["rifCI"]) {
    const char* data_rifCI = data["rifCI"];  // RIF o CI
    //stcG_Cliente.RifCi = String(data_rifCI);
    strncpy(stcG_Cliente.RifCi, data_rifCI, CantMaxCarLin);
  }
  if (data["razSoc"]) {
    const char* data_razSoc = data["razSoc"];
    strncpy(stcG_Cliente.RazSoc, data_razSoc, CantMaxCarLin);
    //stcG_Cliente.RazSoc = String(data_razSoc);
    //Serial.print("RazS: ");
    //Serial.println(data_razSoc);
  }

  if (data["LineAd"]) {
    JsonArray dataLinAd = data["LineAd"];
    byG_NroLinAdCli = data["LineAd"].size();
    // for (int i = 0; i < CantMaxLinAd; i++) {
    //   stcG_Cliente.LineaAd[i][0] = '\0';
    // }
    memset(&stcG_Cliente.LineaAd, 0, sizeof(stcG_Cliente.LineaAd));
    if (byG_NroLinAdCli > CantMaxLinAd) {
      byG_NroLinAdCli = CantMaxLinAd;
    }
    for (int i = 0; i < byG_NroLinAdCli; i++) {
      const char* dataRx = dataLinAd[i];
      strncpy(stcG_Cliente.LineaAd[i], dataRx, CantMaxCarLin);
    }
  }
  stcG_FlagFact.flagIndiClien = 1;
#ifdef useDebug
  Serial.println("\nOutput Estate Cliente");
  Serial.print("RIF/CI: ");
  Serial.println(stcG_Cliente.RifCi);
  Serial.print("RAZON SOCIAL: ");
  Serial.println(stcG_Cliente.RazSoc);
#endif
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado Lineas Com de la
//         Maquina de Estados
//*****************************************************************************
void outputSLCom(void) {
  if (stcG_FlagFact.flagIndComen)
    return;
  stcG_FlagFact.flagIndComen = 1;
  const char* dataRx = doc["data"];
  //stcG_Comentario.Comentario = String(dataRx);
  strncpy(stcG_Comentario.Comentario, dataRx, CantMaxCarLin);
#ifdef useDebug
  Serial.println("\nOutput Estate Lineas Comentarios");
  Serial.println(stcG_Comentario.Comentario);
#endif
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado PRODUCTO de la
//         Maquina de Estados
//*****************************************************************************
void outputSPro(void) {
  memset(stcG_Productos[ui16G_NroProducto].Descripcion1, 0, CantMaxCarArt);
  memset(stcG_Productos[ui16G_NroProducto].Descripcion2, 0, CantMaxCarArt);
  memset(stcG_Productos[ui16G_NroProducto].Descripcion3, 0, CantMaxCarArt);
  JsonObject data = doc["data"];
  const char* carProRx = data["des01"];
  strncpy(stcG_Productos[ui16G_NroProducto].Descripcion1, carProRx, CantMaxCarArt);
  if (data["des02"]) {
    const char* carProRx = data["des02"];
    strncpy(stcG_Productos[ui16G_NroProducto].Descripcion2, carProRx, CantMaxCarArt);
  }
  if (data["des03"]) {
    const char* carProRx = data["des03"];
    strncpy(stcG_Productos[ui16G_NroProducto].Descripcion3, carProRx, CantMaxCarArt);
  }
  stcG_Productos[ui16G_NroProducto].ui64G_Cantidad = data["cant"];
  stcG_Productos[ui16G_NroProducto].ui64G_Precio = data["pre"];
  stcG_Productos[ui16G_NroProducto].ui8G_tipoImp = data["imp"];
  stcG_Productos[ui16G_NroProducto].ui64G_CantXPrec = myFunciones.multPrecioPorCant(stcG_Productos[ui16G_NroProducto].ui64G_Precio, stcG_Productos[ui16G_NroProducto].ui64G_Cantidad);
  ;
  stcG_Productos[ui16G_NroProducto].ui64G_IVA_Monto = 0;
  stcG_Productos[ui16G_NroProducto].ui64G_DescPorc = 0;
  stcG_Productos[ui16G_NroProducto].ui64G_DescMonto = 0;
  stcG_Productos[ui16G_NroProducto].ui64G_RecPorc = 0;
  stcG_Productos[ui16G_NroProducto].ui64G_RecMonto = 0;
  stcG_Productos[ui16G_NroProducto].ui64G_Correccion = 0;
  stcG_Productos[ui16G_NroProducto].ui64G_Anulacion = 0;
  uint8_t uiL_AuxTipo = stcG_Productos[ui16G_NroProducto].ui8G_tipoImp;
  stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto += stcG_Productos[ui16G_NroProducto].ui64G_CantXPrec;
#ifdef useDebug
  Serial.println("\nOutput Estate Productos");

#endif
  ///Incrementa Producto para el Siguiente Articulo
  ui16G_NroProducto++;  //Se incrementa el Numero de Productos Rx
  stcG_FlagFact.flagIndDescPor = 0;
  stcG_FlagFact.flagIndDescMon = 0;
  stcG_FlagFact.flagIndRecPor = 0;
  stcG_FlagFact.flagIndRecMon = 0;
  stcG_FlagFact.flagIndCorrProd = 0;
  stcG_FlagFact.flagIndDescPor = 0;
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado Descuento por Porcentaje de la
//         Maquina de Estados
//*****************************************************************************
void outputSDesPor(void) {
  ///TUUDOO
  //
  if (stcG_FlagFact.flagIndDescPor) {
    return;
  }
  stcG_FlagFact.flagIndDescPor = 1;
  if (doc["data"]) {
    uint16_t uin16L_PorDes = doc["data"];
    //Se verifica si el Porcentaje de Descuento es < 100 %
    if (uin16L_PorDes > PorMaxPer) {
      //byG_CodeTx = ErrorDataInval;
      stcG_CmdTxJson.ui8CodeTx = ErrorDataInval;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return;
    }
    stcG_Productos[ui16G_NroProducto - 1].ui64G_DescPorc = uin16L_PorDes;
    uint64_t ui64L_Des = myFunciones.calculoImpuesto(stcG_Productos[ui16G_NroProducto - 1].ui64G_CantXPrec, stcG_Productos[ui16G_NroProducto - 1].ui64G_DescPorc);
    uint8_t uiL_AuxTipo = stcG_Productos[ui16G_NroProducto - 1].ui8G_tipoImp;
    stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto -= ui64L_Des;
  }
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado Descuento por Monto
//        de la Maquina de Estados
//*****************************************************************************
void outputDesMon(void) {
  ///TUUDOO
  //
  if (stcG_FlagFact.flagIndDescMon) {
    return;
  }
  stcG_FlagFact.flagIndDescMon = 1;
  if (doc["data"]) {
    uint16_t uin16L_MonDes = doc["data"];
    if (uin16L_MonDes > stcG_Productos[ui16G_NroProducto - 1].ui64G_CantXPrec - 1) {
      //byG_CodeTx = ErrorDataInval;
      stcG_CmdTxJson.ui8CodeTx = ErrorDataInval;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return;
    }
    stcG_Productos[ui16G_NroProducto - 1].ui64G_DescMonto = uin16L_MonDes;
    uint8_t uiL_AuxTipo = stcG_Productos[ui16G_NroProducto - 1].ui8G_tipoImp;
    stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto -= stcG_Productos[ui16G_NroProducto - 1].ui64G_DescMonto;
  }
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado Recarga por Porcenraje de la
//         Maquina de Estados
//*****************************************************************************
void outputRecargPor(void) {
  ///TUUDOO
  //
  //Serial.println("✅Output Recarga por Porcentaje");
  if (stcG_FlagFact.flagIndRecPor) {
    return;
  }
  stcG_FlagFact.flagIndRecPor = 1;
  if (doc["data"]) {
    uint16_t uin16L_RecPor = doc["data"];
    if (uin16L_RecPor > PorMaxPer) {
      //byG_CodeTx = ErrorDataInval;
      stcG_CmdTxJson.ui8CodeTx = ErrorDataInval;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return;
    }
    stcG_Productos[ui16G_NroProducto - 1].ui64G_RecPorc = uin16L_RecPor;
    uint8_t uiL_AuxTipo = stcG_Productos[ui16G_NroProducto - 1].ui8G_tipoImp;
    stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto += myFunciones.calculoImpuesto(stcG_Productos[ui16G_NroProducto - 1].ui64G_CantXPrec, stcG_Productos[ui16G_NroProducto - 1].ui64G_RecPorc);
  }
}

//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado Recarga por Monto
//        de la Maquina de Estados
//*****************************************************************************
void outputRecargMon(void) {
  ///TUUDOO
  //
  //Serial.println("✅Output Recarga por Monto");
  if (stcG_FlagFact.flagIndRecMon) {
    return;
  }
  stcG_FlagFact.flagIndRecMon = 1;
  if (doc["data"]) {
    uint16_t uin16L_RecMon = doc["data"];
    if (uin16L_RecMon > stcG_Productos[ui16G_NroProducto - 1].ui64G_CantXPrec - 1) {
      //byG_CodeTx = ErrorDataInval;
      stcG_CmdTxJson.ui8CodeTx = ErrorDataInval;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return;
    }
    stcG_Productos[ui16G_NroProducto - 1].ui64G_RecMonto = uin16L_RecMon;
    uint8_t uiL_AuxTipo = stcG_Productos[ui16G_NroProducto - 1].ui8G_tipoImp;
    stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto += stcG_Productos[ui16G_NroProducto - 1].ui64G_RecMonto;
  }
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado de Correccionde Producto
//        de la Maquina de Estados
//*****************************************************************************
void outputCorrPro(void) {
  ///TUUDOO

  //
  if (stcG_FlagFact.flagIndCorrProd) {
    return;
  }
  stcG_FlagFact.flagIndCorrProd = 1;
  if (doc["data"]) {
    //uint64_t ui64L_TotalProducto = myFunciones.multPrecioPorCant(stcG_Productos[ui16G_NroProducto - 1].ui64G_Precio, stcG_Productos[ui16G_NroProducto - 1].ui64G_Cantidad);;

    stcG_Productos[ui16G_NroProducto - 1].ui64G_Correccion = stcG_Productos[ui16G_NroProducto - 1].ui64G_CantXPrec;
    uint8_t uiL_AuxTipo = stcG_Productos[ui16G_NroProducto - 1].ui8G_tipoImp;
    stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto -= stcG_Productos[ui16G_NroProducto - 1].ui64G_Correccion;
    // Serial.println("Correccion del Producto...");
    // Serial.println(stcG_Productos[ui16G_NroProducto - 1].Descripcion1);
    // Serial.printf("Monto Corregido: %lld\n", stcG_Productos[ui16G_NroProducto - 1].ui64G_Correccion);
    // Serial.printf("Monto CantXPrec: %lld\n", ui64L_TotalProducto);
  }
}

//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado de Anulacion de Producto
//        de la Maquina de Estados
//*****************************************************************************
void outputAnulaProducto(void) {
  ///TUUDOO

  //
  // if (stcG_FlagFact.flagIndAnulProd) {
  //   return;
  // }
  stcG_FlagFact.flagIndAnulProd = 1;
  if (doc["data"]) {
    JsonObject data = doc["data"];
    uint64_t ui64L_Precio = data["pre"];
    uint32_t ui32L_Cant = data["cant"];
    uint8_t ui8L_tipoImp = data["imp"];
    uint64_t ui64L_PrecioCantidad = myFunciones.multPrecioPorCant(ui64L_Precio, ui32L_Cant);
    if (ui64L_PrecioCantidad > stcG_TotalImpuestoArtiulos[ui8L_tipoImp - 1].ui64G_BI_Monto) {
      //Serial.println("🛑Monto de Anulaciona Mayor al Permitido...");
      //byG_CodeTx = ErrorDataInval;
      stcG_CmdTxJson.ui8CodeTx = ErrorDataInval;
      stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
      return;
    }
    //Serial.println("✅Monto de Anulaciona Permitido...");
    stcG_TotalImpuestoArtiulos[ui8L_tipoImp - 1].ui64G_BI_Monto -= ui64L_PrecioCantidad;
    memset(stcG_Productos[ui16G_NroProducto].Descripcion1, 0, CantMaxCarArt);
    memset(stcG_Productos[ui16G_NroProducto].Descripcion2, 0, CantMaxCarArt);
    memset(stcG_Productos[ui16G_NroProducto].Descripcion3, 0, CantMaxCarArt);
    const char* carProRx = data["des01"];
    strncpy(stcG_Productos[ui16G_NroProducto].Descripcion1, carProRx, CantMaxCarArt);
    if (data["des02"]) {
      const char* carProRx = data["des02"];
      strncpy(stcG_Productos[ui16G_NroProducto].Descripcion2, carProRx, CantMaxCarArt);
    }
    if (data["des03"]) {
      const char* carProRx = data["des03"];
      strncpy(stcG_Productos[ui16G_NroProducto].Descripcion3, carProRx, CantMaxCarArt);
    }
    stcG_Productos[ui16G_NroProducto].ui64G_Cantidad = data["cant"];
    stcG_Productos[ui16G_NroProducto].ui64G_Precio = data["pre"];
    stcG_Productos[ui16G_NroProducto].ui8G_tipoImp = data["imp"];
    stcG_Productos[ui16G_NroProducto].ui64G_CantXPrec = 0;
    stcG_Productos[ui16G_NroProducto].ui64G_IVA_Monto = 0;
    stcG_Productos[ui16G_NroProducto].ui64G_DescPorc = 0;
    stcG_Productos[ui16G_NroProducto].ui64G_DescMonto = 0;
    stcG_Productos[ui16G_NroProducto].ui64G_RecPorc = 0;
    stcG_Productos[ui16G_NroProducto].ui64G_RecMonto = 0;
    stcG_Productos[ui16G_NroProducto].ui64G_Correccion = 0;
    stcG_Productos[ui16G_NroProducto].ui64G_Anulacion = ui64L_PrecioCantidad;
    //
    ui16G_NroProducto++;  //Se incrementa el Numero de Productos Rx
    // for (int i = 0; i < ui16G_NroProducto; i++) {
    //   if (ui64L_Precio == stcG_Productos[i].ui64G_Precio &&
    //       ui32L_Cant == stcG_Productos[i].ui64G_Cantidad &&
    //       strcmp(desc1, stcG_Productos[i].Descripcion1) == 0) {
    //     stcG_Productos[i].ui64G_Anulacion = 1;
    //     break;
    //   }
    // }
    //uint64_t ui64L_TotalProducto = myFunciones.multPrecioPorCant(stcG_Productos[ui16G_NroProducto - 1].ui64G_Precio, stcG_Productos[ui16G_NroProducto - 1].ui64G_Cantidad);;
    //stcG_Productos[ui16G_NroProducto - 1].ui64G_Correccion = 1;
    // Serial.println("Correccion del Producto...");
    // Serial.println(stcG_Productos[ui16G_NroProducto - 1].Descripcion1);
    // Serial.printf("Monto Corregido: %lld\n", stcG_Productos[ui16G_NroProducto - 1].ui64G_Correccion);
    // Serial.printf("Monto CantXPrec: %lld\n", ui64L_TotalProducto);
  } else {
    Serial.println("🛑Anulacion sin DATA.....");
  }
  return;
}
//*****************************************************************************
// Funcion para  indicar Fin de Productos lo que debe hacer el Estado Fin de Productos
//         Maquina de Estados
//*****************************************************************************
void outputSFinPro(void) {
  if (stcG_FlagFact.flagIndFinProd) {
    return;
  }
  stcG_FlagFact.flagIndFinProd = 1;
#ifdef useDebug
  Serial.println("\nOutput Estate Fin de Productos");
#endif
  String sIniTotal, sFinTotal;
  String sLineaTotal;
  sIniTotal = "TOTAL";
  long long llTotal = 0;
}
//*****************************************************************************
// Funcion para indicar las Formas de Pago en la Factura en curso
//         Maquina de Estados
// Las formas de Pagos puedes ser:
//    -> Efectivo
//    -> Tarjeta de Debito
//    -> Tarjeta de Credito
//    -> Cheque
//    -> Petro
//    -> Dolares        **Se calcula el Impuesto IGTF
//    -> Bit Coin       **Se calcula el Impuesto IGTF
//*****************************************************************************
void outputSFormPago(void) {
  if (stcG_FlagFact.flagIndFormPag) {
    return;
  }
  //Se convierta a Objetos los Datos
  JsonArray data_Array = doc["data"];
  //uint8_t sizeArrayJseon = data_Array.size();
  ui8G_CountFormPagoRx = data_Array.size();
  if (ui8G_CountFormPagoRx > CantMaxImpPago) {
    ui8G_CountFormPagoRx = CantMaxImpPago;
  }
  if (ui8G_CountFormPagoRx == 0) {
    stcG_FormasPagoRxCMD[0].ui8_Tipo = 0;
    stcG_FormasPagoRxCMD[0].i64_Monto = -1;
    stcG_FormasPagoRxCMD[0].ui32_TasaConv = 0;
  } else {
    for (int i = 0; i < ui8G_CountFormPagoRx; i++) {
      uint8_t tipoFP = data_Array[i]["tipo"];
      int64_t montoFP = data_Array[i]["monto"];  //Si Monto es (-1) todo se cancela con este tipo de Pago
      if (data_Array[i]["tasaConv"]) {
        uint32_t tasaConversion = data_Array[i]["tasaConv"];
        stcG_FormasPagoRxCMD[i].ui32_TasaConv = tasaConversion;
        //stcG_FormasPagoRxCMD[i].ui64_MontoXtasaCam = myFunciones.multNum2D(montoFP, tasaConversion);
      } else {
        stcG_FormasPagoRxCMD[i].ui32_TasaConv = 0;
      }
      stcG_FormasPagoRxCMD[i].ui8_Tipo = tipoFP;
      stcG_FormasPagoRxCMD[i].i64_Monto = montoFP;
      // if(stcG_FormasPagoImp[tipoFP-1].impGraba > 0){
      //   stcG_FormasPagoRxCMD[i].ui64_ImpuestoFP = myFunciones.calculoImpuesto(stcG_FormasPagoRxCMD[i].ui64_MontoXtasaCam, stcG_FormasPagoImp[tipoFP-1].impGraba);
      // }
    }
  }
  for (int i = 0; i < ui8G_CountFormPagoRx; i++) {
    // Serial.printf("La Forma de Pago [%d] = %d\n", i, stcG_FormasPagoRxCMD[i].ui8_Tipo);
    // Serial.printf("El monto del Pago [%d] = %lld\n", i, stcG_FormasPagoRxCMD[i].i64_Monto);
    // Serial.printf("La Tasa de Conversion [%d] = %d\n", i, stcG_FormasPagoRxCMD[i].ui32_TasaConv);
  }
  stcG_FlagFact.flagIndFormPag = 1;
}

//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado de Lineas Adicionales
//         Maquina de Estados
//*****************************************************************************
void outputLinAdic(void) {
  // if (stcG_FlagFact.flagIndPiePag) {
  //   return;
  // }
  // stcG_FlagFact.flagIndPiePag = 1;

  JsonArray data = doc["data"];
  byG_NroLinAdicionales = doc["data"].size();
  if (byG_NroLinAdicionales > CantMaxLinAd)
    byG_NroLinAdicionales = CantMaxLinAd;
  for (int i = 0; i < byG_NroLinAdicionales; i++) {
    const char* dataRx = data[i];
    strncpy(stcG_LineaAdPostF[i].lineaAd, dataRx, CantMaxCarLin - 1);
  }
  Serial.println("\nOutput Lineas Adicionales");
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado Pie de Ticket de la
//         Maquina de Estados
//*****************************************************************************
void outputSPieTick(void) {
  if (stcG_FlagFact.flagIndPiePag) {
    return;
  }
  stcG_FlagFact.flagIndPiePag = 1;
  JsonArray data = doc["data"];

  byte byL_Aux = doc["data"].size();

  if (byG_NroLinAdPieTick + byL_Aux > CantMaxPieTick) {
    //byG_NroEncab = CantMaxEncab;
    byL_Aux = CantMaxPieTick - byG_NroEncab;
  }


  //byG_NroLinAdPieTick = doc["data"].size();
  // if (byG_NroLinAdPieTick > 9)
  //   byG_NroLinAdPieTick = 9;
  for (int i = 0; i < byL_Aux; i++) {
    const char* dataRx = data[i];
    strncpy(stcG_PieTicket[byG_NroLinAdPieTick + i].pieTicket, dataRx, CantMaxCarLin - 1);
  }
  byG_NroLinAdPieTick += byL_Aux;
  Serial.println("\nOutput Estate Pie de Ticket");
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado FIN Factura de la
//         Maquina de Estados
//*****************************************************************************
void outputSFinF(void) {
  if (stcG_FlagFact.flagIndFinFac) {
    //stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    return;
  }
  //stcG_FlagFact.flagIndFinFac = 1;
#ifdef useDebug
  Serial.println("\nOutput Estate Fin Factura");  //changeState
#endif
  if (fImprimirFactura() == ERROR) {
    Serial.println("❌Error en la Funcion fImprimirFactura");
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    return;
  }
  Serial.println("✅La Factura se Imprimio sin Errores....");
  RtcDateTime timeRTC;
  timeRTC = rtc1302.GetDateTime();
  uint32_t timeNow = myFunciones.timeToUnix(timeRTC);
  //Se Almacena el Tiempo transcurrido desde la primera Factura despues del Reporte Z
  stcG_CmdTxJson.ui32G_DataTx = timeNow - stcG_XyZ.stContadores.ui32FechaIniRep;
  if (stcG_CmdTxJson.ui32G_DataTx == 0) {
    stcG_CmdTxJson.ui32G_DataTx == 1;
  }
  Serial.printf("El tiempo Transcurrido para Actualizar es Z es: %d\n", stcG_CmdTxJson.ui32G_DataTx);
  //changeState(State::stateInicial);
  return;
}
//*****************************************************************************
// Funcion para  actualizar La Maquina de estados Para Las Notas de Credito
//         Maquina de Estados
//*****************************************************************************
//*****************************************************************************
// Funcion para  actualizar el Nuemro de Factura Afectada en la NC
//         Maquina de Estados
//*****************************************************************************
void outputSNroFacAfNC(void){

  return;
}
//*****************************************************************************
// Funcion para  actualizar la Fecha de Factura Afectada en la NC
//         Maquina de Estados
//*****************************************************************************
void outputSFechaFacAfNC(void){
  return;
}
//*****************************************************************************
// Funcion para  actualizar el Nuemro de Registro (Serial Ptr) Afectada en la NC
//         Maquina de Estados
//*****************************************************************************
void outputSNroRegAfNC(void){
  return;
}
//*****************************************************************************
// Funcion para  actualizar el RIF/CI de la factura Afectada en la NC
//         Maquina de Estados
//*****************************************************************************
void outputSRifCiAfNC(void){
  return;
}
//*****************************************************************************
// Funcion para  actualizar la Razon Social de la factura Afectada en la NC
//         Maquina de Estados
//*****************************************************************************
void outputSRazSocAfNC(void){
  return;
}
//*****************************************************************************
// Funcion para  generar e Inprimir la Salida de la NC
//         Maquina de Estados
//*****************************************************************************
void outputSFinNC(void){
  return;
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado Imprimir el Reporte X
//         Maquina de Estados
//*****************************************************************************
void outputImpRepX(void) {
#ifdef useDebug
  Serial.println("\nOutput Reporte X");
#endif
  //Se lee el La estructura del Reporte X en la eMC
  // memset(&stcG_XyZ, 0, sizeof(stcG_XyZ));
  char dirFileRepX[MaxCarNameFile];  //Nombre Archivo para Almacenar el Reporte X
  // char fechaFile[20];
  strncpy(dirFileRepX, DirReporte, MaxCarNameFile);
  if (mySD_eMMC.boExistDir(SD_MMC, dirFileRepX)) {
    Serial.println("No Existe Reporte X");
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return;
  }
  strncat(dirFileRepX, DocReporteSENIAT, MaxCarNameFile);
  strncat(dirFileRepX, ExtensionFileSENIAT, MaxCarNameFile);
  if (mySD_eMMC.boExistFile(SD_MMC, dirFileRepX) == ERROR) {
    Serial.print("ERROR abriendo Archvo en la SD");
    ///Falta ver que sehace con este tipo de error
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return;
  }
  mySD_eMMC.strReadFileData(SD_MMC, dirFileRepX, (uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ));
  fImprimirReporteX();
  // RtcDateTime timeRTC;
  // timeRTC = rtc1302.GetDateTime();
  //Se ve si existe el archivo
  // char dirFileRepX[MaxCarNameFile];                     //Nombre Archivo para Almacenar el Reporte X
  // char fechaFile[20];
  strncpy(dirFileRepX, DirReporte, MaxCarNameFile);
  if (mySD_eMMC.boExistDir(SD_MMC, dirFileRepX)) {
    Serial.println("No Existe Reporte X");
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return;
  }
  //sprintf(fechaFile, "%s%04u%02u%02u%s", DocReporteSENIAT, timeRTC.Year(), timeRTC.Month(), timeRTC.Day(), ExtensionFileSENIAT);
  strncat(dirFileRepX, DocReporteSENIAT, MaxCarNameFile);
  strncat(dirFileRepX, ExtensionFileSENIAT, MaxCarNameFile);
  if (mySD_eMMC.boExistFile(SD_MMC, dirFileRepX) == ERROR) {
    Serial.print("ERROR abriendo Archvo en la SD");
    ///Falta ver que sehace con este tipo de error
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return;
  }
  //Se Almacena la Estructura en la eMMC
  mySD_eMMC.boWriteFile(SD_MMC, dirFileRepX, (uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ));
  //memset(&stcG_XyZ, 0, sizeof(stcG_XyZ));
  /*
  memset(&stcG_XyZ.stBIVentas, 0, sizeof(stcG_XyZ.stBIVentas));
  memset(&stcG_XyZ.stFPVentas, 0, sizeof(stcG_XyZ.stFPVentas));
  memset(&stcG_XyZ.stDescuentos, 0, sizeof(stcG_XyZ.stDescuentos));
  memset(&stcG_XyZ.stRecargos, 0, sizeof(stcG_XyZ.stRecargos));
  memset(&stcG_XyZ.stCorrecciones, 0, sizeof(stcG_XyZ.stCorrecciones));
  memset(&stcG_XyZ.stAnulaciones, 0, sizeof(stcG_XyZ.stAnulaciones));
  memset(&stcG_XyZ.stBINotaDebito, 0, sizeof(stcG_XyZ.stBINotaDebito));
  memset(&stcG_XyZ.stFPNotDeb, 0, sizeof(stcG_XyZ.stFPNotDeb));
  memset(&stcG_XyZ.stBINotaCredito, 0, sizeof(stcG_XyZ.stBINotaCredito));
  memset(&stcG_XyZ.stFPNotCre, 0, sizeof(stcG_XyZ.stFPNotCre));
  */
  //🛑🛑Revisar esta funcion
  //mySD_eMMC.strReadFileData(SD_MMC, dirFileRepX, (uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ));

  // for(int i=0; i<CantMaxImpArt; i++){
  //   Serial.printf("Para el Tipo de Impuesto %d el BI es: %lld\n", i+1, stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i]);
  // }
  // File myFile;
  // myFile = SD_MMC.open(dirFileRepX);
  // myFile.read((uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ));
  // myFile.close();
  // Serial.println("***********Ventas**************");
  // for(int i=0; i<CantMaxImpArt; i++){
  //   Serial.printf("Para el Tipo de Impuesto %d el BI es: %lld\n", i+1, stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i]);
  // }
  // Serial.println("***********Descuentos**************");
  // for(int i=0; i<CantMaxImpArt; i++){
  //   Serial.printf("Para el Tipo de Impuesto %d el BI es: %lld\n", i+1, stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i]);
  // }
  // Serial.println("***********Recargos**************");
  // for(int i=0; i<CantMaxImpArt; i++){
  //   Serial.printf("Para el Tipo de Impuesto %d el BI es: %lld\n", i+1, stcG_XyZ.stRecargos.ui64_BI_Impuesto[i]);
  // }
  // Serial.println("***********Correciones**************");
  // for(int i=0; i<CantMaxImpArt; i++){
  //   Serial.printf("Para el Tipo de Impuesto %d el BI es: %lld\n", i+1, stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i]);
  // }
  //  Serial.println("***********Anulaciones**************");
  // for(int i=0; i<CantMaxImpArt; i++){
  //   Serial.printf("Para el Tipo de Impuesto %d el BI es: %lld\n", i+1, stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i]);
  // }
  // Serial.println("***********Ventas Formas de Pago**************");
  // for(int i=0; i<CantMaxImpPago; i++){
  //   Serial.printf("Para el Tipo de Impuesto %d el BI es: %lld\n", i+1, stcG_XyZ.stFPVentas.ui64FormasPago[i]);
  // }
  // imprimeReporteX();
  changeState(State::stateInicial);
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado Imprimir el Reporte Z
//         Maquina de Estados
//*****************************************************************************
void outputGenImpRepZ(void) {
#ifdef useDebug
  Serial.println("\nOutput Reporte Z");
#endif
  memset(&stcG_XyZ, 0, sizeof(stcG_XyZ));
  char dirFileRepX[MaxCarNameFile];  //Nombre Archivo para Almacenar el Reporte X
  char fechaFile[20];
  strncpy(dirFileRepX, DirReporte, MaxCarNameFile);
  if (mySD_eMMC.boExistDir(SD_MMC, dirFileRepX)) {
    Serial.println("No Existe Reporte X");
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return;
  }
  strncat(dirFileRepX, DocReporteSENIAT, MaxCarNameFile);
  strncat(dirFileRepX, ExtensionFileSENIAT, MaxCarNameFile);
  if (mySD_eMMC.boExistFile(SD_MMC, dirFileRepX) == ERROR) {
    Serial.print("ERROR abriendo Archvo en la SD");
    ///Falta ver que sehace con este tipo de error
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return;
  }
  mySD_eMMC.strReadFileData(SD_MMC, dirFileRepX, (uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ));
  fImprimirReporteZ();
  strncpy(dirFileRepX, DirReporte, MaxCarNameFile);
  if (mySD_eMMC.boExistDir(SD_MMC, dirFileRepX)) {
    Serial.println("No Existe Reporte X");
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return;
  }
  //sprintf(fechaFile, "%s%04u%02u%02u%s", DocReporteSENIAT, timeRTC.Year(), timeRTC.Month(), timeRTC.Day(), ExtensionFileSENIAT);
  strncat(dirFileRepX, DocReporteSENIAT, MaxCarNameFile);
  strncat(dirFileRepX, ExtensionFileSENIAT, MaxCarNameFile);
  if (mySD_eMMC.boExistFile(SD_MMC, dirFileRepX) == ERROR) {
    Serial.print("ERROR abriendo Archvo en la SD");
    ///Falta ver que sehace con este tipo de error
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return;
  }
  //Se Almacena la Estructura en la eMMC
  mySD_eMMC.boWriteFile(SD_MMC, dirFileRepX, (uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ));






  //changeState(State::stateInicial);
  return;
}

//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado Get timer de la
//         Maquina de Estados
//*****************************************************************************
void outputGetTimer(void) {
  RtcDateTime timeRTC;
  timeRTC = rtc1302.GetDateTime();
  if (doc["data"] == 1) {
    Serial.println("Se enviara RTC pro Fecha y Hora");
    char datestring[20];
    snprintf_P(datestring,
               countof(datestring),
               PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
               timeRTC.Month(),
               timeRTC.Day(),
               timeRTC.Year(),
               timeRTC.Hour(),
               timeRTC.Minute(),
               timeRTC.Second());
    strncpy(stcG_CmdTxJson.chrG_DataTx, datestring, CantMaxDataRx);
  } else {
    uint32_t timeEpoch = myFunciones.timeToUnix(timeRTC);
    stcG_CmdTxJson.ui32G_DataTx = timeEpoch;
  }
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Estado Set timer de la
//         Maquina de Estados
//*****************************************************************************
void outputSetTimer(void) {
  Serial.println("Funcion outputSetTimer");
  JsonObject data = doc["data"];
  RtcDateTime timeRTC;

  if (data["epoch"]) {
    uint32_t dataEpoch = data["epoch"];
    //Serial.printf("Data Epoch : %d\n",dataEpoch);
    //rtc.setTime(data);
    int anno = year(dataEpoch);
    int mes = month(dataEpoch);
    int dia = day(dataEpoch);
    int hora = hour(dataEpoch);
    int min = minute(dataEpoch);
    int seg = second(dataEpoch);
    timeRTC = RtcDateTime(anno, mes, dia, hora, min, seg);
    rtc1302.SetDateTime(timeRTC);
    //rtc1302.setDateTime
    //Serial.printf("Tiempo Epoch Convertido: %d/%d/%d %d:%d:%d\n",dia,mes,anno,hora,min,seg);
  }
  if (data["anno"] && data["mes"] && data["dia"] && data["hora"] && data["min"] && data["seg"]) {
    //Serial.println("Rx Set Timer por Fecha y Dia 11111111");
    //Todos los Campos data["xxx"] deben ser > 0, si no es asi, no se ejecuta este if
    int anno = data["anno"];
    int mes = data["mes"];
    int dia = data["dia"];
    int hora = data["hora"];
    int min = data["min"];
    int seg = data["seg"];
    timeRTC = RtcDateTime(anno, mes, dia, hora, min, seg);
    Serial.printf("Datos Rx: %02d/%02d/%d %02d:%02d:%02d\n", dia, mes, anno, hora, min, seg);
    rtc1302.SetDateTime(timeRTC);
  }
}

//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Write SPIFF de la
//         Maquina de Estados
//*****************************************************************************
void outputWrSPIFF(void) {
  char chL_NameFile[32];
  String strL_Contenido;
  strncpy(chL_NameFile, "/", 32);
  if (doc["data"]["nameFile"]) {
    const char* nameFile = doc["data"]["nameFile"];
    strncat(chL_NameFile, nameFile, 32);
  } else {
    //byG_CodeTx = ErrorData;
    stcG_CmdTxJson.ui8CodeTx = ErrorData;
    stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
    return;
  }
  if (doc["data"]["contenido"]) {
    JsonObject data_contenido = doc["data"]["contenido"];
    serializeJson(data_contenido, strL_Contenido);
    Serial.println("El Json del Contenido es:");
    Serial.println(strL_Contenido);
  } else {
    //byG_CodeTx = ErrorData;
    stcG_CmdTxJson.ui8CodeTx = ErrorData;
    stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
    return;
  }
  if (writeFile(SPIFFS, chL_NameFile, strL_Contenido.c_str()) == ErrorYes) {
    //byG_CodeTx = ErrorSPIFF;
    stcG_CmdTxJson.ui8CodeTx = ErrorSPIFF;
    stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
    return;
  }
  return;
}
//*****************************************************************************
// Funcion para  actualizar lo que debe hacer el Read SPIFF de la
//         Maquina de Estados
//*****************************************************************************
void outputReSPIFF(void) {
  char chL_NameFile[32];
  String strL_Contenido;
  if (doc["data"]["nameFile"]) {
    const char* nameFile = doc["data"]["nameFile"];
    strncpy(chL_NameFile, "/", 32);
    strncat(chL_NameFile, nameFile, 32);
  } else {
    //byG_CodeTx = ErrorData;
    stcG_CmdTxJson.ui8CodeTx = ErrorData;
    stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
    return;
  }
  Serial.println("Leyendo Archivo desde la SPIFFS...");
  strL_Contenido = readFile(SPIFFS, chL_NameFile);
  if (strL_Contenido.length() == 0) {
    stcG_CmdTxJson.ui8CodeTx = ErrorSPIFF;
    stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
    return;
  }
  // Serial.println("Archivo Leido:");
  // Serial.println(strL_Contenido);
  //strL_Contenido.remove(0,1);
  //strL_Contenido.remove(strL_Contenido.length()-1,1);
  strncpy(stcG_CmdTxJson.chrG_DataTx, strL_Contenido.c_str(), CantMaxDataRx);
  return;
}  //*****************************************************************************
// Funcion para  Colocar a la Impresora como Nueva de Fabrica
//         Maquina de Estados
//*****************************************************************************
void outputInitPtr(void) {
  String strL_Contenido;
  if (doc["data"]) {
    const char* data_SerialPtr = doc["data"]["SerialPtr"];
    JsonObject data_contenido = doc["data"];
    //Para Almacenar tipo JSON en las Memoria eMMC y SPIFFS
    serializeJson(data_contenido, strL_Contenido);
    Serial.println("💾El Json del Contenido es:");
    Serial.println(strL_Contenido);
    //Para Almacenra en la FLASH
    memset(chG_SerialPtr, 0, sizeof(chG_SerialPtr));
    strncpy(chG_SerialPtr, data_SerialPtr, CantMaxSerialPtr);
    Serial.printf("0️⃣El Serial de la Ptr es: %s\n", chG_SerialPtr);
  }
  Serial.println("🔥Se Inicia Borrado de FLASH");
  SerialFlash.eraseAll();
  Serial.println("✔Finaliza Borrado de FLASH");
  SerialFlash.write(AddrIniSerPtr, chG_SerialPtr, sizeof(chG_SerialPtr));
  //Se inicaliza el Archivo en la eMMC para El Reporte X
  memset(&stcG_XyZ, 0, sizeof(stcG_XyZ));
  char dirFileRepX[MaxCarNameFile];
  strncpy(dirFileRepX, DirReporte, MaxCarNameFile);
  strncat(dirFileRepX, DocReporteSENIAT, MaxCarNameFile);
  strncat(dirFileRepX, ExtensionFileSENIAT, MaxCarNameFile);
  //Inicializa El Reporte X en el Archivo ReporteX.txt
  Serial.println("✅Se Inicia el Borrado del Reporte X");
  if (mySD_eMMC.boWriteFile(SD_MMC, dirFileRepX, (uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ)) == ERROR) {
    Serial.print("ERROR abriendo Archvo en la SD");
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return;
  }
  Serial.println("🛑Finaliza el Borrado del Reporte X");
  //Coloca el Serial de la Impresora en el Archivo serialPtreMMC.json
  strncpy(dirFileRepX, NameFileComprobareMMC, MaxCarNameFile);
  Serial.printf("✅Se Inicia escritura del archivo: %s\n", dirFileRepX);
  Serial.printf("✅El Contenido es: %s\n", strL_Contenido);
  Serial.println(strL_Contenido);
  if (mySD_eMMC.boWriteFile(SD_MMC, dirFileRepX, strL_Contenido.c_str()) == ERROR) {
    Serial.print("ERROR abriendo Archvo en la SD");
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    Serial.println("🛑ERROR Escribiendo en la eMMC");
    return;
  }
  Serial.println("🛑Finaliza Escritura del Archivo en la eMMC");
  //Coloca el Seral de la Imprsora en la Memoria SPIFFS

  //Se informa como esta el sistema de Archivo de la SPIFF
  //info fs_info;
  //SPIFFS.info(fs_info);
  Serial.printf("📀Total de Byte de la SPIFFS: %d bytes\n", SPIFFS.totalBytes());
  Serial.printf("📀Total de Byte Usados de la SPIFFS: %d bytes\n", SPIFFS.usedBytes());



  char chL_NameFile[MaxCarNameFile];
  strncpy(chL_NameFile, NameFileComprobareSPIFF, MaxCarNameFile);
  Serial.printf("✅Se Inicia escritura del archivo: %s\n", chL_NameFile);
  Serial.printf("✅El Contenido es: %s\n", strL_Contenido);
  Serial.println(strL_Contenido);
  // if(writeFile(SPIFFS,chL_NameFile, strL_Contenido.c_str()) == ERROR){
  //   //byG_CodeTx = ErrorSPIFF;
  //   stcG_CmdTxJson.ui8CodeTx = ErrorSPIFF;
  //   stcG_FlagFact.flagTxRespuesta = 1;  //Como se Rx Trama incompleta
  //   Serial.println("🛑ERROR Escribiendo en la SPIFF");
  //   return;
  // }
  File file = SPIFFS.open(chL_NameFile, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    stcG_CmdTxJson.ui8CodeTx = ErrorSPIFF;
    Serial.println("🛑ERROR Escribiendo en la SPIFF");
    return;
  }
  if (file.print(strL_Contenido)) {
    Serial.println("- file written");
    file.close();
  } else {
    Serial.println("- write failed");
    //file.close();
    stcG_CmdTxJson.ui8CodeTx = ErrorSPIFF;
    Serial.println("🛑ERROR Escribiendo en la SPIFF");
    file.close();
    return;
  }
  file.close();




  Serial.println("🛑Finaliza Escritura del Archivo en la eMMC");
  stcG_CmdTxJson.ui8CodeTx = ErrorNo;

  memset(chG_SerialPtr, 0, sizeof(chG_SerialPtr));
  SerialFlash.read(AddrIniSerPtr, chG_SerialPtr, sizeof(chG_SerialPtr));
  Serial.printf("💽El dato del Serisl Ptr Leido de la FLASH es: %s\n", chG_SerialPtr);

  return;
}
//*****************************************************************************
// Funcion para  Imprimir la Factura con el comando Fin de Factura
//         Esto se debe hacer al Recibir el Estado Fin de Factura
//*****************************************************************************
//La estructura de Impresion es la Siguiente:

uint8_t fImprimirFactura(void) {
#ifdef timeDebug
  long iniTime;
  iniTime = millis();
#endif
  ///Se Inicializa la Estructura del Indice
  memset(&stcG_Index.chNumCedRif, 0, 1);
  memset(&stcG_Index.ui32CedRif, 0, sizeof(stcG_Index.ui32CedRif));
  memset(&stcG_Index.ui32NumCon, 0, sizeof(stcG_Index.ui32NumCon));
  memset(&stcG_Index.ui32TimeStamp, 0, sizeof(stcG_Index.ui32TimeStamp));
  memset(&stcG_Index.ui32SizeFile, 0, sizeof(stcG_Index.ui32SizeFile));
  memset(&stcG_Index.ui32PosEnArch, 0, sizeof(stcG_Index.ui32PosEnArch));
  ////
  int i;
  chG_ImpPtr[0] = '\0';
  char chL_ArrayIni[CantMaxCarLin];  //Array para Justificar una lina en los extremmos
  char chL_ArrayFin[CantMaxCarLin];  //Array para Justificar una lina en los extremmos
  String strL_Linea;                 //String para linea de Factura Auxiliar
  //Se Verifica que existe el Archivo en la Memoria eMMC para almacenar la Factura
  //El Archivo se almacena en la carpeta "/Seniat"
  //El formato para el nombre del archivo es: "/MT_AAAAMMDD";
  //    Donde:  AAAA  es el Anno
  //            MM    es el Mes
  //            DD    es el Dia
  RtcDateTime timeRTC;
  timeRTC = rtc1302.GetDateTime();
//   char dirFileMT[MaxCarNameFile];  //Nombre Archivo para Almacenar la Factura en Curso
//   //Se copia el directorio **/Seniat**
//   strncpy(dirFileMT, DirSeniat, MaxCarNameFile);
// #ifdef useDebug
//   Serial.print("Directorio de Trabajo: ");
//   Serial.println(dirFileMT);
// #endif
// //Se verifica si existe el Directorio  **/Seniat**
// //Para almacenar la factura en curso
//   if (mySD_eMMC.boExistDir(SD_MMC, dirFileMT)) {
//     if (mySD_eMMC.boCreateDir(SD_MMC, dirFileMT)) {
//       ///OJO RETORNAR ERROR por MEMORIA de TRABAJO
//       Serial.println("Error Creando Directorio de Traajo del Seniat");
//       return (ERROR);
//     }
//   }
  //Se verifica si existe el Directorio  **/Reporte**
  // para almacenar el Reporte X
  char dirFileRepX[MaxCarNameFile];  //Nombre Archivo para Almacenar el Reporte X
  strncpy(dirFileRepX, DirReporte, MaxCarNameFile);
  if (mySD_eMMC.boExistDir(SD_MMC, dirFileRepX)) {
    if (mySD_eMMC.boCreateDir(SD_MMC, dirFileRepX)) {
      ///OJO RETORNAR ERROR por MEMORIA de TRABAJO
      Serial.println("Error Creando Directorio de Reportes del Seniat");
      return (ERROR);
    }
  }
  // #define ERROR     true
  // //Funciones que Retornan sin Errores
  //*****************************************************************************
  //Se genera el Archivo para Almacenar la Factura en Curso
  //    el Formato es /Seniat/DF_YYYYMMDD.txt
  //*****************************************************************************
//   char fechaFile[20];
//   char chL_Lineas[CantMaxCarLin + 1];
//   memset(chL_Lineas, 0, CantMaxCarLin + 1);
//   memset(chL_Lineas, '-', CantMaxCarLin);
//   sprintf(fechaFile, "%s%04u%02u%02u%s", DocFiscSENIAT, timeRTC.Year(), timeRTC.Month(), timeRTC.Day(), ExtensionFileSENIAT);
//   strncat(dirFileMT, fechaFile, MaxCarNameFile);
// // #ifdef useDebug
//   Serial.print("Archivo de Trabajo para Almacenar la Factura de la eMMC: ");
//   Serial.println(dirFileMT);
// // #endif
//   char chL_CabezaFileFac[CantMaxCarLin];
//   memset(chL_CabezaFileFac, 0, CantMaxCarLin);
//   // stcG_XyZ.stContadores.ui32CountFacDia++;
//   // stcG_XyZ.stContadores.ui32UltFact++;
//   // sprintf(chL_CabezaFileFac, "%s %08d\n", FacturaControl, ui32G_CountFact);
//   sprintf(chL_CabezaFileFac, "%s %08d\n", FacturaControl, stcG_XyZ.stContadores.ui32UltFact);
//   if (mySD_eMMC.boExistFile(SD_MMC, dirFileMT) == ERROR) {


//     if (mySD_eMMC.boWriteFile(SD_MMC, dirFileMT, chL_CabezaFileFac) == ERROR) {
//       Serial.print("ERROR abriendo Archvo en la SD");
//       ///Falta ver que sehace con este tipo de error
//       return (ERROR);
//     }
//   } else {
//     //********************Se Guarda en el Archivo*************************
//     if (mySD_eMMC.boAppendFile(SD_MMC, dirFileMT, chL_CabezaFileFac) == ERROR) {
//       Serial.print("ERROR abriendo Archvo en la SD");
//       ///Falta ver que sehace con este tipo de error
//       return (ERROR);
//     }
//   }
  //*****************************************************************************
  //Se genera el Archivo para Almacenar la El Reporte X
  //*****************************************************************************
  //Pruba Hay que Leer el Reporte Z
  memset(&stcG_XyZ, 0, sizeof(stcG_XyZ));
  //
  // sprintf(fechaFile, "%s%04u%02u%02u%s", DocReporteSENIAT, timeRTC.Year(), timeRTC.Month(), timeRTC.Day(), ExtensionFileSENIAT);
  // strncat(dirFileRepX, fechaFile, MaxCarNameFile);
  strncat(dirFileRepX, DocReporteSENIAT, MaxCarNameFile);
  strncat(dirFileRepX, ExtensionFileSENIAT, MaxCarNameFile);
  memset(chL_CabezaFileFac, 0, CantMaxCarLin);
  strncpy(chL_CabezaFileFac, "Abriendo", CantMaxCarLin);
  memset(&stcG_XyZ, 0, sizeof(stcG_XyZ));
  if (mySD_eMMC.boExistFile(SD_MMC, dirFileRepX) == ERROR) {
    //No existe el Archivo del Reporte Z, se Crea
    if (mySD_eMMC.boWriteFile(SD_MMC, dirFileRepX, (uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ)) == ERROR) {
      Serial.print("ERROR abriendo Archvo en la SD");
      ///Falta ver que sehace con este tipo de error
      return (ERROR);
    }
  }

  else {

    mySD_eMMC.strReadFileData(SD_MMC, dirFileRepX, (uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ));
  }

  //*****************************************************************************
  //Se Imprime el Titiulo Principal SENIAT
  //*****************************************************************************
  //La factura se va almcenando en la Variable "strL_GuardaFile" para ir Guardando
  //en el Archivo de trabajo de la Memoria eMMC, con el nombre antes establecido
  //strL_GuardaFile = "";               //Variable Inicializada en Blanco
  //Primera Linea que se Almacenat en la Variable chL_GuardaFile

  //La factura se va almcenando en la Variable "chG_ImpPtr" para luego enviarse
  //al puerto UAR de la Impresora Fiscal
  //Primera Linea que se Almacenat en la Variable chG_ImpPtr
  strncpy(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar y Bold
  strncat(chG_ImpPtr, ESCcmdBold, inCantMaxPsramProd);
  strncat(chG_ImpPtr, chG_tituloFactura, inCantMaxPsramProd);  //Titulo -> SENIAT y
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  strncat(chG_ImpPtr, chG_rifEmpresa, inCantMaxPsramProd);  //Titulo ->  RIF
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdNormal, inCantMaxPsramProd);  //Letra Normal

  //*****************************************************************************
  //Se Imprime Los Encabezados que vienen despues del Titulo y RIF
  //
  //*****************************************************************************
  //Ciclo para Almacerar la Estructura de Encabezados en las Variables
  //chL_GuardaFile y chG_ImpPtr

  for (int i = 0; i < byG_NroEncab; i++) {
    strncat(chG_ImpPtr, stcG_Encabezado[i].Encabezado, inCantMaxPsramProd);
    strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  }
  //Comando ESC para Justificar a la Izquierda
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);
  //*****************************************************************************
  //Se Imprime el Numero de Factura y la Fecha
  //Se imprime la Factura
  //*****************************************************************************
  //Se justifica la Factura y Fecha
  //Linea de:
  //  FACTURA #           00000001
  strncpy(chL_ArrayIni, FacturaNro, CantMaxCarLin);
  //sprintf(chL_ArrayFin, "%08d", ui32G_CountFact);
  stcG_XyZ.stContadores.ui32UltFact++;  //Incrementa el Numero de Factura
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltFact);
  //💾Se guarda Nro Factura para el Indice
  stcG_Index.ui32NumCon = stcG_XyZ.stContadores.ui32UltFact;
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Linea de:
  //  FECHA: DD/MM/AAAA           HORA: 23:30
  sprintf(chL_ArrayIni, "%s%02d/%02d/%4d", FacturaFecha, timeRTC.Day(), timeRTC.Month(), timeRTC.Year());
  sprintf(chL_ArrayFin, "%s%02d:%02d", FacturaHora, timeRTC.Hour(), timeRTC.Minute());
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se actualiza en la Estructura XyZ el Cantdor de Factura
  //stcG_XyZ.stContadores.ui32UltFact = ui32G_CountFact;
  //timeRTC = RtcDateTime(anno, mes, dia, hora, min, seg);
  RtcDateTime timeRTC_Conv = RtcDateTime(timeRTC.Year(), timeRTC.Month(), timeRTC.Day(), timeRTC.Hour(), timeRTC.Minute(), 0);
  stcG_XyZ.stContadores.ui32FechaUF = myFunciones.timeToUnix(timeRTC_Conv);
  //Se pasa la Fecha y Hora al Inice de Busqueda
  stcG_Index.ui32TimeStamp = stcG_XyZ.stContadores.ui32FechaUF;
  //stcG_XyZ.stContadores.ui32CountFacDia++;
  //*****************************************************************************
  //Se imprime Datos del Cliente
  //Se RIF/CI del Cliente
  //*****************************************************************************
  //Para ser almacenados en el Archivo de la eMMC
  //Se Almacena para enviar a la Impresora
  //Para ser almacenados para luego imprimirse en la Ptr
  //Se Centra el Titulo definido por "FacturaDatCli"
  //Si se Rx el Comando Cliente se impriem, de no Recibir el Comando no se Imprima Nada
  if (stcG_FlagFact.flagIndiClien) {
    strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);
    strncat(chG_ImpPtr, FacturaDatCli, inCantMaxPsramProd);
    //Justificar a la Izquierda
    strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);
    // Se almacena RIF/CI
    strncat(chG_ImpPtr, FacturaRifCi, inCantMaxPsramProd);
    strncat(chG_ImpPtr, stcG_Cliente.RifCi, inCantMaxPsramProd);
    strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
    // Se almacena Razon Social
    strncat(chG_ImpPtr, FacturaRazSoc, inCantMaxPsramProd);
    strncat(chG_ImpPtr, stcG_Cliente.RazSoc, inCantMaxPsramProd);
    strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
    //Lineas Adicionales del Cliente para las dos Variables
    if (byG_NroLinAdCli > 0) {
      for (int i = 0; i < byG_NroLinAdCli; i++) {
        //Archivo SD
        //Printer
        strncat(chG_ImpPtr, stcG_Cliente.LineaAd[i], inCantMaxPsramProd);
        strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
      }
    }
    //Se imprime/guarda lineas ----------------
    strncat(chG_ImpPtr, chL_Lineas, inCantMaxPsramProd);
    strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  }


  //*****************************************************************************
  //Se Almacena en SD e Almacena en el buffer que imprime en Ptr los Productos
  //*****************************************************************************
  //Se empieza almacenando en el Registro de la SD
  //uint16_t ui16L_AuxNroProducto;                        ///Variable Auxiliar para Almacenar en la Variable de chL_GuardaFile en grupo de 10 A3ticulos
  int intL_AuxNroProducto;      ///Variable Auxiliar para Almacenar en la Variable de chL_GuardaFile en grupo de 10 A3ticulos
  uint16_t ui16L_IncreDecimaI;  //Se incrementa cada Decima que se Resta ui16L_AuxNroProducto
  uint16_t ui16L_IncreDecimaF;  //Se incrementa cada Decima que se Resta ui16L_AuxNroProducto Pero si intL_AuxNroProducto < 10 se le sma este
  intL_AuxNroProducto = ui16G_NroProducto;
  ui16L_IncreDecimaI = 0;
  ui16L_IncreDecimaF = 0;
  for (int i = 0; i < ui16G_NroProducto; i++) {
    //La cantidad es diferente a 1, Se imprime la Descripcion del Producto en lineas y la Cantidad x Precio en otra Linea
    //Se almacena Descripcion 1
    //Se Multiplica Cantidad por Precio
    //stcG_Productos[i].ui64G_CantXPrec = myFunciones.multPrecioPorCant(stcG_Productos[i].ui64G_Precio, stcG_Productos[i].ui64G_Cantidad);
    //Se ve que tipo de Impuesto Graba
    uint8_t uiL_AuxTipo = stcG_Productos[i].ui8G_tipoImp;
    //Se obtiene la Parte entera y los dos digitos decimales
    //Almacena Descripcion 1
    if (stcG_Productos[i].ui64G_Anulacion > 0) {
      strncat(chG_ImpPtr, Anulacion, inCantMaxPsramProd);
    }
    strncat(chG_ImpPtr, stcG_Productos[i].Descripcion1, inCantMaxPsramProd);
    strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
    //Almacena Descripcion 2 si existe
    if (strlen(stcG_Productos[i].Descripcion2) > 0) {
      //Se almacena Descripcion 2
      strncat(chG_ImpPtr, stcG_Productos[i].Descripcion2, inCantMaxPsramProd);
      strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
    }
    //Almacena Descripcion 3 si existe
    if (strlen(stcG_Productos[i].Descripcion3) > 0) {
      //Se almacena Descripcion 3
      strncat(chG_ImpPtr, stcG_Productos[i].Descripcion3, inCantMaxPsramProd);
      strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
    }
    //Almacena la Linea de Cantidad por Precio en el Array inicio
    if (stcG_Productos[i].ui64G_Cantidad % DecCant) {  //Si > 0 -> Cantidad tiene Decimales
      sprintf(chL_ArrayIni, "  %d.%d%d%d x %lld.%lld%lld",
              VE3(stcG_Productos[i].ui64G_Cantidad), DC(stcG_Productos[i].ui64G_Cantidad), DD(stcG_Productos[i].ui64G_Cantidad), DU(stcG_Productos[i].ui64G_Cantidad),
              VE2(stcG_Productos[i].ui64G_Precio), DD(stcG_Productos[i].ui64G_Precio), DU(stcG_Productos[i].ui64G_Precio));
    } else {
      sprintf(chL_ArrayIni, "  %d x %lld.%lld%lld",
              VE3(stcG_Productos[i].ui64G_Cantidad),
              VE2(stcG_Productos[i].ui64G_Precio), DD(stcG_Productos[i].ui64G_Precio), DU(stcG_Productos[i].ui64G_Precio));
    }
    if (stcG_Productos[i].ui64G_Anulacion > 0) {
      sprintf(chL_ArrayFin, "%s %s -%lld.%lld%lld", stcG_ImpuestoArticulos[uiL_AuxTipo - 1].chG_Abreviatura, chG_SimMoneda,
              VE2(stcG_Productos[i].ui64G_Anulacion), DD(stcG_Productos[i].ui64G_Anulacion), DU(stcG_Productos[i].ui64G_Anulacion));
      //Se acumula Para el Reporte XyZ
      stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Anulaciones += stcG_Productos[i].ui64G_Anulacion;
    } else {
      sprintf(chL_ArrayFin, "%s %s %lld.%lld%lld", stcG_ImpuestoArticulos[uiL_AuxTipo - 1].chG_Abreviatura, chG_SimMoneda,
              VE2(stcG_Productos[i].ui64G_CantXPrec), DD(stcG_Productos[i].ui64G_CantXPrec), DU(stcG_Productos[i].ui64G_CantXPrec));
      //Se acumula Para el Reporte XyZ
      //stcG_XyZ.stBIVentas.ui64_BI_Impuesto[uiL_AuxTipo - 1] += stcG_Productos[i].ui64G_CantXPrec;
    }
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    //strL_Linea += '\n';
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    /////Si posee Descuento por Porcentaje se imprime despues del Producto
    if (stcG_Productos[i].ui64G_DescPorc > 0) {
      sprintf(chL_ArrayIni, "%s (%lld.%lld%lld%%)", Descuento,
              VE2(stcG_Productos[i].ui64G_DescPorc),
              DD(stcG_Productos[i].ui64G_DescPorc),
              DU(stcG_Productos[i].ui64G_DescPorc));
      uint64_t ui64L_Des = myFunciones.calculoImpuesto(stcG_Productos[i].ui64G_CantXPrec, stcG_Productos[i].ui64G_DescPorc);
      sprintf(chL_ArrayFin, "%s -%lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_Des), DD(ui64L_Des), DU(ui64L_Des));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //Se guarda en la Variable para la Impresora
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      //Se acumula Para el Reporte XyZ
      stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Descuento += ui64L_Des;
    }
    //Si posee Descuento por Monto se imprime despues del Producto
    else if (stcG_Productos[i].ui64G_DescMonto > 0) {
      sprintf(chL_ArrayIni, "%s", Descuento);
      sprintf(chL_ArrayFin, "%s -%lld.%lld%lld", chG_SimMoneda,
              VE2(stcG_Productos[i].ui64G_DescMonto), DD(stcG_Productos[i].ui64G_DescMonto), DU(stcG_Productos[i].ui64G_DescMonto));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //Se guarda en la Variable para la Impresora
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      //Se acumula Para el Reporte XyZ
      stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Descuento += stcG_Productos[i].ui64G_DescMonto;
    }
    /////Si posee Recargo por Porcentaje se imprime despues del Producto
    if (stcG_Productos[i].ui64G_RecPorc > 0) {
      Serial.println("🛑Imprime Recargo por Porcentaje");
      sprintf(chL_ArrayIni, "%s (%lld.%lld%lld%%)", Recargo,
              VE2(stcG_Productos[i].ui64G_RecPorc),
              DD(stcG_Productos[i].ui64G_RecPorc),
              DU(stcG_Productos[i].ui64G_RecPorc));
      uint64_t ui64L_Rec = myFunciones.calculoImpuesto(stcG_Productos[i].ui64G_CantXPrec, stcG_Productos[i].ui64G_RecPorc);
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_Rec), DD(ui64L_Rec), DU(ui64L_Rec));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //Se guarda en la Variable para la Impresora
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      //Se acumula Para el Reporte XyZ
      stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Recargo += ui64L_Rec;
    }
    //Si posee Recargo por Monto se imprime despues del Producto
    else if (stcG_Productos[i].ui64G_RecMonto > 0) {
      Serial.println("🛑Imprime Recargo por Monto");
      sprintf(chL_ArrayIni, "%s", Recargo);
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(stcG_Productos[i].ui64G_RecMonto), DD(stcG_Productos[i].ui64G_RecMonto), DU(stcG_Productos[i].ui64G_RecMonto));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //Se guarda en la Variable para la Impresora
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      //Se acumula Para el Reporte XyZ
      stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Recargo += stcG_Productos[i].ui64G_RecMonto;
    }
    //Si posee Correccion del Producto se coloca la Palabra CORREECCION y de Imprime el Producto con - Precio
    if (stcG_Productos[i].ui64G_Correccion > 0) {
      //stcG_Productos[i].ui64G_Correccion = stcG_Productos[i].ui64G_CantXPrec;
      ///////////////////////////////////////////////////////////////////////
      //Serial.println("✅En la Ptr se Envia la Correecion del Producto...");
      ///////////////////////////////////////////////////////////////////////
      strncat(chG_ImpPtr, Correccion, inCantMaxPsramProd);
      strncat(chG_ImpPtr, stcG_Productos[i].Descripcion1, inCantMaxPsramProd);
      strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
      //Almacena Descripcion 2 si existe
      if (strlen(stcG_Productos[i].Descripcion2) > 0) {
        //Se almacena Descripcion 2
        strncat(chG_ImpPtr, stcG_Productos[i].Descripcion2, inCantMaxPsramProd);
        strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
      }
      //Almacena Descripcion 3 si existe
      if (strlen(stcG_Productos[i].Descripcion3) > 0) {
        //Se almacena Descripcion 3
        strncat(chG_ImpPtr, stcG_Productos[i].Descripcion3, inCantMaxPsramProd);
        strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
      }
      //Almacena la Linea de Cantidad por Precio en el Array inicio
      if (stcG_Productos[i].ui64G_Cantidad % DecCant) {  //Si > 0 -> Cantidad tiene Decimales
        sprintf(chL_ArrayIni, "  %d.%d%d%d x %lld.%lld%lld",
                VE3(stcG_Productos[i].ui64G_Cantidad), DC(stcG_Productos[i].ui64G_Cantidad), DD(stcG_Productos[i].ui64G_Cantidad), DU(stcG_Productos[i].ui64G_Cantidad),
                VE2(stcG_Productos[i].ui64G_Precio), DD(stcG_Productos[i].ui64G_Precio), DU(stcG_Productos[i].ui64G_Precio));
      } else {
        sprintf(chL_ArrayIni, "  %d x %lld.%lld%lld",
                VE3(stcG_Productos[i].ui64G_Cantidad),
                VE2(stcG_Productos[i].ui64G_Precio), DD(stcG_Productos[i].ui64G_Precio), DU(stcG_Productos[i].ui64G_Precio));
      }
      sprintf(chL_ArrayFin, "%s %s -%lld.%lld%lld", stcG_ImpuestoArticulos[uiL_AuxTipo - 1].chG_Abreviatura, chG_SimMoneda,
              VE2(stcG_Productos[i].ui64G_CantXPrec), DD(stcG_Productos[i].ui64G_CantXPrec), DU(stcG_Productos[i].ui64G_CantXPrec));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      //Se acumula Para el Reporte XyZ
      stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Correcciones += stcG_Productos[i].ui64G_CantXPrec;
    }
    // else if(stcG_Productos[i].ui64G_Anulacion > 0){
    //   //stcG_Productos[i].ui64G_Anulacion = stcG_Productos[i].ui64G_CantXPrec;
    //   //Por Verificar
    //   strncat(chG_ImpPtr, Anulacion, inCantMaxPsramProd);
    //   strncat(chG_ImpPtr, stcG_Productos[i].Descripcion1, inCantMaxPsramProd);
    //   strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
    //   //Almacena Descripcion 2 si existe
    //   if (strlen(stcG_Productos[i].Descripcion2) > 0) {
    //     //Se almacena Descripcion 2
    //     strncat(chG_ImpPtr, stcG_Productos[i].Descripcion2, inCantMaxPsramProd);
    //     strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
    //   }
    //   //Almacena Descripcion 3 si existe
    //   if (strlen(stcG_Productos[i].Descripcion3) > 0) {
    //     //Se almacena Descripcion 3
    //     strncat(chG_ImpPtr, stcG_Productos[i].Descripcion3, inCantMaxPsramProd);
    //     strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
    //   }
    //   //Almacena la Linea de Cantidad por Precio en el Array inicio
    //   if (stcG_Productos[i].ui64G_Cantidad % DecCant) {  //Si > 0 -> Cantidad tiene Decimales
    //     sprintf(chL_ArrayIni, "  %d.%d%d%d x %lld.%lld%lld",
    //             VE3(stcG_Productos[i].ui64G_Cantidad), DC(stcG_Productos[i].ui64G_Cantidad), DD(stcG_Productos[i].ui64G_Cantidad), DU(stcG_Productos[i].ui64G_Cantidad),
    //             VE2(stcG_Productos[i].ui64G_Precio), DD(stcG_Productos[i].ui64G_Precio), DU(stcG_Productos[i].ui64G_Precio));
    //   } else {
    //     sprintf(chL_ArrayIni, "  %d x %lld.%lld%lld",
    //             VE3(stcG_Productos[i].ui64G_Cantidad),
    //             VE2(stcG_Productos[i].ui64G_Precio), DD(stcG_Productos[i].ui64G_Precio), DU(stcG_Productos[i].ui64G_Precio));
    //   }
    //   sprintf(chL_ArrayFin, "%s %s -%lld.%lld%lld", stcG_ImpuestoArticulos[uiL_AuxTipo - 1].chG_Abreviatura, chG_SimMoneda,
    //           VE2(stcG_Productos[i].ui64G_CantXPrec), DD(stcG_Productos[i].ui64G_CantXPrec), DU(stcG_Productos[i].ui64G_CantXPrec));
    //   strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    //   //strL_Linea += '\n';
    //   strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);




    // }
  }
  //*****************************************************************************
  //Calculo de Total del Impuesto segun referencia de la estructura
  //    "ImpuestoArticulos"
  //El total Acumulado por cada item de Impuesto por los Productos se
  //  Almacenara en la Estructura
  //    "TotalImpuestoArtiulos"
  //Para esto se hace un ciclo FOR muestreando que Producto Graba impuesto
  //*****************************************************************************
  //Se inicalizan los Acumuladores del Total de los Impuestos
  // for (int i = 0; i < ui8G_CountImpArt; i++) {
  //   stcG_TotalImpuestoArtiulos[i].ui64G_IVA_Monto = 0;
  //   stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto = 0;
  //   strncpy(stcG_TotalImpuestoArtiulos[i].impMontoPtr, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarConf);
  //   strncpy(stcG_TotalImpuestoArtiulos[i].impMontoImp, stcG_ImpuestoArticulos[i].impMontoImp, CantMaxCarConf);
  // }
  //Ciclo Para Sumar Los Montos da cada Impuesto, por ejemplo, se suman todos los montos
  //  con y se acumula en:  stcG_TotalImpuestoArtiulos[i].ui64G_MontoImp
  /////**************************************************************************
  //Este calculo se pasa a los OUTPUT, para ir llevando los acumulados
  ////***************************************************************************
  /*Se va a probar llevando acumulado en los OUTPUT
  for (int i = 0; i < ui16G_NroProducto; i++) {
    //Se verifica que tipo de Impuesto Graba el (i) Producto
    //El indexado corresponde al (i-1) tipo de Impuesto en la Estructura
    //  "ImpuestoArticulos"
    uint8_t uiL_AuxTipo = stcG_Productos[i].ui8G_tipoImp;
    // stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto += stcG_Productos[i].ui64G_CantXPrec;
    if (stcG_Productos[i].ui64G_DescPorc > 0) {
      //stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto -= myFunciones.calculoImpuesto(stcG_Productos[i].ui64G_CantXPrec, stcG_Productos[i].ui64G_DescPorc);
    } else if (stcG_Productos[i].ui64G_DescMonto > 0) {
      //stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto -= stcG_Productos[i].ui64G_DescMonto;
    }
    if (stcG_Productos[i].ui64G_RecPorc > 0) {
      //stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto += myFunciones.calculoImpuesto(stcG_Productos[i].ui64G_CantXPrec, stcG_Productos[i].ui64G_RecPorc);
    } else if (stcG_Productos[i].ui64G_RecMonto > 0) {
      //stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto += stcG_Productos[i].ui64G_RecMonto;
    }
    if (stcG_Productos[i].ui64G_Correccion > 0) {
      //stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto -= stcG_Productos[i].ui64G_Correccion;
      //Serial.println("✅Al Producto se Descuenta la Correccion");
    }
    if (stcG_Productos[i].ui64G_Anulacion > 0){
      //stcG_TotalImpuestoArtiulos[uiL_AuxTipo - 1].ui64G_BI_Monto -= stcG_Productos[i].ui64G_Anulacion;
    }
  }
  */
  //Ciclo para calcular el monto del Impuesto segun el acumulado Anterior
  //  se deposita en : stcG_TotalImpuestoArtiulos[].ui64G_MontoPorc
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      stcG_TotalImpuestoArtiulos[i].ui64G_IVA_Monto = myFunciones.calculoImpuesto(stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto, stcG_ImpuestoArticulos[i].valorImp);
    }
  }

  //*****************************************************************************
  //Se imprimen la Suma de los Impuestos de Totalizados de los Productos
  //  calculados Anteriosmente
  //*****************************************************************************
  //Se Almacena en la Variable para escribir en la Memora SD
  //Se imprime/guarda lineas ----------------
  //Se almacena en la Variable para imprimir
  strncat(chG_ImpPtr, chL_Lineas, inCantMaxPsramProd);
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    if (stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoPtr, CantMaxCarLin);
      // sprintf(chL_ArrayFin, "%s %lld.%d%d", chG_SimMoneda,
      //   stcG_TotalImpuestoArtiulos[i].ui64G_MontoImp/100,
      //   (stcG_TotalImpuestoArtiulos[i].ui64G_MontoImp % 100)/10,
      //   stcG_TotalImpuestoArtiulos[i].ui64G_MontoImp % 10);
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto),
              DD(stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto),
              DU(stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      //Variable para Memoria SD
      //Variable para Ptr
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    }
    if (stcG_TotalImpuestoArtiulos[i].ui64G_IVA_Monto > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(stcG_TotalImpuestoArtiulos[i].ui64G_IVA_Monto),
              DD(stcG_TotalImpuestoArtiulos[i].ui64G_IVA_Monto),
              DU(stcG_TotalImpuestoArtiulos[i].ui64G_IVA_Monto));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      //Variable para Memoria SD
      //Variable para Ptr
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    }
  }
  //Se almacena en la Variable para imprimir
  strncat(chG_ImpPtr, chL_Lineas, inCantMaxPsramProd);
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  //*****************************************************************************
  //Se imprimen la Suma de los Productos de cada tipo de Impuesto y el calculo
  //  de cada Impuesto
  //*****************************************************************************
  //Se totaliza este Monto
  // uint64_t ui64L_SubTotal;
  // ui64L_SubTotal = 0;
  //*************************************************************************
  ui64L_SubTotalFact = 0;
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    ui64L_SubTotalFact += stcG_TotalImpuestoArtiulos[i].ui64G_IVA_Monto;
    ui64L_SubTotalFact += stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto;
    stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i] += stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto;
    stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i] += stcG_TotalImpuestoArtiulos[i].ui64G_BI_Descuento;
    stcG_XyZ.stRecargos.ui64_BI_Impuesto[i] += stcG_TotalImpuestoArtiulos[i].ui64G_BI_Recargo;
    stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i] += stcG_TotalImpuestoArtiulos[i].ui64G_BI_Correcciones;
    stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i] += stcG_TotalImpuestoArtiulos[i].ui64G_BI_Anulaciones;
    // stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i] += stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto;
    // stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i] += stcG_TotalImpuestoArtiulos[i].ui64G_BI_Monto;
  }
  strncpy(chL_ArrayIni, FacturaSubTotal, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_SubTotalFact),
          DD(ui64L_SubTotalFact),
          DU(ui64L_SubTotalFact));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  //Variable para Memoria SD
  //Variable para Ptr
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Lineas --------
  //Variable para Memoria SD
  //Se almacena en la Variable para imprimir
  strncat(chG_ImpPtr, chL_Lineas, inCantMaxPsramProd);
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  //*****************************************************************************
  //Se imprimen el Impuesto Especial, La Forma de Pago, que en estos momentos es
  //    IGTF
  //  de cada Impuesto
  //*****************************************************************************
  //Se debe tener las diferentes formas de Pago obtenidas
  //Si ui8G_CountFormPagoRx == 0 Se Inicializa la Forma de Pago tipo 1
  //Se Almacena la Informacion en las Variables de Archivo SD e Impresora
  strncat(chG_ImpPtr, chG_TituloFormPago, inCantMaxPsramProd);
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  ui8G_CountFormPagoTotal = 0;
  if (ui8G_CountFormPagoRx == 0) {
    stcG_TotalImpuestoFormPago[ui8G_CountFormPagoTotal].ui64G_MontoImpEx = 0;
    stcG_TotalImpuestoFormPago[ui8G_CountFormPagoTotal].ui64G_MontoImpLo = ui64L_SubTotalFact;
    stcG_TotalImpuestoFormPago[ui8G_CountFormPagoTotal].ui32G_TasaCambio = 0;
    strncpy(stcG_TotalImpuestoFormPago[ui8G_CountFormPagoTotal].impMontoPtr, stcG_FormasPagoImp[0].chG_descripcion, CantMaxCarLin);
    if (stcG_FormasPagoImp[0].ui16G_ivaPorc > 0) {
      stcG_TotalImpuestoFormPago[ui8G_CountFormPagoTotal].ui64G_IVA_Monto = myFunciones.calculoImpuesto(ui64L_SubTotalFact, stcG_FormasPagoImp[0].ui16G_ivaPorc);
      //strncpy(stcG_TotalImpuestoFormPago[ui8G_CountFormPagoTotal].impMontoPtr, stcG_FormasPagoImp[0].impMontoPtr, CantMaxCarConf);
      strncpy(stcG_TotalImpuestoFormPago[ui8G_CountFormPagoTotal].impMontoImp, stcG_FormasPagoImp[0].chG_impBI_Monto, CantMaxCarConf);
    }
    strncpy(stcG_TotalImpuestoFormPago[i].impFormaPago, stcG_FormasPagoImp[0].chG_descripcion, CantMaxCarLin);
    ui8G_CountFormPagoTotal++;
    //Se almacena en la Estructura XyZ
    stcG_XyZ.stFPVentas.ui64FormasPago[0] += ui64L_SubTotalFact;
  } else {
    //
    Serial.println("Ver lo que se Almacena en la Estructura stcG_TotalImpuestoFormPago");
    Serial.println("******************************************************************");
    //
    for (int i = 0; i < ui8G_CountFormPagoRx; i++) {
      uint8_t tipoFPRx = stcG_FormasPagoRxCMD[i].ui8_Tipo;
      strncpy(stcG_TotalImpuestoFormPago[i].impFormaPago, stcG_FormasPagoImp[tipoFPRx - 1].chG_descripcion, CantMaxCarLin);
      //Si se cumple la siguiente condicion indica que monto en (-1), por lo tanto todo es cancelado con esta forma de PAgo
      if (stcG_FormasPagoRxCMD[i].ui32_TasaConv > 0 && stcG_FormasPagoRxCMD[i].i64_Monto < 0) {
        stcG_TotalImpuestoFormPago[i].ui64G_MontoImpEx = myFunciones.convLo2Ex(ui64L_SubTotalFact, stcG_FormasPagoRxCMD[i].ui32_TasaConv);
        stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo = ui64L_SubTotalFact;
        stcG_TotalImpuestoFormPago[i].ui32G_TasaCambio = stcG_FormasPagoRxCMD[i].ui32_TasaConv;
      } else if (stcG_FormasPagoRxCMD[i].ui32_TasaConv > 0 && stcG_FormasPagoRxCMD[i].i64_Monto > 0) {
        stcG_TotalImpuestoFormPago[i].ui64G_MontoImpEx = stcG_FormasPagoRxCMD[i].i64_Monto;
        stcG_TotalImpuestoFormPago[i].ui32G_TasaCambio = stcG_FormasPagoRxCMD[i].ui32_TasaConv;
        stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo = myFunciones.convEx2Lo(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpEx, stcG_TotalImpuestoFormPago[i].ui32G_TasaCambio);
      } else if (stcG_FormasPagoRxCMD[i].ui32_TasaConv == 0 && stcG_FormasPagoRxCMD[i].i64_Monto < 0) {
        stcG_TotalImpuestoFormPago[i].ui64G_MontoImpEx = 0;
        stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo = ui64L_SubTotalFact;
        stcG_TotalImpuestoFormPago[i].ui32G_TasaCambio = 0;
      } else if (stcG_FormasPagoRxCMD[i].ui32_TasaConv == 0 && stcG_FormasPagoRxCMD[i].i64_Monto > 1) {
        stcG_TotalImpuestoFormPago[i].ui64G_MontoImpEx = 0;
        stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo = stcG_FormasPagoRxCMD[i].i64_Monto;
        stcG_TotalImpuestoFormPago[i].ui32G_TasaCambio = 0;
      }
      if (stcG_FormasPagoImp[tipoFPRx - 1].ui16G_ivaPorc > 0) {
        stcG_TotalImpuestoFormPago[i].ui64G_IVA_Monto = myFunciones.calculoImpuesto(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo, stcG_FormasPagoImp[tipoFPRx - 1].ui16G_ivaPorc);
        strncpy(stcG_TotalImpuestoFormPago[i].impMontoPtr, stcG_FormasPagoImp[tipoFPRx - 1].chG_impBI_Monto, CantMaxCarConf);
        strncpy(stcG_TotalImpuestoFormPago[i].impMontoImp, stcG_FormasPagoImp[tipoFPRx - 1].chG_impIVA_Porc, CantMaxCarConf);
      } else {
        stcG_TotalImpuestoFormPago[i].ui64G_IVA_Monto = 0;
      }
      //Serial.printf("Titulo de la Forma de Pago[%d] = %s\n", i, stcG_TotalImpuestoFormPago[i].impFormaPago);
      if (strlen(stcG_TotalImpuestoFormPago[i].impMontoPtr) > 0) {
        //Serial.printf("Titulo el Monto de la Forma de Pago[%d] = %s\n", i, stcG_TotalImpuestoFormPago[i].impMontoPtr);
      }
      if (strlen(stcG_TotalImpuestoFormPago[i].impMontoImp) > 0) {
        //Serial.printf("Titulo el Impuesto de la Forma de Pago[%d] = %s\n", i, stcG_TotalImpuestoFormPago[i].impMontoImp);
      }
      // Serial.printf("El Monto en Moneda local de [%d] = %lld\n", i, stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo);
      // Serial.printf("El Monto en Moneda Extranjera de [%d] = %lld\n", i, stcG_TotalImpuestoFormPago[i].ui64G_MontoImpEx);
      // Serial.printf("La Tasa de Cambio de [%d] = %d\n", i, stcG_TotalImpuestoFormPago[i].ui32G_TasaCambio);
      // Serial.printf("El Monto del Impuesto que paga de [%d] = %lld\n", i, stcG_TotalImpuestoFormPago[i].ui64G_IVA_Monto);
      stcG_XyZ.stFPVentas.ui64FormasPago[tipoFPRx - 1] += stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo;
    }
    ui8G_CountFormPagoTotal = ui8G_CountFormPagoRx;
  }
  //Se inicialia el Sub Total por Impuesto de Forma de Pago
  ui64L_SubTotalImpFormPago = 0;
  for (int i = 0; i < ui8G_CountFormPagoTotal; i++) {
    if (stcG_TotalImpuestoFormPago[i].ui64G_MontoImpEx > 0) {
      //Se almacena en la Variable para imprimir
      strncat(chG_ImpPtr, stcG_TotalImpuestoFormPago[i].impFormaPago, inCantMaxPsramProd);
      strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
      sprintf(chL_ArrayIni, "  %lld.%lld%lldx%d.%d%d",
              VE2(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpEx),
              DD(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpEx),
              DU(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpEx),
              VE2(stcG_TotalImpuestoFormPago[i].ui32G_TasaCambio),
              DD(stcG_TotalImpuestoFormPago[i].ui32G_TasaCambio),
              DU(stcG_TotalImpuestoFormPago[i].ui32G_TasaCambio));
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo),
              DD(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo),
              DU(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //Variable para Memoria SD
      //Variable para Ptr
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      //stcG_XyZ.stFPVentas.ui64FormasPago[i] = stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo;
      //Serial.printf("La Forma de Pago %d, tiene un valor de %lld", i, stcG_XyZ.stFPVentas.ui64FormasPago[i]);
    } else {
      sprintf(chL_ArrayIni, "%s", stcG_TotalImpuestoFormPago[i].impFormaPago);
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo),
              DD(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo),
              DU(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //Variable para Memoria SD
      //Variable para Ptr
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      //stcG_XyZ.stFPVentas.ui64FormasPago[i] = stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo;
      //Serial.printf("La Forma de Pago %d, tiene un valor de %lld", i, stcG_XyZ.stFPVentas.ui64FormasPago[i]);
    }
    if (stcG_TotalImpuestoFormPago[i].ui64G_IVA_Monto > 0) {
      ui64L_SubTotalImpFormPago += stcG_TotalImpuestoFormPago[i].ui64G_IVA_Monto;
      //Ejemplo, Imprime Monto del Impesto a Grabr "BI G IGTF (3.00%)  Bs. Monto"
      sprintf(chL_ArrayIni, "%s", stcG_TotalImpuestoFormPago[i].impMontoPtr);
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo),
              DD(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo),
              DU(stcG_TotalImpuestoFormPago[i].ui64G_MontoImpLo));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //Variable para Memoria SD
      //Variable para Ptr
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      //Ejemplo, Imprime el Impuesto del Monto del Impesto a Grabr "IGTF (3.00%)  Bs. Impuesto"
      sprintf(chL_ArrayIni, "%s", stcG_TotalImpuestoFormPago[i].impMontoImp);
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(stcG_TotalImpuestoFormPago[i].ui64G_IVA_Monto),
              DD(stcG_TotalImpuestoFormPago[i].ui64G_IVA_Monto),
              DU(stcG_TotalImpuestoFormPago[i].ui64G_IVA_Monto));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //Variable para Ptr
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    }
  }
  //Lineas --------
  //Variable para Memoria SD
  //Se almacena en la Variable para imprimir
  strncat(chG_ImpPtr, chL_Lineas, inCantMaxPsramProd);
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  //*****************************************************************************
  //Se imprimen el TOTAL del Monto de la Factura
  //    SUB-TOTAL + IGTF(si existe)
  //
  //*****************************************************************************
  //Se imprime el Total a cancelar en la Factura
  //Se suma el Sub Total y Todos los Impuestos de la Forma de Pago
  ui64L_TotalMontoFactura = ui64L_SubTotalFact + ui64L_SubTotalImpFormPago;
  sprintf(chL_ArrayIni, "%s", FacturaTotal);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_TotalMontoFactura),
          DD(ui64L_TotalMontoFactura),
          DU(ui64L_TotalMontoFactura));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Variable para Ptr
  //Negritas
  strncat(chG_ImpPtr, ESCcmdBold, inCantMaxPsramProd);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Letra sin Negritas
  strncat(chG_ImpPtr, ESCcmdNormal, inCantMaxPsramProd);  //Letra Normal
  //Lineas --------
  //Variable para Memoria SD
  //Se almacena en la Variable para imprimir
  strncat(chG_ImpPtr, chL_Lineas, inCantMaxPsramProd);
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  //*****************************************************************************
  //
  //  Se imprimen Las Lineas Adicionales si Existen
  //
  //*****************************************************************************
  if (byG_NroLinAdicionales > 0) {
    for (int i = 0; i < byG_NroLinAdicionales; i++) {
      strncat(chG_ImpPtr, stcG_LineaAdPostF[i].lineaAd, inCantMaxPsramProd);
      strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
    }
  }
  //*****************************************************************************
  //
  //  Se imprimen Las Lineas del Pie de Tickets
  //
  //*****************************************************************************
  if (byG_NroLinAdPieTick > 0) {
    for (int i = 0; i < byG_NroLinAdPieTick; i++) {
      strncat(chG_ImpPtr, stcG_PieTicket[i].pieTicket, inCantMaxPsramProd);
      strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
    }
  }
  //*****************************************************************************
  //
  //  Se imprimen el simbolo MH y el Serial de la Impresora
  //
  //*****************************************************************************
  //Ultima Grabada en SD
  //Imprime MH y Serial de ptr
  sprintf(chL_ArrayIni, "%s", MH);
  sprintf(chL_ArrayFin, "%s", chG_SerialPtr);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //********************Se Guarda en el Archivo*************************
  // if (mySD_eMMC.boAppendFile(SD_MMC, dirFileMT, chL_GuardaFile) == ERROR) {
  //   Serial.print("ERROR abriendo Archvo en la SD");
  //   ///Falta ver que sehace con este tipo de error
  //   return;
  // }


  for (int i = 0; i < CantMaxImpArt; i++) {
    Serial.printf("Para el Tipo de Impuesto %d el BI es: %lld\n", i + 1, stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i]);
  }
  for (int i = 0; i < CantMaxImpPago; i++) {
    Serial.printf("Para el Tipo de Pagos %d el BI es: %lld\n", i + 1, stcG_XyZ.stFPVentas.ui64FormasPago[i]);
  }


  //🛑🛑Revisar esta funcion
  stcG_XyZ.stContadores.ui32CountFacDia++;
  if (stcG_XyZ.stContadores.ui32CountFacDia == 1) {
    stcG_XyZ.stContadores.ui32FechaIniRep = myFunciones.timeToUnix(timeRTC);
  }
  if (mySD_eMMC.boWriteFile(SD_MMC, dirFileRepX, (uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ)) == ERROR) {
    Serial.print("ERROR abriendo Archvo en la SD");
    ///Falta ver que sehace con este tipo de error
    return (ERROR);
  }
  // File myFile;
  // myFile = SD_MMC.open(dirFileRepX, FILE_WRITE);
  // myFile.write((const uint8_t*)(&stcG_XyZ) , sizeof(stcG_XyZ));
  // myFile.close();

  //Se almacena en el Archivo de la eMMC La Factura Procesada
  if (mySD_eMMC.boAppendFile(SD_MMC, dirFileMT, chG_ImpPtr) == ERROR) {
    Serial.print("ERROR abriendo Archvo en la SD");
    ///Falta ver que sehace con este tipo de error
    return (ERROR);
  }
  stcG_Index.ui32SizeFile = strlen(chG_ImpPtr);

  strncpy(stcG_Index.pszNameIndex, fileIndiceNroFac, 64);
  stcG_Index.bAddIndex(SD_MMC);



  //Prueba Para Imprimir
  // #ifdef useSerialOutPtr
  Serial.println("Factura se Envia a la Impresora");
  strncat(chG_ImpPtr, "}", inCantMaxPsramProd);
  SerialOutPtr.print(chG_ImpPtr);
  // #endif
  //#ifdef useDebug
  strncat(chG_ImpPtr, "\n\n\n\n", inCantMaxPsramProd);
  Serial.println(chG_ImpPtr);
  //#endif

#ifdef timeDebug
  Serial.print("Time Ejec: ");
  Serial.print(String(millis() - iniTime));
  Serial.println(" mS");
#endif
  Serial.println("❎Saliendo de fImprimirFactura");
  return (SUCCESS);
}




void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

uint8_t writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return ErrorYes;
  }
  if (file.print(message)) {
    Serial.println("- file written");
    //file.close();
  } else {
    Serial.println("- write failed");
    //file.close();
    return ErrorYes;
  }
  file.close();
  return ErrorNoF;
}


String readFile(fs::FS& fs, const char* path) {
  Serial.printf("Reading file: %s\r\n", path);
  String strL_Cont = "";
  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return (strL_Cont);
  }

  Serial.println("- read from file:");
  while (file.available()) {
    //Serial.write(file.read());
    strL_Cont += char(file.read());
  }
  file.close();
  return (strL_Cont);
}

void fImprimirReporteX(void) {
  //Inicializa la Variable donde sa Almacenara el Reporte X
  char chL_ArrayIni[CantMaxCarLin];  //Array para Justificar una lina en los extremmos
  char chL_ArrayFin[CantMaxCarLin];  //Array para Justificar una lina en los extremmos
  String strL_Linea;                 //String para linea de Factura Auxiliar
  char chL_Lineas[CantMaxCarLin + 1];
  memset(chL_Lineas, 0, CantMaxCarLin + 1);
  memset(chL_Lineas, '-', CantMaxCarLin);
  RtcDateTime timeRTC;
  timeRTC = rtc1302.GetDateTime();
  memset(chG_ImpPtr, 0, inCantMaxPsramProd);
  uint8_t countNoFisc = 0;  //Contador de lineas para NO FISCAL
  //La factura se va almcenando en la Variable "chG_ImpPtr" para luego enviarse
  //al puerto UART de la Impresora Fiscal
  //Primera Linea que se Almacenat en la Variable chG_ImpPtr
  strncpy(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar y Bold
  strncat(chG_ImpPtr, ESCcmdBold, inCantMaxPsramProd);
  strncat(chG_ImpPtr, chG_tituloFactura, inCantMaxPsramProd);  //Titulo -> SENIAT y
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  strncat(chG_ImpPtr, chG_rifEmpresa, inCantMaxPsramProd);  //Titulo ->  RIF
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdNormal, inCantMaxPsramProd);  //Letra Normal

  //*****************************************************************************
  //Se Imprime Los Encabezados que vienen despues del Titulo y RIF
  //
  //*****************************************************************************
  //Ciclo para Almacerar la Estructura de Encabezados en las Variables
  //chL_GuardaFile y chG_ImpPtr
  for (int i = 0; i < byG_NroEncab; i++) {
    strncat(chG_ImpPtr, stcG_Encabezado[i].Encabezado, inCantMaxPsramProd);
    strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  }
  //Se imprime el Sub Titulo Reporte X
  strncat(chG_ImpPtr, ReporteXPtr, inCantMaxPsramProd);
  //Comando ESC para Justificar a la Izquierda
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);
  //*****************************************************************************
  //Se Imprime el Numero de Documento No Fiscal y la Fecha
  //Se imprime el Numero DNF
  //*****************************************************************************
  //Se justifica la Factura y Fecha
  //Linea de:
  //  FACTURA #           00000001
  strncpy(chL_ArrayIni, NoFiscal, CantMaxCarLin);
  stcG_XyZ.stContadores.ui32UltDNF++;  //Se incrementa el DNF
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltDNF);
  //Se Hace Respaldo del Contador en la SPIFFS
  stcG_Contadores.ui32UltDNF = stcG_XyZ.stContadores.ui32UltDNF;
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Linea de:
  //  FECHA: DD/MM/AAAA           HORA: 23:30
  //Se Almacena en la estructura la Fecha del Ultimo DNF
  stcG_XyZ.stContadores.ui32FechaDNF = myFunciones.timeToUnix(timeRTC);
  //stcG_XyZ.stContadores.ui32UltDNF++;
  //Se respalda en la SPIFFS
  stcG_Contadores.ui32FechaDNF = stcG_XyZ.stContadores.ui32FechaDNF;
  stcG_Contadores.ui32UltDNF = stcG_XyZ.stContadores.ui32UltDNF;
  sprintf(chL_ArrayIni, "%s%02d/%02d/%4d", FacturaFecha, timeRTC.Day(), timeRTC.Month(), timeRTC.Year());
  sprintf(chL_ArrayFin, "%s%02d:%02d", FacturaHora, timeRTC.Hour(), timeRTC.Minute());
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime el Mensaje NO FISCAL en NEGRITA y Centrado
  // strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar y Bold
  // strncat(chG_ImpPtr, ESCcmdBold, inCantMaxPsramProd);
  // strncat(chG_ImpPtr, MsjNO_FISCAL, inCantMaxPsramProd);
  // strncat(chG_ImpPtr, ESCcmdNormal, inCantMaxPsramProd);  //Letra Normal
  // strncat(chG_ImpPtr, MediosPago, inCantMaxPsramProd);
  // strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);   //Comando ESC para Justificar a la Izquierda
  AgragaNoFiscal();
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, ESCcmdNormal, inCantMaxPsramProd);  //Letra Normal
  strncat(chG_ImpPtr, MediosPago, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Comando ESC para Justificar a la Izquierda
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //***********************************************************
  //Se Imprimen los Metodos de PAgo
  //***********************************************************
  uint64_t ui64L_TotalGav = 0;
  for (int i = 0; i < ui8G_CountFormPago; i++) {
    if (stcG_XyZ.stFPVentas.ui64FormasPago[i] > 0) {
      ui64L_TotalGav += stcG_XyZ.stFPVentas.ui64FormasPago[i];
      sprintf(chL_ArrayIni, "%s", stcG_FormasPagoImp[i].chG_descripcion);
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(stcG_XyZ.stFPVentas.ui64FormasPago[i]),
              DD(stcG_XyZ.stFPVentas.ui64FormasPago[i]),
              DU(stcG_XyZ.stFPVentas.ui64FormasPago[i]));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //Variable para Memoria SD
      //Variable para Ptr
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      //Si el Contador es Mayor a 3 se Imprime la Linea NO FISCAL
      if (++countNoFisc > MaxCanLinNF - 1) {
        countNoFisc = 0;
        AgragaNoFiscal();
      }
    }
  }
  //***********************************************************
  //Se imprime el Total de la Gaveta
  //***********************************************************
  sprintf(chL_ArrayIni, "%s", TotalGaveta);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_TotalGav),
          DD(ui64L_TotalGav),
          DU(ui64L_TotalGav));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Variable para Memoria SD
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //***********************************************************
  //Se imprime VENTAS y Facturas del Dia
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, Ventas, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime los Numeros de Factura en el Dia
  sprintf(chL_ArrayIni, "%s", NroFacDia);
  sprintf(chL_ArrayFin, "%d", stcG_XyZ.stContadores.ui32CountFacDia);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Variable para Memoria SD
  //Variable para Ptr
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //***********************************************************
  //Se imprime RECARGOS
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjRecargos, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //uint8_t countNoFisc = 1;    //Se llevan 2 lineas para NO FISCAL
  uint64_t ui64L_AcumBI = 0;   //Se acumula las BI de los Recargos
  uint64_t ui64L_AcumImp = 0;  //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stRecargos.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stRecargos.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stRecargos.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stRecargos.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Memoria SD
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    //Se incrementa el COntado NO FISCAL
    if (++countNoFisc > MaxCanLinNF - 1) {
      countNoFisc = 0;
      AgragaNoFiscal();
    }
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stRecargos.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      if (++countNoFisc > MaxCanLinNF - 1) {
        countNoFisc = 0;
        AgragaNoFiscal();
      }
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total de Recargos
  strncpy(chL_ArrayIni, MsjSubTotRecargos, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total IVA Recargos
  strncpy(chL_ArrayIni, MsjIVARecargos, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total Recargos
  strncpy(chL_ArrayIni, MsjTotRecargos, CantMaxCarLin);
  uint64_t auxAc = ui64L_AcumBI + ui64L_AcumImp;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //***********************************************************
  //Se imprime DESCUENTOS
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjDescuentos, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  ui64L_AcumBI = 0;   //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;  //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Memoria SD
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    //Se incrementa el COntado NO FISCAL
    if (++countNoFisc > MaxCanLinNF - 1) {
      countNoFisc = 0;
      AgragaNoFiscal();
    }
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      if (++countNoFisc > MaxCanLinNF - 1) {
        countNoFisc = 0;
        AgragaNoFiscal();
      }
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total de Recargos
  strncpy(chL_ArrayIni, MsjSubTotDescuentos, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total IVA Recargos
  strncpy(chL_ArrayIni, MsjIVADescuentos, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total Recargos
  strncpy(chL_ArrayIni, MsjTotDescuentos, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //***********************************************************
  //Se imprime ANULACIONES
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjAnulaciones, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  ui64L_AcumBI = 0;   //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;  //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    //Se incrementa el COntado NO FISCAL
    if (++countNoFisc > MaxCanLinNF - 1) {
      countNoFisc = 0;
      AgragaNoFiscal();
    }
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      if (++countNoFisc > MaxCanLinNF - 1) {
        countNoFisc = 0;
        AgragaNoFiscal();
      }
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total de Recargos
  strncpy(chL_ArrayIni, MsjSubTotAnulaciones, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total IVA Recargos
  strncpy(chL_ArrayIni, MsjIVAAnulaciones, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total Recargos
  strncpy(chL_ArrayIni, MsjTotAnulaciones, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //***********************************************************
  //Se imprime CORRECCIONES
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjCorrecciones, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  ui64L_AcumBI = 0;   //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;  //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    //Se incrementa el COntado NO FISCAL
    if (++countNoFisc > MaxCanLinNF - 1) {
      countNoFisc = 0;
      AgragaNoFiscal();
    }
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      if (++countNoFisc > MaxCanLinNF - 1) {
        countNoFisc = 0;
        AgragaNoFiscal();
      }
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total de Recargos
  strncpy(chL_ArrayIni, MsjSubTotCorrecciones, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total IVA Recargos
  strncpy(chL_ArrayIni, MsjIVACorrecciones, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total Recargos
  strncpy(chL_ArrayIni, MsjTotCorrecciones, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //***********************************************************
  //Se imprime VENTAS
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjVentas, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  ui64L_AcumBI = 0;   //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;  //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Memoria SD
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    //Se incrementa el COntado NO FISCAL
    if (++countNoFisc > MaxCanLinNF - 1) {
      countNoFisc = 0;
      AgragaNoFiscal();
    }
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      if (++countNoFisc > MaxCanLinNF - 1) {
        countNoFisc = 0;
        AgragaNoFiscal();
      }
    }
  }
  uint64_t ui64L_AcumBI_FP = 0;   //Se acumula las BI del IGTF de las Venta
  uint64_t ui64L_AcumImp_FP = 0;  //Se acumula las Impuestos del IGTF de las Venta
  for (int i = 0; i < ui8G_CountFormPago; i++) {
    if (stcG_FormasPagoImp[i].ui16G_ivaPorc > 0) {
      ui64L_AcumBI_FP += stcG_XyZ.stFPVentas.ui64FormasPago[i];
      ui64L_AcumImp_FP += myFunciones.calculoImpuesto(stcG_XyZ.stFPVentas.ui64FormasPago[i], stcG_FormasPagoImp[i].ui16G_ivaPorc);
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total de Recargos
  strncpy(chL_ArrayIni, MsjSubTotVentas, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //El IVa del IGTF
  strncpy(chL_ArrayIni, MsjIGTFVentas, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp_FP),
          DD(ui64L_AcumImp_FP),
          DU(ui64L_AcumImp_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total IVA Recargos
  strncpy(chL_ArrayIni, MsjIVAVentas, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total VENTAS
  strncpy(chL_ArrayIni, MsjTotVentas, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp + ui64L_AcumImp_FP;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Muestra BI IGTF
  strncpy(chL_ArrayIni, MsjBI_IGTF, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI_FP),
          DD(ui64L_AcumBI_FP),
          DU(ui64L_AcumBI_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //***********************************************************
  //Se imprime NOTAS DE DEBITO
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjNotaDebito, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  ui64L_AcumBI = 0;   //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;  //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, MsjInicialesND, CantMaxCarLin);
    strncat(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stBINotaDebito.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stBINotaDebito.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stBINotaDebito.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stBINotaDebito.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Memoria SD
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    //Se incrementa el COntado NO FISCAL
    if (++countNoFisc > MaxCanLinNF - 1) {
      countNoFisc = 0;
      AgragaNoFiscal();
    }
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, MsjInicialesND, CantMaxCarLin);
      strncat(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stBINotaDebito.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      if (++countNoFisc > MaxCanLinNF - 1) {
        countNoFisc = 0;
        AgragaNoFiscal();
      }
    }
  }
  ui64L_AcumBI_FP = 0;   //Se acumula las BI del IGTF de las ND
  ui64L_AcumImp_FP = 0;  //Se acumula las Impuestos del IGTF de las ND
  for (int i = 0; i < ui8G_CountFormPago; i++) {
    if (stcG_FormasPagoImp[i].ui16G_ivaPorc > 0) {
      ui64L_AcumBI_FP += stcG_XyZ.stFPNotDeb.ui64FormasPago[i];
      ui64L_AcumImp_FP += myFunciones.calculoImpuesto(stcG_XyZ.stFPNotDeb.ui64FormasPago[i], stcG_FormasPagoImp[i].ui16G_ivaPorc);
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total de ND
  strncpy(chL_ArrayIni, MsjSubTotNotaDebito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //El IVa del IGTF
  strncpy(chL_ArrayIni, MsjIGTFNotaDebito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp_FP),
          DD(ui64L_AcumImp_FP),
          DU(ui64L_AcumImp_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total IVA ND
  strncpy(chL_ArrayIni, MsjIVANotaDebito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total VENTAS
  strncpy(chL_ArrayIni, MsjTotNotaDebito, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp + ui64L_AcumImp_FP;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Muestra BI IGTF ND
  strncpy(chL_ArrayIni, MsjInicialesND, CantMaxCarLin);
  strncat(chL_ArrayIni, MsjBI_IGTF, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI_FP),
          DD(ui64L_AcumBI_FP),
          DU(ui64L_AcumBI_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //***********************************************************
  //Se imprime NOTAS DE CREDITO
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjNotaCredito, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  ui64L_AcumBI = 0;   //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;  //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, MsjInicialesNC, CantMaxCarLin);
    strncat(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stBINotaCredito.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stBINotaCredito.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stBINotaCredito.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stBINotaCredito.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Memoria SD
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    //Se incrementa el COntado NO FISCAL
    if (++countNoFisc > MaxCanLinNF - 1) {
      countNoFisc = 0;
      AgragaNoFiscal();
    }
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, MsjInicialesNC, CantMaxCarLin);
      strncat(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stBINotaCredito.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      if (++countNoFisc > MaxCanLinNF - 1) {
        countNoFisc = 0;
        AgragaNoFiscal();
      }
    }
  }
  ui64L_AcumBI_FP = 0;   //Se acumula las BI del IGTF de las ND
  ui64L_AcumImp_FP = 0;  //Se acumula las Impuestos del IGTF de las ND
  for (int i = 0; i < ui8G_CountFormPago; i++) {
    if (stcG_FormasPagoImp[i].ui16G_ivaPorc > 0) {
      ui64L_AcumBI_FP += stcG_XyZ.stFPNotCre.ui64FormasPago[i];
      ui64L_AcumImp_FP += myFunciones.calculoImpuesto(stcG_XyZ.stFPNotCre.ui64FormasPago[i], stcG_FormasPagoImp[i].ui16G_ivaPorc);
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total de NC
  strncpy(chL_ArrayIni, MsjSubTotNotaCredito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //El IVa del IGTF
  strncpy(chL_ArrayIni, MsjIGTFNotaCredito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp_FP),
          DD(ui64L_AcumImp_FP),
          DU(ui64L_AcumImp_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total IVA ND
  strncpy(chL_ArrayIni, MsjIVANotaCredito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Sub Total VENTAS
  strncpy(chL_ArrayIni, MsjTotNotaCredito, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp + ui64L_AcumImp_FP;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Muestra BI IGTF ND
  strncpy(chL_ArrayIni, MsjInicialesNC, CantMaxCarLin);
  strncat(chL_ArrayIni, MsjBI_IGTF, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI_FP),
          DD(ui64L_AcumBI_FP),
          DU(ui64L_AcumBI_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //***********************************************************
  //Se imprime Los Contadores
  //***********************************************************
  //Se imprime Ultima Factura y Fecha
  strncpy(chL_ArrayIni, MsjUltimaFactura, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltFact);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Linea de:
  //  FECHA: DD/MM/AAAA           HORA: 23:30
  sprintf(chL_ArrayIni, "%s%02d/%02d/%4d", FacturaFecha, day(stcG_XyZ.stContadores.ui32FechaUF), month(stcG_XyZ.stContadores.ui32FechaUF), year(stcG_XyZ.stContadores.ui32FechaUF));
  sprintf(chL_ArrayFin, "%s%02d:%02d", FacturaHora, hour(stcG_XyZ.stContadores.ui32FechaUF), minute(stcG_XyZ.stContadores.ui32FechaUF));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Ultima Nota Debito
  strncpy(chL_ArrayIni, MsjUltimaND, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltNDeb);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Ultima Nota Credito
  strncpy(chL_ArrayIni, MsjUltimaNC, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltNCre);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Ultima DNF
  strncpy(chL_ArrayIni, MsjUltimaDNF, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltDNF);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Fecha y Hora del Ultimo DNF
  sprintf(chL_ArrayIni, "%s%02d/%02d/%4d", FacturaFecha, day(stcG_XyZ.stContadores.ui32FechaDNF), month(stcG_XyZ.stContadores.ui32FechaDNF), year(stcG_XyZ.stContadores.ui32FechaDNF));
  sprintf(chL_ArrayFin, "%s%02d:%02d", FacturaHora, hour(stcG_XyZ.stContadores.ui32FechaDNF), minute(stcG_XyZ.stContadores.ui32FechaDNF));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Ultima RMF
  strncpy(chL_ArrayIni, MsjUltimaRMF, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltRMF);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Fecha y Hora del Ultimo RMF
  sprintf(chL_ArrayIni, "%s%02d/%02d/%4d", FacturaFecha, day(stcG_XyZ.stContadores.ui32FechaUltRMF), month(stcG_XyZ.stContadores.ui32FechaUltRMF), year(stcG_XyZ.stContadores.ui32FechaUltRMF));
  sprintf(chL_ArrayFin, "%s%02d:%02d", FacturaHora, hour(stcG_XyZ.stContadores.ui32FechaUltRMF), minute(stcG_XyZ.stContadores.ui32FechaUltRMF));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //Imprime MH y Serial de ptr
  sprintf(chL_ArrayIni, "%s", MH);
  sprintf(chL_ArrayFin, "%s", chG_SerialPtr);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);

  char dirFileMT[MaxCarNameFile];  //Nombre Archivo para Almacenar la Factura en Curso
  strncpy(dirFileMT, DirReporte, MaxCarNameFile);
  if (mySD_eMMC.boExistDir(SD_MMC, dirFileMT)) {
    if (mySD_eMMC.boCreateDir(SD_MMC, dirFileMT)) {
      ///OJO RETORNAR ERROR por MEMORIA de TRABAJO
      Serial.println("Error Creando Directorio de Traajo del Seniat");
      return;
    }
  }
  //Se Almacen la Estructura XyZ con los Nuevos Valores en la eMMC

  //Se Almacena el Documento No Fiscal
  char fechaFile[20];
  sprintf(fechaFile, "%s%04u%02u%02u%s", DocNoFiscSENIAT, timeRTC.Year(), timeRTC.Month(), timeRTC.Day(), ExtensionFileSENIAT);
  strncat(dirFileMT, fechaFile, MaxCarNameFile);
  if (mySD_eMMC.boExistFile(SD_MMC, dirFileMT) == ERROR) {
    if (mySD_eMMC.boWriteFile(SD_MMC, dirFileMT, chG_ImpPtr) == ERROR) {
      Serial.print("ERROR abriendo Archvo en la SD");
      ///Falta ver que sehace con este tipo de error
      return;
    }
  } else {
    //********************Se Guarda en el Archivo*************************
    if (mySD_eMMC.boAppendFile(SD_MMC, dirFileMT, chG_ImpPtr) == ERROR) {
      Serial.print("ERROR abriendo Archvo en la SD");
      ///Falta ver que sehace con este tipo de error
      return;
    }
  }


  //Se Envia a la Impresora
  Serial.println("Se envia Reporte a la Impresora....");
  Serial.println("Factura se Envia a la Impresora");
  strncat(chG_ImpPtr, "}", inCantMaxPsramProd);
  SerialOutPtr.print(chG_ImpPtr);
  //Prueba de Imprimir el Reporte Local
  Serial.println(chG_ImpPtr);
}
//Funcion que Agrega el Tiulo NO FISCAL a la Variable que Imprime
void AgragaNoFiscal(void) {
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar y Bold
  strncat(chG_ImpPtr, ESCcmdBold, inCantMaxPsramProd);
  strncat(chG_ImpPtr, MsjNO_FISCAL, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdNormal, inCantMaxPsramProd);     //Letra Normal
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  //mySD_eMMC.boCreateDir(fs::FS &fs, const char *path)
}















void fImprimirReporteZ(void) {
  //Inicializa la Variable donde sa Almacenara el Reporte X
  char chL_ArrayIni[CantMaxCarLin];  //Array para Justificar una lina en los extremmos
  char chL_ArrayFin[CantMaxCarLin];  //Array para Justificar una lina en los extremmos
  String strL_Linea;                 //String para linea de Factura Auxiliar
  char chL_Lineas[CantMaxCarLin + 1];
  memset(chL_Lineas, 0, CantMaxCarLin + 1);
  memset(chL_Lineas, '-', CantMaxCarLin);
  RtcDateTime timeRTC;
  timeRTC = rtc1302.GetDateTime();
  memset(chG_ImpPtr, 0, inCantMaxPsramProd);
  uint8_t countNoFisc = 0;  //Contador de lineas para NO FISCAL
  //La factura se va almcenando en la Variable "chG_ImpPtr" para luego enviarse
  //al puerto UART de la Impresora Fiscal
  //Primera Linea que se Almacenat en la Variable chG_ImpPtr
  strncpy(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar y Bold
  strncat(chG_ImpPtr, ESCcmdBold, inCantMaxPsramProd);
  strncat(chG_ImpPtr, chG_tituloFactura, inCantMaxPsramProd);  //Titulo -> SENIAT y
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  strncat(chG_ImpPtr, chG_rifEmpresa, inCantMaxPsramProd);  //Titulo ->  RIF
  strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdNormal, inCantMaxPsramProd);  //Letra Normal

  //*****************************************************************************
  //Se Imprime Los Encabezados que vienen despues del Titulo y RIF
  //
  //*****************************************************************************
  //Ciclo para Almacerar la Estructura de Encabezados en las Variables
  //chL_GuardaFile y chG_ImpPtr
  for (int i = 0; i < byG_NroEncab; i++) {
    strncat(chG_ImpPtr, stcG_Encabezado[i].Encabezado, inCantMaxPsramProd);
    strncat(chG_ImpPtr, "\n", inCantMaxPsramProd);
  }
  //Se imprime el Sub Titulo Reporte Z
  strncat(chG_ImpPtr, ReporteZPtr, inCantMaxPsramProd);
  //Comando ESC para Justificar a la Izquierda
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);
  //*****************************************************************************
  //Se Imprime el Numero de Documento No Fiscal y la Fecha
  //Se imprime el Numero DNF
  //*****************************************************************************
  //Se justifica la Factura y Fecha
  //Linea de:
  //  FACTURA #           00000001
  //stcG_XyZ.stContadores.ui32UltDNF++;                             //Se incrementa el DNF
  stcG_XyZ.stContadores.ui32UltRepZ++;  //Se incrementa el Nro de Rep Z
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltRepZ);
  //Se Hace Respaldo del Contador en la SPIFFS
  stcG_Contadores.ui32UltRepZ = stcG_XyZ.stContadores.ui32UltRepZ;
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Linea de:
  //  FECHA: DD/MM/AAAA           HORA: 23:30
  //Se Almacena en la estructura la Fecha del Ultimo DNF
  stcG_XyZ.stContadores.ui32FechaRepZ = myFunciones.timeToUnix(timeRTC);
  //stcG_XyZ.stContadores.ui32UltDNF++;
  //Se respalda en la SPIFFS
  stcG_Contadores.ui32FechaRepZ = stcG_XyZ.stContadores.ui32FechaRepZ;
  //stcG_Contadores.ui32UltDNF = stcG_XyZ.stContadores.ui32UltDNF;
  sprintf(chL_ArrayIni, "%s%02d/%02d/%4d", FacturaFecha, timeRTC.Day(), timeRTC.Month(), timeRTC.Year());
  sprintf(chL_ArrayFin, "%s%02d:%02d", FacturaHora, timeRTC.Hour(), timeRTC.Minute());
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  //***********************************************************
  //Se Imprimen los Metodos de PAgo
  //***********************************************************
  uint64_t ui64L_TotalGav = 0;
  for (int i = 0; i < ui8G_CountFormPago; i++) {
    if (stcG_XyZ.stFPVentas.ui64FormasPago[i] > 0) {
      ui64L_TotalGav += stcG_XyZ.stFPVentas.ui64FormasPago[i];
      sprintf(chL_ArrayIni, "%s", stcG_FormasPagoImp[i].chG_descripcion);
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(stcG_XyZ.stFPVentas.ui64FormasPago[i]),
              DD(stcG_XyZ.stFPVentas.ui64FormasPago[i]),
              DU(stcG_XyZ.stFPVentas.ui64FormasPago[i]));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //Variable para Memoria SD
      //Variable para Ptr
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    }
  }
  //***********************************************************
  //Se imprime el Total de la Gaveta
  //***********************************************************
  sprintf(chL_ArrayIni, "%s", TotalGaveta);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_TotalGav),
          DD(ui64L_TotalGav),
          DU(ui64L_TotalGav));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Variable para Memoria SD
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  if (++countNoFisc > MaxCanLinNF - 1) {
    countNoFisc = 0;
    AgragaNoFiscal();
  }
  //***********************************************************
  //Se imprime VENTAS y Facturas del Dia
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, Ventas, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  //Se imprime los Numeros de Factura en el Dia
  sprintf(chL_ArrayIni, "%s", NroFacDia);
  sprintf(chL_ArrayFin, "%d", stcG_XyZ.stContadores.ui32CountFacDia);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Variable para Memoria SD
  //Variable para Ptr
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //***********************************************************
  //Se imprime RECARGOS
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjRecargos, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  //uint8_t countNoFisc = 1;    //Se llevan 2 lineas para NO FISCAL
  uint64_t ui64L_AcumBI = 0;   //Se acumula las BI de los Recargos
  uint64_t ui64L_AcumImp = 0;  //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stRecargos.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stRecargos.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stRecargos.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stRecargos.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Memoria SD
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    //Se incrementa el COntado NO FISCAL
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stRecargos.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total de Recargos
  strncpy(chL_ArrayIni, MsjSubTotRecargos, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  //Sub Total IVA Recargos
  strncpy(chL_ArrayIni, MsjIVARecargos, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  //Sub Total Recargos
  strncpy(chL_ArrayIni, MsjTotRecargos, CantMaxCarLin);
  uint64_t auxAc = ui64L_AcumBI + ui64L_AcumImp;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //***********************************************************
  //Se imprime DESCUENTOS
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjDescuentos, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  ui64L_AcumBI = 0;                                          //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;                                         //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Memoria SD
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    //Se incrementa el COntado NO FISCAL
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stDescuentos.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total de Recargos
  strncpy(chL_ArrayIni, MsjSubTotDescuentos, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total IVA Recargos
  strncpy(chL_ArrayIni, MsjIVADescuentos, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total Recargos
  strncpy(chL_ArrayIni, MsjTotDescuentos, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //***********************************************************
  //Se imprime ANULACIONES
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjAnulaciones, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  ui64L_AcumBI = 0;                                          //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;                                         //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stAnulaciones.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total de Recargos
  strncpy(chL_ArrayIni, MsjSubTotAnulaciones, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  //Sub Total IVA Recargos
  strncpy(chL_ArrayIni, MsjIVAAnulaciones, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total Recargos
  strncpy(chL_ArrayIni, MsjTotAnulaciones, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //***********************************************************
  //Se imprime CORRECCIONES
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjCorrecciones, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  ui64L_AcumBI = 0;                                          //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;                                         //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stCorrecciones.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      if (++countNoFisc > MaxCanLinNF - 1) {
        countNoFisc = 0;
        AgragaNoFiscal();
      }
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total de Recargos
  strncpy(chL_ArrayIni, MsjSubTotCorrecciones, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total IVA Recargos
  strncpy(chL_ArrayIni, MsjIVACorrecciones, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total Recargos
  strncpy(chL_ArrayIni, MsjTotCorrecciones, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //***********************************************************
  //Se imprime VENTAS
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjVentas, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  ui64L_AcumBI = 0;                                          //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;                                         //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Memoria SD
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stBIVentas.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    }
  }
  uint64_t ui64L_AcumBI_FP = 0;   //Se acumula las BI del IGTF de las Venta
  uint64_t ui64L_AcumImp_FP = 0;  //Se acumula las Impuestos del IGTF de las Venta
  for (int i = 0; i < ui8G_CountFormPago; i++) {
    if (stcG_FormasPagoImp[i].ui16G_ivaPorc > 0) {
      ui64L_AcumBI_FP += stcG_XyZ.stFPVentas.ui64FormasPago[i];
      ui64L_AcumImp_FP += myFunciones.calculoImpuesto(stcG_XyZ.stFPVentas.ui64FormasPago[i], stcG_FormasPagoImp[i].ui16G_ivaPorc);
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total de Recargos
  strncpy(chL_ArrayIni, MsjSubTotVentas, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //El IVa del IGTF
  strncpy(chL_ArrayIni, MsjIGTFVentas, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp_FP),
          DD(ui64L_AcumImp_FP),
          DU(ui64L_AcumImp_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total IVA Recargos
  strncpy(chL_ArrayIni, MsjIVAVentas, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total VENTAS
  strncpy(chL_ArrayIni, MsjTotVentas, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp + ui64L_AcumImp_FP;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Muestra BI IGTF
  strncpy(chL_ArrayIni, MsjBI_IGTF, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI_FP),
          DD(ui64L_AcumBI_FP),
          DU(ui64L_AcumBI_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //***********************************************************
  //Se imprime NOTAS DE DEBITO
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjNotaDebito, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  ui64L_AcumBI = 0;                                          //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;                                         //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, MsjInicialesND, CantMaxCarLin);
    strncat(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stBINotaDebito.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stBINotaDebito.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stBINotaDebito.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stBINotaDebito.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Memoria SD
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, MsjInicialesND, CantMaxCarLin);
      strncat(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stBINotaDebito.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
      if (++countNoFisc > MaxCanLinNF - 1) {
        countNoFisc = 0;
        AgragaNoFiscal();
      }
    }
  }
  ui64L_AcumBI_FP = 0;   //Se acumula las BI del IGTF de las ND
  ui64L_AcumImp_FP = 0;  //Se acumula las Impuestos del IGTF de las ND
  for (int i = 0; i < ui8G_CountFormPago; i++) {
    if (stcG_FormasPagoImp[i].ui16G_ivaPorc > 0) {
      ui64L_AcumBI_FP += stcG_XyZ.stFPNotDeb.ui64FormasPago[i];
      ui64L_AcumImp_FP += myFunciones.calculoImpuesto(stcG_XyZ.stFPNotDeb.ui64FormasPago[i], stcG_FormasPagoImp[i].ui16G_ivaPorc);
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total de ND
  strncpy(chL_ArrayIni, MsjSubTotNotaDebito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //El IVa del IGTF
  strncpy(chL_ArrayIni, MsjIGTFNotaDebito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp_FP),
          DD(ui64L_AcumImp_FP),
          DU(ui64L_AcumImp_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total IVA ND
  strncpy(chL_ArrayIni, MsjIVANotaDebito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total VENTAS
  strncpy(chL_ArrayIni, MsjTotNotaDebito, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp + ui64L_AcumImp_FP;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Muestra BI IGTF ND
  strncpy(chL_ArrayIni, MsjInicialesND, CantMaxCarLin);
  strncat(chL_ArrayIni, MsjBI_IGTF, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI_FP),
          DD(ui64L_AcumBI_FP),
          DU(ui64L_AcumBI_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //***********************************************************
  //Se imprime NOTAS DE CREDITO
  //***********************************************************
  strncat(chG_ImpPtr, ESCcmdCenter, inCantMaxPsramProd);  //Comandos ESC para Centrar
  strncat(chG_ImpPtr, MsjNotaCredito, inCantMaxPsramProd);
  strncat(chG_ImpPtr, ESCcmdIzquierda, inCantMaxPsramProd);  //Justificar Izquierda
  ui64L_AcumBI = 0;                                          //Se acumula las BI de los Recargos
  ui64L_AcumImp = 0;                                         //Se acumula las Impuestos de los Recargos
  for (int i = 0; i < ui8G_CountImpArt; i++) {
    strncpy(chL_ArrayIni, MsjInicialesNC, CantMaxCarLin);
    strncat(chL_ArrayIni, stcG_ImpuestoArticulos[i].impMontoPtr, CantMaxCarLin);
    sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
            VE2(stcG_XyZ.stBINotaCredito.ui64_BI_Impuesto[i]),
            DD(stcG_XyZ.stBINotaCredito.ui64_BI_Impuesto[i]),
            DU(stcG_XyZ.stBINotaCredito.ui64_BI_Impuesto[i]));
    strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
    ui64L_AcumBI += stcG_XyZ.stBINotaCredito.ui64_BI_Impuesto[i];
    //strL_Linea += '\n';
    //Variable para Memoria SD
    //Variable para Ptr
    strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    if (stcG_ImpuestoArticulos[i].valorImp > 0) {
      strncpy(chL_ArrayIni, MsjInicialesNC, CantMaxCarLin);
      strncat(chL_ArrayIni, stcG_TotalImpuestoArtiulos[i].impMontoImp, CantMaxCarLin);
      uint64_t ui64L_CalculoImp = myFunciones.calculoImpuesto(stcG_XyZ.stBINotaCredito.ui64_BI_Impuesto[i], stcG_ImpuestoArticulos[i].valorImp);
      ui64L_AcumImp += ui64L_CalculoImp;
      sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
              VE2(ui64L_CalculoImp),
              DD(ui64L_CalculoImp),
              DU(ui64L_CalculoImp));
      strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
      //strL_Linea += '\n';
      strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
    }
  }
  ui64L_AcumBI_FP = 0;   //Se acumula las BI del IGTF de las ND
  ui64L_AcumImp_FP = 0;  //Se acumula las Impuestos del IGTF de las ND
  for (int i = 0; i < ui8G_CountFormPago; i++) {
    if (stcG_FormasPagoImp[i].ui16G_ivaPorc > 0) {
      ui64L_AcumBI_FP += stcG_XyZ.stFPNotCre.ui64FormasPago[i];
      ui64L_AcumImp_FP += myFunciones.calculoImpuesto(stcG_XyZ.stFPNotCre.ui64FormasPago[i], stcG_FormasPagoImp[i].ui16G_ivaPorc);
    }
  }
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total de NC
  strncpy(chL_ArrayIni, MsjSubTotNotaCredito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI),
          DD(ui64L_AcumBI),
          DU(ui64L_AcumBI));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //El IVa del IGTF
  strncpy(chL_ArrayIni, MsjIGTFNotaCredito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp_FP),
          DD(ui64L_AcumImp_FP),
          DU(ui64L_AcumImp_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total IVA ND
  strncpy(chL_ArrayIni, MsjIVANotaCredito, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumImp),
          DD(ui64L_AcumImp),
          DU(ui64L_AcumImp));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Sub Total VENTAS
  strncpy(chL_ArrayIni, MsjTotNotaCredito, CantMaxCarLin);
  auxAc = ui64L_AcumBI + ui64L_AcumImp + ui64L_AcumImp_FP;
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(auxAc),
          DD(auxAc),
          DU(auxAc));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //strL_Linea += '\n';
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Muestra BI IGTF ND
  strncpy(chL_ArrayIni, MsjInicialesNC, CantMaxCarLin);
  strncat(chL_ArrayIni, MsjBI_IGTF, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%s %lld.%lld%lld", chG_SimMoneda,
          VE2(ui64L_AcumBI_FP),
          DD(ui64L_AcumBI_FP),
          DU(ui64L_AcumBI_FP));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //***********************************************************
  //Se imprime Los Contadores
  //***********************************************************
  //Se imprime Ultima Factura y Fecha
  strncpy(chL_ArrayIni, MsjUltimaFactura, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltFact);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Linea de:
  //  FECHA: DD/MM/AAAA           HORA: 23:30
  sprintf(chL_ArrayIni, "%s%02d/%02d/%4d", FacturaFecha, day(stcG_XyZ.stContadores.ui32FechaUF), month(stcG_XyZ.stContadores.ui32FechaUF), year(stcG_XyZ.stContadores.ui32FechaUF));
  sprintf(chL_ArrayFin, "%s%02d:%02d", FacturaHora, hour(stcG_XyZ.stContadores.ui32FechaUF), minute(stcG_XyZ.stContadores.ui32FechaUF));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Ultima Nota Debito
  strncpy(chL_ArrayIni, MsjUltimaND, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltNDeb);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Ultima Nota Credito
  strncpy(chL_ArrayIni, MsjUltimaNC, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltNCre);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Ultima DNF
  strncpy(chL_ArrayIni, MsjUltimaDNF, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltDNF);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Fecha y Hora del Ultimo DNF
  sprintf(chL_ArrayIni, "%s%02d/%02d/%4d", FacturaFecha, day(stcG_XyZ.stContadores.ui32FechaDNF), month(stcG_XyZ.stContadores.ui32FechaDNF), year(stcG_XyZ.stContadores.ui32FechaDNF));
  sprintf(chL_ArrayFin, "%s%02d:%02d", FacturaHora, hour(stcG_XyZ.stContadores.ui32FechaDNF), minute(stcG_XyZ.stContadores.ui32FechaDNF));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Ultima RMF
  strncpy(chL_ArrayIni, MsjUltimaRMF, CantMaxCarLin);
  sprintf(chL_ArrayFin, "%08d", stcG_XyZ.stContadores.ui32UltRMF);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Fecha y Hora del Ultimo RMF
  sprintf(chL_ArrayIni, "%s%02d/%02d/%4d", FacturaFecha, day(stcG_XyZ.stContadores.ui32FechaUltRMF), month(stcG_XyZ.stContadores.ui32FechaUltRMF), year(stcG_XyZ.stContadores.ui32FechaUltRMF));
  sprintf(chL_ArrayFin, "%s%02d:%02d", FacturaHora, hour(stcG_XyZ.stContadores.ui32FechaUltRMF), minute(stcG_XyZ.stContadores.ui32FechaUltRMF));
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Se imprime/guarda lineas ----------------
  strL_Linea = String(chL_Lineas);
  strL_Linea += '\n';
  //Se guarda en la Variable para la Impresora
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);
  //Incrementa Contador NO FISCAL
  //Imprime MH y Serial de ptr
  sprintf(chL_ArrayIni, "%s", MH);
  sprintf(chL_ArrayFin, "%s", chG_SerialPtr);
  strL_Linea = myFunciones.funcionlJustificar(chL_ArrayIni, chL_ArrayFin);
  strncat(chG_ImpPtr, strL_Linea.c_str(), inCantMaxPsramProd);

  char dirFileMT[MaxCarNameFile];  //Nombre Archivo para Almacenar la Factura en Curso
  strncpy(dirFileMT, DirReporte, MaxCarNameFile);
  if (mySD_eMMC.boExistDir(SD_MMC, dirFileMT)) {
    if (mySD_eMMC.boCreateDir(SD_MMC, dirFileMT)) {
      ///OJO RETORNAR ERROR por MEMORIA de TRABAJO
      Serial.println("Error Creando Directorio de Traajo del Seniat");
      return;
    }
  }
  //Se Almacen la Estructura Z con los Nuevos Valores en la FLASH
  //Apuntador de la Estructura por Pagina
  char* pData[3];
  pData[0] = (char*)&stcG_XyZ;
  pData[1] = (char*)&stcG_XyZ + sizePage;
  pData[2] = (char*)&stcG_XyZ + sizePage + sizePage;
  //Tamaño de la Estructura
  int tamanoStc = sizeof(stcG_XyZ);
  //Numero de Paginasa segun el Tamano de la Estructura
  //byte nroPag = tamanoStc/sizePage;
  //Se Almacena Varia
  int bytePag = tamanoStc;
  // if(tamanoStc%sizePage){
  //   nroPag++;
  // }
  uint16_t addrZindex = stcG_XyZ.stContadores.ui32UltRepZ * NroPagXyZ * sizePage;
  uint16_t byteWrite;
  for (int i = 0; i < NroPagXyZ; i++) {
    if (bytePag > sizePage) {
      byteWrite = sizePage;
    } else {
      byteWrite = bytePag;
    }
    SerialFlash.write(addrZindex + (i * sizePage), (uint8_t*)pData[i], byteWrite);
    bytePag -= sizePage;
  }

  //Se Inicializa la Estructura XyZ para Almcaenarla en la eMMC
  inicializaAcumXyZ();






  //Se Envia a la Impresora
  Serial.println("Se envia Reporte a la Impresora....");
  Serial.println("Factura se Envia a la Impresora");
  strncat(chG_ImpPtr, "}", inCantMaxPsramProd);
  SerialOutPtr.print(chG_ImpPtr);
  //Prueba de Imprimir el Reporte Local
  Serial.println(chG_ImpPtr);

  return;
}



void inicializaAcumXyZ(void) {
  memset(&stcG_XyZ.stBIVentas, 0, sizeof(stcG_XyZ.stBIVentas));
  memset(&stcG_XyZ.stFPVentas, 0, sizeof(stcG_XyZ.stFPVentas));
  memset(&stcG_XyZ.stDescuentos, 0, sizeof(stcG_XyZ.stDescuentos));
  memset(&stcG_XyZ.stRecargos, 0, sizeof(stcG_XyZ.stRecargos));
  memset(&stcG_XyZ.stCorrecciones, 0, sizeof(stcG_XyZ.stCorrecciones));
  memset(&stcG_XyZ.stAnulaciones, 0, sizeof(stcG_XyZ.stAnulaciones));
  memset(&stcG_XyZ.stBINotaDebito, 0, sizeof(stcG_XyZ.stBINotaDebito));
  memset(&stcG_XyZ.stFPNotDeb, 0, sizeof(stcG_XyZ.stFPNotDeb));
  memset(&stcG_XyZ.stBINotaCredito, 0, sizeof(stcG_XyZ.stBINotaCredito));
  memset(&stcG_XyZ.stFPNotCre, 0, sizeof(stcG_XyZ.stFPNotCre));
  stcG_XyZ.stContadores.ui32CountFacDia = 0;
}








void generaEstructuraReporteXZ(void) {
}
void leerReporteXZ(void) {
}
//Se imprime el Reporte X
void imprimeReporteX(void) {
  //memset()
}

void generaReporteZ(void) {
}

uint8_t boCompruebaCambio_Memoria(void) {
  //Se lee el Ultimo Reporte X
  char chL_SerialPtrSPIFF[CantMaxSerialPtr];
  char chL_SerialPtreMMC[CantMaxSerialPtr];
  char chL_SerialPtrFLASH[CantMaxSerialPtr];
  String strL_Contenido;
  String fileContenido;
  char nameFile[MaxCarNameFile];  //Nombre Archivo para Almacenar el Reporte X
  char fechaFile[20];
  //Se Verifica que El Archvo que posee el Serial en la SPIFFs este y se lea
  strncpy(nameFile, NameFileComprobareSPIFF, MaxCarNameFile);
  strL_Contenido = readFile(SPIFFS, nameFile);
  Serial.println("💾El Archivo Leido en la SPIFF es: ");
  Serial.println(strL_Contenido);
  if (strL_Contenido.length() > 0) {
    doc.clear();
    DeserializationError error = deserializeJson(doc, strL_Contenido);
    if (error) {
      Serial.print("Error convirtiendo JSON SPIFFS desde: ");
      Serial.println(error.c_str());
      return (ERROR_SPIFF);
    }
    // const char* dataSerialP = doc["serialPtr"];
    // Serial.printf("La Prueba de doc[serialPtr] es: %s\n", dataSerialP);
    if (doc["SerialPtr"]) {
      const char* dataSerial = doc["SerialPtr"];
      strncpy(chL_SerialPtrSPIFF, dataSerial, CantMaxSerialPtr);
      strncpy(chG_SerialPtr, dataSerial, CantMaxSerialPtr);
      Serial.printf("💾Serial Ptr desde SPIFFS: %s\n", chL_SerialPtrSPIFF);
      Serial.println(chL_SerialPtrSPIFF);
    } else {
      Serial.println("❌No hay doc[serialPtr] en la SPIFF");
    }
  }
  strncpy(nameFile, NameFileComprobareMMC, MaxCarNameFile);
  strL_Contenido = "";
  strL_Contenido = mySD_eMMC.strReadFile(SD_MMC, nameFile);
  Serial.printf("💾Serial Ptr desde eMMC: %s\n", strL_Contenido.c_str());
  Serial.println(strL_Contenido);
  if (strL_Contenido.length() > 0) {
    doc.clear();
    DeserializationError error = deserializeJson(doc, strL_Contenido);
    if (error) {
      Serial.print("Error convirtiendo JSON eMMC desde: ");
      Serial.println(error.c_str());
      return (ERROR_SPIFF);
    }
    if (doc["SerialPtr"]) {
      const char* dataSerial = doc["SerialPtr"];
      strncpy(chL_SerialPtreMMC, dataSerial, CantMaxSerialPtr);
      Serial.printf("💾Serial Ptr desde eMMC: %s\n", chL_SerialPtreMMC);
    } else {
      Serial.println("❌No hay doc[serialPtr] en la eMMC");
    }
  }
  Serial.printf("💽Serial Ptr desde SPIFF: %s\n", chL_SerialPtrSPIFF);
  SerialFlash.read(AddrIniSerPtr, chL_SerialPtrFLASH, sizeof(chL_SerialPtrFLASH));
  Serial.printf("💽Serial Ptr desde FLASH: %s\n", chL_SerialPtrFLASH);
  if (strcmp(chL_SerialPtrSPIFF, chL_SerialPtreMMC) != 0) {
    stcG_FlagErr.fErroreMMC = 1;
  }
  if (strcmp(chL_SerialPtrSPIFF, chL_SerialPtrFLASH) != 0) {
    stcG_FlagErr.fErrorFlash = 1;
    Serial.println("❌Memoria FLASH Cambiada....");
    return (ERROR_SPIFF);
  }
  if (strcmp(chL_SerialPtrSPIFF, chL_SerialPtreMMC) != 0) {
    stcG_FlagErr.fErroreMMC = 1;
    Serial.println("❌Memoria eMMC Cambiada....");
    return (ERROR_SPIFF);
  }
  Serial.println("✅Memoria FLASH y eMMC No han sido cambiadas....");
  return SUCCESS;
  //Se lee la SPIFFS
}

bool bfVerificarFileFact(RtcDateTime timeFactura){
  char dirFileMT[MaxCarNameFile];  //Nombre Archivo para Almacenar la Factura en Curso
  //Se copia el directorio **/Seniat**
    strncpy(dirFileMT, DirSeniat, MaxCarNameFile);
  #ifdef useDebug
    Serial.print("Directorio de Trabajo: ");
    Serial.println(dirFileMT);
  #endif
  //Se verifica si existe el Directorio  **/Seniat**
  //Para almacenar la factura en curso
  if (mySD_eMMC.boExistDir(SD_MMC, dirFileMT)) {
    if (mySD_eMMC.boCreateDir(SD_MMC, dirFileMT)) {
      ///OJO RETORNAR ERROR por MEMORIA de TRABAJO
      Serial.println("Error Creando Directorio de Traajo del Seniat");
      return (ERROR);
    }
  }
  char fechaFile[20];
  char chL_Lineas[CantMaxCarLin + 1];
  memset(chL_Lineas, 0, CantMaxCarLin + 1);
  memset(chL_Lineas, '-', CantMaxCarLin);
  sprintf(fechaFile, "%s%04u%02u%02u%s", DocFiscSENIAT, timeRTC.Year(), timeRTC.Month(), timeRTC.Day(), ExtensionFileSENIAT);
  strncat(dirFileMT, fechaFile, MaxCarNameFile);
// #ifdef useDebug
  Serial.print("Archivo de Trabajo para Almacenar la Factura de la eMMC: ");
  Serial.println(dirFileMT);
// #endif
  char chL_CabezaFileFac[CantMaxCarLin];
  memset(chL_CabezaFileFac, 0, CantMaxCarLin);
  // stcG_XyZ.stContadores.ui32CountFacDia++;
  // stcG_XyZ.stContadores.ui32UltFact++;
  // sprintf(chL_CabezaFileFac, "%s %08d\n", FacturaControl, ui32G_CountFact);
  sprintf(chL_CabezaFileFac, "%s %08d\n", FacturaControl, stcG_XyZ.stContadores.ui32UltFact);
  if (mySD_eMMC.boExistFile(SD_MMC, dirFileMT) == ERROR) {


    if (mySD_eMMC.boWriteFile(SD_MMC, dirFileMT, chL_CabezaFileFac) == ERROR) {
      Serial.print("ERROR abriendo Archvo en la SD");
      ///Falta ver que sehace con este tipo de error
      return (ERROR);
    }
  } else {
    //********************Se Guarda en el Archivo*************************
    if (mySD_eMMC.boAppendFile(SD_MMC, dirFileMT, chL_CabezaFileFac) == ERROR) {
      Serial.print("ERROR abriendo Archvo en la SD");
      ///Falta ver que sehace con este tipo de error
      return (ERROR);
    }
  }
  return SUCCESS;
}



bool bfLeerReporteX(void) {
  char dirFileRepX[MaxCarNameFile];  //Nombre Archivo para Almacenar el Reporte X
  // char fechaFile[20];
  strncpy(dirFileRepX, DirReporte, MaxCarNameFile);
  if (mySD_eMMC.boExistDir(SD_MMC, dirFileRepX)) {
    Serial.println("No Existe Reporte X");
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return ERROR;
  }
  strncat(dirFileRepX, DocReporteSENIAT, MaxCarNameFile);
  strncat(dirFileRepX, ExtensionFileSENIAT, MaxCarNameFile);
  if (mySD_eMMC.boExistFile(SD_MMC, dirFileRepX) == ERROR) {
    Serial.print("ERROR abriendo Archvo en la SD");
    ///Falta ver que sehace con este tipo de error
    stcG_CmdTxJson.ui8CodeTx = ErroreMMC;
    stcG_FlagFact.flagTxRespuesta = 1;  //Se Tx la Respuesta al HOST
    return ERROR;
  }
  mySD_eMMC.strReadFileData(SD_MMC, dirFileRepX, (uint8_t*)(&stcG_XyZ), sizeof(stcG_XyZ));
  return SUCCESS;
}