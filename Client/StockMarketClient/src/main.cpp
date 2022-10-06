#include "client.h"
#include <limits>

void show_menu()
{
    std::cout << "\033c"; // clear screen
    std::cout << "================================================================================\n"
                 "Enter:\n"
                 "* 1 to see all active offers\n\n"
                 "* 2 followed by pair of integers - USD volume and price in RUB - to publish BUY offer( 2 <volume> <price> )\n"
                 "Example: 2 10 62 (an order to buy 10 USD at a price of 62 RUB/USD)\n\n"
                 "* 3 followed by pair of integers - USD volume and price in RUB - to publish SELL offer( 3 <volume> <price> )\n"
                 "Example: 3 50 61 (an order to sell 50 USD at a price of 61 RUB/USD)\n\n"
                 "* 4 to see your balance\n\n"
                 "* 5 to show your active offers\n\n"
                 "* 6 to show your completed offers\n\n"
                 "================================================================================\n\n";
}

std::optional<std::pair<uint32_t, uint32_t>> parse_user_publish(std::string& in)
{
    size_t delimPos = 0;
    std::pair<uint32_t, uint32_t> res;

    if ((delimPos = in.find(' ')) == std::string::npos)
    {
        std::cout << "[-]First delimiter(' ') not found\n";
        return std::nullopt;
    }
    in.erase(0, delimPos + 1);
    if ((delimPos = in.find(' ')) == std::string::npos)
    {
        std::cout << "[-]Second delimiter(' ') not found\n";
        return std::nullopt;
    }

    auto volumeStr = in.substr(0, delimPos);
    auto priceStr = in.substr(delimPos+1, in.size());

    if (!std::all_of(volumeStr.begin(), volumeStr.end(), ::isdigit) ||
        !std::all_of(priceStr.begin(), priceStr.end(), ::isdigit))
    {
        std::cout << "[-]Incorrect volume or price\n";
        return std::nullopt;
    }

    try {
        res.first = std::stoul(volumeStr);
        res.second = std::stoul(priceStr);
    } catch (...) {
        std::cout << "[-]Incorrect volume or price\n";
        return std::nullopt;
    }

    return res;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: ./client <server ip> <server port>\n";
        return 1;
    }

    uint16_t port = 0;
    std::string ip(argv[1]);
    try {
        port = uint16_t(std::stoi(argv[2]));
    } catch (...) {
        std::cout << "Incorrect port number: " << port << "\n";
        return 1;
    }

    std::cout << "Connecting to " << ip << ":" << port << " ...\n";

    // connect to server
    SMClient client;
    if (!client.connect(ip, port))
        return 1;

    // launch user input thread
    tps::net::tsqueue<std::string>input;
    std::thread userInputThread = std::thread([&input]()
    {
        std::string in;
        while (1)
        {
            std::getline(std::cin, in);
            input.push_back(in);
        }
    });
    userInputThread.detach();

    // wait for CONNACK
    client.incoming().wait();
    client.incoming().pop_front();

    show_menu();
    while (1)
    {
        input.wait();
        if (!input.empty())
        {
            auto in = input.pop_front();
            show_menu();
            switch (in[0])
            {
                case '1':
                    std::cout << "[*]CURRENT OFFERS:\n";
                    client.req_offers(SM_REQ_ALL_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
                    break;
                case '2':
                {
                    auto res = parse_user_publish(in);
                    if (res)
                    {
                        auto [volume, price] = res.value();
                        std::cout << "[*]PUBLISHING BUY OFFER: " << volume << " USD for " << price << " RUB/USD\n";
                        client.publish_new_offer(SM_BUY, SM_CUR_USD, SM_CUR_RUB, volume, price);
                        break;
                    }
                    continue;
                }
                case '3':
                {
                    auto res = parse_user_publish(in);
                    if (res)
                    {
                        auto [volume, price] = res.value();
                        std::cout << "[*]PUBLISHING SELL OFFER: " << volume << " USD for " << price << " RUB/USD\n";
                        client.publish_new_offer(SM_SELL, SM_CUR_USD, SM_CUR_RUB, volume, price);
                        break;
                    }
                    continue;
                }
                case '4':
                    std::cout << "[*]CURRENT BALANCE:\n";
                    client.req_balance({SM_CUR_USD, SM_CUR_RUB});
                    break;
                case '5':
                    std::cout << "[*]YOUR ACTIVE OFFERS:\n";
                    client.req_offers(SM_REQ_CLIENT_ACTIVE_OFFS, SM_CUR_USD, SM_CUR_RUB);
                    break;
                case '6':
                    std::cout << "[*]YOUR COMPLETED OFFERS:\n";
                    client.req_offers(SM_REQ_PAST_OFFS, SM_CUR_USD, SM_CUR_RUB);
                    break;
                default:
                    std::cout << "[-]UNKNOWN COMMAND:" << in << "\n";
                    continue;
            }
        }

        client.incoming().wait();
        while (!client.incoming().empty())
        {
            auto msg = client.incoming().pop_front().msg;
            switch (msg.hdr.id)
            {
                case SM_CONNACK:
                case SM_PUBLISH_ACK:
                    break;
                case SM_PUBLISH_REJECTED_ACK:
                    std::cout << "[-]SERVER REJECTED YOUR OFFER\nPossible casuse: offer completion exceeds balance cap\n";
                    break;
                case SM_REQ_BALANCE_ACK:
                {
                    sm_req_balance_ack ack;
                    ack.unpack(msg);

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
                    std::cout << "[!]UNEXPECTED TYPE:" << msg.hdr.id << "\n";
                    break;
            }
        }
    }

    return 0;
}

