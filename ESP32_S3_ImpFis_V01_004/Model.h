#pragma once
#ifndef model_h
#define model_h
#include <Arduino.h>
#include "Definiciones.h"

//Se Definen los Estados a Utilizar por la Maquina de Estado Finita
enum State {
  stateInicial,     //Estado Inicial
  //Estados pra generar la Factura
  stateEncabezado,  //Encabezados
  stateCliente,     //RIF o CI del Cliente y Razon Social
  stateLinea_Com,    //Lineas Adicionales Comentarios
  stateProductos,    //Productos para la Factura
  stateDescPor,       //Descuento por Porcentaje del Producto
  stateDesMonto,      //Descuento por Monto de Producto
  stateRecarPor,      //Recarga por Porcantaje de Producto
  stateRecarMonto,     //Recarga por Monto de Producto
  stateCorrecProd,    //Correccion de Producto
  stateAnulaProd,     //Anulacion Producto
  stateFinProducto,  //Fin de guardar Productos
  stateFormaPago,    //Indica la Forma de PAgo de la Factura (Efectivo, T.Devito...)
  stateLineaAdic,    //Lineas Adicionales despues de los Productos
  statePieTicket,    //Informacion del Pie de Ticket
  stateFin_Factura,   //Fin de la Factura
  //Estados para generar las Notas de Creditos
  stateEncabezadoNC,  //Encabezados de la NC
  //Solo NC
  stateNroFactAfNC,        //Estado Para recibir La factura a ejecutar la NC
  stateFechaFacAfNC,    //Estado Para recibir La fecha de la factura a ejecutar la NC
  stateNroRegAfNC,     //Numero de Control o Serial de la Impresora Fiscal
  stateRifCiAfNC,     //Estado Para Recibir el Nro de Registro (RIF/CI)
  stateRazSocAfNC,      //Estado Para Recibir la Razon Social de la NC
  //Fin Solo NC
  stateLinAdi1NC,     //Lineas Adicionales despues de la Razon Social a Procesar de la NC
  stateLinComNC,      //Estado para la Linea de Comentario de la NC
  stateProductoNC,    //Estado para ingresar el Producto/Articulo de la NC
  stateDescPorNC,       //Descuento por Porcentaje del Producto NC
  stateDesMontoNC,      //Descuento por Monto de Producto NC
  stateRecarPorNC,      //Recarga por Porcantaje de Producto NC
  stateRecarMontoNC,     //Recarga por Monto de Producto NC
  stateCorrecProdNC,    //Correccion de Producto NC
  stateAnulaProdNC,     //Anulacion Producto NC
  stateFinProductoNC,  //Fin de guardar Productos NC
  stateFormaPagoNC,    //Indica la Forma de PAgo de la Factura (Efectivo, T.Devito...) NC
  stateLineaAdicNC,    //Lineas Adicionales despues de los Productos
  statePieTicketNC,    //Informacion del Pie de Ticket
  stateFin_NC,            //Fin de la Nota de Credito
  //Estados para Configurar la Impresora
  stateImpRepX,        //Imprimir Reporte X
  stateGenImpRepZ,        //Generar e Imprimir Reporte Z
  stateGetTimer,           //Lee el Tiempo del RTC
  stateSetTimer,            //Configura el Reloj en el RTC
  stateWrSPIFF,           //Escribe Archivo en la SPIFF
  stateReSPIFF,            //Lee Archivo en la SPIFF
  stateInitPtr            //Inicializa las Memorias para colocar las Memoras como Nuevas

};
//Entradas que se derivan para Cambiar la Maquina de Estado
enum Input {
  Unknown,          //Entrada Desconocida
  //Estados pra generar la Factura
  InpEnc,           //Se Recibio el Comando Encabezado
  InpCli,           //Se Recibio el Comando Cliente
  InpLinCom,        //Se Recibio el Comando Comentario
  InpProd,          //Se Recibio el Comando Producto
  InpDesPor,        //Se Recibio el Comando Descuento por Porcentaje
  InpDesMon,        //Se Recibio el Comando Descuento por Monto
  InpRecPor,        //Se Recibio el Comando Recargo por Porcentaje
  InpRecMon,        //Se Recibio el Comando Recargo por Monto
  InpCorrProd,      //Se Recibio el Comando Correcicion de Producto
  InpAnulProd,        //Se Recibio el Comando Anulacion de Producto
  InpFormPa,           //Se Recibio el Comando de la Forma de PAgo
  InpFinProd,       //Se Recibio el Comando Fin de Producto
  InpLinAdi,          //Se Recibe el Comando de Lineas Adicionales
  InpPieTic,        //Se Recibio el Comando Pie de Pagina
  InpFinF,           //Se Recibio el Comando Fin de Factura
  //Estados para generar las Notas de Creditos
  InpEncNC,           //Se Recibio el Comando Encabezado de la NC
  //Solo NC
  InpNroFactAfNC,       //Se Rc el Comando del Nro de Factura a ejecutar la NC
  InpFechaFacAfNC,       //Se Rx el comando Fecha de la Factura a ejecutar la NC
  InpNroRegAfNC,        //Se Rx El Numero de Control o Serial de la Impresora
  InpRifCiAfNC,          // Se Rx el Comando de Nro de Registro (CI/RIF) a procesar la NC
  InpRazSocAfNC,        //Se Rx el comando de Razon Social a Procesar la NC
  //Fin Solo NC
  InpLinAdi1NC,       //Se Rx el Comndo de Linea adicional despues de la RazonSocil/CI
  InpLinComNC,        //Se Recibio el Comando Comentario de la NC
  InpProdNC,          //Se Recibio el Comando Producto para la Nota de Credito (NC)
  InpDesPorNC,        //Se Recibio el Comando Descuento por Porcentaje NC
  InpDesMonNC,        //Se Recibio el Comando Descuento por Monto NC
  InpRecPorNC,        //Se Recibio el Comando Recargo por Porcentaje NC
  InpRecMonNC,        //Se Recibio el Comando Recargo por Monto NC
  InpCorrProdNC,      //Se Recibio el Comando Correcicion de Producto NC
  InpAnulProdNC,        //Se Recibio el Comando Anulacion de Producto NC
  InpFormPaNC,           //Se Recibio el Comando de la Forma de PAgo NC
  InpFinProdNC,       //Se Recibio el Comando Fin de Producto NC
  InpLinAdiNC,          //Se Recibe el Comando de Lineas Adicionales
  InpPieTicNC,        //Se Recibio el Comando Pie de Pagina
  InpFinNC,           //Se Recibio el Comando Fin de  NC
  //Estados para Configurar la Impresora
  InpImpRepX,          //Se Recibo el Comando para Imprimir el Reporte X
  InpGenImpRepZ,       //Se Recibio el Comando para Generar e Imprimir el Reporte Z
  InpGetTimer,         //Se Rx el Comando Get Timer
  InpSetTimer,           //Se Rx el Comando Set Timer
  InpWriteSPIFF,        //Escribe Archivo en la SPIFF
  InpReadSPIFF,          //Lee Archivo en la SPIFF
  InpInitPtr              //Se Rx el Comando para Colocar la Impresora como Nueva
};
//Se definen las Estructuras para ser Utilizadas como Datos en cada Estado
//del proceso de Rx de la Factura
//*************************************************************************
//Estructura para Almacenar los Valores Recibidos en el Comando Encabezado
//*************************************************************************
struct Encabezado {
  char Encabezado[CantMaxCarLin];
};
extern Encabezado stcG_Encabezado[CantMaxEncab];                            //Se Almacenan las Lineas Adicionales Rx del Host para la Factura
//*************************************************************************
//Estructura para Almacenar los Valores Recibidos en el Comando Cliente
//*************************************************************************
struct Cliente {
  char RifCi[CantMaxCarLin];
  char RazSoc[CantMaxCarLin];
  char LineaAd[CantMaxLinAd][CantMaxCarLin];
};
extern Cliente stcG_Cliente;                                                //Se Almacena el Cliente Rx desde el Host
//*************************************************************************
//Estructura para Almacenar los Valores Recibidos en el Comando Comentario
//*************************************************************************
struct Comentario {
  char Comentario[CantMaxCarLin];
};
extern Comentario stcComentario;                                            //Se Almacena las Lineas de Comentario que van despues del Cliente Rx del Host
//*************************************************************************
//Estructura para Almacenar los Valores Recibidos en el Comando Producto
// Se declarara un Array de Productos definiendo la cantidad definido por:
//    CantMaxProd en el archivo Definiciones.h
//*************************************************************************
struct Productos {
  uint8_t ui8G_tipoImp;                         //Tipo de Impuesto 1-n Configurado en Archvo SPIFFS
  uint64_t ui64G_Precio;                        //Precio del Producto -> Entero con dos decimales
  uint16_t ui64G_Cantidad;                      //Cantidad de Productos -> Entero con tres decimales
  uint64_t ui64G_CantXPrec;                     //Resultado de Multiplicar Cantidad x Precio
  uint64_t ui64G_DescPorc;                      //Descuento por porcentaje (si aplica), del Producto, aqui se Almacena el Porcentaje
  //uint64_t ui64G_MontoDescPorc;               //Monto del Descuento por porcentaje (si aplica), del Producto
  uint64_t ui64G_DescMonto;                     //Descuento por Monto (si aplica), del Producto, Aqui se Coloca el Monto del Descuento
  uint64_t ui64G_RecPorc;                       //Recarga por porcentaje (si aplica), del Producto, aqui se Almacena el Porcentaje
  //uint64_t ui64G_MontoRecPorc;                  //Recarga por porcentaje (si aplica), del Producto, Aqui se Coloca el Monto del Descuento
  uint64_t ui64G_RecMonto;                      //Recarga por monto (si aplica), del Producto
  uint64_t ui64G_Correccion;                     //Correccion (si aplica), del Producto
  uint64_t ui64G_Anulacion;                     //Anulacion (si aplica), del Producto
  char Descripcion1[CantMaxCarArt];             //Primera Linea de Descripcion del Producto (Obligatoria)
  char Descripcion2[CantMaxCarArt];             //Segunda Linea de Descripcion del Producto (Opcional)
  char Descripcion3[CantMaxCarArt];             //Tercera Linea de Descripcion del Producto (Opcional)
  uint64_t ui64G_IVA_Monto;                    //Se almacena el Monto del Impuesto que graba el Articulo
};
//*************************************************************************
//Estructura para Almacenar los Valores Obligatorios de la Nota de Credito
//*************************************************************************
struct NotaCreditoObli{
  uint32_t NroFacAfec;
  char FecFFactAfec[CantMaxCarLin];
  char NroRegFacAfec[CantMaxCarLin];
  char RifCiFacAfec[CantMaxCarLin];
  char RazSocFacAfec[CantMaxCarLin];
};
//*************************************************************************
//Estructura para Almacenar los Valores Recibidos en el Comando Pie de Ticket
//*************************************************************************
struct PieTicket {
  char pieTicket[CantMaxCarLin];
};
extern PieTicket stcG_PieTicket[CantMaxPieTick];                            //Se Almacena el Pie de Ticket Rx desde el Host
//*************************************************************************
//Estructura para Almacenar los Valores Recibidos en el Comando Lineas Adicionales
//*************************************************************************
struct LineaAdPostF {
  char lineaAd[CantMaxCarLin];
};
extern LineaAdPostF stcG_LineaAdPostF[CantMaxLinAdPF];                      //Se Almacenan las Lineas Adicionales Post Productos Rx del Host para la Factura
//*************************************************************************
//Estructura para Almacenar los Valores Recibidos en el Comando Codigo Barra Pie de Ticket
//*************************************************************************
struct CodBarrPT {
  char CodBarr[CantMaxCarLin];
};
//*************************************************************************
//Estructura para Almacenar los Tipos de Impuestos que Graban los Articulos
//    La Cantidad Maxima de los Impuestos se definen en el archivo Definiciones.h
//     con #define CantMaxImpArt   20
//*************************************************************************
struct ImpuestoArticulos{
  char chG_descripcion[CantMaxCarConf];                     //Descripcion de la Forma de Pago, ejm, Excento, T. DEBITO
  char chG_Abreviatura[CantMaxCarConf];                     //La Abreviatura del Articulo (E), (G)
  char impMontoPtr[CantMaxCarConf];
  char impMontoImp[CantMaxCarConf];
  short valorImp;
};
extern ImpuestoArticulos stcG_ImpuestoArticulos[CantMaxImpArt];             //Se Almacena Los Impuestos que debe tener la Factura (desde archivo configParam.json)
//*************************************************************************
//Estructura para Almacenar los Tipos de Impuestos a las Formas de Pago
//    La Cantidad Maxima de los Impuestos se definen en el archivo Definiciones.h
//     con #define CantMaxImpPago   20
//*************************************************************************
struct FormasPagoImp{
  //char tituloFormPag[CantMaxCarLin];                //Tipo de la Forma de Pago, ejm, EFECTIVO, T. DEBITO
  char chG_descripcion[CantMaxCarConf];                   //Descripcion de la Forma de Pago, ejm, EFECTIVO, T. DEBITO
  char chG_impBI_Monto[CantMaxCarConf];                   //Monto de la Base Impositiva a pagar Impuesto
  char chG_impIVA_Porc[CantMaxCarConf];                   //IVA a pagar Impueto segun la Base Impositiva
  uint16_t ui16G_ivaPorc;                                 //Valor en Porcentaje del Iva a cancelar Impuetso por FP
};
extern FormasPagoImp stcG_FormasPagoImp[CantMaxImpPago];                    //Se Almacenan las Posibles formas de Pago (desde archivo configParam.json)
//*************************************************************************
//Estructura para Almacenar los Tipos de Impuestos a las Formas de Pago
//    Que se van a computar en la Factura, Se debe recibir por el Comando
//    CMD_FormPago, y se Almacena en esta estuctura
//        Esta estructura esta relacionada por tipo con el i de la estuctura de 
//        struct FormasPagoImp
//     con #define CantMaxImpPago   20
//*************************************************************************
struct FormasPagoRxCMD{
  uint8_t ui8_Tipo;
  int64_t i64_Monto;
  uint32_t ui32_TasaConv;
  uint64_t ui64_MontoXtasaCam;
  //uint64_t ui64_ImpuestoFP;
};
extern FormasPagoRxCMD stcG_FormasPagoRxCMD[CantMaxImpPago];                //Formas de PAgo en la Factura se Rx desde el HOST
//*************************************************************************
//Estructura para Almacenar los Valores de Impuestos de La Factura en Curso
//  Sumando dependiendo de los Articulos
//    Se indexa con la Estructura struct ImpuestoArticulos
//*************************************************************************
struct TotalImpuestoArtiulos {
  uint64_t ui64G_BI_Monto;                          //Monto Total a Imprimir que Corresponde al Impuesto i
  uint64_t ui64G_IVA_Monto;                         //Porcentaje en Moneda Local de Impuesto i
  uint64_t ui64G_BI_Descuento;                      //Monto Total de los Descuentos por tipo de Impuesto i
  uint64_t ui64G_BI_Recargo;                      //Monto Total de los Recargos por tipo de Impuesto i
  uint64_t ui64G_BI_Correcciones;                      //Monto Total de las Correcciones por tipo de Impuesto i
  uint64_t ui64G_BI_Anulaciones;                      //Monto Total de las Anulaciones por tipo de Impuesto i
  char impMontoPtr[CantMaxCarConf];                 //leyenda que se imprime con el monto ej. "BI G (16.00%)"
  char impMontoImp[CantMaxCarConf];                 //leyenda que se imprime con el Impuesto ej. "IVA G (16.00%)"
};
extern TotalImpuestoArtiulos stcG_TotalImpuestoArtiulos[CantMaxImpArt];     //Se suman todos los Impuestos de los Productos para Imprimirse en Factura
//*************************************************************************
//Estructura para Almacenar los Valores de Impuestos de La Factura en Curso
//  Sumando dependiendo de la Forma de Pago
//    Se indexa con la Estructura struct FormasPagoImp
//*************************************************************************
struct TotalImpuestoFormPago {
  uint64_t ui64G_MontoImpLo;                          //Monto que Corresponde al Impuesto i en Moneda Local
  uint64_t ui64G_MontoImpEx;                          //Monto que Corresponde al Impuesto i en Moneda Extranjera
  uint64_t ui64G_IVA_Monto;                         //Porcentaje en Moneda Local de Impuesto i
  uint32_t ui32G_TasaCambio;                        //Tasa de Cambio si hay que convertir Un Tipo de Moneda a Otro
  char impFormaPago[CantMaxCarConf];
  char impMontoPtr[CantMaxCarConf];                 //leyenda que se imprime con el monto ej. "BI IGTF (3.00%)"
  char impMontoImp[CantMaxCarConf];                 //leyenda que se imprime con el Impuesto ej. "IGTF (3.00%)"
};
extern TotalImpuestoFormPago stcG_TotalImpuestoFormPago[CantMaxImpPago];    //Se suman todos los Impuestos (si hay), de las Formas de Pago para Imprimir


//*************************************************************************
//Estructura para Almacenar Acumulados Diarios de la Base Imponible (BI)
//  que se acumulan en las Factura, Notas de Debitos, Creditos, Descuentos
//    Recargos, Anulaciones, etc.
//*************************************************************************
struct TotalAcumuladoBI{
  uint64_t ui64G_BI_Ventas[CantMaxImpArt];
  uint64_t ui64G_BI_Descuento[CantMaxImpArt];
  uint64_t ui64G_BI_Recargo[CantMaxImpArt];
  uint64_t ui64G_BI_Correcciones[CantMaxImpArt];
  uint64_t ui64G_BI_Anulaciones[CantMaxImpArt];
  uint64_t ui64G_BI_NotaDebito[CantMaxImpArt];
  uint64_t ui64G_BI_NotaCredito[CantMaxImpArt];
};
extern TotalAcumuladoBI stcG_TotalAcumuladoBI;

//*************************************************************************
//Estructura para Almacenar Acumulados Diarios de la Formas de Pago (FP)
//  que se acumulan en las Factura, Notas de Debitos, Creditos, 
//    , , etc.
//*************************************************************************
struct TotalAcumuladoFP{
  uint64_t ui64G_FP_Ventas[CantMaxImpPago];
  uint64_t ui64G_FP_NotaDebito[CantMaxImpPago];
  uint64_t ui64G_FP_NotaCredito[CantMaxImpPago];
};
extern TotalAcumuladoFP stcG_TotalAcumuladoFP;


//*************************************************************************
//Estructura para Basica para los Reportes X y Z
//  Es la Estructura de Impuestos por Articulos y por Forma de PAgos
//    Se incluye los recargos e descuentos e IGTF
//*************************************************************************
struct ImpuestosProd{
  uint64_t BI_Monto;
  uint64_t IVA_Graba;
};
//*************************************************************************
//Estructura para Almacenar los Contadores de Documentos, en este caso:
//  Ultimo Reporte Z, X, Factura, Nota Debito, Nota Credito
//    Se lee y escribe el la memoria SPIFFS
//*************************************************************************
// struct ContadoDocumentos{
//   uint32_t ui32G_UltZ;                          //Ultimo reporte Z
//   uint32_t ui32G_UltX;                         //Ultimo reporte X
//   uint32_t ui32G_UltFac;                         //Ultimo Num Factura
//   uint32_t ui32G_UltND;                         //Ultimo Num Nota Debito
//   uint32_t ui32G_UltNC;                         //Ultimo Num Nota Credito
//   uint32_t ui32G_time24H;                       //Tiempo para Saber si Pasaron 24 despues del Inicio Ciclo de Factura
// };
// extern ContadoDocumentos stcG_ContadoDocumentos;     //Estructura de Contadores de Documentos
//*************************************************************************
//Estructura para Banderras que sealizan error
//*************************************************************************
struct FlagErr {
  union {
    uint16_t flag;
    struct {
      uint16_t fErrorSPIFFS : 1;     //Indica Error por EEPROM Interna ESP
      uint16_t fErrorFlash : 1;  //Indica Error por Memoria Flash
      uint16_t fErroreMMC : 1;     //Indica Error por Memoria SD
      uint16_t fErrorPSRAM:1;
      uint16_t fErrorUART : 1;   //Indica Error com UART
      uint16_t fErrorRTC : 1;
      uint16_t fErrorTimeO:1;    //Indica Error por Time Out Rx un Comando Del UART
      uint16_t fError : 4;       //Reservado
    };
  };
};
//Flag que Indican Error
extern FlagErr stcG_FlagErr;
//*************************************************************************
//Estructura para Banderras que sealizan estado de Comandos
//*************************************************************************
struct FlagFact {
  union {
    uint16_t flagFact;
    struct {
      uint16_t flagDocProc:1;         //si = 1 Si hay un Documento en Proceso (Factra, DNF, NC, ND, etc)
      uint16_t flagIindEncab : 1;     //si = 1 se Rx el comando Encabezado
      uint16_t flagFactFecha : 1;     //si = 1 se Registro la Fecha y # Factura
      uint16_t flagIndiClien : 1;     //si = 1 se Rx el comando Cliente 
      uint16_t flagIndComen : 1;      //si = 1 se Rx el comando Comentario
      uint16_t flagIndProd : 1;       //si = 1 se Rx el comando Producto
      uint16_t flagIndDescPor:1;       //si = 1 se Rx el comando Descuento por Porcantaje
      uint16_t flagIndDescMon:1;       //si = 1 se Rx el comando Descuento por Monto
      uint16_t flagIndRecPor:1;       //si = 1 se Rx el comando Recargo por Porcantaje
      uint16_t flagIndRecMon:1;       //si = 1 se Rx el comando Recargo por Monto
      uint16_t flagIndCorrProd:1;       //si = 1 se Rx el comando Correccion por Producto
      uint16_t flagIndAnulProd:1;       //si = 1 se Rx el comando Anulacion de Producto
      uint16_t flagIndFinProd : 1;       //si = 1 se Rx el comando Producto
      uint16_t flagIndFormPag : 1;       //si = 1 se Rx el comando Producto
      uint16_t flagIndPiePag : 1;       //si = 1 se Rx el comando Producto
      uint16_t flagIndFinFac : 1;     //si = 1 se Rx el comando Fin de Factura
      uint16_t flagTxRespuesta:1;     //Si =1 Envia Respuesta al Host
      //byte
    };
  };
};
//Flag que Indican Estatus de la Factura
extern FlagFact stcG_FlagFact;
//*************************************************************************
//Estructura para Almacenar los Valores de Impuestos de los Articulos
//  Se declarara un Array de esta estructura segun la cantidad de Impuestos
//  configuradas en el Archivo "infoPtr.txt"
//*************************************************************************
struct TotalTipoImpuestoArticulo{
  uint64_t ui64TotolMontoImp;                 //Total del Monto de los Articulos para el Tipo de Impuesto 
  uint64_t ui64TotolPorcImp;                  //Total del Monto del Porcentaje de Impuesto
};
//*************************************************************************
//Estructura para Almacenar los Valores de Respuesta a cada Comando Rx
//  son "cmd" -> Igual al comando Rx
//      "code" -> Codigo de Rx (0 sin Errores)
//      "data" -> Data a enviar (es Opcional, Solo Algunos CMD lo Envian)
//  configuradas en el Archivo "infoPtr.txt"
//*************************************************************************
struct CmdTxJson{
  char chrG_CmdTx[CantMaxCmdRx];                 //Total del Monto de los Articulos para el Tipo de Impuesto 
  uint8_t ui8CodeTx;                  //Total del Monto del Porcentaje de Impuesto
  char chrG_DataTx[CantMaxDataRx];
  uint32_t ui32G_DataTx;
};
extern CmdTxJson stcG_CmdTxJson;
//*************************************************************************
//Estructura para Almacenar los Reportes X o Z
//  A su ves se componen de tres estructuras basicas y una Compuestas  
//  por las tres anteriores
//  
//*************************************************************************
//Estructura Impuestos segun el tipo de Articulos, por ejemplo
//  Exonerado, Iva, Reducido etc
//*************************************************************************
struct Impuestos
{
  uint64_t  ui64_BI_Impuesto[CantMaxImpArt];
};
//*************************************************************************
//Estructura de Formas De Pago segun el como se Cancela la Factura, por ejemplo
//  Efectivo, T. Credito, Dolares etc
//*************************************************************************
struct FormasDePago
{
  uint64_t ui64FormasPago[CantMaxImpPago];
};
//*************************************************************************
//Estructura de Contadores, donde se lleva el Numero de Ultimos Documentos
//  asi como sus fechas de:
//  Factura, Nota Debito, Nota Credito etc
//*************************************************************************
struct Contadores
{
  uint32_t  ui32CountFacDia;
  uint32_t  ui32UltFact ;
  uint32_t  ui32FechaUF ;
  uint32_t  ui32UltNCre ;
  uint32_t  ui32UltNDeb ;
  uint32_t  ui32UltDNF ;                    //Numero del Yltimo Documento No Fiscal -> Se utiliza para el Reporte X
  uint32_t  ui32FechaDNF ;
  uint32_t  ui32UltRMF ;
  uint32_t  ui32FechaUltRMF ;
  uint32_t  ui32UltRepZ ;
  uint32_t  ui32FechaRepZ ;                      
  uint32_t  ui32Factor ;
  uint32_t  ui32FechaIniRep;
};
extern Contadores stcG_Contadores;
//*************************************************************************
//Estructura de Para Generar los Documentos de Reporte X y Z
//  Se basan en las tres estucturas Anteriores:
//  
//*************************************************************************
struct XyZ
{
  uint16_t SizeRepZ;
  uint8_t  SizeImpArt;
  uint8_t  SizeForPag;
  struct  Impuestos   stBIVentas ;
  struct  FormasDePago  stFPVentas ;
  struct  Impuestos   stDescuentos ;
  struct  Impuestos   stRecargos ;
  struct  Impuestos   stCorrecciones ;
  struct  Impuestos   stAnulaciones ;
  struct  Impuestos   stBINotaDebito ;
  struct  FormasDePago  stFPNotDeb ;
  struct  Impuestos   stBINotaCredito ;
  struct  FormasDePago  stFPNotCre ;
  struct  Contadores  stContadores ;
};
extern XyZ stcG_XyZ;



//*************************************************************************
//Variable *** chImpPtr *** de tipo char
//  En esta variable se Almacena toda la Informacion que se enviara en la 
//  a la Impresora Fiscal
//*************************************************************************
extern char *chImpPtr;
//*************************************************************************
//Variable *** ui32G_CountFact *** de tipo char
//  En esta variable se Almacena toda la Informacion que se enviara en la 
//  a la Impresora Fiscal
//*************************************************************************
extern uint32_t ui32G_CountFact;
//*************************************************************************
//Variable *** ui8G_CountFormPagoRx *** de tipo uint8
//  En esta variable se lancantidad de Formas de Pago recibidas por el comando 
//  Si no llega la Forma de Pago se Inicializa en el Tipo 1
//*************************************************************************
extern uint8_t ui8G_CountFormPagoRx;
//*************************************************************************
//Variable *** ui8G_CountFormPagoImp *** de tipo uint8
//  En esta variable es la cantidad de Formas de Pago a Ser Impresas en Factura 
//  Si no llega la Forma de Pago se Imprime la FP Tipo 1
//*************************************************************************
extern uint8_t ui8G_CountFormPagoImp;
//*************************************************************************
//Variable *** ui64L_SubTotalFact *** de tipo uint64_t
//  En esta variable se Coloca el Sup Total de la Factura
//  Es la Suma la Suma de los montos de los Productos + El impuesto (si aplica)
//*************************************************************************
extern uint64_t ui64L_SubTotalFact;
//*************************************************************************
//Variable *** ui64L_SubTotalImpFormPago *** de tipo uint64_t
//  En esta variable se Coloca el Sup Total de la Suma de los Impuestos
//  por la forma de Pago (si aplica)
//*************************************************************************
extern uint64_t ui64L_SubTotalImpFormPago;
//*************************************************************************
//Variable *** ui64L_TotalMontoFactura *** de tipo uint64_t
//  En esta variable que Almacena el Monto Total a cancelar en la 
//  Factuar
//*************************************************************************
extern uint64_t ui64L_TotalMontoFactura;
//*************************************************************************
//Variable *** chG_TituloFormPago *** de tipo char
//  En esta variable que Almacena el Titulo que se Muestra en la
//  Forma de Pago a Cancelar
//*************************************************************************
extern char chG_TituloFormPago[CantMaxCarLin];
//*************************************************************************
//Variable *** chImpPtr *** de tipo char
//  En esta variable se Almacena toda la Informacion que se enviara en la 
//  a la Impresora Fiscal
//*************************************************************************
extern State currentState;  //Se define la Variable currentState para la Maquina de Estado
extern Input stcG_stcG_currentInput;
extern char chG_SerialPtr[CantMaxSerialPtr];                                  //Se Almacena el Serial de la Impresora ej. SS00000001 (desde archivo configParam.json)
extern char chG_SimMoneda[CantMaxCarConf];                                  //Simbolo de la Moneda Local (Bs) (desde archivo configParam.json)
extern char chG_tituloFactura[CantMaxCarLin];                               //Se coloca el Titulo de la Factura SENIAT y RIF (desde archivo configParam.json)
extern char chG_rifEmpresa[CantMaxCarLin];                                  //Se coloca el RIF de la Empresa en la Factura (desde archivo configParam.json)












#endif