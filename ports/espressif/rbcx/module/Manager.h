#pragma once

#include "./__init__.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "supervisor/background_callback.h"


void *rbcxmgr_create(mp_obj_t py_wrapper, int pin_rx, int pin_tx);
void rbcxmgr_destroy(void *self);

void rbcxmgr_sendCoprocReq(void *self, const CoprocReq *msg);

struct rbcxmgr_versionStat_cb_ctx {
    mp_obj_t py_wrapper;
    CoprocStat_VersionStat versionStat;
};

void rbcxmgr_bgcb_versionStat(void *ctxVoid);

struct rbcxmgr_ultrasoundStat_cb_ctx {
    background_callback_t callback;
    mp_obj_t py_wrapper;
    CoprocStat_UltrasoundStat ultrasoundStat;
};

void rbcxmgr_bgcb_ultrasoundStat(void *ctxVoid);

#ifdef __cplusplus
};
#endif
