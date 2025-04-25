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
    P2P_RESP_INVALID,
    P2P_RESP_NOT_ALLOWED,
    P2P_RESP_SERVER_ERROR,

    P2P_MSG_MAX = 255
} p2p_msg_type_e;

typedef struct {
    /* HEADER 18 bytes */
    uint8_t src_deui[8];
    uint8_t dst_deui[8];
    uint8_t type;
    uint8_t length;

    uint8_t payload[P2P_PAYLOAD_MAX_SIZE];
} p2p_msg_t;

const uint8_t P2P_MSG_HEADER_SIZE = 18;
const uint8_t P2P_MSG_MAX_SIZE = sizeof(p2p_msg_t);

typedef void (* p2p_server_recv_cb_t)(uint8_t src_deui[8], p2p_msg_type_e type, p2p_msg_t data);

class P2PServer {
    public:
        P2PServer();

        /** Enable server interface
         * @return true on success false otherwise
         */
        bool enable();

        /** Disable server interface (this will make changes efective)
         * @return true if lorawan interface and configuration values where
         *         efectively loaded false otherwise
         */
        bool commit_changes(void);

        /** Listen for incomming messages
         * @param[in,out] credentials LoraWAN credential information
         * @param[in,out] lorawan_params LoraWAN parameter cfgs
         * @param[in,out] datetime_cfgs EdLoTiming configurations
         * @param[in,out] datetime EdLoDatetime configurations
         *
         * @note This function blocks the program during execution
         * @note This Function modifies paramaters once it finishes 
         */
        bool listen(
                sLoRa *credentials,
                sLoRaParameters *lorwan_params,
                EdLoTiming *datetime_cfgs,
                EdLoDatetime *datetime);

        /**
         *
         */
        // bool close();

        /**
         *
         */
        bool save();

        /**
         *
         */
        void handle_msg(p2p_msg_t request);
    private:
        bool is_listening = false;
        bool is_enabled = false;

        /** Responds a ping request
         * @param[out] resp response message
         * @param[in] req request message
         */
        bool pong(p2p_msg_t *resp, p2p_msg_t req);

        /** Responds a get request
         * @param[out] resp response message
         * @param[in] req request message
         */
        bool handle_get(p2p_msg_t *resp, p2p_msg_t req);

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
