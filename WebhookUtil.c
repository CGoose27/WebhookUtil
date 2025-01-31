#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_help(char *prog_name) {
  printf("Usage: %s <URL> [-l <latency>] [-t <times>] [-q] [-h] [-T] [-P] [-N] -[M] \n", prog_name);
  printf("Options:\n");
  printf(
      "  -l <latency>    Set the latency between pings (default: 1 second)\n");
  printf(
      "  -t <times>      Set the number of times to ping (default: 1 time)\n");
  printf("  -T <threads>    Set the number of threads to create (default: 1 "
         "thread)\n");
  printf("  -q              Quiet mode (suppress success message)\n");
  printf("  -h              Display this help message\n");
  printf("  -P <url>        Set the webhook's profile url (default: \"https://example.com/avatar.png\")\n");
  printf("  -N <string>     Set the webhook's user name (default: \"Webhook Spammer\")\n");
  printf(
      "  -M <string>     Set the webhook's message content (default: \"Webhook Spammer\")\n");
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t totalSize = size * nmemb;
  char *data = (char *)contents;
  char **response_ptr = (char **)userp;

  *response_ptr = realloc(*response_ptr, strlen(*response_ptr) + totalSize + 1);
  if (*response_ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    return 0;
  }

  strncat(*response_ptr, data, totalSize);
  return totalSize;
}

struct pingArgs {
  char *url;
  char *payload;
  int times;
  int latency;
  char quiet_mode;
};

void *pingThread(void *arg) {
  struct pingArgs args = *(struct pingArgs *)arg;

  CURL *curl;
  CURLcode res;
  curl_global_init(CURL_GLOBAL_DEFAULT);

  for (int i = 0; i < args.times; i++) {
    curl = curl_easy_init();
    if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, args.url);
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      curl_easy_setopt(
          curl, CURLOPT_HTTPHEADER,
          curl_slist_append(NULL, "Content-Type: application/json"));

      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, args.payload);

      res = curl_easy_perform(curl);

      if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
      } else {
        if (!args.quiet_mode) {
          printf("Ping %d: JSON sent successfully\n", i + 1);
        }
      }

      curl_easy_cleanup(curl);
    }
    if (i < args.times - 1) {
      sleep(args.latency);
    }
  }

  curl_global_cleanup();
  return NULL;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    print_help(argv[0]);
    return 1;
  }

  char *url = NULL;

  char *pUrl = "https://example.com/avatar.png";
  char *pName = "Webhook Spammer";
  char *pContent = "Hello, world!";

  int latency = 1;
  int times = 1;
  int quiet_mode = 0;
  int threads = 1;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) { // Help
      print_help(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) { // Latency
      latency = atoi(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) { // Times
      times = atoi(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "-q") == 0) { // quiet mode
      quiet_mode = 1;
    } else if (strcmp(argv[i], "-T") == 0 && i + 1 < argc) { // thread amount
      threads = atoi(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "-P") == 0 && i + 1 < argc) { // Profile URL
      pUrl = argv[i + 1];
      i++;
    } else if (strcmp(argv[i], "-N") == 0 && i + 1 < argc) { // Profile NAME
      pName = argv[i + 1];
      i++;
    } else if (strcmp(argv[i], "-M") == 0 && i + 1 < argc) { // Message CONTENT
      pContent = argv[i + 1];
      i++;
    } else {
      url = argv[i];
    }
  }

  if (url == NULL) {
    fprintf(stderr, "URL is required.\n");
    print_help(argv[0]);
    return 1;
  }

  char payload[256];
  snprintf(
      payload, sizeof(payload),
      "{\"content\": \"%s\", \"username\": \"%s\", \"avatar_url\": \"%s\"}",
      pContent, pName, pUrl);

  struct pingArgs args = {url, payload, times, latency, quiet_mode};

  if (threads == 1) {
    pingThread((void *)&args);
  } else {
    pthread_t *thls = malloc(sizeof(pthread_t) * threads);

    for (int a = 0; a < threads; a++) {
      pthread_t threadID;
      pthread_create(&threadID, NULL, &pingThread, (void *)&args);
      thls[a] = threadID;
    }

    for (int a = 0; a < threads; a++) {
      pthread_join(thls[a], NULL);
    }

    free(thls);
  }

  return 0;
}