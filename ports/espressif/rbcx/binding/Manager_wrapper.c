#include <stdint.h>
#include <string.h>
#include "py/objproperty.h"
#include "py/runtime.h"
#include "py/runtime0.h"
#include "shared-bindings/util.h"

#include "rbcx/binding/Manager_wrapper.h"
#include "rbcx/module/Manager.h"


STATIC mp_obj_t rbcx_manager_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *pos_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_py_manager_wrapper, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_pin_rx, MP_ARG_REQUIRED | MP_ARG_INT},
        { MP_QSTR_pin_tx, MP_ARG_REQUIRED | MP_ARG_INT},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, NULL, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rbcx_manager_obj_t *self = m_new_obj(rbcx_manager_obj_t);
    self->cpp_manager = rbcxmgr_create(args[0].u_obj, args[1].u_int, args[2].u_int);
    self->base.type = &rbcx_manager_type;
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t rbcx_manager___del__(mp_obj_t self_in) {
    rbcx_manager_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->cpp_manager) {
        rbcxmgr_destroy(self->cpp_manager);
    }
    self->cpp_manager = NULL;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rbcx_manager___del___obj, rbcx_manager___del__);

STATIC mp_obj_t rbcx_manager_sendVersionRequest(mp_obj_t self_in) {
    rbcx_manager_obj_t *self = MP_OBJ_TO_PTR(self_in);

    CoprocReq msg = {
        .which_payload = CoprocReq_versionReq_tag,
    };

    rbcxmgr_sendCoprocReq(self->cpp_manager, &msg);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rbcx_manager_sendVersionRequest_obj, rbcx_manager_sendVersionRequest);

STATIC mp_obj_t rbcx_manager_sendUtsPingRequest(mp_obj_t self_in, mp_obj_t uts_idx) {
    rbcx_manager_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t uts_index = mp_obj_get_int(uts_idx);

    CoprocReq msg = {
        .which_payload = CoprocReq_ultrasoundReq_tag,
        .payload = {
            .ultrasoundReq = {
                .utsIndex = uts_index,
                .which_utsCmd = CoprocReq_UltrasoundReq_singlePing_tag,
            },
        },
    };

    rbcxmgr_sendCoprocReq(self->cpp_manager, &msg);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(rbcx_manager_sendUtsPingRequest_obj, rbcx_manager_sendUtsPingRequest);


STATIC const mp_rom_map_elem_t rbcx_manager_locals_dict_table[] = {
    // Methods
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&rbcx_manager___del___obj) },
    { MP_ROM_QSTR(MP_QSTR_sendVersionRequest), MP_ROM_PTR(&rbcx_manager_sendVersionRequest_obj) },
    { MP_ROM_QSTR(MP_QSTR_sendUtsPingRequest), MP_ROM_PTR(&rbcx_manager_sendUtsPingRequest_obj) },
};
STATIC MP_DEFINE_CONST_DICT(rbcx_manager_locals_dict, rbcx_manager_locals_dict_table);


const mp_obj_type_t rbcx_manager_type = {
    { &mp_type_type },
    .name = MP_QSTR_Manager,
    .make_new = rbcx_manager_make_new,
    .locals_dict = (mp_obj_dict_t *)&rbcx_manager_locals_dict,
};
