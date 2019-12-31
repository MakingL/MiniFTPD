//
// Created by MLee on 2019/12/31.
//

#include "configure.h"
#include "common.h"
#include "utility.h"

namespace configure {
    const char *config_file = "config.yaml";    /* 配置文件 */

    std::string SERVER_LISTEN_HOST;
    int SERVER_LISTEN_PORT;

    void parse_config_file() {
        YAML::Node node = YAML::LoadFile(configure::config_file);

        if (!node["server"].IsDefined()) {
            std::cerr << "Server config information isn't configured correctly" << std::endl;
            return;
        }

        auto server_config = node["server"];
        SERVER_LISTEN_HOST = server_config["listen_host"].as<std::string>();
        SERVER_LISTEN_PORT = server_config["listen_port"].as<int>();

        utility::debug_info(std::string("Server listen host: ") + SERVER_LISTEN_HOST);
        utility::debug_info(std::string("Server listen port: ") + std::to_string(SERVER_LISTEN_PORT));

    }

}