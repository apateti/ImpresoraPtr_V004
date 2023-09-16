#ifndef funciones_h
#define funciones_h
#include <Arduino.h>
#include "Definiciones.h"
#include "model.h"
#include <time.h>
#include <TimeLib.h>
#include <RtcDS1302.h>
class Funciones {
  public:
    Funciones();
    bool funcionlTotal(const char* verTotal);
    bool funcionlTotalString(String verTotal);
    uint64_t multPrecioPorCant(uint64_t precioPro, uint32_t cantProd);
    uint64_t calculoImpuesto(uint64_t precioPro, uint32_t Impuesto);
    String funcionlJustificar(char* textIni, char* textFin);
    String funcionlJustificar(String textIni, String textFin);
    uint64_t multNum2D(uint64_t numero1, uint32_t numero2);
    //String funcionlJustificar(String textIni, String textFin, String textCen);
    void calculoTotalImp(void);
    uint64_t convLo2Ex(uint64_t monLoc, uint32_t tasaCam);
    uint64_t convEx2Lo(uint64_t monExt, uint32_t tasaCam);
    uint32_t timeToUnix(RtcDateTime timeFH);
};

#endif