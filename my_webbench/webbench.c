/*
 * File: webbench.
 * Author: Jack Shi
 */

#include "socket.c"
#include <getopt.h>
#include <signal.h>
#include <strings.h>
#include <sys/param.h>
#include <time.h>
#include <unistd.h>

void benchcore(const char *host, const int port, const char *req);
static int bench(void);
static void build_request(const char *url);

/*
 * what is the http/0.9?
 * Initial version of HTTP - a simple client-sever request-response,
 * telenet-friendly protocol Request nature:signle-line (method + path for
 * requeted document) Methods supported: GET only Response type: hypertext only
 * Connection nature: terminated immediately after the Response
 * No HTTP headers (cannot transfer other content type files), No status/error
 * codes.No URLS, No versioning.
 * */

/*
 GET: check and find out something from server
 POST: send changed features to the server
 PUT : send new features to the server
 DELETE: remove features from sever.
 CONNECT: allow server to log into another page
 options: allow client check servers performance
 trace:  echo back the response sent by server. mainly using in test.
 */

typedef enum { HTTP09 = 0, HTTP10, HTTP11 } http_t;

#define METHOD_GET 0
#define METHOD_HEAD 1
#define METHOD_OPTIONS 2
#define METHOD_TRACE 3
#define PROGARM_VERSION "1.0"
#define MAX_REQUEST_SIZE 2048

typedef struct {
  int speed;
  int failed;
  unsigned int bytes;
  volatile unsigned int timerexpired;
  int method;
  int clients;
  int force;
  int force_reload;
  int proxyport;
  char *proxyhost;
  int benchtime;
  char host[MAXHOSTNAMELEN]; // MAXHOSTNAMELEN 64 in linux kernel
  char request[MAX_REQUEST_SIZE];
  http_t http_type;
  int mypipe[2];
} webbench_t;

webbench_t webbench = {

    .http_type = HTTP10,
    .clients = 1,
    .force_reload = 0,
    .force = 0,
    .benchtime = 30,
    .method = METHOD_GET,
    .proxyhost = NULL,
    .proxyport = 80 // it is the casual port for http. Note this is the client
                    // side. the connection function will send hand shake to the
                    // sever and check the connection.
};

/*
 * option structure explaination:
 * struct option {
 *  const char* name;
 *  int has_args;
 *  int *flag;
 *  int val;
 * }
 *
 * name: the name of the long option
 * has_arg: no_argument(or 0) required_argument(or 1)
 * flag specifieds how result are retuned for a logn option.If flag is NULL,
 then  getopt_long()  re‐ turns  val.  (For example, the calling program may set
 val to the equivalent short option charac‐ ter.)  Otherwise, getopt_long()
 returns 0, and flag points to a variable which is set to  val  if the option is
 found, but left unchanged if the option is not found.
 * */

static const struct option long_options[] = {
    {"force", no_argument, &webbench.force, 1},         // so force should be 1
    {"reload", no_argument, &webbench.force_reload, 1}, // so force should be 1
    {"time", required_argument, NULL, 't'},             // so force should be 1
    {"help", no_argument, NULL, '?'},
    {"http09", no_argument, NULL, '9'},
    {"http10", no_argument, NULL, '1'},
    {"http11", no_argument, NULL, '2'},
    {"get", no_argument, &webbench.method, METHOD_GET},
    {"head", no_argument, &webbench.method, METHOD_HEAD},
    {"options", no_argument, &webbench.method, METHOD_OPTIONS},
    {"trace", no_argument, &webbench.method, METHOD_TRACE},
    {"version", no_argument, NULL, 'V'},
    {"proxy", required_argument, NULL, 'p'},
    {"clients", required_argument, NULL, 'c'},
    {NULL, 0, NULL, 0}};

static void alarm_handler(int signal) { webbench.timerexpired = 1; }

static void usage(void) {
  fprintf(
      stderr,
      "webbench [option]... URL\n"
      "  -f|--force               Don't wait for reply from server.\n"
      "  -r|--reload              Send reload request - Pragma: no-cache.\n"
      "  -t|--time <sec>          Run benchmark for <sec> seconds. Default "
      "30.\n"
      "  -p|--proxy <server:port> Use proxy server for request.\n"
      "  -c|--clients <n>         Run <n> HTTP clients at once. Default one.\n"
      "  -9|--http09              Use HTTP/0.9 style requests.\n"
      "  -1|--http10              Use HTTP/1.0 protocol.\n"
      "  -2|--http11              Use HTTP/1.1 protocol.\n"
      "  --get                    Use GET request method.\n"
      "  --head                   Use HEAD request method.\n"
      "  --options                Use OPTIONS request method.\n"
      "  --trace                  Use TRACE request method.\n"
      "  -?|-h|--help             This information.\n"
      "  -V|--version             Display program version.\n");
}

int main(int argc, char **argv) {
  int opt = 0;
  int options_index = 0;
  char *tmp = NULL;
  if (argc == 1) {
    usage();
    return 2;
  }

  while ((opt = getopt_long(argc, argv, "912Vfrt:p:c:?h", long_options,
                            &options_index)) != EOF) {
    switch (opt) {
    case 0:
      break;
    case 'f':
      webbench.force = 1;
      break;
    case 'r':
      webbench.force_reload = 1;
      break;
    case '9':
      webbench.http_type = HTTP09;
      break;
    case '1':
      webbench.http_type = HTTP10;
      break;
    case '2':
      webbench.http_type = HTTP11;
      break;
    case 'V':
      printf("This is Jack Shi's webbench\n. The Version is :");
      printf(PROGARM_VERSION "\n");
      exit(0);
    case 't':
      webbench.benchtime = atoi(optarg);
      break; // optarg is the global var provide the current ortarg
    case ':':
    case 'h':
    case '?':
      usage();
      return 2;
      break;
    case 'c':
      webbench.clients = atoi(optarg);
      break;
    /*proxy server parsing server:port format*/
    case 'p':
      tmp = strrchr(optarg, ':');
      webbench.proxyhost = optarg; /*The argument after the option is saved in
                                      the optarg variables.*/
      if (tmp == NULL)
        break;
      if (tmp == optarg) {
        fprintf(stderr, "Error in option --proxy %s: Missing hostname.\n",
                optarg);
        return 2;
      }
      if (tmp == optarg + strlen(optarg) - 1) {
        fprintf(stderr, "Error in option --proxy %s Port number is missing!!\n",
                optarg);
        return 2;
      }
      *tmp = '\0';
      webbench.proxyport = atoi(tmp + 1); // get port number
      break;
    default:
      fprintf(stderr, "option typing error...!\n");
      exit(0);
    }
  }
  /*The variable optind is the index of the next element to be processed in
   * argv. The system initializes this value to 1.*/
  /*In this project the web address should append at the end of the cmd. Hence,
   * optind should less then argc*/
  // printf("The optind number is %d\n", optind);
  // printf("The argc number is %d\n", argc);

  if (optind == argc) {
    fprintf(stderr, "webbench: Missing ULR!\n");
    usage();
    return 2;
  }

  if (webbench.clients == 0)
    webbench.clients = 1;
  if (webbench.benchtime == 0)
    webbench.benchtime = 60;

  fprintf(stdout, "Webbench- Simple Webbench" PROGARM_VERSION "\n"
                  "This software is write by Jack Shi\n");
  build_request(argv[optind]);

  printf("\nBenchmarking: ");

  switch (webbench.method) {
  case METHOD_GET:
  default:
    printf("GET");
    break;
  case METHOD_OPTIONS:
    printf("OPTIONS");
    break;
  case METHOD_HEAD:
    printf("HEAD");
    break;
  case METHOD_TRACE:
    printf("TRACE");
    break;
  }
  printf(" %s", argv[optind]);
  switch (webbench.http_type) {
  case HTTP09:
    printf(" (using HTTP/0.9)");
    break;
  case HTTP11:
    printf("  (using HTTP/1.1)");
    break;
  default:
    break;
  }
  printf("\n");
  if (webbench.clients == 1)
    printf("1 client");
  else
    printf("%d clients", webbench.clients);

  printf(", running %d sec", webbench.benchtime);
  if (webbench.force)
    printf(", early socket close");
  if (webbench.proxyhost != NULL)
    printf(", via proxy sever %s:%d\n", webbench.proxyhost, webbench.proxyport);
  if (webbench.force_reload)
    printf(", force_reload");
  printf(".\n");
  return bench();
}

static void build_request(const char *url) {
  char tmp[10];
  int i;

  bzero(webbench.host, sizeof(webbench.host));
  bzero(webbench.request, sizeof(webbench.request));

  if (webbench.force_reload && webbench.proxyhost != NULL &&
      webbench.http_type < HTTP10) {
    webbench.http_type = HTTP10;
  }
  if (webbench.method == METHOD_HEAD && webbench.http_type <= HTTP10) {
    webbench.http_type = HTTP10;
  }
  if (webbench.method == METHOD_OPTIONS && webbench.http_type < HTTP11) {
    webbench.http_type = HTTP11;
  }
  if (webbench.method == METHOD_TRACE && webbench.http_type < HTTP11) {
    webbench.http_type = HTTP11;
  }

  switch (webbench.method) {
  case METHOD_GET:
    strcpy(webbench.request, "GET");
    break;
  case METHOD_HEAD:
    strcpy(webbench.request, "HEAD");
    break;
  case METHOD_OPTIONS:
    strcpy(webbench.request, "OPTIONS");
    break;
  case METHOD_TRACE:
    strcpy(webbench.request, "TRACE");
    break;
  default:
    fprintf(stderr, "Invalid method!\n");
    break;
  }

  strcat(webbench.request, " ");

  if (NULL == strstr(url, "://")) {
    fprintf(stderr, "\n%s: is not a valid URL.\n", url);
    exit(2);
  }
  if (strlen(url) > 1500) {
    fprintf(stderr, "URL is too long.\n");
    exit(2);
  }
  if (webbench.proxyhost == NULL) {
    if (0 != strncasecmp("http://", url, 7)) {
      fprintf(stderr, "\nInvalid URL syntax -hostname don't end with '/'\n");
      exit(2);
    }
  }

  i = strstr(url, "://") - url + 3;
  if (strchr(url + i, '/') == NULL) {
    fprintf(stderr, "\nInvalid URL syntax - hostname donot end by '\'\n");
    exit(2);
  }

  if (webbench.proxyhost == NULL) {
    /*To get the host name ex http://www.gogle.com:20 --> www.google.com */
    if (index(url + i, ':') != NULL &&
        index(url + i, ':') < index(url + i, '/')) {
      strncpy(webbench.host, url + i, strchr(url + i, ':') - url - i);
      bzero(tmp, 10);
      strncpy(tmp, index(url + i, ':') + 1,
              strchr(url + i, '/') - index(url + i, ':') - 1);
      webbench.proxyport = atoi(tmp);
      if (webbench.proxyport == 0)
        webbench.proxyport = 80;
    } else {
      strncpy(webbench.host, url + i, strcspn(url + i, "/"));
    }
  } else {
    strcat(webbench.request, url);
  }

  if (webbench.http_type == HTTP10)
    strcat(webbench.request, " HTTP/1.0");
  else if (webbench.http_type == HTTP11)
    strcat(webbench.request, " HTTP/1.1");
  strcat(webbench.request, "\r\n");

  if (webbench.http_type > HTTP09)
    strcat(webbench.request, "User-Agent: WebBench " PROGARM_VERSION "\r\n");

  if (webbench.proxyhost == NULL && webbench.http_type > HTTP09) {
    strcat(webbench.request, "Host: ");
    strcat(webbench.request, webbench.host);
    strcat(webbench.request, "\r\n");
  }
  if (webbench.force_reload && webbench.proxyhost != NULL) {
    strcat(webbench.request, "Pragma: no-cache\r\n");
  }

  if (webbench.http_type > HTTP10) {
    strcat(webbench.request, "Connection: close\r\n");
  }
  if (webbench.http_type > HTTP09)
    strcat(webbench.request, "\r\n");
}

static int bench(void) {
  int i, j, k;
  pid_t pid = 0;
  FILE *f;
  /*This part just to check the avaibility of target server.*/
  i = Socket(webbench.proxyhost == NULL ? webbench.host : webbench.proxyhost,
             webbench.proxyport);

  if (i < 0) {
    fprintf(stderr,
            "\nConnect to server failed. Aborting benchmark operation %s %d\n",
            __FILE__, __LINE__);
    return 1;
  }
  close(i);
  /*create pipe.*/
  if (pipe(webbench.mypipe)) {
    perror("pipe failed");
  }
  /*
   * Step 1: one process generate one parent one child.the child pid is equal to
   * 0 then go to sleep. the parent will go to the scecond around. Step 2: the
   * parent fork another child,child go to sleep and parent go back the the
   * third around. Step 3: same as above. step 4: after 1 second all the child
   * will wake up and break from the for loop.
   * */
  for (i = 0; i < webbench.clients; i++) {
    pid = vfork();
    if (pid <= (pid_t)0) {
      sleep(1);
      break;
    }
  }

  if (pid < (pid_t)0) {
    fprintf(stderr, "problems forking work no. %d", i);
    perror("fork failed");
    return 3;
  }

  if (pid == (pid_t)0) {
    /*I am child.*/
    if (webbench.proxyhost == NULL) {
      benchcore(webbench.host, webbench.proxyport, webbench.request);
    } else {
      benchcore(webbench.proxyhost, webbench.proxyport, webbench.request);
    }

    close(webbench.mypipe[0]);

    f = fdopen(webbench.mypipe[1], "w");
    if (f == NULL) {
      perror("Open pip for reading failed.\n");
      return 3;
    }

    fprintf(f, "%d %d %d\n", webbench.speed, webbench.failed, webbench.bytes);
    // fprintf(f, "%d %d %d\n", 1, 1, 1);
    fclose(f);
    _exit(0);
  } else {
    /*here is the parent process*/
    close(webbench.mypipe[1]);
    f = fdopen(webbench.mypipe[0], "r");
    if (f == NULL) {
      perror("Open pip for reading failed.\n");
      return 3;
    }

    setvbuf(f, NULL, _IONBF,
            0); // send the data directly to other side of the pipe
    webbench.speed = 0;
    webbench.failed = 0;
    webbench.bytes = 0;

    while (1) {
      int value = fscanf(f, "%d %d %d\n", &i, &j, &k); // read then out.
      if (value < 2) {
        fprintf(stderr, "Some of our childrens died.\n");
        break;
      }
      webbench.speed += i;
      webbench.failed += j;
      webbench.bytes += k;
      if (--webbench.clients == 0)
        break;
    }
    fclose(f);

    printf("\nSpeed=%d pages/min, %d bytes/sec. \nRequests: %d succeed, %d "
           "failed.\n",
           (int)((webbench.speed + webbench.failed) /
                 (webbench.benchtime / 60.0f)),
           (int)(webbench.bytes / (float)webbench.benchtime), webbench.speed,
           webbench.failed);
  }

  return i;
}

void benchcore(const char *host, const int port, const char *req) {
  int rlen;
  char buff[1500];
  int s, i;
  struct sigaction sa;

  /* setup alarm signal handler*/
  sa.sa_handler = alarm_handler;
  sa.sa_flags = 0;

  if (sigaction(SIGALRM, &sa, NULL))
    exit(3);

  alarm(webbench.benchtime);

  rlen = strlen(req);

nexttry:
  while (1) {
    if (webbench.timerexpired) {
      if (webbench.failed > 0) {
        webbench.failed--;
      }
      return;
    }

    s = Socket(host, port); // connect client with server
    if (s < 0) {
      webbench.failed++;
      continue;
    } // if cannot connect with the server then mark as failed.
    if (rlen != write(s, req, rlen)) {
      webbench.failed++;
      close(s);
      continue;
    }
    if (webbench.http_type == HTTP09)
      if (shutdown(s, SHUT_WR)) {
        webbench.failed++;
        close(s);
        continue;
      }

    if (webbench.force == 0) {
      while (1) {
        if (webbench.timerexpired)
          break;
        i = read(s, buff, sizeof(buff));
        if (i < 0) {
          webbench.failed++;
          close(s);
          goto nexttry;
        } else {
          if (i == 0) {
            break;
          } else {
            webbench.bytes += i;
          }
        }
      }
    }
    if (close(s)) {
      webbench.failed++;
      continue;
    }
    webbench.speed++;
  }
}
