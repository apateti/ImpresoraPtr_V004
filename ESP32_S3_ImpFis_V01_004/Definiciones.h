#include "WString.h"
#ifndef Definiciones_h
#define Definiciones_h
#include <Arduino.h>
//
//****************************************************************************
//RTC1302
// #include <ThreeWire.h>
// #include <RtcDS1302.h>
// #include <Wire.h>
// #define DS1302_CLK_PIN 7
// #define DS1302_IO_PIN 6
// #define DS1302_CE_PIN 4
// ThreeWire myWire(DS1302_IO_PIN, DS1302_CLK_PIN, DS1302_CE_PIN);
// RtcDS1302<ThreeWire> rtc1302(myWire);  //OBJETO De TIPO RtcDS1302
//****************************************************************************
//Permite configurar y utilizar el Puerto UART que se conecta a la Impresora Fiscal
#define useSerialOutPtr
//***************************************************************************************
//🔴 Serial Printer
#ifdef useSerialOutPtr
//HardwareSerial SerialOutPtr(1);
#define ptrRx 21
#define ptrTx 47
#endif
//***************************************************************************************
//Para Probar enviando trama a traves del ESP32 clasic por BT
#define useESP32_BT 
//Para mostrar por UART el Debuj del Programa en Ejecucuin
//#define useDebug 
//Para mostrar por UART Tiempos de de Programa en Ejecucuin
#define timeDebug
//Permite configurar y utilizar la Memoria PSRAM externa
#define UsePSRAM
//Permite configurar y utilizar el Puerto UART que se conecta a la Impresora Fiscal
//#define UseSerialPtr
//Permite configurar y utilizar el Puerto UART que se conecta al HOST
#define UseSerialPC
//Permite configurar y utilizar el Puerto USB Nativo para que trabaje CDC y se conecta al HOST
#define useSerialUSB
//Macros para Extraer Numeros Enteros y su parte Decimales
#define VE2(V) V/100              //Valor entero de Dos Decimales
#define VE3(V) V/1000             //Valor entero de Tres Decimales
#define DU(D) (D%10)&0x0F                //Valor Decimal de la Unidad
#define DD(D) ((D%100)/10)&0x0F          //Valor Decimal de la Decima
#define DC(D) ((D%1000)/100)&0x0F        //Valor Decimal de la Centecima
#define DecCant 1000                     //Indica que Cantidad posee tres digitos Decimales
//Time Out del puerto UART conectado al Host
#define timeOutUART 100
//Errores utilizados en la Rx de datos por UART
#define ErrorYes false
#define ErrorNoF true
//Funciones que Retornan Errores
#define ERROR     true
//Funciones que Retornan sin Errores
#define SUCCESS   false
//Tipos de ERRORES por HARDWARE
#define ERROR_eMMC      0x01
#define ERROR_Flash     0x02
#define ERROR_RTC       0x03
#define ERROR_HW        0x04
#define ERROR_SPIFF     0x05
#define ERROR_FLASH     0x06
//Tipos de ERRORES por Configuracion desde la Memoria Trabajo (MT) eMMC
#define ERROR_CONF_MT     0x10
//Codigos de Errores de Respuesta al Host (Errores de Trama)
#define ErrorNo 0x00          //No Hay Errores
#define ErrorFrame 0x01       //Error en la Trama, JSON Incompleto
#define ErrorData 0x02        //Error en el JSON, un dato Incompleto o no adecuado (int mal Rx)
#define ErrorBlank 0x03       //Error en el JSON, JSON vacio
#define ErrorMemory 0x04      //Error interno, Memoria Insuficiente
#define ErrorCmdUnk 0x05      //Error Comando que se Rx no es Reconocido
#define ErrorUnk 0x06         //Error Desconocido
#define ErrorTimeO 0x07       //Error por Time Out, sucede cuando el JSON Rx Muy extenso > timeOutUAR o no Recibe EOT (0x04)
//Codigos de Errores de Respuesta al Host (Errores de Logica)
#define ErrorCmd 0x10        //Error Logica, no se Rx en el JSON "cmd" (decimal 16)
#define ErrorCmNa 0x11        //Error Logica, Comando Rx esta fuera de Secuancia (decimal 17)
#define ErrorCmIn 0x12        //Error Logica, Comando Rx incompleto (decimal 18)
#define ErrorTotal 0x13       //Error se Recibio la Palabra TOTAL (decimal 19)
#define ErrorDatBlank 0x14    //Error se Rx datos Vacios o np se Rx datod Validos (decimal 20)
#define ErrorDataInval 0x15   //Error se Rx Dato Invalido por ejemplo, esta fuera de rango o no es un Numero  (decimal 21)
#define ErrorSPIFF  0x16      //Error al Leer o Escribir en la Memoria SPIFFS
#define ErroreMMC  0x17      //Error al Leer o Escribir en la Memoria eMMC
//Configuracion Impresora
#define MultCantCarFile 100
#define CantMaxCarLin 32            //Numero de Caracteres en una linea de la Impresora
#define CantMaxCarArt 23            //Numero de Caracteres Maximo en una linea por Descripcion de Articulos
#define CantMaxCarConf 23          //Numero de Caracteres Maximo en una para Variables de Inicializacion (ej. Serial Ptr)
#define CantMaxSerialPtr 11
#define CantMaxImpArt   8        //Cantidad Maxima de Impuesto que Graban los Articulos (ej. IVA, Lujo)
#define CantMaxImpPago   10       //Cantidad Maxima de Impuesto que Graban los Articulos (ej. IGTF)
//Se definen constantes para las Tramas Recibidas
#define CantMaxProd 4500        //Numero maximo Productos a Rx por Comando
#define CantMaxEncab 9           //Numero maximo de encabezados a Rx
#define CantMaxLinAd 9           //Numero maximo de Lineas adicionales con el Cliente
#define CantMaxLinAdPF 9         //Numero maximo de Lineas adicionales Post Cierre Factura
#define CantMaxLinAdPF 9         //Numero maximo de Lineas adicionales Post Cierre Factura
#define CantMaxPieTick 9         //Numero maximo de Lineas adicionales al Pie de Ticket
#define CaracMaxRx 1024      //Numero maximo de Caracteres a Rx por UART
#define CaracMaxCmdTx 10      //Numero maximo de Caracteres para almacenar la Respuesta del Comando Rx
//Se definan Constantes de Utilidadec
#define CantMaxDirFile 100    //Cantidad Maxima pra almacenar la Direccion y Nombre de Archivos de eMMC
#define CantMaxCaracRx 1024      //Numero maximo de Caracteres a Rx por UART
//Codigos de Rx en Json
#define CantMaxCmdRx 32
#define CantMaxDataRx 2048
//
#define CantMaxCarCiRif 20
//
#define MaxCarNameFile  100
//Constante para saber si pasan 24 para los Reportes
#define Segundos24H   86400
//Se definen los Numeros de Lineeas que se Imprime NO FISCAL
#define MaxCanLinNF 4
//Nombres de Memoria eMMC
const char DirConfig[] = "/Configuration";
const char NameFileConfig[] = "/configParam.json";
const char NameFileComprobareMMC[] = "/serialPtreMMC.json";       //Archivo para Comprobar Cambios en la eMMC
const char NameFileComprobareSPIFF[] = "/serialPtrSPIFF.json";       //Archivo para Comprobar Cambios en la eMMC
const char DirSeniat[] = "/Seniat";
const char DirReporte[] = "/Reporte";
//const char InicioFileSENIAT[] = "/MT_";
const char DocFiscSENIAT[] = "/DF_";
const char DocNoFiscSENIAT[] = "/DNF_";
const char DocNotCredSENIAT[] = "/DNC_";
const char DocNotDebSENIAT[] = "/DND_";
const char DocReporteSENIAT[] = "/ReporteX";
const char ExtensionFileSENIAT[] = ".txt";
//Nombre de Memoria SPIFFS
const char NameFileConfigSPIFFS[] = "/configSPIFFS.json";        //Archivo de Configuracion para los Impuestos de Articulos y Formas de Pago
const char NameFileContDocSPIFFS[] = "/contadores.json";        //Nombre del Archivo que lleva los Contadores de Documentos (Z, X,Fac, ND, NC)
const char NameFileParaFacSPIFF[] = "/paramFacSPIFF.json";      //Archivo para Almacenar los Parametros de la Factura, Encabezado y pie de Factura
//
const char fileIndiceNroFac[] = "/IndiceNroFac.ind";             //Archivo para llevar el Indice de Factura por Numero y Fecha
const char fileIndiceNotCre[] = "/IndiceNroNotCre.ind";          //Archivo para llevar el Indice de Nota Credito por Numero y Fecha
const char fileIndiceNotDeb[] = "/IndiceNroNotDeb.ind";          //Archivo para llevar el Indice de Nota Credito por Numero y Fecha
const char fileIndiceDocNF[] = "/IndiceNroDocNF.ind";          //Archivo para llevar el Indice de Nota Credito por Numero y Fecha
const char fileIndiceRepZ[] = "/IndiceNroRepZ.ind";          //Archivo para llevar el Indice de Nota Credito por Numero y Fecha


//Se definen los Comando de Errores que se Denben Tx como Respuestas
const char CMD_Desconocido[] = "UNK";       //Comando Desconocido

//Se definen los Comando Rx en la Trama JSON

//Comandos para Generar Una Factura Fiscal
const char CMD_EncabezadoF[] = "encF";          //Comando Encabezado
const char CMD_ClienteF[] = "cliF";             //Comando Cliente
const char CMD_ComentarioF[] = "comF";          //Comando Comentario
const char CMD_ProductoF[] = "proF";            //Comando Productos/Articulos
const char CMD_DescPorcF[] = "desPoF";            //Comando Descuento por Porcentaje (Ultimo Producto)
const char CMD_DescMontF[] = "desMoF";            //Comando Descuento por Monto (Ultimo Producto)
const char CMD_RecPorcF[] = "recPoF";            //Comando Recargo por Porcentaje (Ultimo Producto)
const char CMD_RecMontF[] = "recMoF";            //Comando Recargo por Monto (Ultimo Producto)
const char CMD_CorrProF[] = "corPoF";            //Comando de Correccion de Producto, No tiene Agumentos (Elimina el monto del Producto)
const char CMD_AnulaProF[] = "anuPoF";            //Comando de Anulacion de Producto, Argumentos (Descripcion1, Precio, Cantidad), Busca el Producto y lo Anula
const char CMD_EndProductoF[] = "endPoF";        //Comando Fin de Producto
const char CMD_LineAdicionalesF[] = "linAdF";        //Comando de Lineas adicionales despues de fin de Producto
const char CMD_PieTicketF[] = "pieTiF";          //Comando Pie de Ticket. despues de Lineas Adicionales
const char CMD_FormPagoF[] = "fpaF";            //Comando Forma de Pago
const char CMD_EndFactura[] = "endFac";         //Comando Fin de Factura
//Comandos para gererar la Nota de Credito
const char CMD_EncabezadoNC[] = "encNC";          //Comando Encabezado
const char CMD_NroFacNC[] = "nroFacNC";             //Comando Numero de Factura que Regustra la NC
const char CMD_FechaFacNC[] = "fechFacNC";        //Comando que Rx la Fecha de la Factura que Registra la NC
const char CMD_ContSeriNC[] = "conSerNC";         //Comando que Rx la el Control/Serial Ptr que Registra la NC
const char CMD_RirCiNC[] = "rifCiNC";              //Comando que Rx el RIF/CI que Registra la NC
const char CMD_RazSocNC[] = "proF";            //Comando Rx el la Razon Social que Registra la NC
const char CMD_ComNC[] = "comNC";             //Comando Rx el la el Comentario que va despues de la Fecha/Hora de la NC
const char CMD_ProdNC[] = "prodNC";             //Comando Rx el los Productos de la NC
const char CMD_DescPorcNC[] = "desPoNC";            //Comando Descuento por Porcentaje (Ultimo Producto)
const char CMD_DescMontNC[] = "desMoNC";            //Comando Descuento por Monto (Ultimo Producto)
const char CMD_RecPorcNC[] = "recPoNC";            //Comando Recargo por Porcentaje (Ultimo Producto)
const char CMD_RecMontNC[] = "recMoNC";            //Comando Recargo por Monto (Ultimo Producto)
const char CMD_CorrProNC[] = "corPoNC";            //Comando de Correccion de Producto, No tiene Agumentos (Elimina el monto del Producto)
const char CMD_AnulaProNC[] = "anuPoNc";            //Comando de Anulacion de Producto, Argumentos (Descripcion1, Precio, Cantidad), Busca el Producto y lo Anula
const char CMD_EndProductoNC[] = "endPoNc";        //Comando Fin de Producto
const char CMD_LineAdicionalesNC[] = "linAdNC";        //Comando de Lineas adicionales despues de fin de Producto
const char CMD_PieTicketNC[] = "pieTiNC";          //Comando Pie de Ticket. despues de Lineas Adicionales
const char CMD_FormPagoNC[] = "fpaNC";            //Comando Forma de Pago
const char CMD_EndNotaCredito[] = "endNC";         //Comando Fin de Factura

//Comandos de Control y de Reportes
const char CMD_setTimer[] = "setTime";              //Comando Time
const char CMD_GetTime[] = "getTime";
const char CMD_WriteFileSPIFF[] = "wFileSPIFF"; //Escribe un Archivo en la Memoria SPIFFS
const char CMD_ReadFileSPIFF[] = "rFileSPIFF"; //Lee un Archivo en la Memoria SPIFFS
const char CMD_WriteFileeMMC[] = "wFileeMMC"; //Escribe un Archivo en la Memoria SPIFFS
const char CMD_ReadFileeMMC[] = "rFileeMMC"; //Lee un Archivo en la Memoria SPIFFS 123
const char CMD_ImpRepX[] = "impRepX";           //Comando para que imprima Reporte X
const char CMD_GenImpRepZ[] = "genImpRepZ";           //Comando para que imprima Reporte Z
const char CMD_ImpRepZ[] = "genImpRepZ";           //Comando para que imprima Reporte Z
const char CMD_InicializaPtr[] = "IniciPtr";      //Inicializa Impresora de Fabrica
//Comandos ESC/POS
//Justificar Centrar
const char ESCcmdCenter[4] = {0x1B, 0x61, 0x31};
//Justificar Izq
const char ESCcmdIzquierda[4] = {0x1B, 0x61, 0x30};
//caracter BOLD
const char ESCcmdBold[4] = {0x1B, 0x45, 0x31};
//caracter NormalOLD
const char ESCcmdNormal[4] = {0x1B, 0x45, 0x30};
///

//Constanteas de char que se imprimen en la Factura
const char FacturaControl[] = "//Factura Control #:";      //Inicio de Titulo para cada Factura del Archivo en SD
const char FacturaNro[] = "FACTURA #";
const char FacturaFecha[] = "FECHA: ";
const char FacturaHora[] = "HORA: ";
const char FacturaDatCli[] = "DATOS DEL CLIENTE\n";
const char FacturaRifCi[] = "RIF/CI: ";
const char FacturaRazSoc[] = "RAZON SOCIAL: ";
const char FacturaSubTotal[] = "SUB TOTAL";
const char FacturaTotal[] = "TOTAL";
const char Descuento[] = "DESC";
const char Recargo[] = "RECAR";
const char Correccion[] = "CORRECCION\n";
const char Anulacion[] = "ANULACION\n";
const char MH[] = "MH";
//Para Reporte Z
const char ReporteZPtr[] = "REPORTE Z\n";
//Para Reporte X
const char ReporteXPtr[] = "REPORTE X\n";
const char NoFiscal[] = "NO FISCAL:";
const char MsjNO_FISCAL[] = "NO FISCAL\n";
const char MediosPago[] = "MEDIOS DE PAGO\n";
const char TotalGaveta[] = "TOTAL GAVETA";
const char Ventas[] = "VENTAS\n";
const char NroFacDia[] = "#FACT DEL DIA";
//Msj Recargos
const char MsjRecargos[] = "RECARGOS\n";
const char MsjSubTotRecargos[] = "SUBTTL RECARGOS";
const char MsjIVARecargos[] = "IVA RECARGOS";
const char MsjTotRecargos[] = "TOTAL RECARGOS";
//Msj Descuentos
const char MsjDescuentos[] = "DESCUENTOS\n";
const char MsjSubTotDescuentos[] = "SUBTTL DESCUENTOS";
const char MsjIVADescuentos[] = "IVA DESCUENTOS";
const char MsjTotDescuentos[] = "TOTAL DESCUENTOS";
//Msj ANULACIONES
const char MsjAnulaciones[] = "ANULACIONES\n";
const char MsjSubTotAnulaciones[] = "SUBTTL ANULACIONES";
const char MsjIVAAnulaciones[] = "IVA ANULACIONES";
const char MsjTotAnulaciones[] = "TOTAL ANULACIONES";
//Msj CORRECCIONES
const char MsjCorrecciones[] = "CORRECCIONES\n";
const char MsjSubTotCorrecciones[] = "SUBTTL CORRECCIONES";
const char MsjIVACorrecciones[] = "IVA CORRECCIONES";
const char MsjTotCorrecciones[] = "TOTAL CORRECCIONES";
//Msj VENTAS
const char MsjVentas[] = "VENTAS\n";
const char MsjSubTotVentas[] = "SUBTTL VENTA";
const char MsjIVAVentas[] = "IVA VENTA";
const char MsjTotVentas[] = "TOTAL VENTA";
const char MsjIGTFVentas[] = "IGTF VENTA (3.00%)";
const char MsjBI_IGTF[] = "BI IGTF (3.00%)";
//Msj NOTA DE DEBITO
const char MsjNotaDebito[] = "NOTA DE DEBITO\n";
const char MsjInicialesND[] = "ND.";
const char MsjSubTotNotaDebito[] = "SUBTTL NOTA DEBITO";
const char MsjIVANotaDebito[] = "IVA NOTA DEBITO";
const char MsjTotNotaDebito[] = "TOTAL NOTA DEBITO";
const char MsjIGTFNotaDebito[] = "IGTF NOTA DEBITO (3.00%)";
//const char MsjBI_IGTF[] = "BI IGTF (3.00%)";
//Msj NOTA DE CREDITO
const char MsjNotaCredito[] = "NOTA DE CREDITO\n";
const char MsjInicialesNC[] = "NC.";
const char MsjSubTotNotaCredito[] = "SUBTTL NOTA CREDITO";
const char MsjIVANotaCredito[] = "IVA NOTA CREDITO";
const char MsjTotNotaCredito[] = "TOTAL NOTA CREDITO";
const char MsjIGTFNotaCredito[] = "IGTF NOTA CREDITO (3.00%)";
//Msj Finales del Reporte
const char MsjUltimaFactura[] = "ULTIMA FACTURA";
const char MsjUltimaND[] = "ULT.NOTA.DEBITO";
const char MsjUltimaNC[] = "ULT.NOTA.CREDITO";
const char MsjUltimaDNF[] = "ULTIMO DNF";
const char MsjUltimaRMF[] = "ULTIMO RMF";



#define PorMaxPer 9999                                    //Porcentaje Maximo Permitido para Los Descuentos y Recargos


//Direcciones Utilizadas para la Memoria tipo FLASH
#define AddrIniSerPtr 0             //Direccion donde se Almacena comienza a almacenar el Serial de la Ptr
//Define tamaño de la Pagina de la Memoria FLASH
#define sizePage 256
#define NroPagXyZ 3

#endif