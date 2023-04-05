#include "configbus.h"

ConfigBus *ConfigBus::s_instance = 0;

#ifdef USE_IBUS
#include "configbusibus.h"
#else
#include "configbusdbus.h"
#endif
