
#include "edgetx.h"
#include "yaml_node.h"

#include "yaml_datastructs_funcs.cpp"

#if defined(PCBX10)
#include "yaml_datastructs_x10.cpp"
#elif defined(PCBX12S)
#include "yaml_datastructs_x12s.cpp"
#else
#error "Board not supported by YAML storage"
#endif
