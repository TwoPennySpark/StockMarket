#include "net_client.h"
#include "ms_types.h"
#include "client.h"

//#include <set>

//typedef struct test test_t;
//using pTest = std::shared_ptr<test_t>;
//typedef struct test
//{
//    bool operator< (const pTest& off) const
//    { return price < off->price; }

//    test(int _price): price(_price) {}

//    int price;
//    int volume;
//}test_t;

//std::deque<pTest> lol;
//pTest off1 = std::make_shared<test_t>(10);
//pTest off2 = std::make_shared<test_t>(20);
//pTest off3 = std::make_shared<test_t>(30);
//pTest off4 = std::make_shared<test_t>(40);
//pTest off41 = std::make_shared<test_t>(40);

//lol.emplace_back(off1);lol.emplace_back(off2);
//lol.emplace_back(off3);lol.emplace_back(off4);
//lol.emplace_back(off41);

//std::vector<std::deque<pTest>::iterator> kek;
//for (std::deque<pTest>::iterator it = lol.begin(); it != lol.end(); it++)
//    if (((*it)->price / 10) % 2 == 1)
//        kek.emplace_back(it);

//for (auto& it: kek)
//{
//    std::cout << (*it)->price << "\n";
//    lol.erase(it);
//}

//return 0;

int main()
{
    SMClient client;

    std::string ip = "127.0.0.1";
//    std::cout << "[*]Enter server ip:\n";
//    std::getline(std::cin, ip);

    client.connect(ip, 5000);

    tps::net::tsqueue<std::string>input;
    // TODO : detach
    std::thread IOThread = std::thread([&input]()
    {
        std::string in = " ";
        while (1)
        {
//            in[0] = kbhit();
            std::getline(std::cin, in);

            input.push_back(in);
        }
    });

    bool flag = false;
    while (1)
    {
        if (!input.empty())
        {
            auto in = input.pop_front();
            if (in.size() == 1 && flag)
            {
                switch (in[0])
                {
                case '1':
                    std::cout << "[*]REQUESTING ALL ACTIVE OFFERS\n";
                    client.req_offers(SM_REQ_ALL_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
                    break;
                case '2':
                    std::cout << "[*]PUBLISHING BUY OFFER\n";
                    client.publish_new_offer(SM_BUY, SM_CUR_USD, SM_CUR_RUB, 10, 62);
                    break;
                case '3':
                    std::cout << "[*]PUBLISHING SELL OFFER\n";
                    client.publish_new_offer(SM_SELL, SM_CUR_USD, SM_CUR_RUB, 5, 60);
                    break;
                case '4':
                    std::cout << "[*]REQUESTING BALANCE\n";
                    client.req_balance({SM_CUR_USD, SM_CUR_RUB});
                    break;
                case '5':
                    std::cout << "[*]REQUESTING CLIENT's ACTIVE OFFERS\n";
                    client.req_offers(SM_REQ_CLIENT_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
                    break;
                case '6':
                    std::cout << "[*]REQUESTING PAST OFFERS\n";
                    client.req_offers(SM_REQ_PAST_OFFS, SM_CUR_USD, SM_CUR_RUB);
                    break;
                default:
                    std::cout << "[-]UNKNOWN COMMAND:" << in << "\n";
                    break;
                }
            }
        }

        if (!client.incoming().empty())
        {
            auto msg = client.incoming().pop_front().msg;
            switch (msg.hdr.id)
            {
                case SM_CONNACK:
                    std::cout << "[*]CONNECTED\n";
                    flag = true;
                    break;
                case SM_REQ_BALANCE_ACK:
                {
                    sm_req_balance_ack ack;
                    ack.unpack(msg);

                    std::cout << "YOUR BALANCE:\n";
                    for (auto& [cur, balance]: ack.vBalance)
                    {
                        switch (cur)
                        {
                            case SM_CUR_USD:
                                std::cout << "USD: " << balance << "\n";
                                break;
                            case SM_CUR_RUB:
                                std::cout << "RUB: " << balance << "\n";
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                }
                case SM_OFFS_ACK:
                {
                    sm_req_offs_ack ack;
                    ack.unpack(msg);

                    if (!ack.vOffs.size())
                        std::cout << "[*]NO SUCH OFFERS\n";

                    for (auto& [type, volume, price]: ack.vOffs)
                    {
                        std::cout << ((type == SM_SELL) ? "SELL:" : "BUY :") << " ";
                        std::cout << volume << " USD for " << price << " RUB/USD\n";
                    }
                    break;
                }
                default:
                {
                    std::cout << "[!]UNEXPECTED TYPE:" << msg.hdr.id << "\n";
                    break;
                }
            }
        }
//            std::cout << "\033c";
    }

    return 0;
}

