//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ AERA
//_/_/ Autocatalytic Endogenous Reflective Architecture
//_/_/ 
//_/_/ Copyright (c) 2018-2026 Jeff Thompson
//_/_/ Copyright (c) 2018-2026 Kristinn R. Thorisson
//_/_/ Copyright (c) 2018-2026 Icelandic Institute for Intelligent Machines
//_/_/ http://www.iiim.is
//_/_/ 
//_/_/ Copyright (c) 2010-2012 Eric Nivel, Thor List
//_/_/ Center for Analysis and Design of Intelligent Agents
//_/_/ Reykjavik University, Menntavegur 1, 102 Reykjavik, Iceland
//_/_/ http://cadia.ru.is
//_/_/ 
//_/_/ Part of this software was developed by Eric Nivel
//_/_/ in the HUMANOBS EU research project, which included
//_/_/ the following parties:
//_/_/
//_/_/ Autonomous Systems Laboratory
//_/_/ Technical University of Madrid, Spain
//_/_/ http://www.aslab.org/
//_/_/
//_/_/ Communicative Machines
//_/_/ Edinburgh, United Kingdom
//_/_/ http://www.cmlabs.com/
//_/_/
//_/_/ Istituto Dalle Molle di Studi sull'Intelligenza Artificiale
//_/_/ University of Lugano and SUPSI, Switzerland
//_/_/ http://www.idsia.ch/
//_/_/
//_/_/ Institute of Cognitive Sciences and Technologies
//_/_/ Consiglio Nazionale delle Ricerche, Italy
//_/_/ http://www.istc.cnr.it/
//_/_/
//_/_/ Dipartimento di Ingegneria Informatica
//_/_/ University of Palermo, Italy
//_/_/ http://diid.unipa.it/roboticslab/
//_/_/
//_/_/
//_/_/ --- HUMANOBS Open-Source BSD License, with CADIA Clause v 1.0 ---
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without
//_/_/ modification, is permitted provided that the following conditions
//_/_/ are met:
//_/_/ - Redistributions of source code must retain the above copyright
//_/_/   and collaboration notice, this list of conditions and the
//_/_/   following disclaimer.
//_/_/ - Redistributions in binary form must reproduce the above copyright
//_/_/   notice, this list of conditions and the following disclaimer 
//_/_/   in the documentation and/or other materials provided with 
//_/_/   the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its
//_/_/   contributors may be used to endorse or promote products
//_/_/   derived from this software without specific prior 
//_/_/   written permission.
//_/_/   
//_/_/ - CADIA Clause: The license granted in and to the software 
//_/_/   under this agreement is a limited-use license. 
//_/_/   The software may not be used in furtherance of:
//_/_/    (i)   intentionally causing bodily injury or severe emotional 
//_/_/          distress to any person;
//_/_/    (ii)  invading the personal privacy or violating the human 
//_/_/          rights of any person; or
//_/_/    (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
//_/_/ CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
//_/_/ INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
//_/_/ MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
//_/_/ DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
//_/_/ CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
//_/_/ BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
//_/_/ SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
//_/_/ INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
//_/_/ WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
//_/_/ NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
//_/_/ OF SUCH DAMAGE.
//_/_/ 
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#ifndef core_utils_h
#define core_utils_h

#include "types.h"
#include <stdio.h>

#include <iostream>
#include <string>
#include <atomic>

#if defined WINDOWS
#include <sys/timeb.h>
#include <time.h>
#elif defined LINUX || defined MACOS
#include <dlfcn.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
// #undef HANDLE
// #define HANDLE pthread_cond_t*
#endif

#ifdef WINDOWS
#define SOCKETWOULDBLOCK WSAEWOULDBLOCK
#else
#define SOCKETWOULDBLOCK EWOULDBLOCK
#endif

#ifndef SD_BOTH
#define SD_BOTH 2
#endif

#define R250_LEN 250
#define R521_LEN 521

// Wrapping of OS-dependent functions
namespace core {

bool core_dll WaitForSocketReadability(socket s, int32 timeout);
bool core_dll WaitForSocketWriteability(socket s, int32 timeout);

class core_dll Error {
public:
  static int32 GetLastOSErrorNumber();
  static bool GetOSErrorMessage(char* buffer, uint32 buflen, int32 err = -1);
  static bool PrintLastOSErrorMessage(const char* title);
  static void PrintBinary(void* p, uint32 size, bool asInt, const char* title = NULL);
};

#if defined WINDOWS
#elif defined LINUX || defined MACOS
struct SemaTex {
  pthread_mutex_t mutex;
  pthread_cond_t semaphore;
};
#endif

class core_dll SharedLibrary {
private:
  shared_object library_;
public:
  SharedLibrary();
  ~SharedLibrary();
  SharedLibrary *load(const char *fileName);
  void* getFunction(const char *functionName);
};

class core_dll Thread {
private:
  thread thread_;
  bool is_meaningful_;
protected:
  Thread();
public:
  template<class T> static T *New(thread_function f, void *args);
  static void TerminateAndWait(Thread **threads, uint32 threadCount);
  static void TerminateAndWait(Thread *thread);
  static void Wait(Thread **threads, uint32 threadCount);
  static void Wait(Thread *thread);
  static void Sleep(std::chrono::milliseconds ms);
  static void Sleep(std::chrono::system_clock::duration ms) { Sleep(std::chrono::duration_cast<std::chrono::milliseconds>(ms)); }
  static void Sleep(); // inifnite
  virtual ~Thread();
  void start(thread_function f);
  void suspend();
  void resume();
  void terminate();
};

class core_dll TimeProbe { // requires Time::Init()
private:
  int64 cpu_counts_;
  int64 getCounts();
public:
  void set();    // initialize
  void  check(); // gets the cpu count elapsed between set() and check()
};

class core_dll Time { // TODO: make sure time stamps are consistent when computed by different cores
  friend class TimeProbe;
private:
  static float64 Period_;
  static Timestamp InitTime_;
public:
  static void Init(uint32 r); // detects the hardware timing capabilities; r: time resolution in us (on windows xp: max ~1000; use 1000, 2000, 5000 or 10000)
  static Timestamp Get();     // timestamp since 01/01/1970

  static std::string ToString_year(Timestamp timestamp);    // day_name day_number month year hour:minutes:seconds:milliseconds:microseconds GMT since 01/01/1970.
};

class core_dll Host {
public:
  typedef char host_name[255];
  static uint8 Name(char *name); // name size=255; return the actual size
};

class core_dll Semaphore {
private:
  semaphore s_;
protected:
  static const uint32 Infinite;
public:
  Semaphore(uint32 initialCount, uint32 maxCount);
  ~Semaphore();
  bool acquire(uint32 timeout = Infinite); // returns true if timedout
  void release(uint32 count = 1);
  void reset();
};

class core_dll Mutex {
private:
  mutex m_;
protected:
  static const uint32 Infinite;
public:
  Mutex();
  ~Mutex();
  bool acquire(uint32 timeout = Infinite); // returns true if timedout
  void release();
};

class core_dll CriticalSection {
private:
  critical_section cs_;
public:
  CriticalSection();
  ~CriticalSection();
  void enter();
  void leave();
};

class core_dll Timer {
public:
#if defined MACOS
  struct MacOSTimerState {
    pthread_t thread;
    volatile bool should_stop;
    volatile uint64_t deadline_us;
    volatile uint64_t period_us;
    struct SemaTex sematex;
  } macos_timer_;
#endif
private:
#if defined WINDOWS
  timer t_;
#elif defined LINUX
  timer_t timer;
  struct SemaTex sematex;
#endif
protected:
  static const uint32 Infinite;
public:
  Timer();
  ~Timer();
  void start(std::chrono::microseconds deadline, std::chrono::milliseconds period = std::chrono::seconds(0));   // deadline in us, period in ms.
  bool wait(uint32 timeout = Infinite);            // timeout in ms; returns true if timedout.
};

class core_dll Event {
private:
#if defined WINDOWS
  event e_;
#elif defined LINUX
  // TODO.
#endif
public:
  Event();
  ~Event();
  void wait();
  void fire();
  void reset();
};

class core_dll SignalHandler {
public:
  static void Add(signal_handler h);
  static void Remove(signal_handler h);
};

uint8 core_dll BSR(word data); // BitScanReverse

class core_dll FastSemaphore : // lock-free under no contention
  public Semaphore {
private:
  std::atomic_int32_t count_; // minus the number of waiting threads
  const int32 maxCount_; // max number of threads allowed to run
public:
  FastSemaphore(uint32 initialCount, uint32 maxCount); // initialCount >=0
  ~FastSemaphore();
  void acquire();
  void release();
};

class core_dll String {
public:
  static int32 StartsWith(const std::string &s, const std::string &str);
  static int32 EndsWith(const std::string &s, const std::string &str);
  static void MakeUpper(std::string &str);
  static void MakeLower(std::string &str);
  static void Trim(std::string& str, const char* chars2remove = " ");
  static void TrimLeft(std::string& str, const char* chars2remove = " ");
  static void TrimRight(std::string& str, const char* chars2remove = " ");
  static void ReplaceLeading(std::string& str, const char* chars2replace, char c);
};

class core_dll Random {
private:
  static int32 r250_index_;
  static int32 r521_index_;
  static uint32 r250_buffer_[R250_LEN];
  static uint32 r521_buffer_[R521_LEN];
public:
  static void Init();

  float32 operator ()(uint32 range); // returns a value in [0,range].
};
}

#include "utils.tpl.cpp"


#endif
