#include "job.h"
#include <stdio.h>
#include <stdlib.h>
// Initialize the jobs table
Jobs_table *init_jobs(void) {
  Jobs_table *jobs = (Jobs_table *)malloc(sizeof(Jobs_table));
  if (jobs == NULL) {
    perror("Failed to allocate memory for jobs table");
    exit(EXIT_FAILURE);
  }
  jobs->list = (Job_t *)malloc(INITIAL_CAPACITY * sizeof(Job_t));
  if (jobs->list == NULL) {
    perror("Failed to allocate memory for jobs list");
    free(jobs);
    exit(EXIT_FAILURE);
  }
  jobs->counter = 0;
  jobs->capacity = INITIAL_CAPACITY;
  return jobs;
}

void add_job(Jobs_table *jobs, Job_t job) {
  if (jobs->counter == jobs->capacity) {
    jobs->capacity *= 2;
    jobs->list = (Job_t *)realloc(jobs->list, jobs->capacity * sizeof(Job_t));
    if (jobs->list == NULL) {
      perror("Failed to allocate memory for expanding jobs list");
      exit(EXIT_FAILURE);
    }
  }
  jobs->list[jobs->counter] = job;
  jobs->counter++;
}

void remove_job(Jobs_table *jobs, Job_t job) {
  size_t i;
  for (i = 0; i < jobs->counter; ++i) {
    if (jobs->list[i].pid == job.pid) {
      free(jobs->list[i].command); // Free the command string
      // Shift remaining jobs to fill the gap
      for (size_t j = i; j < jobs->counter - 1; ++j) {
        jobs->list[j] = jobs->list[j + 1];
      }
      jobs->counter--;
      return;
    }
  }
}

void free_jobs(Jobs_table *jobs) {
  for (size_t i = 0; i < jobs->counter; ++i) {
    free(jobs->list[i].command);
  }
  free(jobs->list);
  free(jobs);
}

void print_jobs(Jobs_table *jobs) {
  Job_t *list = jobs->list;
  for (size_t i = 0; i < jobs->counter; i++) {
    printf("[%d] %d\t%s\n", list[i].job_id, list[i].pid, list[i].command);
  }
}
