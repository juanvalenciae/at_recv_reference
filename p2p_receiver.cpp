#include "p2p_receiver.h"

P2PServer CfgServer = P2PServer();

static void lora_recv_cb(rui_lora_p2p_recv_t data);

void lora_recv_cb(rui_lora_p2p_recv_t data)
{
    p2p_msg_t req;
    DEBUG_SERIAL_PRINTF("msg received (%d):", data.BufferSize);
    for (int i=0; i < data.BufferSize; i++) {
        DEBUG_SERIAL_PRINTF("%02X", data.Buffer[i]);
    }
    DEBUG_SERIAL_PRINTF("\r\n");

    if (data.BufferSize < P2P_MSG_HEADER_SIZE) {
        DEBUG_SERIAL_PRINTF("lora_recv_cb() Error: %d bytes received lower than %d\r\n",
                data.BufferSize, P2P_MSG_HEADER_SIZE);
        return;
    }

    if (memcpy(req.src_deui, data.Buffer, 8) == NULL) {
        DEBUG_SERIAL_PRINTF("lora_recv_cb() Error: unable to get request.src_deui\r\n");
        return;
    }

    if (memcpy(req.dst_deui, data.Buffer+8, 8) == NULL) {
        DEBUG_SERIAL_PRINTF("lora_recv_cb() Error: unable to get request.dst_deui\r\n");
        return;
    }

    req.type = data.Buffer[16];
    req.length = data.Buffer[17];

    for (int i=0; i < req.length && i < P2P_PAYLOAD_MAX_SIZE; i++) {
        req.payload[i] = data.Buffer[i+18];
    }
    CfgServer.handle_msg(req);
}

P2PServer::P2PServer()
{
}


bool P2PServer::enable()
{
    bool is_ok;
    this->is_enabled = false;

    is_ok = api.lora.nwm.set();
    DEBUG_SERIAL_PRINTF("Set Node device work mode      %s\r\n", (is_ok) ? "Success" : "Fail");

    is_ok = api.lora.pfreq.set(900000000);
    DEBUG_SERIAL_PRINTF("Set P2P mode frequency         %s\r\n", (is_ok) ? "Success" : "Fail");
    is_ok = api.lora.psf.set(12);
    DEBUG_SERIAL_PRINTF("Set P2P mode spreading factor  %s\r\n", (is_ok) ? "Success" : "Fail");
    is_ok = api.lora.pbw.set(125);
    DEBUG_SERIAL_PRINTF("Set P2P mode bandwidth         %s\r\n", (is_ok) ? "Success" : "Fail");
    is_ok = api.lora.pcr.set(0);
    DEBUG_SERIAL_PRINTF("Set P2P mode code rate         %s\r\n", (is_ok) ? "Success" : "Fail");
    is_ok = api.lora.ppl.set(100);
    DEBUG_SERIAL_PRINTF("Set P2P mode preamble length   %s\r\n", (is_ok) ? "Success" : "Fail");
    is_ok = api.lora.ptp.set(22);
    DEBUG_SERIAL_PRINTF("Set P2P mode tx power          %s\r\n", (is_ok) ? "Success" : "Fail");
    is_ok = api.lora.registerPRecvCallback(lora_recv_cb);
    DEBUG_SERIAL_PRINTF("lora_recv_cb() load            %s\r\n", (is_ok) ? "Success" : "Fail");

    if (is_ok == true) {
        Serial.printf(
                "==== P2PServer Enable Success =====\r\n"
                "frequency: %9dHz \tbandwidth: %dkHz\r\n"
                "Spreading Factor: %d \tTx power: %d\r\n"
                "Preamble Length: %d \tCode Rate: %d\r\n"
                "===================================\r\n",
                api.lora.pfreq.get(), api.lora.pbw.get(),
                api.lora.psf.get(), api.lora.ptp.get(),
                api.lora.ppl.get(), api.lora.pcr.get()
                );

        this->is_enabled = true;
        return true;
    } else {
        Serial.printf("setup() Error initializing LoRa Interface\r\n");
    }
}

bool P2PServer::save()
{
    /* LoRa Params */
    bool ret;

    ret = api.lorawan.mask.set(&(this->lorawan_parameters->sub_band_mask));
    RETURN_IF_FALSE(ret, false, "P2PServer::commit() Error: unable to set LoRaWAN mask\r\n");

    ret = api.lorawan.adr.set((bool)this->lorawan_parameters->adr_enabled);
    RETURN_IF_FALSE(ret, false, "P2PServer::commit() Error: unable to set ADR\r\n");

    ret = api.lorawan.dr.set(this->lorawan_parameters->dr);
    RETURN_IF_FALSE(ret, false, "P2PServer::commit() Error: unable to set DR value\r\n");

    ret = api.lorawan.rx1dl.set(this->lorawan_parameters->rx1_delay);
    RETURN_IF_FALSE(ret, false, "P2PServer::commit() Error: unable to set rx1dl\r\n");

    ret = api.lorawan.rx2dl.set(this->lorawan_parameters->rx2_delay);
    RETURN_IF_FALSE(ret, false, "P2PServer::commit() Error: unable to set rx2dl\r\n");

    ret = api.lorawan.rety.set(this->lorawan_parameters->retry);
    RETURN_IF_FALSE(ret, false, "P2PServer::commit() Error: unable to set rety\r\n");

    ret = api.lorawan.nwm.set();
    RETURN_IF_FALSE(ret, false, "P2PServer::commit() Error: unable to set LoRaWAN mode\r\n");

    return true;
}


bool P2PServer::listen(
        sLoRa *lorawan_credentials,
        sLoRaParameters *lorawan_params,
        EdLoTiming *send_params_cfg,
        EdLoDatetime *datetime_cfg)
{
    bool is_ok;
    if (this->is_enabled == false) {
        Serial.printf("P2PServer is not enabled\r\n");
        return false;
    }

    this->lorawan_credentials = lorawan_credentials;
    this->lorawan_parameters = lorawan_params;
    this->edlo_timinig_cfgs = send_params_cfg;
    this->edlo_datetime_cfgs = datetime_cfg;

    this->is_listening = true;
    is_ok = api.lora.precv(65533); // Permanent RX mode and still allows TX without calling
    uint32_t now = millis();
    while (this->is_listening && this->is_enabled) {
        //if (millis() - now > 60 * 1000) {
        //    this->is_listening = false;
        //}
        Serial.printf("P2P Listen Mode\r\n");
        delay(10000);
    }
    api.lora.precv(0);
}

bool P2PServer::pong(p2p_msg_t *resp, p2p_msg_t req)
{
    if (memcpy(resp->dst_deui, req.src_deui, 8) == NULL) {
        DEBUG_SERIAL_PRINTF("P2PServer::pong() Error: unable to set dst address");
        return false;
    }
    if (memcpy(resp->src_deui, req.dst_deui, 8) == NULL) {
        DEBUG_SERIAL_PRINTF("P2PServer::pong() Error: unable to set src address");
        return false;
    }

    resp->type = (uint8_t)((req.length == 0) ? P2P_RESP_PONG : P2P_RESP_INVALID);
    resp->length = 0;
    return true;
}

bool P2PServer::handle_get(p2p_msg_t *resp, p2p_msg_t req)
{
    if (memcpy(resp->dst_deui, req.src_deui, 8) == NULL) {
        DEBUG_SERIAL_PRINTF("P2PServer::pong() Error: unable to set dst address");
        return false;
    }
    if (memcpy(resp->src_deui, req.dst_deui, 8) == NULL) {
        DEBUG_SERIAL_PRINTF("P2PServer::pong() Error: unable to set src address");
        return false;
    }
    int ret;

    char *payload = (char *)req.payload;
    DEBUG_SERIAL_PRINTF("ascii payload: %s\r\n", (char *)req.payload);
    if (req.length == 0 || payload[req.length] != '\0') {
        resp->length = 0;
        resp->type = (uint8_t)P2P_RESP_INVALID;
        DEBUG_SERIAL_PRINTF("request payload length == 0 or non-null terminated string (%02X)\r\n",
                payload[req.length]);
        return true;
    }

    if (!strncmp(payload, "LORAWAN", P2P_PAYLOAD_MAX_SIZE)) {
        ret = snprintf((char *)resp->payload, P2P_PAYLOAD_MAX_SIZE,
                    "LORAWAN dr=%d adr=%d retries=%d rxdl1=%d rxdl2=%d",
                    this->lorawan_parameters->dr,
                    this->lorawan_parameters->adr_enabled,
                    this->lorawan_parameters->retry,
                    this->lorawan_parameters->rx1_delay,
                    this->lorawan_parameters->rx2_delay);
    } else if (!strncmp(payload, "LORACRED", P2P_PAYLOAD_MAX_SIZE)) {
        ret = snprintf((char *)resp->payload, P2P_PAYLOAD_MAX_SIZE,
                    "CREDENTIALS\r\n"
                    "\tDEV_KEY=%02x%02x%02x%02x%02x%02x%02x%02x\r\n",
                    "\tAPP_EUI=%02x%02x%02x%02x%02x%02x%02x%02x\r\n",
                    "\tAPP_KEY=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",
                    DEVEUI(this->lorawan_credentials->dev_eui),
                    DEVEUI(this->lorawan_credentials->app_eui),
                    DEVEUI(this->lorawan_credentials->app_key));
    } else if (!strncmp(payload, "DATETIME", P2P_PAYLOAD_MAX_SIZE)) {
        ret = snprintf((char *)resp->payload, P2P_PAYLOAD_MAX_SIZE,
                    "DATETIME (yOff-m-d HH:MM:SS)=%02d-%02d-%02d %02d:%02d:%02d",
                    this->edlo_datetime_cfgs->yOff,
                    this->edlo_datetime_cfgs->m,
                    this->edlo_datetime_cfgs->d,
                    this->edlo_datetime_cfgs->hh,
                    this->edlo_datetime_cfgs->mm,
                    this->edlo_datetime_cfgs->ss);
    } else if (!strncmp(payload, "TIMING", P2P_PAYLOAD_MAX_SIZE)) {
        ret = snprintf((char *)resp->payload, P2P_PAYLOAD_MAX_SIZE,
                    "TIMING delta_max_period=%d period_to_send=%d\r\n"
                    "\tLAST_SAVED (yOff-m-d HH:MM:SS) %02d-%02d-%02d %02d:%02d:%02d\r\n",
                    this->edlo_timinig_cfgs->delta_max_period,
                    this->edlo_timinig_cfgs->period_to_send,
                    this->edlo_timinig_cfgs->LastDatetimeSaved.yOff,
                    this->edlo_timinig_cfgs->LastDatetimeSaved.m,
                    this->edlo_timinig_cfgs->LastDatetimeSaved.d,
                    this->edlo_timinig_cfgs->LastDatetimeSaved.hh,
                    this->edlo_timinig_cfgs->LastDatetimeSaved.mm,
                    this->edlo_timinig_cfgs->LastDatetimeSaved.ss);
    } else {
        ret = snprintf((char *)resp->payload, P2P_PAYLOAD_MAX_SIZE,
                    "'%s' Not implemented", payload);
    }

    

    if (ret < 0) {
        DEBUG_SERIAL_PRINTF("handle_msg() Error: snprintf returned %d", ret);
        resp->type = (uint8_t)P2P_RESP_INVALID;
        resp->length = 0;
        return false;
    } else if (ret + 1 > P2P_PAYLOAD_MAX_SIZE) {
        DEBUG_SERIAL_PRINTF("handle_msg() Error: returned %d", ret);
        resp->type = (uint8_t)P2P_RESP_SERVER_ERROR;
        resp->length = 0;
        return false;
    } else {
        resp->type = P2P_RESP_OK;
        resp->length = ret+1;
    }
    return true;
}

void P2PServer::handle_msg(p2p_msg_t req)
{
    if (memcmp(req.dst_deui, this->lorawan_credentials->dev_eui, 8)) {
        DEBUG_SERIAL_PRINTF("Ignoring message: ");
        DEBUG_SERIAL_PRINTF(DEVEUI_STR " != " DEVEUI_STR "\r\n",
                DEVEUI(req.dst_deui), DEVEUI(this->lorawan_credentials->dev_eui));
        return;
    }

    DEBUG_SERIAL_PRINTF("received message:");
    DEBUG_SERIAL_PRINTF(" src=" DEVEUI_STR, DEVEUI(req.src_deui));
    DEBUG_SERIAL_PRINTF(" dst=" DEVEUI_STR, DEVEUI(req.dst_deui));
    DEBUG_SERIAL_PRINTF(" type=%d", req.type);
    DEBUG_SERIAL_PRINTF(" length=%d\r\n", req.length);

    bool is_ok;
    uint8_t *out_data, out_data_size;
    p2p_msg_t resp;
    switch ((p2p_msg_type_e)req.type) {
    case P2P_REQ_PING:
        is_ok = this->pong(&resp, req);
        break;
    case P2P_REQ_GET:
        is_ok = this->handle_get(&resp, req);
        break;
    default:
        is_ok = false;
        break;
    }

    if (is_ok == false) {
        DEBUG_SERIAL_PRINTF("P2P::handle_msg() Error: error with msg\r\n");
        delay(4000);    // this prevents MCU stall
        return;
    }

    out_data = (uint8_t *)&resp;
    out_data_size = P2P_MSG_HEADER_SIZE + resp.length;

    is_ok = api.lora.psend(out_data_size, out_data);
    if (is_ok == false) {
        DEBUG_SERIAL_PRINTF("P2P::handle_msg() Error: lora.psend() call failed\r\n");
        return;
    }

    DEBUG_SERIAL_PRINTF("response msg(%d): ", out_data_size);
    for (int i=0; i < out_data_size; i++) {
        DEBUG_SERIAL_PRINTF("%02X", out_data[i]);
    }
    DEBUG_SERIAL_PRINTF("\r\n");
}
