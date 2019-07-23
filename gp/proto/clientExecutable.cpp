//
// Created by Jaloliddin Erkiniy on 9/11/18.
//
#include "gp_client.h"
#include "gp_client_data.h"

#include <thread>

int main()
{
    using namespace std::chrono_literals;
    gp_environment env = { 0, 0, false, strdup("testPass"), strdup("MacBook Pro 13"), strdup("10.14.15"), strdup("0.0.1"), strdup("en"), strdup("/docs")};

    auto client = gp_client_create(env);
    gp_client_pause(client);
    gp_client_resume(client);

    while (true) {
        std::this_thread::sleep_for(2s);
    }

    return 0;
}
