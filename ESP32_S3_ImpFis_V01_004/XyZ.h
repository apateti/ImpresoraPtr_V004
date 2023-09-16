#include <stdint.h>


struct MediosDePago
{
  uint64_t ui64Medios[8] ;
};


struct Impuestos
{
  uint64_t  ui64Impuestos[8] ;
};

struct Contadores
{
  uint8_t   ui8G_Factor ;
  uint32_t  ui32G_UltFact ;
  uint32_t  ui32G_Fecha ;
  uint32_t  ui32UltNCre ;
  uint32_t  ui32UltNDeb ;
  uint32_t  ui32UltDNF ;
  uint32_t  ulFechaDNF ;
  uint32_t  ui32UlrTR ;
  uint32_t  ui32FechaTR ; 
};

struct XyZ
{
  struct  Impuestos   stRecargos ;
  struct  Impuestos   stDescuentos ;
  struct  Impuestos   stAnulaciones ;
  struct  Impuestos   stCorrecciones ;
  struct  Impuestos   stVentas ;
  struct  Impuestos   stNotaDebito ;
  struct  Impuestos   stNotaCredito ;
  struct  Contadores  stContadires ;
  struct  MediosDePago  stMedios ;
};