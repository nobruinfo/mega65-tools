#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include "logging.h"

static FILE *log_file = NULL;
static unsigned char log_level = LOG_NOTE, log_fallback = 1;
#define LOG_TEMP_MESSAGE_MAX 1023
static char log_temp_message[LOG_TEMP_MESSAGE_MAX + 1] = "";

static char *log_level_name[] = { "CRIT", "ERRO", "WARN", "NOTE", "INFO", "DEBG" };
static char *log_level_name_low[] = { "crit", "erro", "warn", "note", "info", "debu" };

int log_parse_level(char *levelarg)
{
  int loglevel = -1, i;
  char *endp;

  loglevel = strtol(levelarg, &endp, 10);
  if (*endp != 0) {
    loglevel = -1;
    for (i = 0; i < LOG_DEBUG; i++) {
      if (!strncmp(levelarg, log_level_name_low[i], 4))
        loglevel = i;
    }
  }

  return loglevel;
}

void log_setup(FILE *outfile, const int level)
{
  if (outfile != NULL)
    log_file = outfile;
  else
    log_file = stderr;
  log_level = level;
  log_fallback = 0;
}

void log_raiselevel(const int level)
{
  if (log_level < level)
    log_level = level;
}

void log_format(const int level, const char *message, va_list args)
{
  if (message == NULL)
    message = log_temp_message;

  if (log_fallback) {
    vfprintf(stderr, message, args);
    return;
  }

  if (level > log_level)
    return;

  char date[32];
  char outstring[1024];
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  strftime(date, 31, "%Y-%m-%dT%H:%M:%S", gmtime(&(currentTime.tv_sec)));

#ifndef __linux__
  // hack: apple and windows sometimes make %03d really long...
  char milli[128];
  snprintf(milli, 127, "%03ld", currentTime.tv_usec / 1000);
  milli[3] = 0;
  int pos = snprintf(outstring, 1023, "%s.%sZ %s ", date, milli, log_level_name[level]);
#else
  int pos = snprintf(outstring, 1023, "%s.%03ldZ %s ", date, currentTime.tv_usec / 1000, log_level_name[level]);
#endif
  int pos2 = vsnprintf(outstring + pos, 1023 - pos, message, args);
  if (outstring[strlen(outstring) - 1] != '\n') {
    outstring[pos + pos2] = '\n';
    outstring[pos + pos2 + 1] = 0;
  }

  fprintf(log_file, outstring);
}

void log_concat(char *message, ...)
{
  if (message == NULL) {
    log_temp_message[0] = 0;
    return;
  }

  va_list args;
  char temp[LOG_TEMP_MESSAGE_MAX];

  va_start(args, message);
  vsnprintf(temp, LOG_TEMP_MESSAGE_MAX, message, args);
  strncat(log_temp_message, temp, LOG_TEMP_MESSAGE_MAX);
  va_end(args);
}

void log_crit(const char *message, ...)
{
  va_list args;
  va_start(args, message);
  log_format(LOG_CRIT, message, args);
  va_end(args);
}

void log_error(const char *message, ...)
{
  va_list args;
  va_start(args, message);
  log_format(LOG_ERROR, message, args);
  va_end(args);
}

void log_warn(const char *message, ...)
{
  va_list args;
  va_start(args, message);
  log_format(LOG_WARN, message, args);
  va_end(args);
}

void log_note(const char *message, ...)
{
  va_list args;
  va_start(args, message);
  log_format(LOG_NOTE, message, args);
  va_end(args);
}

void log_info(const char *message, ...)
{
  va_list args;
  va_start(args, message);
  log_format(LOG_INFO, message, args);
  va_end(args);
}

void log_debug(const char *message, ...)
{
  va_list args;
  va_start(args, message);
  log_format(LOG_DEBUG, message, args);
  va_end(args);
}
