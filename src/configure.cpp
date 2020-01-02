//
// Created by MLee on 2019/12/31.
//

#include <cassert>
#include "configure.h"
#include "common.h"
#include "utility.h"

namespace configure {
    const char *config_file = "config.yaml";    /* 配置文件 */

    std::string SERVER_LISTEN_HOST;
    std::string FORCE_PASSIVE_SERVER_IP;
    int SERVER_LISTEN_PORT;

    int PORT_CONN_PORT;

    int PASV_PORT_LOW;
    int PASV_PORT_HIGH;

    void parse_config_file() {
        YAML::Node node = YAML::LoadFile(configure::config_file);

        if (!node["server"].IsDefined()) {
            std::cerr << "Server config information isn't configured correctly" << std::endl;
            exit(EXIT_FAILURE);
        }

        auto server_config = node["server"];
        SERVER_LISTEN_HOST = server_config["listen_host"].as<std::string>();
        SERVER_LISTEN_PORT = server_config["listen_port"].as<int>();

        utility::debug_info(std::string("Server listen host: ") + SERVER_LISTEN_HOST);
        utility::debug_info(std::string("Server listen port: ") + std::to_string(SERVER_LISTEN_PORT));
        assert(!SERVER_LISTEN_HOST.empty());
        assert(SERVER_LISTEN_PORT > 0 && SERVER_LISTEN_PORT < 65536);

        if (!node["PORT"].IsDefined()) {
            std::cerr << "Server PORT config information isn't configured correctly" << std::endl;
            exit(EXIT_FAILURE);
        }

        auto port_config = node["PORT"];
        PORT_CONN_PORT = port_config["data_connection_port"].as<int>();
        utility::debug_info(std::string("PORT data port: ") + std::to_string(PORT_CONN_PORT));

        assert(PORT_CONN_PORT > 0 && PORT_CONN_PORT < 65536);

        if (!node["PASV"].IsDefined()) {
            std::cerr << "Server PASV config information isn't configured correctly" << std::endl;
            exit(EXIT_FAILURE);
        }

        auto pasv_config = node["PASV"];
        PASV_PORT_LOW = pasv_config["port_low"].as<int>();
        PASV_PORT_HIGH = pasv_config["port_high"].as<int>();

        if (PASV_PORT_LOW > PASV_PORT_HIGH) {
            std::cerr << "PASV port high must higher than port low" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (pasv_config["force_passive_ip"].IsDefined()) {
            FORCE_PASSIVE_SERVER_IP = pasv_config["force_passive_ip"].as<std::string>();
            utility::debug_info(std::string("Force Server IP Address: ") + FORCE_PASSIVE_SERVER_IP);
        } else {
            FORCE_PASSIVE_SERVER_IP = "";
        }

        utility::debug_info(std::string("PASV port low: ") + std::to_string(PASV_PORT_LOW));
        utility::debug_info(std::string("PASV port high: ") + std::to_string(PASV_PORT_HIGH));
        assert(PASV_PORT_LOW > 0 && PASV_PORT_LOW < 65536);
        assert(PASV_PORT_HIGH > 0 && PASV_PORT_HIGH < 65536);
        assert(PASV_PORT_HIGH >= PASV_PORT_LOW);
    }

}