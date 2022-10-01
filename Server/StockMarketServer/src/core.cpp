#include "core.h"

std::optional<std::reference_wrapper<pClient>> core::find_client(pConnection &netClient)
{
    auto it = clients.find(netClient);
    if (it != clients.end())
        return it->second;

    return std::nullopt;
}

pClient &core::add_new_client(pConnection &netClient)
{
    auto newClient = std::make_shared<client_t>(netClient);
    auto it = clients.emplace(std::move(netClient), std::move(newClient));

//    return it.first->second;
}

void core::delete_client(pClient &client)
{

}
