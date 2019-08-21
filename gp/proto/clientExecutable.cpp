//
// Created by Jaloliddin Erkiniy on 9/11/18.
//
#include "gp_client.h"
#include "gp_client_data.h"

#include <thread>

int main()
{
    using namespace std::chrono_literals;

    gp_environment env = { .api_id = 0,
                           .layer = 0,
                           .disable_updates = 0,
                           .encryption_password = strdup("testPass"),
                           .device_model = strdup("mac mini Jalol"),
                           .system_version = strdup("10.14.15"),
                           .app_version = strdup("0.0.1"),
                           .lang_code = strdup("en"),
                           .documents_folder = strdup("/Users/jaloliddinerkiniy/Desktop/TestKeychain") };

    auto client = gp_client_create(&env);
    gp_client_pause(client);
    gp_client_resume(client);

    while (true) {
        std::this_thread::sleep_for(2s);

        auto globalTime = gp_client_get_global_time(client);
        printf("Global time is %lf\n", globalTime);
    }

    return 0;
}
