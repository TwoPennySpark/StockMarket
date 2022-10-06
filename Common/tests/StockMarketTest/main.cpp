#include "server.h"
#include "client.h"

const std::string SERV_ADDR = "127.0.0.1";
uint16_t SERV_PORT = 5000;

void server_launch()
{
    static std::thread serverThread([]()
    {
        SMServer server(SERV_PORT);
        server.start();
        server.update();
    });
    serverThread.detach();
    sleep(1); // postpone clients start
}

tps::net::message<packet_type> get_msg(SMClient& client);

std::unique_ptr<SMClient> client_launch()
{
    auto client = std::make_unique<SMClient>();
    client->connect(SERV_ADDR, SERV_PORT);
    get_msg(*client); // get SM_CONNACK

    return client;
}

tps::net::message<packet_type> get_msg(SMClient& client)
{
    if (client.incoming().empty())
        client.incoming().wait();

    return client.incoming().pop_front().msg;
}

template <typename T>
T* get_pkt(SMClient& client)
{
    auto msg = get_msg(client);
    auto m = sm_packet::create(msg);

    return dynamic_cast<T*>(m.release());
}

void test1()
{ // from example
    auto client1 = client_launch();
    auto client2 = client_launch();
    auto client3 = client_launch();

    int32_t c1Volume = 10, c1Price = 62;
    int32_t c2Volume = 20, c2Price = 63;
    int32_t c3Volume = 50, c3Price = 61;

    // one is selling, two are buying
    client1->publish_new_offer(SM_BUY, SM_CUR_USD, SM_CUR_RUB, c1Volume, c1Price); get_msg(*client1);
    client2->publish_new_offer(SM_BUY, SM_CUR_USD, SM_CUR_RUB, c2Volume, c2Price); get_msg(*client2);
    client3->publish_new_offer(SM_SELL, SM_CUR_USD, SM_CUR_RUB, c3Volume, c3Price);get_msg(*client3);

    // get balance
    {
    client1->req_balance({SM_CUR_USD, SM_CUR_RUB});
    client2->req_balance({SM_CUR_USD, SM_CUR_RUB});
    client3->req_balance({SM_CUR_USD, SM_CUR_RUB});

    auto pkt1 = get_pkt<sm_req_balance_ack>(*client1);
    auto pkt2 = get_pkt<sm_req_balance_ack>(*client2);
    auto pkt3 = get_pkt<sm_req_balance_ack>(*client3);

    assert(pkt1->vBalance[0].second == c1Volume);
    assert(pkt1->vBalance[1].second == -c1Volume*c1Price);
    assert(pkt2->vBalance[0].second == c2Volume);
    assert(pkt2->vBalance[1].second == -c2Volume*c2Price);
    assert(pkt3->vBalance[0].second == -c1Volume-c2Volume);
    assert(pkt3->vBalance[1].second == c1Volume*c1Price + c2Volume*c2Price);
    }

    // get all active offers - only one remaining from client3
    {
    client1->req_offers(SM_REQ_ALL_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
    auto pkt1 = get_pkt<sm_req_offs_ack>(*client1);
    auto& [side, volume, price] = pkt1->vOffs[0];
    assert(side == SM_SELL && volume == (c3Volume-c2Volume-c1Volume) && price == c3Price);
    }

    // get client3's active offers - only one remaining, same as before
    {
    client3->req_offers(SM_REQ_CLIENT_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
    auto pkt1 = get_pkt<sm_req_offs_ack>(*client3);
    auto& [side, volume, price] = pkt1->vOffs[0];
    assert(side == SM_SELL && volume == (c3Volume-c2Volume-c1Volume) && price == c3Price);
    }

    // get completed offers: one from client1, one from client2 and two partial offers from client3
    {
    client1->req_offers(SM_REQ_PAST_OFFS, SM_CUR_USD, SM_CUR_RUB);
    client2->req_offers(SM_REQ_PAST_OFFS, SM_CUR_USD, SM_CUR_RUB);
    client3->req_offers(SM_REQ_PAST_OFFS, SM_CUR_USD, SM_CUR_RUB);

    auto pkt1 = get_pkt<sm_req_offs_ack>(*client1);
    auto pkt2 = get_pkt<sm_req_offs_ack>(*client2);
    auto pkt3 = get_pkt<sm_req_offs_ack>(*client3);

    auto& [side1, volume1, price1] = pkt1->vOffs[0];
    assert(side1 == SM_BUY && volume1 == c1Volume && price1 == c1Price);

    auto& [side2, volume2, price2] = pkt2->vOffs[0];
    assert(side2 == SM_BUY && volume2 == c2Volume && price2 == c2Price);

    auto& [side30, volume30, price30] = pkt3->vOffs[0];
    auto& [side31, volume31, price31] = pkt3->vOffs[1];
    assert(side30 == SM_SELL && volume30 == c2Volume && price30 == c2Price);
    assert(side31 == SM_SELL && volume31 == c1Volume && price31 == c1Price);
    }
}

void test2()
{ // test1() reverse
    auto client1 = client_launch();
    auto client2 = client_launch();
    auto client3 = client_launch();

    int32_t c1Volume = 10, c1Price = 61;
    int32_t c2Volume = 20, c2Price = 62;
    int32_t c3Volume = 50, c3Price = 63;

    // two are selling, one is buying
    client1->publish_new_offer(SM_SELL, SM_CUR_USD, SM_CUR_RUB, c1Volume, c1Price); get_msg(*client1);
    client2->publish_new_offer(SM_SELL, SM_CUR_USD, SM_CUR_RUB, c2Volume, c2Price); get_msg(*client2);
    client3->publish_new_offer(SM_BUY, SM_CUR_USD, SM_CUR_RUB, c3Volume, c3Price);get_msg(*client3);

    // get balance
    {
    client1->req_balance({SM_CUR_USD, SM_CUR_RUB});
    client2->req_balance({SM_CUR_USD, SM_CUR_RUB});
    client3->req_balance({SM_CUR_USD, SM_CUR_RUB});

    auto pkt1 = get_pkt<sm_req_balance_ack>(*client1);
    auto pkt2 = get_pkt<sm_req_balance_ack>(*client2);
    auto pkt3 = get_pkt<sm_req_balance_ack>(*client3);

    assert(pkt1->vBalance[0].second == -c1Volume);
    assert(pkt1->vBalance[1].second == c1Volume*c3Price);
    assert(pkt2->vBalance[0].second == -c2Volume);
    assert(pkt2->vBalance[1].second == c2Volume*c3Price);
    assert(pkt3->vBalance[0].second == c1Volume+c2Volume);
    assert(pkt3->vBalance[1].second == -c1Volume*c3Price-c2Volume*c3Price);
    }

    // get all active offers - only one remaining from client3
    {
    client1->req_offers(SM_REQ_ALL_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
    auto pkt1 = get_pkt<sm_req_offs_ack>(*client1);
    auto& [side, volume, price] = pkt1->vOffs[0];
    assert(side == SM_BUY && volume == (c3Volume-c2Volume-c1Volume) && price == c3Price);
    }

    // get client3's active offers - only one remaining, same as before
    {
    client3->req_offers(SM_REQ_CLIENT_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
    auto pkt1 = get_pkt<sm_req_offs_ack>(*client3);
    auto& [side, volume, price] = pkt1->vOffs[0];
    assert(side == SM_BUY && volume == (c3Volume-c2Volume-c1Volume) && price == c3Price);
    }

    // get completed offers: one from client1, one from client2 and two partial offers from client3
    {
    client1->req_offers(SM_REQ_PAST_OFFS, SM_CUR_USD, SM_CUR_RUB);
    client2->req_offers(SM_REQ_PAST_OFFS, SM_CUR_USD, SM_CUR_RUB);
    client3->req_offers(SM_REQ_PAST_OFFS, SM_CUR_USD, SM_CUR_RUB);

    auto pkt1 = get_pkt<sm_req_offs_ack>(*client1);
    auto pkt2 = get_pkt<sm_req_offs_ack>(*client2);
    auto pkt3 = get_pkt<sm_req_offs_ack>(*client3);

    auto& [side1, volume1, price1] = pkt1->vOffs[0];
    assert(side1 == SM_SELL && volume1 == c1Volume && price1 == c3Price);

    auto& [side2, volume2, price2] = pkt2->vOffs[0];
    assert(side2 == SM_SELL && volume2 == c2Volume && price2 == c3Price);

    auto& [side30, volume30, price30] = pkt3->vOffs[0];
    auto& [side31, volume31, price31] = pkt3->vOffs[1];
    assert(side30 == SM_BUY && volume30 == c1Volume && price30 == c3Price);
    assert(side31 == SM_BUY && volume31 == c2Volume && price31 == c3Price);
    }
}

void test3()
{// check non-crossing prices
    auto client1 = client_launch();
    auto client2 = client_launch();

    uint32_t c1Volume = 100, c1Price = 61;
    uint32_t c2Volume = 200, c2Price = 62;

    // one is selling, one is buying
    client1->publish_new_offer(SM_BUY, SM_CUR_USD, SM_CUR_RUB, c1Volume, c1Price); get_msg(*client1);
    client2->publish_new_offer(SM_SELL, SM_CUR_USD, SM_CUR_RUB, c2Volume, c2Price); get_msg(*client2);

    // get active offers
    client1->req_offers(SM_REQ_ALL_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
    auto pkt1 = get_pkt<sm_req_offs_ack>(*client1);
    auto& [side1, volume1, price1] = pkt1->vOffs[0];
    assert(side1 == SM_BUY && volume1 == (c1Volume) && price1 == c1Price);
    auto& [side2, volume2, price2] = pkt1->vOffs[1];
    assert(side2 == SM_SELL && volume2 == (c2Volume) && price2 == c2Price);
}

void test4()
{// check multiple offers covered by one
    auto client1 = client_launch();
    auto client2 = client_launch();

    std::vector<uint32_t> c1Volume(10, 10);
    uint32_t c1Price = 10;
    for (size_t i = 0; i < c1Volume.size(); i++)
        client1->publish_new_offer(SM_BUY, SM_CUR_USD, SM_CUR_RUB, c1Volume[i], c1Price); get_msg(*client1);

    uint32_t c2Volume = 100, c2Price = 5;
    client2->publish_new_offer(SM_SELL, SM_CUR_USD, SM_CUR_RUB, c2Volume, c2Price); get_msg(*client2);

    client2->req_offers(SM_REQ_ALL_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
    auto pkt1 = get_pkt<sm_req_offs_ack>(*client2);
    assert(pkt1->vOffs.size() == 0);

    client2->req_offers(SM_REQ_PAST_OFFS, SM_CUR_USD, SM_CUR_RUB);
    auto pkt2 = get_pkt<sm_req_offs_ack>(*client2);
    assert(pkt2->vOffs.size() == c1Volume.size());
}

void test5()
{// check greater values
    auto client1 = client_launch();

    uint32_t c1Volume = std::numeric_limits<uint32_t>::max(), c1Price = std::numeric_limits<uint32_t>::max();
    uint32_t c2Volume = std::numeric_limits<uint16_t>::max(), c2Price = std::numeric_limits<uint16_t>::max();

    client1->publish_new_offer(SM_BUY, SM_CUR_USD, SM_CUR_RUB, c1Volume, c1Price);
    auto pkt1 = get_pkt<sm_packet>(*client1);
    assert(pkt1->type == SM_PUBLISH_REJECTED_ACK);
    client1->publish_new_offer(SM_SELL, SM_CUR_USD, SM_CUR_RUB, c2Volume, c2Price);
    auto pkt2 = get_pkt<sm_packet>(*client1);
    assert(pkt2->type == SM_PUBLISH_REJECTED_ACK);

    client1->req_offers(SM_REQ_ALL_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
    auto pkt3 = get_pkt<sm_req_offs_ack>(*client1);
    assert(pkt3->vOffs.size() == 0);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: ./test <port number>\n";
        return 1;
    }

    try {
        SERV_PORT = uint16_t(std::stoi(argv[1]));
    } catch (...) {
        std::cout << "[-]Incorrect port number:" << SERV_PORT << "\n";
        return 1;
    }

    server_launch();
    test1();
    test2();
    test3();
    test4();
    test5();

    sleep(1);
    std::cout << "======================================================\n";
    std::cout << "[+]TESTS PASSED SUCCESSFULLY\n";
    exit(0);
}
