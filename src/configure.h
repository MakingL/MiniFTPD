//
// Created by MLee on 2019/12/31.
//

#ifndef MINIFTPD_CONFIGURE_H
#define MINIFTPD_CONFIGURE_H

#include <string>
#include "yaml-cpp/yaml.h"

namespace configure {
    /* 配置信息 */
    extern std::string SERVER_LISTEN_HOST;
    extern std::string FORCE_PASSIVE_SERVER_IP;
    extern  int SERVER_LISTEN_PORT;

    extern int PORT_CONN_PORT;

    extern int PASV_PORT_LOW;
    extern int PASV_PORT_HIGH;

    void parse_config_file();   /* 从配置文件中解析配置信息 */

    YAML::Node get_node(const YAML::Node &node, const std::string &node_name);

    template<typename T>
    void get_val(const YAML::Node &node, const std::string &property, T &val);
}

#endif //MINIFTPD_CONFIGURE_H
