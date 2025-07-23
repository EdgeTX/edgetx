#include "timer_native.h"

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

struct timer_async_call_t {
  timer_async_func_t func;
  void *param1;
  uint32_t param2;
};

struct timer_req_t {
  enum type {
    cmd_start,
    cmd_stop,
    cmd_pend_func,
  };

  type cmd;

  union {
    timer_handle_t *timer;
    timer_async_call_t func_call;
  };
};

class timer_queue {

  std::unique_ptr<std::thread> _thread;
  bool _running = false;

  std::deque<timer_req_t> _cmds;
  std::mutex _cmds_mutex;
  std::condition_variable _cmds_condition;

  std::vector<timer_handle_t*> _timers;
  std::vector<timer_async_call_t> _funcs;

  time_point_t _current_time;
  
  timer_queue();

  void update_current_time();
  void sort_timers();
  void main_loop();
  void trigger_timers();
  void async_calls();
  void process_cmds();
  void send_cmd(timer_req_t&& req);
  void stop();

public:
  timer_queue(timer_queue const &) = delete;
  void operator=(timer_queue const &) = delete;

  static timer_queue &instance();
  static void destroy();

  static void create_timer(timer_handle_t *timer, timer_func_t func, const char *name,
                           unsigned period, bool repeat);

  void start();

  void start_timer(timer_handle_t *timer);
  void stop_timer(timer_handle_t *timer);

  void pend_function(timer_async_func_t func, void* param1, uint32_t param2);
};
