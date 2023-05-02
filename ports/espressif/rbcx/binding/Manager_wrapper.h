#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "py/obj.h"
#include "supervisor/background_callback.h"

typedef struct {
    mp_obj_base_t base;
    void *cpp_manager;
    background_callback_t bg_set_version;
} rbcx_manager_obj_t;

extern const mp_obj_type_t rbcx_manager_type;

#ifdef __cplusplus
};
#endif
