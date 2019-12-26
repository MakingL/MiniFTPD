//
// Created by MLee on 2019/12/25.
//

#ifndef MINIFTPD_FTP_CODES_H
//#define MINIFTPD_FTP_CODES_H

namespace ftp_response_code {
    /* FTP 响应状态码 */
    /* 1yz: 预备状态 */
    const int kFTP_DATACONN = 150;
    /* 2yz: 完成状态 */
    const int kFTP_NOOPOK = 200;
    const int kFTP_TYPEOK = 200;
    const int kFTP_PORTOK = 200;
    const int kFTP_EPRTOK = 200;
    const int kFTP_UMASKOK = 200;
    const int kFTP_CHMODOK = 200;
    const int kFTP_EPSVALLOK = 200;
    const int kFTP_STRUOK = 200;
    const int kFTP_MODEOK = 200;
    const int kFTP_PBSZOK = 200;
    const int kFTP_PROTOK = 200;
    const int kFTP_OPTSOK = 200;
    const int kFTP_ALLOOK = 202;
    const int kFTP_FEAT = 211;
    const int kFTP_STATOK = 211;
    const int kFTP_SIZEOK = 213;
    const int kFTP_MDTMOK = 213;
    const int kFTP_STATFILE_OK = 213;
    const int kFTP_SITEHELP = 214;
    const int kFTP_HELP = 214;
    const int kFTP_SYSTOK = 215;
    const int kFTP_GREET = 220;
    const int kFTP_GOODBYE = 221;
    const int kFTP_ABOR_NOCONN = 225;
    const int kFTP_TRANSFEROK = 226;
    const int kFTP_ABOROK = 226;
    const int kFTP_PASVOK = 227;
    const int kFTP_EPSVOK = 229;
    const int kFTP_LOGINOK = 230;
    const int kFTP_AUTHOK = 234;
    const int kFTP_CWDOK = 250;
    const int kFTP_RMDIROK = 250;
    const int kFTP_DELEOK = 250;
    const int kFTP_RENAMEOK = 250;
    const int kFTP_PWDOK = 257;
    const int kFTP_MKDIROK = 257;
    /* 3yz: 中间状态 */
    const int kFTP_GIVEPWORD = 331;
    const int kFTP_RESTOK = 350;
    const int kFTP_RNFROK = 350;
    /* 4yz: 暂时拒绝状态 */
    const int kFTP_IDLE_TIMEOUT = 421;
    const int kFTP_DATA_TIMEOUT = 421;
    const int kFTP_TOO_MANY_USERS = 421;
    const int kFTP_IP_LIMIT = 421;
    const int kFTP_IP_DENY = 421;
    const int kFTP_TLS_FAIL = 421;
    const int kFTP_BADSENDCONN = 425;
    const int kFTP_BADSENDNET = 426;
    const int kFTP_BADSENDFILE = 451;
    /* 5yz: 永久拒绝状态 */
    const int kFTP_BADCMD = 500;
    const int kFTP_BADOPTS = 501;
    const int kFTP_COMMANDNOTIMPL = 502;
    const int kFTP_NEEDUSER = 503;
    const int kFTP_NEEDRNFR = 503;
    const int kFTP_BADPBSZ = 503;
    const int kFTP_BADPROT = 503;
    const int kFTP_BADSTRU = 504;
    const int kFTP_BADMODE = 504;
    const int kFTP_BADAUTH = 504;
    const int kFTP_NOSUCHPROT = 504;
    const int kFTP_NEEDENCRYPT = 522;
    const int kFTP_EPSVBAD = 522;
    const int kFTP_DATATLSBAD = 522;
    const int kFTP_LOGINERR = 530;
    const int kFTP_NOHANDLEPROT = 536;
    const int kFTP_FILEFAIL = 550;
    const int kFTP_NOPERM = 550;
    const int kFTP_UPLOADFAIL = 553;
}


#endif //MINIFTPD_FTP_CODES_H
