#include "py/obj.h"
#include "py/runtime.h"

#include "rbcx/binding/Manager_wrapper.h"

STATIC const mp_rom_map_elem_t rbcx_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_rbcx_native) },
    { MP_ROM_QSTR(MP_QSTR_Manager), MP_ROM_PTR(&rbcx_manager_type) },
};

STATIC MP_DEFINE_CONST_DICT(rbcx_module_globals, rbcx_module_globals_table);

const mp_obj_module_t rbcx_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&rbcx_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_rbcx_native, rbcx_module, CIRCUITPY_RBCX);
