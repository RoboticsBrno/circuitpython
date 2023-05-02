#include "Manager.h"

#include "comm/src/coproc_link_parser.h"
#include "comm/src/rbcx.pb.h"

#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <mutex>



extern "C" {
//#include "py/mpprint.h"

typedef void (*mp_print_strn_t)(void *data, const char *str, size_t len);

typedef struct _mp_print_t {
    void *data;
    mp_print_strn_t print_strn;
} mp_print_t;

extern const mp_print_t mp_plat_print;

int mp_printf(const mp_print_t *print, const char *fmt, ...);

extern uint32_t supervisor_ticks_ms32(void);

};



class RbcxManager {
public:
    RbcxManager(mp_obj_t py_wrapper): m_py_wrapper(py_wrapper) {
        memset(&m_bg_versionStat, 0, sizeof(m_bg_versionStat));
    }

    RbcxManager(const RbcxManager&) = delete;
    ~RbcxManager() {}

    void init(int pin_rx, int pin_tx) {
        const uart_config_t uart_config = {
            .baud_rate = 921600,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        };
        ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, pin_tx, pin_rx,
            UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 1024, 0, 0, NULL, 0));

        mp_printf(&mp_plat_print, "Starting RBCXmanager\n");

        xTaskCreate(&RbcxManager::consumerRoutine, "rbmanager_loop", 4096, this, 5, NULL);
    }

    void sendCoprocReq(const CoprocReq *msg) {
        uint8_t encodingBuf[rb::CoprocCodec::MaxFrameSize];
        auto encodedLen = m_codec.encodeWithHeader(&CoprocReq_msg, msg, encodingBuf, sizeof(encodingBuf));
        uart_write_bytes(UART_NUM_2, (const char*)encodingBuf, encodedLen);
    }

private:
    static constexpr const char *TAG = "RbcxManager";
    static void consumerRoutine(void *selfPtr) {
        auto *self = (RbcxManager*)selfPtr;

        rb::CoprocLinkParser<CoprocStat, &CoprocStat_msg> parser(self->m_codec);

        while (true) {
            uint8_t byte;
            if (uart_read_bytes(UART_NUM_2, &byte, 1, portMAX_DELAY) != 1) {
                ESP_LOGE(TAG, "Invalid uart read\n");
                continue;
            }

            if (!parser.add(byte))
                continue;

            const auto& msg = parser.lastMessage();
            switch(msg.which_payload) {
                case CoprocStat_versionStat_tag: {
                    memset((void*)&self->m_bg_versionStat, 0, sizeof(background_callback_t));
                    self->m_bg_versionStat_ctx.py_wrapper = self->m_py_wrapper;
                    self->m_bg_versionStat_ctx.versionStat = msg.payload.versionStat;
                    background_callback_add(&self->m_bg_versionStat, rbcxmgr_bgcb_versionStat, &self->m_bg_versionStat_ctx);
                    break;
                }
                case CoprocStat_ultrasoundStat_tag: {
                    const auto& p = msg.payload.ultrasoundStat;

                    auto *ctx = (rbcxmgr_ultrasoundStat_cb_ctx*)malloc(sizeof(rbcxmgr_ultrasoundStat_cb_ctx));
                    memset((void*)&ctx->callback, 0, sizeof(background_callback_t));
                    ctx->py_wrapper = self->m_py_wrapper;
                    ctx->ultrasoundStat = p;

                    background_callback_add(&ctx->callback, rbcxmgr_bgcb_ultrasoundStat, ctx);
                    break;
                }
                case CoprocStat_powerAdcStat_tag:
                    break;
                default:
                    mp_printf(&mp_plat_print, "Got unhandled RBCX message %d\n", msg.which_payload);
            }
        }
    }

    mp_obj_t m_py_wrapper;
    rb::CoprocCodec m_codec;

    background_callback_t m_bg_versionStat;
    rbcxmgr_versionStat_cb_ctx m_bg_versionStat_ctx;
};


void *rbcxmgr_create(mp_obj_t py_wrapper, int pin_rx, int pin_tx) {
    RbcxManager *man = new RbcxManager(py_wrapper);
    man->init(pin_rx, pin_tx);
    return (void*)man;
}

void rbcxmgr_destroy(void *self) {
    delete (RbcxManager *)self;
}

void rbcxmgr_sendCoprocReq(void *self, const CoprocReq *msg) {
    ((RbcxManager *)self)->sendCoprocReq(msg);
}
