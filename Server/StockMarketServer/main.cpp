#include "server.h"

typedef struct test test_t;

using pTest = std::shared_ptr<test_t>;

typedef struct test
{
    bool operator< (const pTest& off) const
    { return price < off->price; }

    test(int _price): price(_price) {}

    int price;
    int volume;
}test_t;


int main()
{
    std::multiset<pTest> lol;
    pTest off1 = std::make_shared<test_t>(10);
    pTest off2 = std::make_shared<test_t>(20);
    pTest off3 = std::make_shared<test_t>(30);
    pTest off4 = std::make_shared<test_t>(40);
    pTest off41 = std::make_shared<test_t>(40);

    lol.emplace(off1);lol.emplace(off2);
    lol.emplace(off3);lol.emplace(off4);
    lol.emplace(off41);

    std::vector<std::multiset<pTest>::iterator> kek;
    for (auto it = lol.begin(); it != lol.end(); it++)
        if (((*it)->price / 10) % 2 == 1)
            kek.emplace_back(it);

    SMServer server(5000);
    server.start();
    server.update();

    std::cout << "END\n";
    return 0;
}
