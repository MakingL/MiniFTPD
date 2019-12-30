#ifndef _COMMON_H_
#define _COMMON_H_

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>
#include <crypt.h>
#include <fcntl.h>
#include <cerrno>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/capability.h>
#include <sys/syscall.h>
#include <sys/sendfile.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <ctime>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/prctl.h>


#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <iostream>

/*注意\后面不要有空格-while后面也不需要分号*/
/*
#define ERR_EXIT(m) \
  do \
  { \
    perror(m); \
	exit(EXIT_FAILURE); \
  } \
  while (0)

*/

#define MAX_COMMAND_LINE 1024
#define MAX_COMMAND 32
#define MAX_ARG 1024
#define MAX_PATH_LEN 1024
#define MINIFTP_CONF "miniftpd.conf"

#define MAX_EVENT 30

#endif /* _COMMON_H_ */
