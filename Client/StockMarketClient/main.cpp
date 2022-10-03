#include "net_client.h"
#include "ms_types.h"
#include "client.h"

int kbhit(void)
{
    struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    auto ch = getchar();
    ungetc(ch, stdin);
    return ch;
}

int myrandom(int start, int end)
{
    return start + (std::rand() % ( end - start + 1));
}

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

    bool flag = true;
    while (1)
    {
        if (!input.empty())
        {
            auto in = input.pop_front();
            if (in.size() == 1)
            {
                switch (in[0])
                {
                case '1':
//                    flag = false;
                    std::cout << "[*]REQUESTING ALL ACTIVE OFFERS\n";
                    client.req_offers(SM_REQ_ALL_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
                    break;
                case '2':
                    std::cout << "[*]PUBLISHING SELL OFFER\n";
                    client.publish_new_offer(SM_BUY, SM_CUR_USD, SM_CUR_RUB, 10, 62);
                    break;
                case '3':
                    std::cout << "[*]PUBLISHING BUY OFFER\n";
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
                case 'q':
                    flag = true;
                    break;
                default:
                    break;
                }
            }
        }

        if (!client.incoming().empty())
        {
            std::cout << "NEW MSG\n";
            auto msg = client.incoming().pop_front().msg;

            switch (msg.hdr.id)
            {
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
                    break;
                }
                default:
                {
                    std::cout << "[!]UNEXPECTED TYPE:" << msg.hdr.id << "\n";
                    break;
                }
            }
        }

//        if (!flag)
//        {
//            std::cout << "\033c";

//            std::vector<std::pair<int, int>> offs(myrandom(1, 10));
//            for (auto& off: offs)
//            {
//                off.first = myrandom(1, 100);
//                off.second = myrandom(1, 100);
//            }

//            std::cout << "======================================\n";
//            for (auto& off: offs)
//                std::cout << off.first << ":" << off.second << "\n";

//            usleep(100*1000);
//        }
    }

    return 0;
}

