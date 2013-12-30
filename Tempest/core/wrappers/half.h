#ifndef HALF_H
#define HALF_H

#include <cstdint>
#include <cstring>

namespace Tempest{

class Half {
  public:
    static const unsigned int  MIN_BIASED_EXP_AS_SINGLE_FP_EXP = 0x38000000;
    // max exponent value in single precision that will be converted
    // to Inf or Nan when stored as a half-float
    static const unsigned int  MAX_BIASED_EXP_AS_SINGLE_FP_EXP = 0x47800000;

    // 255 is the max exponent biased value
    static const unsigned int  FLOAT_MAX_BIASED_EXP = (0xFF << 23);

    static const unsigned int  HALF_FLOAT_MAX_BIASED_EXP = (0x1F << 10);

    uint16_t v;
    Half& operator = ( float fin ){
      uint32_t    x;//    = *(unsigned int *)(&fin);
      memcpy( &x, &fin, sizeof(x) );

      uint32_t    sign = (uint16_t)(x >> 31);
      uint32_t    mantissa = x & ((1 << 23) - 1);
      uint32_t    exp      = x & FLOAT_MAX_BIASED_EXP;

      if (exp >= MAX_BIASED_EXP_AS_SINGLE_FP_EXP) {
        if (mantissa && (exp == FLOAT_MAX_BIASED_EXP)) {
          mantissa = (1 << 23) - 1;
          } else {
          mantissa = 0;
          }
        v = (((uint16_t)sign) << 15) | (uint16_t)(HALF_FLOAT_MAX_BIASED_EXP) |
              (uint16_t)(mantissa >> 13);
        } else
      if (exp <= MIN_BIASED_EXP_AS_SINGLE_FP_EXP) {
        exp = (MIN_BIASED_EXP_AS_SINGLE_FP_EXP - exp) >> 23;
        mantissa >>= (14 + exp);

        v = (((uint16_t)sign) << 15) | (uint16_t)(mantissa);
        } else {
        v = (((uint16_t)sign) << 15) |
            (uint16_t)((exp - MIN_BIASED_EXP_AS_SINGLE_FP_EXP) >> 13) |
            (uint16_t)(mantissa >> 13);
        }

      return *this;
      }

    operator float() const{
      uint32_t    sign     = (uint32_t)(v >> 15);
      uint32_t    mantissa = (uint32_t)(v & ((1 << 10) - 1));
      uint32_t    exp      = (uint32_t)(v & HALF_FLOAT_MAX_BIASED_EXP);
      uint32_t    f;

      if (exp == HALF_FLOAT_MAX_BIASED_EXP) {
        exp = FLOAT_MAX_BIASED_EXP;
        if (mantissa)
          mantissa = (1 << 23) - 1;
        }
      else if (exp == 0x0) {
        if (mantissa) {
          mantissa <<= 1;
          exp = MIN_BIASED_EXP_AS_SINGLE_FP_EXP;
          while ((mantissa & (1 << 10)) == 0) {
            mantissa <<= 1;
            exp -= (1 << 23);
            }
          mantissa &= ((1 << 10) - 1);
          mantissa <<= 13;
          }
        } else {
        mantissa <<= 13;
        exp = (exp << 13) + MIN_BIASED_EXP_AS_SINGLE_FP_EXP;
        }

      f = (sign << 31) | exp | mantissa;

      float re;
      memcpy( &re, &f, sizeof(re) );
      return re;//*((float *)&f);
      }
  };

}

#endif // HALF_H
