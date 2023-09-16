#include "Funciones.h"


Funciones::Funciones() {
}
//*****************************************************************************
// Funcion funcionlTotal Verifica si existe la palabra TOTAL en el char Rx
//    
//    Si es detectada la palabra TOTAL retorna TRUE
//    Si NO es detectada la palabra TOTAL retorna FALSE
//*****************************************************************************
bool Funciones::funcionlTotal(const char* verTotal) {
  //Serial.println("Comparar Total....");
  char str[CantMaxCarLin];
  int j;
  for (j = 0; j < strlen(verTotal); j++) {
    str[j] = verTotal[j];
  }
  str[j] = '\0';
  //Serial.println(str);
  //Serial.println(strupr(str));
  if (strstr(strupr(str), "TOTAL")) {
    //Serial.println("\nse ha detectado la palabra: <total>");
    return true;
  }
  return false;
}
//*****************************************************************************
// Funcion funcionlTotalString Verifica si existe la palabra TOTAL en el STRING Rx
//    
//    Si es detectada la palabra TOTAL retorna TRUE
//    Si NO es detectada la palabra TOTAL retorna FALSE
//*****************************************************************************
bool Funciones::funcionlTotalString(String verTotal) {
  //Serial.println("Comparar Total....");
  int tamString = verTotal.length();
  char str[tamString + 1];
  int j;
  for (j = 0; j < (verTotal.length()); j++) {
    str[j] = verTotal[j];
  }
  str[j] = '\0';
  //Serial.println(str);
  //Serial.println(strupr(str));
  if (strstr(strupr(str), "TOTAL")) {
    //Serial.println("\nse ha detectado la palabra: <total>");
    return true;
  }
  return false;
}
//*****************************************************************************
// Funcion multPrecioPorCant 
//    Multiplica PrecioProducto * Cantidad
//    Formato (PrecioProducto) es Long Long pero se establece los dos primeros digitos 
//          la parte decimal del numero
//    Formato (Cantidad) es Short pero se establece los tres primeros digitos 
//          la parte decimal del numero
// La Funcion almacena los Resultados en al Estructura de Datos stcAuxProductos
//    En (stcAuxProductos.CantXPrec) el resultado de la multiplicacion en entero Long Long
//          tomando los dos primeros digitos la parte decimal del numero
//    En (stcAuxProductos.PrimerDec) el primer Digito de la parte Decimal (un Byte)
//    En (stcAuxProductos.SegundoDec) el segundo Digito de la parte Decimal (un Byte)
//*****************************************************************************
uint64_t Funciones::multPrecioPorCant(uint64_t precioPro, uint32_t cantProd) {
  uint64_t u64TotalProd;  //Variable para almacenar precioPro * cantProd
  if (!precioPro && !cantProd) {
    return 0;
  }
  u64TotalProd = precioPro * cantProd;                //Se multiplica precioPro * cantProd
  uint8_t terDec = ((u64TotalProd % 1000) / 100) & 0x0F; //Se verifica si el Tercer Decimal es > 4
  if (terDec > 4) {                                   //De ser >4 se Suma 1 a partir del segundo Decimal
    u64TotalProd += 1000;
  }
  //Se ajusta la multiplicacion para que quede un entero, donde los dos primeros digitos se tomaran
  //  como los decimales de la multiplicacion, los otros son los enteros
  u64TotalProd /= 1000;
  // stcG_AuxProductos.CantXPrec = u64TotalProd;
  // //Se almacena el primer digito decimal
  // stcG_AuxProductos.PrimerDec = ((u64TotalProd % 100)/10)&0x0F;
  // //Se almacena el segundo digito decimal
  // stcG_AuxProductos.SegundoDec = (u64TotalProd%10)&0x0F;
  //Funcion retorna con true
  return u64TotalProd;
}
//*****************************************************************************
// Funcion multPrecioPorCant 
//    Multiplica PrecioProducto * Cantidad
//    Formato (PrecioProducto) es Long Long pero se establece los dos primeros digitos 
//          la parte decimal del numero
//    Formato (Cantidad) es Short pero se establece los tres primeros digitos 
//          la parte decimal del numero
// La Funcion almacena los Resultados en al Estructura de Datos stcAuxProductos
//    En (stcAuxProductos.CantXPrec) el resultado de la multiplicacion en entero Long Long
//          tomando los dos primeros digitos la parte decimal del numero
//    En (stcAuxProductos.PrimerDec) el primer Digito de la parte Decimal (un Byte)
//    En (stcAuxProductos.SegundoDec) el segundo Digito de la parte Decimal (un Byte)
//*****************************************************************************
uint64_t Funciones::multNum2D(uint64_t numero1, uint32_t numero2) {
  uint64_t u64TotalProd;  //Variable para almacenar precioPro * cantProd
  if (!numero1 && !numero2) {
    return 0;
  }
  u64TotalProd = numero1 * numero2;                //Se multiplica precioPro * cantProd
  uint8_t segDec = DD(u64TotalProd); //Se verifica si el Tercer Decimal es > 4
  if (segDec > 4) {                                   //De ser >4 se Suma 1 a partir del segundo Decimal
    u64TotalProd += 100;
  }
  //Se ajusta la multiplicacion para que quede un entero, donde los dos primeros digitos se tomaran
  //  como los decimales de la multiplicacion, los otros son los enteros
  u64TotalProd /= 100;
  // stcG_AuxProductos.CantXPrec = u64TotalProd;
  // //Se almacena el primer digito decimal
  // stcG_AuxProductos.PrimerDec = ((u64TotalProd % 100)/10)&0x0F;
  // //Se almacena el segundo digito decimal
  // stcG_AuxProductos.SegundoDec = (u64TotalProd%10)&0x0F;
  //Funcion retorna con true
  return u64TotalProd;
}
//*****************************************************************************
// Funcion calculoImpuesto 
//    Calcula el Impuesto del PrecioProducto = (PrecioProducto * Impuesto)/100
//    Formato (PrecioProducto) es Long Long pero se establece los dos primeros digitos 
//          la parte decimal del numero
//    Formato (Impuesto) es Short pero se establece los dos primeros digitos 
//          la parte decimal del numero
// La Funcion almacena los Resultados en al Estructura de Datos auxImpuesto
//    En (stcAuxImpuesto.CalImp) el resultado del calculo del Impuesto
//          tomando los dos primeros digitos la parte decimal del numero
//    En (stcAuxImpuesto.PrimerDec) el primer Digito de la parte Decimal (un Byte)
//    En (stcAuxImpuesto.SegundoDec) el segundo Digito de la parte Decimal (un Byte)
//*****************************************************************************
uint64_t Funciones::calculoImpuesto(uint64_t precioPro, uint32_t Impuesto){
  //Serial.println("Calculo de Impuesto");
  if (!precioPro && !Impuesto) {
    return 0;
  }
  precioPro *= Impuesto;
  uint8_t cuartoDec = ((precioPro % 10000) / 1000) & 0x0F;  //Se verifica si el Cuarto Decimal es > 4
  if (cuartoDec > 4) {       
    //Serial.println("Impuesto cuarto Dig >4");                            //De ser >4 se Suma 1 a partir del segundo Decimal
    precioPro += 10000;
  }
  precioPro /= 10000;                               //Se divide /100 por el % y 100 por los Dig del Impuesto
  // byte primerDec = ((precioPro % 100)/10)&0x0F;     //Se obtiene el primer digito Decimal
  // byte segundoDec = (precioPro%10)&0x0F;            //Se obtiene el Segundo digito Decimal
  // stcG_AuxImpuesto.PrimerDec = primerDec;
  // stcG_AuxImpuesto.SegundoDec = segundoDec;
  // stcG_AuxImpuesto.CalImp = precioPro;
  return (precioPro);
}
//*****************************************************************************
// Funcion funcionlJustificar Justifica dos Textos ajustandolos en los extremos
//    Los Argumentos son Apuntadores char*
//    El Argumento textIni estara al inicio de la Linea
//    El Argumento textFin estara al Final de la Linea
//    Retornara un String con el texto ajustado
//*****************************************************************************
String Funciones::funcionlJustificar(char* textIni, char* textFin) {
  String strRetornoLinea;
  byte bLongLinIni, bLongLinFin, espacios;
  bLongLinIni = strlen(textIni);
  bLongLinFin = strlen(textFin);
  espacios = CantMaxCarLin - (bLongLinIni+bLongLinFin);
  strRetornoLinea = String(textIni);
  for(int i=0; i<espacios; i++){
    strRetornoLinea += " ";
  } 
  strRetornoLinea += String(textFin);
  strRetornoLinea += '\n';
  return strRetornoLinea;
}
//*****************************************************************************
// Funcion funcionlJustificar Justifica dos Textos ajustandolos en los extremos
//    Los Argumentos son String
//    El Argumento textIni estara al inicio de la Linea
//    El Argumento textFin estara al Final de la Linea
//    Retornara un String con el texto ajustado
//*****************************************************************************
String Funciones::funcionlJustificar(String textIni, String textFin) {
  String strRetornoLinea;
  byte bLongLinIni, bLongLinFin, espacios;
  bLongLinIni = textIni.length();
  bLongLinFin = textFin.length();
  espacios = CantMaxCarLin - (bLongLinIni+bLongLinFin);
  strRetornoLinea = textIni;
  for(int i=0; i<espacios; i++){
    strRetornoLinea += " ";
  } 
  strRetornoLinea += textFin;
  strRetornoLinea += '\n';
  return strRetornoLinea;
}
//*****************************************************************************
// Funcion funcionlJustificar Justifica tres Textos ajustandolos en los extremos
//  y el centro
//    Los Argumentos son String
//    El Argumento textIni estara al inicio de la Linea
//    El Argumento TexCen estara en el centro de la linea
//    El Argumento textFin estara al Final de la Linea
//    Retornara un String con el texto ajustado
//*****************************************************************************
// String Funciones::funcionlJustificar(String textIni, String textFin, String textCen) {
//   String strRetornoLinea;
//   byte bLongLinIni, bLongLinFin, espacios, bCentro;
//   bLongLinIni = textIni.length();
//   bLongLinFin = textFin.length();
//   bCentro = CantMaxCarLin - (charCent+bLongLinIni);
//   espacios = CantMaxCarLin - (bLongLinIni+bLongLinFin);
//   strRetornoLinea = textIni;
//   for(int i=0; i<espacios; i++){
//     if(i == bCentro){
//       strRetornoLinea += textCen[0];
//     }else if(i == bCentro+1){
//       strRetornoLinea += textCen[1];
//     }else if(i == bCentro+2){
//       strRetornoLinea += textCen[2];
//     }else{
//       strRetornoLinea += " ";
//     }
//   } 
//   strRetornoLinea += textFin;
//   strRetornoLinea += '\n';
//   return strRetornoLinea;
// }
//*****************************************************************************
// Funcion calculoTotalImp calcula todos los impuestos de la Factura en Curso
//    Los Argumentos son String
//    El Argumento textIni estara al inicio de la Linea
//    El Argumento textFin estara al Final de la Linea
//    Retornara un String con el texto ajustado
//*****************************************************************************
void Funciones::calculoTotalImp(void){
  int i = 0;
  // stcG_TotalImpuesto.llTotalMontoExento = 0;
  // stcG_TotalImpuesto.llTotalImpIVA = 0;
  // stcG_TotalImpuesto.llTotalMontoIVA = 0;
  // stcG_TotalImpuesto.llTotalImpRed = 0;
  // stcG_TotalImpuesto.llTotalMontoRed = 0;
  // stcG_TotalImpuesto.llTotalImpLuj = 0;
  // stcG_TotalImpuesto.llTotalMontoLuj = 0;
  // stcG_TotalImpuesto.llTotalImpPer = 0;
  // stcG_TotalImpuesto.llTotalMontoPer = 0;
  // stcG_TotalImpuesto.llTotalImpIGTF = 0;
  // stcG_TotalImpuesto.llTotalMontoIGTF = 0;
  
  // for(i=0; i < u16G_NroProducto; i++){
  //   if(stcG_Productos[i].grabImpI){
  //     stcG_TotalImpuesto.llTotalImpIVA += stcG_Productos[i].grabImpI;
  //     stcG_TotalImpuesto.llTotalMontoIVA += stcG_Productos[i].CantXPrec;
  //   } else if(stcG_Productos[i].grabImpR){
  //     stcG_TotalImpuesto.llTotalImpRed += stcG_Productos[i].grabImpR;
  //     stcG_TotalImpuesto.llTotalMontoRed += stcG_Productos[i].CantXPrec;
  //   } else if(stcG_Productos[i].grabImpL){
  //     stcG_TotalImpuesto.llTotalImpLuj += stcG_Productos[i].grabImpL;
  //     stcG_TotalImpuesto.llTotalMontoLuj += stcG_Productos[i].CantXPrec;
  //   } else if(stcG_Productos[i].grabImpP){
  //     stcG_TotalImpuesto.llTotalImpPer += stcG_Productos[i].grabImpP;
  //     stcG_TotalImpuesto.llTotalMontoPer += stcG_Productos[i].CantXPrec;
  //   } else if(stcG_Productos[i].grabImpD){
  //     stcG_TotalImpuesto.llTotalImpIGTF += stcG_Productos[i].grabImpD;
  //     stcG_TotalImpuesto.llTotalMontoIGTF += stcG_Productos[i].CantXPrec;
  //   }else{
  //     stcG_TotalImpuesto.llTotalMontoExento += stcG_Productos[i].CantXPrec;
  //   }
  // }
}

//*****************************************************************************
// Funcion convLo2Ex calcula el monto de conversion de Moneda Local (Bs) a
//    Moneda Extranjera (Dolares)
//    Los Argumentos son: monLoc -> Moneda Local
//    El Argumento monLoc -> Es la Moneda Local
//    El Argumento tasaCam -> Es la Tasa de Cambio a moneda Extranjera
//    Retornara un uint64 entero tomando lo primeros dos digitos como Decimales
//*****************************************************************************
#define NUMERO_DECIMALES 3
uint64_t Funciones::convLo2Ex(uint64_t monLoc, uint32_t tasaCam){
  float numerando = monLoc;
  numerando /= 100.0;
  //Serial.printf("Moneda Local en float= %f\n", numerando);
  float denominador = tasaCam;
  denominador /= 100.0;
  // Serial.printf("Tasa de Cambio en float= %f\n", denominador);
  float conversion = numerando/denominador;
  // Serial.printf("La Conversion en Float es: %f\n", conversion);
  uint64_t parteEntera = uint64_t(conversion);
  // Serial.printf("La Parte Entera es: %lld\n", parteEntera);
  float decimalesCv = conversion - parteEntera;
  uint32_t parteDecimal = decimalesCv * pow(10,NUMERO_DECIMALES);
  // Serial.printf("La Parte Decimal es: %d\n", parteDecimal);
  if((parteDecimal%10)>4){
    parteDecimal += 10;
  }
  uint64_t ui64L_Resultado = parteEntera*pow(10,2) + (((parteDecimal%1000)/100)&0x0F) * pow(10,1) + (((parteDecimal%100)/10)&0x0F);
  //Serial.printf("La Conversion en Entera es: %lld\n", ui64L_Resultado);
  //return (parteEntera*pow(10,2) + ((parteDecimal%1000)/100)*pow(10,1) + ((parteDecimal%100)/10));
  return (ui64L_Resultado);
}

uint64_t Funciones::convEx2Lo(uint64_t monExt, uint32_t tasaCam){
  monExt *= tasaCam;
  uint8_t segundoDec = ((monExt % 100) / 10) & 0x0F;  //Se verifica si el Cuarto Decimal es > 4
  if (segundoDec > 4) {       
    monExt += 100;
  }
  monExt /= 100;                               //Se divide /100 por por los decimales
  return (monExt);
}

uint32_t Funciones::timeToUnix(RtcDateTime timeFH){
  struct tm timeEp;
  timeEp.tm_mday = timeFH.Day();
  timeEp.tm_mon = timeFH.Month()-1;
  timeEp.tm_year = timeFH.Year()-1900;
  //
  timeEp.tm_hour = timeFH.Hour();
  timeEp.tm_min = timeFH.Minute();
  timeEp.tm_sec = timeFH.Second();
  //Serial.printf("Anno: %d - Mes: %d - Dia: %d\n", time_now_tm.tm_year, time_now_tm.tm_mon, time_now_tm.tm_mday);
  //Serial.printf("Hora: %d - Minuto: %d - Segund: %d\n", time_now_tm.tm_hour, time_now_tm.tm_min, time_now_tm.tm_sec);
  //time_t time_now_epoch = mktime(&time_now_tm);
  uint32_t time_now_epoch = mktime(&timeEp);
  return (time_now_epoch);
}