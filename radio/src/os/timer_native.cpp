#include "timer.h"
#include "debug.h"
#include "timer_native_impl.h"

#include <algorithm>

using namespace std::chrono_literals;

static timer_queue* _instance = nullptr;
static std::mutex _instance_mut;

bool _timer_cmp(timer_handle_t *lh, timer_handle_t *rh) {
  return lh->next_trigger < rh->next_trigger;
}

timer_queue::timer_queue() {
  start();
}

void timer_queue::update_current_time() {
  _current_time = std::chrono::steady_clock::now();
}

void timer_queue::sort_timers() {
  std::sort(_timers.begin(), _timers.end(), _timer_cmp);
}

timer_queue& timer_queue::instance()
{
  std::lock_guard<std::mutex> lock(_instance_mut);
  if (!_instance) {
    _instance = new timer_queue();
  }
  return *_instance;
}

void timer_queue::destroy()
{
  std::lock_guard<std::mutex> lock(_instance_mut);
  if (_instance) {
    _instance->stop();
    delete _instance;
    _instance = nullptr;
  }
}

void timer_queue::start()
{
  std::lock_guard<std::mutex> lock(_cmds_mutex);
  if (!_running) {
    _running = true;
    _thread = std::make_unique<std::thread>([&]() { main_loop(); });
  }
}

void timer_queue::stop() {
  std::unique_lock<std::mutex> lock(_cmds_mutex);
  if (_running) {
    _running = false;
    lock.unlock();
    _cmds_condition.notify_one();
  }
  _thread->join();
  TRACE("<timer_queue> stopped");
}

void timer_queue::create_timer(timer_handle_t *timer, timer_func_t func, const char *name,
                            unsigned period, bool repeat) {
  timer->func = func;
  timer->name = name;
  timer->period = period;
  timer->repeat = repeat;
  timer->next_trigger = time_point_t{};
}

void timer_queue::send_cmd(timer_req_t&& req)
{
  {
    std::lock_guard lock(_cmds_mutex);
    _cmds.emplace_back(req);
  }
  _cmds_condition.notify_one();
}

void timer_queue::start_timer(timer_handle_t *timer) {
  send_cmd(timer_req_t{.cmd = timer_req_t::cmd_start, .timer = timer});
}

void timer_queue::stop_timer(timer_handle_t *timer) {
  send_cmd(timer_req_t{.cmd = timer_req_t::cmd_stop, .timer = timer});
}

void timer_queue::pend_function(timer_async_func_t func, void *param1,
                                uint32_t param2) {
  send_cmd(timer_req_t{
      .cmd = timer_req_t::cmd_pend_func,
      .func_call = {.func = func, .param1 = param1, .param2 = param2}});
}

void timer_queue::main_loop() {

  TRACE("<timer_queue> started");
  while (true) {
    {
      std::unique_lock lock(_cmds_mutex);
      time_point_t until;
      
      update_current_time();
      process_cmds();

      if (_timers.size() > 0) {
        timer_handle_t* t = _timers[0];
        until = t->next_trigger;
      } else {
        until = _current_time + std::chrono::milliseconds(1000);
      }

      std::cv_status result = _cmds_condition.wait_until(lock, until);
      if (!_running) break;
    }

    async_calls();
    trigger_timers();
  }
}

void timer_queue::process_cmds()
{
  int added = 0;
  while (!_cmds.empty()) {
    timer_req_t req = _cmds.back();
    _cmds.pop_back();

    if (req.cmd == timer_req_t::cmd_pend_func) {
      if (req.func_call.func) {
        _funcs.emplace_back(std::move(req.func_call));
      }
    } else {
      timer_handle_t *t = req.timer;
      auto pos = std::find(_timers.begin(), _timers.end(), t);
      switch (req.cmd) {
      case timer_req_t::cmd_start:
        t->next_trigger = _current_time + std::chrono::milliseconds(t->period);
        t->active = true;
        if (pos == _timers.end()) {
          _timers.emplace_back(t);
          added++;
        }
        break;

      case timer_req_t::cmd_stop:
        if (pos != _timers.end()) {
          _timers.erase(pos);
        }
        break;

      default:
        break;
      }
    }
  }
  if (added) sort_timers();
}

void timer_queue::trigger_timers() {
  int triggered = 0;
  for (auto t : _timers) {
    if (t->next_trigger <= _current_time) {
      if (t->repeat) {
        t->next_trigger += std::chrono::milliseconds(t->period);
      } else {
        t->active = false;
        stop_timer(t);
      }
      t->func(t);
      triggered++;
    } else {
      break;
    }
  }
  if (triggered) sort_timers();
}

void timer_queue::async_calls() {
  for (auto f : _funcs) {
    f.func(f.param1, f.param2);
  }
  _funcs.clear();
}

int timer_create(timer_handle_t* h, timer_func_t func, const char* name,
                 unsigned period, bool repeat)
{
  if (!h || !func) return -1;
  timer_queue::create_timer(h, func, name, period, repeat);
  return 0;
}

bool timer_is_created(timer_handle_t* h)
{
  return h && h->func;
}

bool timer_is_active(timer_handle_t* h)
{
  return h && h->active;
}

int timer_start(timer_handle_t* h)
{
  if (!timer_is_created(h)) return -1;
  timer_queue::instance().start_timer(h);
  return 0;
}

int timer_stop(timer_handle_t* h)
{
  if (!timer_is_created(h)) return -1;
  timer_queue::instance().stop_timer(h);
  return 0; 
}

int timer_set_period(timer_handle_t *h, unsigned period)
{
  if (!timer_is_created(h)) return -1;
  h->period = period;
  timer_queue::instance().start_timer(h);
  return 0;
}
