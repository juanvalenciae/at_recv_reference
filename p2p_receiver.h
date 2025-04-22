#ifndef P2PSERVER_H
#define P2PSERVER_H
#include <Arduino.h>
#include <stdint.h>
#include <string.h>

#include "common.h"

const uint32_t P2P_PAYLOAD_MAX_SIZE = 100;

typedef enum {
    P2P_REQ_PING = 0,
    P2P_REQ_GET,
    P2P_REQ_SET,

    P2P_RESP_OK = 128,
    P2P_RESP_PONG,
    P2P_RESP_TIMEOUT,
    P2P_RESP_NOT_ALLOWED,
    P2P_RESP_INVALID,

    P2P_MSG_MAX = 255
} p2p_msg_type_e;

typedef struct {
    uint8_t src_deui[8];
    uint8_t dst_deui[8];
    uint8_t type;
    uint8_t length; // If 0 the server will return data type parameters
    uint8_t payload[P2P_PAYLOAD_MAX_SIZE];
} at_p2p_msg_t; // Messages from client

typedef void (* p2p_server_recv_cb_t)(uint8_t src_deui[8], p2p_msg_type_e type, at_p2p_msg_t data);

class P2PServer {
    public:
        P2PServer();

        /** @Description enable server interface
         * @return true on success false otherwise
         */
        bool enable();

        /** @Description disable server interface (this will make changes efective)
         * @return true if lorawan interface and configuration values where
         *         efectively loaded false otherwise
         */
        bool disable(void);

        /**
         *
         */
        bool listen(
                const sLoRa *credentials,
                sLoRaParameters *lorwan_params,
                EdLoTiming *datetime_cfgs,
                EdLoDatetime *datetime);

        /**
         *
         */
        bool close();

        /**
         *
         */
        bool save();

        /**
         *
         */
        void handle_msg(at_p2p_msg_t request);
    private:
        bool is_listening = false;
        bool is_enabled = false;

        /** Response a ping request
         * @param[out] resp response message
         * @param[in] req request message
         */
        bool pong(at_p2p_msg_t *resp, at_p2p_msg_t req);

        /** Configuration Structures
         * For structures which can be modified there is a pointer for the
         * configuration structure and a variable for the buffer
         */
        sLoRaParameters *lorawan_parameters;
        sLoRa *lorawan_credentials;
        EdLoDatetime *edlo_datetime_cfgs;
        EdLoTiming *edlo_timinig_cfgs;
};

extern P2PServer CfgServer;
#endif
