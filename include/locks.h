#ifndef LOCKS_H
#define LOCKS_H

void init_lock_manager();

void acquire_read_locks(int flight_id);

void acquire_write_lock(int flight_id);

void release_lock(int flight id);

#endif