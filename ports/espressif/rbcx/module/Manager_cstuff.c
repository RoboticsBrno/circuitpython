#include "py/runtime.h"
#include "Manager.h"

#include "comm/src/rbcx.pb.h"
extern uint32_t supervisor_ticks_ms32(void);


void rbcxmgr_bgcb_versionStat(void *ctxVoid) {
    struct rbcxmgr_versionStat_cb_ctx *ctx = ctxVoid;

    mp_obj_t dest[2];
    mp_load_method(ctx->py_wrapper, MP_QSTR__on_coproc_versionStat, dest);

    const mp_obj_t args[4] = {
        ctx->py_wrapper,
        mp_obj_new_str((const char *)ctx->versionStat.revision, sizeof(ctx->versionStat.revision)),
        mp_obj_new_int(ctx->versionStat.number),
        mp_obj_new_bool(ctx->versionStat.dirty),
    };

    mp_call_function_n_kw(dest[0], 4, 0, args);
}


void rbcxmgr_bgcb_ultrasoundStat(void *ctxVoid) {
    struct rbcxmgr_ultrasoundStat_cb_ctx *ctx = ctxVoid;

    mp_obj_t dest[2];
    mp_load_method(ctx->py_wrapper, MP_QSTR__on_coproc_ultrasoundStat, dest);

    const mp_obj_t args[4] = {
        ctx->py_wrapper,
        mp_obj_new_int(ctx->ultrasoundStat.utsIndex),
        mp_obj_new_int(ctx->ultrasoundStat.roundtripMicrosecs),
    };

    mp_call_function_n_kw(dest[0], 3, 0, args);
    free(ctx);
}
