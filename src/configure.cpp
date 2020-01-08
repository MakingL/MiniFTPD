//
// Created by MLee on 2019/12/31.
//

#include <cassert>
#include "configure.h"
#include "common.h"
#include "utility.h"
/* Cmake 生成 */
#include "config.h"

namespace configure {
    const char *config_file = CONFIG_FILE_PATH;    /* 配置文件 */

    std::string SERVER_LISTEN_HOST;
    std::string FORCE_PASSIVE_SERVER_IP;
    int SERVER_LISTEN_PORT;

    int PORT_CONN_PORT;

    int PASV_PORT_LOW;
    int PASV_PORT_HIGH;

    int MAX_UPLOAD_SPEED; /* 最大上传速度 Bytes/s */
    int MAX_DOWNLOAD_SPEED; /* 最大下载速度 Bytes/s */

    YAML::Node get_node(const YAML::Node &node, const std::string &node_name) { /* 获取节点 */
        if (!node[node_name.c_str()].IsDefined()) {
            std::cerr << "Configure information:" << node_name << " isn't configured correctly" << std::endl;
            std::cerr << "Please configure it in file: " << configure::config_file << std::endl;
            exit(EXIT_FAILURE);
        }
        return node[node_name.c_str()];;
    }

    template<typename T>
    void get_val(const YAML::Node &node, const std::string &property, T &val) { /* 获得配置信息 */
        if (!node[property.c_str()].IsDefined()) {
            std::cerr << "Configure information:" << property << " isn't configured correctly" << std::endl;
            std::cerr << "Please configure it in file: " << configure::config_file << std::endl;
            exit(EXIT_FAILURE);
        }
        val = node[property.c_str()].as<T>();;
    }

    void parse_config_file() {
        YAML::Node node;
        try {
            node = YAML::LoadFile(configure::config_file);
        } catch (const std::exception &e) {
            std::cerr << "Cannot parse configure file: " << configure::config_file << ". Exception: " << e.what()
                      << std::endl;
            std::cerr << "Please check it for existence." << std::endl;
            exit(EXIT_FAILURE);
        }

        auto server_config = get_node(node, "server");
        get_val(server_config, "listen_host", SERVER_LISTEN_HOST);
        get_val(server_config, "listen_port", SERVER_LISTEN_PORT);

        utility::debug_info(std::string("Server listen host: ") + SERVER_LISTEN_HOST);
        utility::debug_info(std::string("Server listen port: ") + std::to_string(SERVER_LISTEN_PORT));
        assert(!SERVER_LISTEN_HOST.empty());
        assert(SERVER_LISTEN_PORT > 0 && SERVER_LISTEN_PORT < 65536);

        auto port_config = get_node(node, "PORT");
        get_val(port_config, "data_connection_port", PORT_CONN_PORT);

        utility::debug_info(std::string("PORT data port: ") + std::to_string(PORT_CONN_PORT));
        assert(PORT_CONN_PORT > 0 && PORT_CONN_PORT < 65536);

        auto pasv_config = get_node(node, "PASV");
        get_val(pasv_config, "port_low", PASV_PORT_LOW);
        get_val(pasv_config, "port_high", PASV_PORT_HIGH);

        if (PASV_PORT_LOW > PASV_PORT_HIGH) {
            std::cerr << "PASV port high must higher than port low" << std::endl;
            exit(EXIT_FAILURE);
        }

        utility::debug_info(std::string("PASV port low: ") + std::to_string(PASV_PORT_LOW));
        utility::debug_info(std::string("PASV port high: ") + std::to_string(PASV_PORT_HIGH));
        assert(PASV_PORT_LOW > 0 && PASV_PORT_LOW < 65536);
        assert(PASV_PORT_HIGH > 0 && PASV_PORT_HIGH < 65536);
        assert(PASV_PORT_HIGH >= PASV_PORT_LOW);

        if (pasv_config["force_passive_ip"].IsNull()) {
            FORCE_PASSIVE_SERVER_IP = "";
        } else {
            get_val(pasv_config, "force_passive_ip", FORCE_PASSIVE_SERVER_IP);
        }
        utility::debug_info(std::string("Force Server IP Address: ") + FORCE_PASSIVE_SERVER_IP);

        auto service_config = get_node(node, "SERVICE");
        get_val(service_config, "max_upload_speed", MAX_UPLOAD_SPEED);
        get_val(service_config, "max_download_speed", MAX_DOWNLOAD_SPEED);

        utility::debug_info(std::string("max upload speed: ") + std::to_string(MAX_UPLOAD_SPEED));
        utility::debug_info(std::string("max download speed: ") + std::to_string(MAX_DOWNLOAD_SPEED));
        assert(MAX_UPLOAD_SPEED > 0);
        assert(MAX_DOWNLOAD_SPEED > 0);
    }

}