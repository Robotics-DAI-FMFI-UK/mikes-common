#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/time.h>

#include "mikes_logs.h"
#include "core/config_mikes.h"
#include "../../bites/util.h"

static char *log_filename;
static long long start_time;

void init_mikes_logs()
{
  char *filename_str = "/usr/local/logs/delivery/%ld_%s";
  char *lastlog = "/usr/local/logs/delivery/last";
  char *filename_base = "mikes.log";

  start_time = msec();

  log_filename = (char *)malloc(strlen(filename_str) + 20 + strlen(filename_base));
  if (log_filename == 0)
  {
    perror("mikes:logs malloc");
    exit(1);
  }

  time_t tm;
  time(&tm);
  sprintf(log_filename, filename_str, tm, filename_base);

  FILE *f = fopen(log_filename, "w+");
  if (f == 0)
  {
    perror("mikes:logs");
    printf("Could not open log file %s\n", log_filename);
    exit(1);
  }
  fclose(f);

  unlink(lastlog);
  symlink(log_filename, lastlog);

  if (mikes_config.print_all_logs_to_console) mikes_log(ML_INFO, "printing all logs to console");
  char ctm[40];
  sprintf(ctm, "%s", ctime(&tm));
  ctm[strlen(ctm) - 1] = 0;
  mikes_log(ML_INFO, ctm);
}

static char *log_type_str[4] = { "INFO", "WARN", " ERR", "DEBG" };

FILE *try_opening_log(unsigned int log_type)
{
  FILE *f = fopen(log_filename, "a+");
  if (!f)
      perror("mikes:logs try_opening_log");
  else
  {
      if ((log_type < 0) || (log_type > ML_MAX_TYPE))
      {
          fprintf(f, "WARN: unrecognized log type %d\n", log_type);
          log_type = ML_ERR;
      }
  }
  return f;
}

long get_run_time()
{
  return (long)(msec() - start_time);
}

void mikes_log(unsigned int log_type, char *log_msg)
{
  if ((log_type == ML_DEBUG) && !mikes_config.print_debug_logs) return;
  long run_time = get_run_time();

  FILE *f = try_opening_log(log_type);
  if (f)
  {
      fprintf(f, "%05ld.%03d %s: %s\n", run_time / 1000L, (int)(run_time % 1000L), log_type_str[log_type], log_msg);
      fclose(f);
  }

  if (mikes_config.print_all_logs_to_console)
    printf("%s: %s\n", log_type_str[log_type], log_msg);
}

void mikes_log_str(unsigned int log_type, char *log_msg, const char *log_msg2)
{
  if ((log_type == ML_DEBUG) && !mikes_config.print_debug_logs) return;
  long run_time = get_run_time();

  FILE *f = try_opening_log(log_type);
  if (f)
  {
      fprintf(f, "%05ld.%03d %s: %s%s\n", run_time / 1000L, (int)(run_time % 1000L), log_type_str[log_type], log_msg, log_msg2);
      fclose(f);
  }

  if (mikes_config.print_all_logs_to_console)
    printf("%s: %s%s\n", log_type_str[log_type], log_msg, log_msg2);
}

void mikes_log_val2(unsigned int log_type, char *log_msg, int val, int val2)
{
  if ((log_type == ML_DEBUG) && !mikes_config.print_debug_logs) return;
  long run_time = get_run_time();

  FILE *f = try_opening_log(log_type);
  if (f)
  {
      fprintf(f, "%05ld.%03d %s: %s %d %d\n", run_time / 1000L, (int)(run_time % 1000L), log_type_str[log_type], log_msg, val, val2);
      fclose(f);
  }

  if (mikes_config.print_all_logs_to_console)
    printf("%s: %s %d %d\n", log_type_str[log_type], log_msg, val, val2);
}

void mikes_log_double2(unsigned int log_type, char *log_msg, double val, double val2)
{
  if ((log_type == ML_DEBUG) && !mikes_config.print_debug_logs) return;
  long run_time = get_run_time();

  FILE *f = try_opening_log(log_type);
  if (f)
  {
      fprintf(f, "%05ld.%03d %s: %s %e %e\n", run_time / 1000L, (int)(run_time % 1000L), log_type_str[log_type], log_msg, val, val2);
      fclose(f);
  }

  if (mikes_config.print_all_logs_to_console)
    printf("%s: %s %e %e\n", log_type_str[log_type], log_msg, val, val2);
}

void mikes_log_val(unsigned int log_type, char *log_msg, int val)
{
  if ((log_type == ML_DEBUG) && !mikes_config.print_debug_logs) return;
  long run_time = get_run_time();

  FILE *f = try_opening_log(log_type);
  if (f)
  {
      fprintf(f, "%05ld.%03d %s: %s %d\n", run_time / 1000L, (int)(run_time % 1000L), log_type_str[log_type], log_msg, val);
      fclose(f);
  }

  if (mikes_config.print_all_logs_to_console)
    printf("%s: %s %d\n", log_type_str[log_type], log_msg, val);
}

void mikes_log_double(unsigned int log_type, char *log_msg, double val)
{
  if ((log_type == ML_DEBUG) && !mikes_config.print_debug_logs) return;
  long run_time = get_run_time();

  FILE *f = try_opening_log(log_type);
  if (f)
  {
      fprintf(f, "%05ld.%03d %s: %s %e\n", run_time / 1000L, (int)(run_time % 1000L), log_type_str[log_type], log_msg, val);
      fclose(f);
  }

  if (mikes_config.print_all_logs_to_console)
    printf("%s: %s %e\n", log_type_str[log_type], log_msg, val);
}
