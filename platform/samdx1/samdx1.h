#ifndef _SAMDX1_H_
#define _SAMDX1_H_

#if   defined(__SAMD11C14A__) 
  #include "samd11.h"
#elif defined(__SAMD21E17A__)
  #include "samd21.h"
#else
  #error Library does not support the specified device.
#endif


#endif
