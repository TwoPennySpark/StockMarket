#ifndef SERVER_H
#define SERVER_H

#include "net_server.h"
#include "core.h"

class SMServer: public tps::net::server_interface<packet_type>
{
public:
    SMServer(uint16_t port): server_interface(port) {}
    ~SMServer() override {}

    void send_reply(pConnection& netClient, sm_packet& reply);

    void handle_connect         (pConnection& netClient);
    void handle_publish_off     (pClient& client, sm_publish& pkt);
    void handle_balance_req     (pClient& client, sm_req_balance& pkt);
    void handle_active_offs_req (pClient& client, sm_req_offs& pkt);
    void handle_all_offs_req    (pClient& client, sm_req_offs& pkt);
    void handle_past_offs_req   (pClient& client, sm_req_offs& pkt);
    void handle_error           (pClient& client);

protected:
    virtual bool on_client_connect    (pConnection client) override;
    virtual void on_client_disconnect (pConnection client) override;

    virtual void on_message(pConnection netClient,
                            tps::net::message<packet_type>& msg) override;

private:
    Core m_core;
};

#endif
