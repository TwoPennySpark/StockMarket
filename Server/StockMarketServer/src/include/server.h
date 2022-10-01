#ifndef SERVER_H
#define SERVER_H

#include "net_server.h"
#include "ms_types.h"
#include "core.h"

class SMServer: public tps::net::server_interface<packet_type>
{
public:
    SMServer(uint16_t port): server_interface(port) {}
    ~SMServer() override {}

protected:
    virtual bool on_client_connect    (pConnection client) override;
    virtual void on_client_disconnect (pConnection client) override;
//    virtual bool on_first_message     (pConnection netClient,
//                                       tps::net::message<packet_type>& msg) override;

    virtual void on_message(pConnection netClient,
                            tps::net::message<packet_type>& msg) override;

private:
//    void handle_connect     (pConnection& netClient, sm_connect& pkt);

    core_t m_core;
};

#endif
