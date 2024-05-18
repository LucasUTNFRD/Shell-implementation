#ifndef JOB_H
#define JOB_H

#define INITIAL_CAPACITY 10
#include <unistd.h>
typedef struct {
  pid_t pid;
  int job_id;
  char *command;
} Job_t;

typedef struct {
  Job_t *list;     // Dynamic array of jobs
  size_t counter;  // Total number of jobs ever added
  size_t capacity; // Current capacity of the job_list array
} Jobs_table;

Jobs_table *init_jobs(void);

void add_job(Jobs_table *jobs, Job_t job);
void remove_job(Jobs_table *jobs, Job_t job);
void free_jobs(Jobs_table *jobs);
void print_jobs(Jobs_table *jobs);

#endif // !JOB_H
