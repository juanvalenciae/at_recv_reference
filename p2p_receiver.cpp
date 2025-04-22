#include "p2p_receiver.h"

P2PServer CfgServer = P2PServer();

static void lora_recv_cb(rui_lora_p2p_recv_t data);

void lora_recv_cb(rui_lora_p2p_recv_t data)
{
    at_p2p_msg_t req;
    uint8_t deveui[8];
    api.lorawan.deui.get(deveui, 8);

    memcpy(req.src_deui, data.Buffer, 8);
    memcpy(req.dst_deui, data.Buffer+8, 8);
    req.type = data.Buffer[16];
    req.length = data.Buffer[17];

    if (req.length > 0) {
        memcpy(req.payload, data.Buffer + 18, req.length);
    }

    DEBUG_SERIAL_PRINTF("msg received: ");
    DEBUG_SERIAL_PRINTF(" src=" DEVEUI_STR, DEVEUI(req.src_deui));
    DEBUG_SERIAL_PRINTF(" dst=" DEVEUI_STR, DEVEUI(req.dst_deui));
    DEBUG_SERIAL_PRINTF(" type=%d", req.type);
    DEBUG_SERIAL_PRINTF(" length=%d\r\n", req.length);
    CfgServer.handle_msg(req);
}

P2PServer::P2PServer()
{
    bool ret_ok;
    ret_ok = api.lora.registerPRecvCallback(lora_recv_cb);
}


bool P2PServer::enable()
{
    this->is_enabled = false;
    RETURN_IF_FALSE(api.lora.nwm.set(), "P2P Start Failure \r\n");
    RETURN_IF_FALSE(api.lora.pfreq.set(868000000), "Set P2P mode frequency Failure \r\n");
    RETURN_IF_FALSE(api.lora.psf.set(12), "Set P2P mode spreading factor Failure \r\n");
    RETURN_IF_FALSE(api.lora.pbw.set(125), "Set P2P mode bandwidth Failure \r\n");
    RETURN_IF_FALSE(api.lora.pcr.set(0), "Set P2P mode code rate Failure \r\n");
    RETURN_IF_FALSE(api.lora.ppl.set(sizeof(at_p2p_msg_t)), "Set P2P mode preamble length Failure \r\n");
    RETURN_IF_FALSE(api.lora.ptp.set(22), "Set P2P mode tx power Failure \r\n");
    DEBUG_SERIAL_PRINTF(
            "P2PServer Enable Success\r\n"
            "frequency: %9dHz \tbandwidth: %dkHz\r\n"
            "Spreading Factor: %d \tTx power: %d\r\n"
            "Preamble Length: %d \tCode Rate: %d\r\n",
            api.lora.pfreq.get(), api.lora.pbw.get(),
            api.lora.psf.get(), api.lora.ptp.get(),
            api.lora.ppl.get(), api.lora.pcr.get()
            );
    this->is_enabled = true;
    return true;
}

bool P2PServer::disable()
{
    api.lorawan.mask.set(&(this->lorawan_parameters->sub_band_mask));
    api.lorawan.adr.set((bool)this->lorawan_parameters->adr_enabled);
    api.lorawan.dr.set(this->lorawan_parameters->dr);
    api.lorawan.rx1dl.set(this->lorawan_parameters->rx1_delay);
    api.lorawan.rx2dl.set(this->lorawan_parameters->rx2_delay);
    api.lorawan.rety.set(this->lorawan_parameters->retry);
    api.lorawan.nwm.set();
}


bool P2PServer::listen(
        const sLoRa *lorawan_credentials,
        sLoRaParameters *lorawan_params,
        EdLoTiming *datetime_cfg,
        EdLoDatetime *send_params_cfg)
{
    bool is_ok;
    RETURN_IF_FALSE(this->is_enabled, "P2PServer is not enabled");

    this->is_listening = true;
    is_ok = api.lora.precv(65534); // Stay in RX mode until api.lora.precv(0) is called
    unsigned long now = millis() / 1000;
    while (this->is_listening) {
        if (now - (millis() / 1000) > 360) {
            this->is_listening = false;
        }
        DEBUG_SERIAL_PRINTF("P2P Listen Mode");
        delay(1000);
    }
    api.lora.precv(0);
}

bool P2PServer::pong(at_p2p_msg_t *resp, at_p2p_msg_t req)
{
    memcpy(resp->dst_deui, req.src_deui, 8);
    memcpy(resp->src_deui, req.dst_deui, 8);

    resp->type = (uint8_t)((req.length == 0) ? P2P_RESP_PONG : P2P_RESP_INVALID);
    resp->length = 0;
    return true;
}

void P2PServer::handle_msg(at_p2p_msg_t req)
{
    if (memcmp(this->lorawan_credentials->dev_eui, req.dst_deui, 8) != 0) return;

    bool ret;
    at_p2p_msg_t resp;

    switch ((p2p_msg_type_e)req.type) {
    case P2P_REQ_PING:
        ret = this->pong(&resp, req);
        break;
    default:
        ret = false;
        break;
    }

    uint8_t lora_data[sizeof(at_p2p_msg_t)];
    uint8_t lora_length = 18 + resp.length;
    memcpy(lora_data, &resp, 18);
    memcpy(lora_data+18, resp.payload, resp.length);
    if (ret == true) {
        api.lora.psend(lora_length, (uint8_t *)lora_data);
    }
}
