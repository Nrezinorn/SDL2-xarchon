/*--------------------------------------------------------------------------*/
/* network interface support                                                */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <stdlib.h>
#include <stdarg.h>

#ifdef NETWORKING
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "main.h"
#include "network.h"
#include "actors.h"
#include "canvas.h"
#include "board.h"

/*--------------------------------------------------------------------------*/
/* define                                                                   */
/*--------------------------------------------------------------------------*/

#define FONT_NAME      "-misc-fixed-medium-*-normal-*-20-0-*-*-*-*-iso8859-1"

#define TIMEOUT        3
#define RETRIES        4

#define ACK            (htonl(12345678))

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void network_print(char *format, ...);

#ifdef NETWORKING
static int network_create_socket(int port);
static void *network_thread_server(NETWORK_CONFIG *config);
static void *network_thread_client(NETWORK_CONFIG *config);
static void network_end_game(void);
static int network_write(long info);
static long network_read(void);
#endif

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

#ifdef NETWORKING
static NETWORK_CONFIG *configs[2];
static int network_side;
static int network_socket = 0;
static struct sockaddr_in network_remote_name;
static int network_ready;
static int network_count;

static pthread_t network_thread;
static pthread_mutex_t network_mutex;
#endif

static void *font = NULL;
static int font_color;
static int font_y;

/*--------------------------------------------------------------------------*/
/* network_print                                                            */
/*--------------------------------------------------------------------------*/

void network_print(char *format, ...)
{
   char str[128];
   va_list argptr;
   int w, h;
#ifdef NETWORKING
   typedef void (*r_t)(void *);

   pthread_cleanup_push((r_t)pthread_mutex_unlock, (void *)&network_mutex);
   pthread_mutex_lock(&network_mutex);
#endif

   if (font == NULL) {
      font = canvas_font_load(FONT_NAME);
      font_color = canvas_alloc_color(255, 0, 0);
   }

   va_start(argptr, format);
   vsprintf(str, format, argptr);
   va_end(argptr);

   canvas_font_size(str, font, &w, &h);
   canvas_font_print(str, (CANVAS_WIDTH - w) / 2, font_y, font, font_color);
   font_y += h * 2;
   canvas_refresh();
#ifdef DEBUG
   printf("%s\n", str);
#endif

#ifdef NETWORKING
   pthread_mutex_unlock(&network_mutex);
   pthread_cleanup_pop(1);
#endif
}

/*--------------------------------------------------------------------------*/
/* network_start                                                            */
/*--------------------------------------------------------------------------*/

int network_start(int side, NETWORK_CONFIG *config)
{
   int e;
#ifdef NETWORKING
   void *func;
   struct timespec tv;

   configs[side] = config;
   configs[!side] = NULL;
   network_side = side;
   if (network_socket != 0) {
      close(network_socket);
      network_socket = 0;
   }
   network_ready = 0;
#endif
   font_y = CANVAS_HEIGHT / 8;          /* somewhere at the top */
   canvas_clear();
   canvas_install_colormap(1);

#ifdef NETWORKING
   pthread_mutex_init(&network_mutex, NULL);
   func = network_thread_server;        /* default to server-mode */
   if (side == 0)                       /* but if light side is network */
      func = network_thread_client;     /*    then we're in client-mode */
   e = pthread_create(&network_thread, NULL, func, configs[side]);
   if (e != 0)
      network_print("Network:  pthread_create() failed:  %s\n", strerror(e));
   else {
      e = pthread_detach(network_thread);
      if (e != 0)
         network_print("Network:  pthread_detach() failed:  %s\n", strerror(e));
   }

   while (!network_ready) {
      pthread_mutex_lock(&network_mutex);
      e = !main_handle_events(0) || canvas_key_down(0xFF1B);
      pthread_mutex_unlock(&network_mutex);
      if (e) {
         pthread_cancel(network_thread);
         if (network_socket != 0) {
            close(network_socket);
            network_socket = 0;
         }
         canvas_key_event(NULL);        /* reset Escape (and all other) */
         canvas_install_colormap(0);
         return 0;
      }
      tv.tv_sec = 0;
      tv.tv_nsec = 200000000;
      nanosleep(&tv, NULL);
   }
   pthread_cancel(network_thread);
   fcntl(network_socket, F_SETFL, O_NONBLOCK);
   network_count = 0;

   return 1;

#else /* ! NETWORKING */
   network_print("Networking support has not been compiled");
   while (1) {
      e = !main_handle_events(0) || canvas_key_down(0xFF1B);
      if (e) {
         canvas_key_event(NULL);        /* reset Escape (and all other) */
         canvas_install_colormap(0);
         return 0;
      }
   }
#endif

}

#ifdef NETWORKING

/*--------------------------------------------------------------------------*/
/* network_create_socket                                                    */
/*--------------------------------------------------------------------------*/

int network_create_socket(int port)
{
   int sock;
   int yesno;
   struct linger linger;
   int err = 0;
   struct sockaddr_in name;

   sock = socket(PF_INET, SOCK_DGRAM, 0);
   if (sock < 0) {
      network_print("Network:  socket() failed:  %s", strerror(errno));
      pthread_exit(NULL);
   }

   if (err == 0) {
      yesno = 1;
      err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yesno, sizeof(yesno));
   }
   if (err == 0) {
      yesno = 0;
      err = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yesno, sizeof(yesno));
   }
   if (err == 0) {
      linger.l_onoff = 0;
      err = setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
   }
   if (err < 0) {
      network_print("Network:  setsockopt() failed:  %s", strerror(errno));
      pthread_exit(NULL);
   }

   name.sin_family = AF_INET;
   name.sin_addr.s_addr = htonl(INADDR_ANY);
   name.sin_port = htons(port);
   if (bind(sock, (struct sockaddr *)&name,
            sizeof(struct sockaddr_in)) < 0) {
      network_print("Network:  bind() failed:  %s", strerror(errno));
      pthread_exit(NULL);
   }

   return sock;
}

/*--------------------------------------------------------------------------*/
/* network_thread_server                                                    */
/*--------------------------------------------------------------------------*/

void *network_thread_server(NETWORK_CONFIG *config)
{
   struct sockaddr_in name;
   int name_len;
   char dummy[8];
   int n;

   network_socket = network_create_socket(config->port);
   network_print("Network:  waiting for dark side to connect on port %d",
                 config->port);

   while (1) {
      name_len = sizeof(struct sockaddr_in);
      n = recvfrom(network_socket, dummy, 1, 0, (struct sockaddr *)&name, &name_len);
      if (n < 0) {
         network_print("Network:  recvfrom() failed:  %s", strerror(errno));
         pthread_exit(NULL);
      }
      if (n == 1 && dummy[0] == '?')
         break;
   }

   network_print("Network:  connection from %s, port %d",
                 inet_ntoa(name.sin_addr), ntohs(name.sin_port));
   dummy[0] = '!';
   *((long *)&dummy + 1) = htonl(random_index);
   if (sendto(network_socket, dummy, 8, 0, (struct sockaddr *)&name, sizeof(struct sockaddr_in)) < 0) {
      network_print("Network:  sendto() failed:  %s", strerror(errno));
      pthread_exit(NULL);
   }
   sleep(1);

   memcpy(&network_remote_name, &name, sizeof(struct sockaddr_in));
   network_ready = 1;
   return NULL;
}

/*--------------------------------------------------------------------------*/
/* network_thread_client                                                    */
/*--------------------------------------------------------------------------*/

void *network_thread_client(NETWORK_CONFIG *config)
{
   struct sockaddr_in name;
   int name_len;
   struct hostent *host;
   char dummy[8] = "?";
   int n;

   network_socket = network_create_socket(0);

   network_print("Network:  resolving %s", config->address);
   host = gethostbyname(config->address);
   if (host == NULL) {
      network_print("Network:  gethostbyname() failed:  error %d", h_errno);
      pthread_exit(NULL);
   }
   name.sin_family = AF_INET;
   name.sin_addr = *(struct in_addr *)host->h_addr;
   name.sin_port = htons(config->port);

   network_print("Network:  connecting to %s on port %d",
                 inet_ntoa(name.sin_addr), config->port);
   if (sendto(network_socket, dummy, 1, 0, (struct sockaddr *)&name, sizeof(struct sockaddr_in)) < 0) {
      network_print("Network:  sendto() failed:  %s", strerror(errno));
      pthread_exit(NULL);
   }
   memcpy(&network_remote_name, &name, sizeof(struct sockaddr_in));

   network_print("Network:  waiting for light side to acknowledge");
   while (1) {
      name_len = sizeof(struct sockaddr_in);
      n = recvfrom(network_socket, dummy, 8, 0, (struct sockaddr *)&name, &name_len);
      if (n < 0) {
         network_print("Network:  recvfrom() failed:  %s", strerror(errno));
         pthread_exit(NULL);
      }
      if (n == 8 && dummy[0] == '!') {
         random_index = ntohl(*((long *)&dummy + 1));
         break;
      }
   }

   network_ready = 1;
   return NULL;
}

#endif /* NETWORKING */

/*--------------------------------------------------------------------------*/
/* network_turn                                                             */
/*--------------------------------------------------------------------------*/

void network_turn(int side, int mode, COMMAND *cmd)
{
#ifdef NETWORKING
   network_side = side;
#endif
}

/*--------------------------------------------------------------------------*/
/* network_end_game                                                         */
/*--------------------------------------------------------------------------*/

void network_end_game(void)
{
   font_y = CANVAS_HEIGHT / 8;          /* somewhere at the top */
   network_print("Network:  game ended due to broken connection");
   canvas_refresh();
   board_end_game();
}

#ifdef NETWORKING

/*--------------------------------------------------------------------------*/
/* network_write                                                            */
/*--------------------------------------------------------------------------*/

int network_write(long info)
{
   int retry;
   fd_set fds;
   struct timeval tv1, tv2;
   long data[2];
   struct sockaddr_in name;
   int name_len;

   /* send a packet to the other side, and don't continue until */
   /* we get an acknowledgement for this packet.                */

   for (retry = 1; retry <= RETRIES; retry++) {
      data[0] = htonl(network_count);
      data[1] = htonl(info);
      sendto(network_socket, data, sizeof(data), 0,
             (struct sockaddr *)&network_remote_name, sizeof(struct sockaddr_in));
#ifdef DEBUG
      printf("WRITE:  Sent info packet with count=%d, info=%ld\n", network_count, info);
#endif

      gettimeofday(&tv2, NULL);
      tv2.tv_sec += TIMEOUT;
      while (1) {
#ifndef DEBUG
         gettimeofday(&tv1, NULL);
         if (tv1.tv_sec >= tv2.tv_sec)
            break;
#endif
         tv1.tv_sec = 1;
         tv1.tv_usec = 0;
         FD_ZERO(&fds);
         FD_SET(network_socket, &fds);
         if (select(network_socket + 1, &fds, NULL, NULL, &tv1) <= 0)
            continue;
         name_len = sizeof(struct sockaddr_in);
         if (recvfrom(network_socket, data, sizeof(data), 0,
                      (struct sockaddr *)&name, &name_len) != sizeof(data))
            continue;
         if (name.sin_addr.s_addr != network_remote_name.sin_addr.s_addr ||
             name.sin_port != network_remote_name.sin_port)
            continue;
#ifdef DEBUG
         printf("WRITE:  Received packet, count=%ld, info=%ld\n", ntohl(data[0]), ntohl(data[1]));
#endif
         if (ntohl(data[0]) == network_count && data[1] == ACK)
            return 0;
      }
#ifdef DEBUG
      printf("Try %d for count %d FAILED.  Retrying\n", retry, network_count);
#endif
   }
   printf("Network has died.\n");
   network_end_game();
   return -1;
}

/*--------------------------------------------------------------------------*/
/* network_read                                                             */
/*--------------------------------------------------------------------------*/

long network_read(void)
{
   fd_set fds;
   struct timeval tv1, tv2;
   long data[2];
   struct sockaddr_in name;
   int name_len;
   long info;

   /* receive a packet from the other side and acknowledge it.  */
   /* old packets are discarded.                                */

   gettimeofday(&tv2, NULL);
   tv2.tv_sec += TIMEOUT * RETRIES;
   while (1) {
#ifndef DEBUG
      gettimeofday(&tv1, NULL);
      if (tv1.tv_sec >= tv2.tv_sec) {
         network_end_game();
         return -1;
      }
#endif
      tv1.tv_sec = 1;
      tv1.tv_usec = 0;
      FD_ZERO(&fds);
      FD_SET(network_socket, &fds);
      if (select(network_socket + 1, &fds, NULL, NULL, &tv1) <= 0)
         continue;
      name_len = sizeof(struct sockaddr_in);
      if (recvfrom(network_socket, data, sizeof(data), 0, (struct sockaddr *)&name, &name_len) != sizeof(data))
         continue;
      if (name.sin_addr.s_addr != network_remote_name.sin_addr.s_addr ||
          name.sin_port != network_remote_name.sin_port)
         continue;
#ifdef DEBUG
      printf("READ :  Received packet, count=%d, info=%d\n", ntohl(data[0]), ntohl(data[1]));
#endif

      if (data[1] == ACK)
         continue;
      info = ntohl(data[1]);
      data[1] = ACK;
      sendto(network_socket, data, sizeof(data), 0,
             (struct sockaddr *)&network_remote_name, sizeof(struct sockaddr_in));
#ifdef DEBUG
      printf("READ :  Sent ack  packet with same count\n");
#endif
      if (ntohl(data[0]) == network_count) {
#ifdef DEBUG
         printf("READ : Returning with info=%d\n", info);
#endif
         return info;
      }
   }
}

#endif /* NETWORKING */

/*--------------------------------------------------------------------------*/
/* network_frame                                                            */
/*--------------------------------------------------------------------------*/

void network_frame(int *keys_down)
{
#ifdef NETWORKING
   long info;
   int i;

   network_count++;
#ifdef DEBUG
   printf("*FRM*: Network Count = %d  for  Side = %d\n", network_count, network_side);
#endif

   if (configs[network_side] != NULL) { /* network side */
      memset(keys_down, 0, STATE_MOVE_COUNT * sizeof(int));
      info = network_read();
      if (info != -1)
         for (i = 0; i < STATE_MOVE_COUNT; i++)
            keys_down[i] = (info & (1 << i)) == (1 << i);

   } else {                             /* real player side */
      info = 0;
      for (i = 0; i < STATE_MOVE_COUNT; i++)
         info |= (keys_down[i] != 0) << i;
      network_write(info);
   }
#endif /* NETWORKING */
}

/*--------------------------------------------------------------------------*/
/* network_config_read                                                      */
/*--------------------------------------------------------------------------*/

void network_config_read(FILE *fp, NETWORK_CONFIG *config)
{
   fscanf(fp, "%64s %d", config->address, &config->port);
}

/*--------------------------------------------------------------------------*/
/* network_config_write                                                     */
/*--------------------------------------------------------------------------*/

void network_config_write(FILE *fp, NETWORK_CONFIG *config)
{
   fprintf(fp, "%-64s %d\n", config->address, config->port);
}
