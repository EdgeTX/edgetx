/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "task.h"
#include "debug.h"
#include "timer_native_impl.h"

#include <chrono>
#include <memory>
#include <vector>
#include <mutex>
#include <string>

static std::condition_variable _stop_cv;
static std::mutex _stop_m;

static std::mutex _tasks_m;
static std::vector<task_handle_t*> _tasks;
static bool _stop_tasks = false;

void task_sleep_ms(uint32_t ms)
{
  std::unique_lock<std::mutex> lk(_stop_m);
  _stop_cv.wait_for(lk, std::chrono::milliseconds(ms));
}

void task_sleep_until(time_point_t* tp, uint32_t inc)
{
  *tp += std::chrono::duration<uint32_t, std::milli>{inc};

  std::unique_lock<std::mutex> lk(_stop_m);
  _stop_cv.wait_until(lk, *tp);
}

static void stop_tasks()
{
  std::lock_guard lock(_tasks_m);
  _stop_tasks = true;
}

static bool next_task_to_stop(task_handle_t*& task)
{
  std::lock_guard lock(_tasks_m);

  // no more tasks
  if (_tasks.empty()) return false;

  // grab the next one
  task = _tasks.back();
  _tasks.pop_back();

  return true;
}

void task_shutdown_all()
{
  stop_tasks();
  _stop_cv.notify_all();

  task_handle_t* task = nullptr;
  while (next_task_to_stop(task)) {
    task->_thread_handle->join();
    task->_thread_handle = {};
  }

  timer_queue::destroy();
  _stop_tasks = false;
}

struct run_context {
  task_func_t func;
  std::string name;
};

static void* _task_stub(run_context* p)
{
  std::unique_ptr<run_context> ctx{(run_context*)p};
  auto name = ctx->name.c_str();

  TRACE("<%s> started", name);
  ctx->func();
  TRACE("<%s> stopped", name);

  return nullptr;
}

void task_create(task_handle_t* h, task_func_t func, const char* name,
                 void* stack, unsigned stack_size, unsigned priority)
{
  std::lock_guard lock(_tasks_m);
  if (_stop_tasks) return;

  h->_stack_size = stack_size;
  run_context* ctx = new run_context{func, name};
  h->_thread_handle = std::make_unique<std::thread>([=]() { _task_stub(ctx); });
  if (h->_thread_handle) _tasks.emplace_back(h);
}

bool task_running()
{
  return !_stop_tasks;
}

unsigned task_get_stack_usage(task_handle_t* h)
{
  // fake...
  return 0;
}

unsigned task_get_stack_size(task_handle_t* h)
{
  return h->_stack_size * 4;
}

void mutex_create(mutex_handle_t* h)
{
  (void)h;
}

bool mutex_lock(mutex_handle_t* h)
{
  if (!h) return false;
  h->lock();
  return true;
}

void mutex_unlock(mutex_handle_t* h)
{
  if (!h) return;
  h->unlock();
}

bool mutex_trylock(mutex_handle_t* h)
{
  if (!h) return false;
  return h->try_lock();
}


