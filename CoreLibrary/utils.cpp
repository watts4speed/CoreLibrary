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

#include "utils.h"

using namespace std::chrono;

#if defined WINDOWS
#include <intrin.h>
#pragma intrinsic (_InterlockedDecrement)
#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedExchange)
#pragma intrinsic (_InterlockedExchange64)
#pragma intrinsic (_InterlockedCompareExchange)
#pragma intrinsic (_InterlockedCompareExchange64)
#elif defined LINUX || defined MACOS
#endif

#include <algorithm>
#include <cctype>
#include <ctime>


#define R250_IA (sizeof(uint32)*103)
#define R250_IB (sizeof(uint32)*R250_LEN-R250_IA)
#define R521_IA (sizeof(uint32)*168)
#define R521_IB (sizeof(uint32)*R521_LEN-R521_IA)

namespace core {

#if defined LINUX || defined MACOS
bool CalcTimeout(struct timespec &timeout, uint32 ms) {

  struct timeval now;
  if (gettimeofday(&now, NULL) != 0)
    return false;

  timeout.tv_sec = now.tv_sec + ms / 1000;
  long us = now.tv_usec + ms % 1000;
  if (us >= 1000000) {
    timeout.tv_sec++;
    us -= 1000000;
  }
  timeout.tv_nsec = us * 1000; // usec -> nsec
  return true;
}

uint64 GetTime() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL))
    return 0;
  return (tv.tv_usec + tv.tv_sec * 1000000LL);
}
#endif

void Error::PrintBinary(void* p, uint32 size, bool asInt, const char* title) {
  if (title != NULL)
    printf("--- %s %u ---\n", title, size);
  unsigned char c;
  for (uint32 n = 0; n < size; n++) {
    c = *(((unsigned char*)p) + n);
    if (asInt)
      printf("[%u] ", (unsigned int)c);
    else
      printf("[%c] ", c);
    if ((n > 0) && ((n + 1) % 10 == 0))
      printf("\n");
  }
  printf("\n");
}

SharedLibrary::SharedLibrary() : library_(NULL) {
}

SharedLibrary::~SharedLibrary() {
#if defined WINDOWS
  if (library_)
    FreeLibrary(library_);
#elif defined LINUX || defined MACOS
  if (library_)
    dlclose(library_);
#endif
}

SharedLibrary *SharedLibrary::load(const char *fileName) {
#if defined WINDOWS
  library_ = LoadLibrary(TEXT(fileName));
  if (!library_) {

    DWORD error = GetLastError();
    std::cerr << "> Error: unable to load shared library " << fileName << " :" << error << std::endl;
    return NULL;
  }
#elif defined MACOS
  // On macOS, try loading the specified file with .dylib extension.
  // If that fails, fall back to dlopen(NULL) to access symbols compiled into the binary.
  const char* suffix = ".dylib";
  char *libraryName = (char *)calloc(1, strlen(fileName) + strlen(suffix) + 1);
  strcat(libraryName, fileName);
  if (strstr(fileName + (strlen(fileName) > 6 ? strlen(fileName) - 6 : 0), ".dylib") == NULL &&
      strstr(fileName + (strlen(fileName) > 3 ? strlen(fileName) - 3 : 0), ".so") == NULL &&
      strstr(fileName + (strlen(fileName) > 4 ? strlen(fileName) - 4 : 0), ".dll") == NULL) {
    strcat(libraryName, suffix);
  }
  library_ = dlopen(libraryName, RTLD_NOW | RTLD_GLOBAL);
  free(libraryName);
  if (!library_) {
    // Fall back to loading the current binary's symbols (operators compiled in statically).
    library_ = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);
    if (!library_) {
      std::cout << "> Error: unable to load shared library " << fileName << " :" << dlerror() << std::endl;
      return NULL;
    }
  }
#elif defined LINUX
  /*
   * libraries on Linux are called 'lib<name>.so'
   * if the passed in fileName does not have those
   * components add them in.
   */
  char *libraryName = (char *)calloc(1, strlen(fileName) + 6 + 1);
  if (strstr(fileName, "lib") == NULL) {
    strcat(libraryName, "lib");
  }
  strcat(libraryName, fileName);
  if (strstr(fileName + (strlen(fileName) - 3), ".so") == NULL) {
    strcat(libraryName, ".so");
  }
  library_ = dlopen(libraryName, RTLD_NOW | RTLD_GLOBAL);
  if (!library_) {
    std::cout << "> Error: unable to load shared library " << fileName << " :" << dlerror() << std::endl;
    free(libraryName);
    return NULL;
  }
  free(libraryName);
#endif
  return this;
}


void* SharedLibrary::getFunction(const char* functionName) {
  void* function = NULL;
#if defined WINDOWS
  if (library_) {

    function = GetProcAddress(library_, functionName);
    if (!function) {

      DWORD error = GetLastError();
      std::cerr << "GetProcAddress > Error: " << error << std::endl;
    }
  }
#elif defined LINUX || defined MACOS
  if (library_) {
    function = dlsym(library_, functionName);
    if (!function) {
      std::cout << "> Error: unable to find symbol " << functionName << " :" << dlerror() << std::endl;
    }
  }
#endif
  return function;
}

////////////////////////////////////////////////////////////////////////////////////////////////

void Thread::TerminateAndWait(Thread **threads, uint32 threadCount) {
  if (!threads)
    return;
  for (uint32 i = 0; i < threadCount; i++) {
    threads[i]->terminate();
    Thread::Wait(threads[i]);
  }
}

void Thread::TerminateAndWait(Thread *thread) {
  if (!thread)
    return;
  thread->terminate();
  Thread::Wait(thread);
}

void Thread::Wait(Thread **threads, uint32 threadCount) {

  if (!threads)
    return;
#if defined WINDOWS
  for (uint32 i = 0; i < threadCount; i++)
    WaitForSingleObject(threads[i]->thread_, INFINITE);
#elif defined LINUX || defined MACOS
  for (uint32 i = 0; i < threadCount; i++)
    pthread_join(threads[i]->thread_, NULL);
#endif
}

void Thread::Wait(Thread *thread) {

  if (!thread)
    return;
#if defined WINDOWS
  WaitForSingleObject(thread->thread_, INFINITE);
#elif defined LINUX || defined MACOS
  pthread_join(thread->thread_, NULL);
#endif
}

void Thread::Sleep(milliseconds ms) {
#if defined WINDOWS
  ::Sleep((uint32)ms.count());
#elif defined LINUX || defined MACOS
  // we are actually being passed millisecond, so multiply up
  usleep(ms.count() * 1000);
#endif
}

void Thread::Sleep() {
#if defined WINDOWS
  ::Sleep(INFINITE);
#elif defined LINUX || defined MACOS
  while (true)
    sleep(1000);
#endif
}

Thread::Thread() : is_meaningful_(false) {
  thread_ = 0;
}

Thread::~Thread() {
#if defined WINDOWS
  // ExitThread(0);
  if (is_meaningful_)
    CloseHandle(thread_);
#elif defined LINUX || defined MACOS
  // delete(thread_);
#endif
}

void Thread::start(thread_function f) {
#if defined WINDOWS
  thread_ = CreateThread(NULL, 65536, f, this, 0, NULL); // 64KB: minimum initial stack size
#elif defined LINUX || defined MACOS
  pthread_create(&thread_, NULL, f, this);
#endif
  is_meaningful_ = true;
}

void Thread::suspend() {
#if defined WINDOWS
  SuspendThread(thread_);
#elif defined LINUX || defined MACOS
  pthread_kill(thread_, SIGSTOP);
#endif
}

void Thread::resume() {
#if defined WINDOWS
  ResumeThread(thread_);
#elif defined LINUX || defined MACOS
  pthread_kill(thread_, SIGCONT);
#endif
}

void Thread::terminate() {
#if defined WINDOWS
  TerminateThread(thread_, 0);
#elif defined LINUX || defined MACOS
  pthread_cancel(thread_);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

void TimeProbe::set() {

  cpu_counts_ = getCounts();
}

int64 TimeProbe::getCounts() {
#if defined WINDOWS
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return counter.QuadPart;
#elif defined LINUX || defined MACOS
  static struct timeval tv;
  static struct timezone tz;
  gettimeofday(&tv, &tz);
  return (((int64)tv.tv_sec) * 1000000) + (int64)tv.tv_usec;
#endif
}

void TimeProbe::check() {

  cpu_counts_ = getCounts() - cpu_counts_;
}

////////////////////////////////////////////////////////////////////////////////////////////////

#if defined WINDOWS
typedef LONG NTSTATUS;
typedef NTSTATUS(__stdcall *NSTR)(ULONG, BOOLEAN, PULONG);
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
bool NtSetTimerResolution(IN ULONG RequestedResolution, IN BOOLEAN Set, OUT PULONG ActualResolution);
#elif defined LINUX || defined MACOS
  // TODO
#endif

float64 Time::Period_;

Timestamp Time::InitTime_;

void Time::Init(uint32 r) {
#if defined WINDOWS
  NTSTATUS nts;
  HMODULE NTDll = ::LoadLibrary("NTDLL");
  ULONG actualResolution = 0;
  if (NTDll) {

    NSTR pNSTR = (NSTR)::GetProcAddress(NTDll, "NtSetTimerResolution"); // undocumented win xp sys internals
    if (pNSTR)
      nts = (*pNSTR)(10 * r, true, &actualResolution); // in 100 ns units
  }
  LARGE_INTEGER f;
  QueryPerformanceFrequency(&f);
  Period_ = 1000000.0 / f.QuadPart; // in us
  struct _timeb local_time;
  _ftime(&local_time);
  auto now = Timestamp(microseconds((int64)(local_time.time * 1000 + local_time.millitm) * 1000));
  // The QueryPerformanceCounter in Get() may not start at zero, so subtract it initially.
  InitTime_ = Timestamp(seconds(0));
  InitTime_ = now - Get().time_since_epoch();
#elif defined LINUX || defined MACOS
  // The steady_clock in Get() may not start at zero, so subtract it initially.
  InitTime_ = system_clock_us::now() - duration_cast<microseconds>(steady_clock::now().time_since_epoch());
#endif
}

Timestamp Time::Get() {
#if defined WINDOWS
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return InitTime_ + microseconds((uint64)(counter.QuadPart * Period_));
#elif defined LINUX || defined MACOS
  return InitTime_ + duration_cast<microseconds>(steady_clock::now().time_since_epoch());
#endif
}

std::string Time::ToString_year(Timestamp timestamp) {
  // For now, assume all times are after the epoch. Take the absolute value to be sure.
  uint64 t = abs(duration_cast<microseconds>(timestamp.time_since_epoch()).count());

  uint64 us = t % 1000;
  uint64 ms = t / 1000;
  uint64 s = ms / 1000;
  ms = ms % 1000;

  time_t _gmt_time = s;
  struct tm   *_t = gmtime(&_gmt_time);

  std::string _s = asctime(_t); // _s is: Www Mmm dd hh:mm:ss yyyy but we want: Www Mmm dd yyyy hh:mm:ss:msmsms:ususus
  std::string year = _s.substr(_s.length() - 5, 4);
  _s.erase(_s.length() - 6, 5);
  std::string hh_mm_ss = _s.substr(_s.length() - 9, 8);
  hh_mm_ss += ":";
  hh_mm_ss += std::to_string(ms);
  hh_mm_ss += ":";
  hh_mm_ss += std::to_string(us);

  _s.erase(_s.length() - 9, 9);
  _s += year;
  _s += " ";
  _s += hh_mm_ss;
  _s += " GMT";

  return _s;
}

////////////////////////////////////////////////////////////////////////////////////////////////

uint8 Host::Name(char *name) {
#if defined WINDOWS
  DWORD s = 255;
  GetComputerName(name, &s);
  return (uint8)s;
#elif defined LINUX || defined MACOS
  struct utsname utsname;
  uname(&utsname);
  strcpy(name, utsname.nodename);
  return strlen(name);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

#if defined WINDOWS
const uint32 Semaphore::Infinite = INFINITE;
#elif defined LINUX || defined MACOS
  /*
   * Normally this should be SEM_VALUE_MAX but apparently the <semaphore.h> header
   * does not define it. The documents I have read indicate that on Linux it is
   * always equal to INT_MAX - so use that instead.
   */
const uint32 Semaphore::Infinite = INT_MAX;
#endif

Semaphore::Semaphore(uint32 initialCount, uint32 maxCount) {
#if defined WINDOWS
  s_ = CreateSemaphore(NULL, initialCount, maxCount, NULL);
#elif defined LINUX || defined MACOS
  sem_init(&s_, 0, initialCount);
#endif
}

Semaphore::~Semaphore() {
#if defined WINDOWS
  CloseHandle(s_);
#elif defined LINUX || defined MACOS
  sem_destroy(&s_);
#endif
}

bool Semaphore::acquire(uint32 timeout) {
#if defined WINDOWS
  uint32 r = WaitForSingleObject(s_, timeout);
  return r == WAIT_TIMEOUT;
#elif defined MACOS
  // sem_timedwait is not available on macOS; use a poll loop with sem_trywait
  if (timeout == INT_MAX) {
    return sem_wait(&s_) == 0;
  }
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 1000000; // 1ms poll interval
  uint32 elapsed_ms = 0;
  while (elapsed_ms < timeout) {
    if (sem_trywait(&s_) == 0)
      return true;
    nanosleep(&ts, NULL);
    elapsed_ms++;
  }
  return false;
#elif defined LINUX
  struct timespec t;
  int r;

  CalcTimeout(t, timeout);
  r = sem_timedwait(&s_, &t);
  return r != 0;
#endif
}

void Semaphore::release(uint32 count) {
#if defined WINDOWS
  ReleaseSemaphore(s_, count, NULL);
#elif defined LINUX || defined MACOS
  for (uint32 c = 0; c < count; c++)
    sem_post(&s_);
#endif
}

void Semaphore::reset() {
#if defined WINDOWS
  bool r;
  do
    r = acquire(0);
  while (!r);
#elif defined LINUX || defined MACOS
  bool r;
  do
    r = acquire(0);
  while (!r);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

#if defined WINDOWS
const uint32 Mutex::Infinite = INFINITE;
#elif defined LINUX || defined MACOS
  /*
   * Normally this should be SEM_VALUE_MAX but apparently the <semaphore.h> header
   * does not define it. The documents I have read indicate that on Linux it is
   * always equal to INT_MAX - so use that instead.
   */
const uint32 Mutex::Infinite = INT_MAX;
#endif

Mutex::Mutex() {
#if defined WINDOWS
  m_ = CreateMutex(NULL, false, NULL);
#elif defined LINUX || defined MACOS
  pthread_mutex_init(&m_, NULL);
#endif
}

Mutex::~Mutex() {
#if defined WINDOWS
  CloseHandle(m_);
#elif defined LINUX || defined MACOS
  pthread_mutex_destroy(&m_);
#endif
}

bool Mutex::acquire(uint32 timeout) {
#if defined WINDOWS
  uint32 r = WaitForSingleObject(m_, timeout);
  return r == WAIT_TIMEOUT;
#elif defined LINUX || defined MACOS
  auto start = Time::Get();
  auto uTimeout = microseconds(timeout * 1000);

  while (pthread_mutex_trylock(&m_) != 0) {
    Thread::Sleep(milliseconds(10));
    if (Time::Get() - start >= uTimeout)
      return false;
  }
  return true;

#endif
}

void Mutex::release() {
#if defined WINDOWS
  ReleaseMutex(m_);
#elif defined LINUX || defined MACOS
  pthread_mutex_unlock(&m_);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

CriticalSection::CriticalSection() {
#if defined WINDOWS
  InitializeCriticalSection(&cs_);
#elif defined LINUX || defined MACOS
  // Use a recursive mutex to match Windows CRITICAL_SECTION behavior (re-entrant).
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&cs_, &attr);
  pthread_mutexattr_destroy(&attr);
#endif
}

CriticalSection::~CriticalSection() {
#if defined WINDOWS
  DeleteCriticalSection(&cs_);
#elif defined LINUX || defined MACOS
  pthread_mutex_destroy(&cs_);
#endif
}

void CriticalSection::enter() {
#if defined WINDOWS
  EnterCriticalSection(&cs_);
#elif defined LINUX || defined MACOS
  pthread_mutex_lock(&cs_);
#endif
}

void CriticalSection::leave() {
#if defined WINDOWS
  LeaveCriticalSection(&cs_);
#elif defined LINUX || defined MACOS
  pthread_mutex_unlock(&cs_);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

#if defined WINDOWS
const uint32 Timer::Infinite = INFINITE;
#elif defined LINUX || defined MACOS
const uint32 Timer::Infinite = INT_MAX;
#endif

#if defined LINUX
static void timer_signal_handler(int sig, siginfo_t *siginfo, void *context) {
  SemaTex* sematex = (SemaTex*)siginfo->si_value.sival_ptr;
  if (sematex == NULL)
    return;
  pthread_mutex_lock(&sematex->mutex);
  pthread_cond_broadcast(&sematex->semaphore);
  pthread_mutex_unlock(&sematex->mutex);
}
#endif

#if defined MACOS
static void* macos_timer_thread(void* arg) {
  Timer::MacOSTimerState* s = (Timer::MacOSTimerState*)arg;
  struct timespec ts;
  if (s->deadline_us > 0) {
    ts.tv_sec = s->deadline_us / 1000000;
    ts.tv_nsec = (s->deadline_us % 1000000) * 1000;
    nanosleep(&ts, NULL);
  }
  while (!s->should_stop) {
    pthread_mutex_lock(&s->sematex.mutex);
    pthread_cond_broadcast(&s->sematex.semaphore);
    pthread_mutex_unlock(&s->sematex.mutex);
    if (s->period_us == 0)
      break;
    ts.tv_sec = s->period_us / 1000000;
    ts.tv_nsec = (s->period_us % 1000000) * 1000;
    nanosleep(&ts, NULL);
  }
  return NULL;
}
#endif

Timer::Timer() {
#if defined WINDOWS
  t_ = CreateWaitableTimer(NULL, false, NULL);
  if (t_ == NULL) {
    printf("Error creating timer\n");
  }
#elif defined MACOS
  pthread_cond_init(&macos_timer_.sematex.semaphore, NULL);
  pthread_mutex_init(&macos_timer_.sematex.mutex, NULL);
  macos_timer_.should_stop = true;
  macos_timer_.thread = 0;
  macos_timer_.deadline_us = 0;
  macos_timer_.period_us = 0;
#elif defined LINUX
  pthread_cond_init(&sematex.semaphore, NULL);
  pthread_mutex_init(&sematex.mutex, NULL);

  struct sigaction sa;
  struct sigevent timer_event;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;   /* Real-Time signal */
  sa.sa_sigaction = timer_signal_handler;
  sigaction(SIGRTMIN, &sa, NULL);

  timer_event.sigev_notify = SIGEV_SIGNAL;
  timer_event.sigev_signo = SIGRTMIN;
  timer_event.sigev_value.sival_ptr = (void *)&sematex;
  int ret = timer_create(CLOCK_REALTIME, &timer_event, &timer);
  if (ret != 0) {
    printf("Error creating timer: %d\n", ret);
  }
#endif
}

Timer::~Timer() {
#if defined WINDOWS
  CloseHandle(t_);
#elif defined MACOS
  macos_timer_.should_stop = true;
  if (macos_timer_.thread) {
    pthread_join(macos_timer_.thread, NULL);
    macos_timer_.thread = 0;
  }
  pthread_cond_destroy(&macos_timer_.sematex.semaphore);
  pthread_mutex_destroy(&macos_timer_.sematex.mutex);
#elif defined LINUX
  pthread_cond_destroy(&sematex.semaphore);
  pthread_mutex_destroy(&sematex.mutex);
  timer_delete(timer);
#endif
}

void Timer::start(microseconds deadline, milliseconds period) {
#if defined WINDOWS
  LARGE_INTEGER _deadline; // in 100 ns intervals
  _deadline.QuadPart = -10LL * deadline.count(); // negative means relative
  bool r = SetWaitableTimer(t_, &_deadline, (long)period.count(), NULL, NULL, 0);
  if (!r) {
    printf("Error arming timer\n");
  }
#elif defined MACOS
  macos_timer_.should_stop = true;
  if (macos_timer_.thread) {
    pthread_join(macos_timer_.thread, NULL);
    macos_timer_.thread = 0;
  }
  macos_timer_.deadline_us = (uint64_t)deadline.count();
  macos_timer_.period_us = (uint64_t)period.count() * 1000;
  macos_timer_.should_stop = false;
  pthread_create(&macos_timer_.thread, NULL, macos_timer_thread, &macos_timer_);
#elif defined LINUX
  struct itimerspec newtv;
  sigset_t allsigs;

  uint64 t = deadline.count();
  uint64 p = period.count() * 1000;
  newtv.it_interval.tv_sec = p / 1000000;
  newtv.it_interval.tv_nsec = (p % 1000000) * 1000;
  newtv.it_value.tv_sec = t / 1000000;
  newtv.it_value.tv_nsec = (t % 1000000) * 1000;

  pthread_mutex_lock(&sematex.mutex);

  int ret = timer_settime(timer, 0, &newtv, NULL);
  if (ret != 0) {
    printf("Error arming timer: %d\n", ret);
  }
  sigemptyset(&allsigs);

  pthread_mutex_unlock(&sematex.mutex);
#endif
}

bool Timer::wait(uint32 timeout) {
#if defined WINDOWS
  uint32 r = WaitForSingleObject(t_, timeout);
  return r == WAIT_TIMEOUT;
#elif defined MACOS
  bool res;
  struct timespec ttimeout;
  pthread_mutex_lock(&macos_timer_.sematex.mutex);
  if (timeout == INT_MAX) {
    res = (pthread_cond_wait(&macos_timer_.sematex.semaphore, &macos_timer_.sematex.mutex) == 0);
  } else {
    CalcTimeout(ttimeout, timeout);
    res = (pthread_cond_timedwait(&macos_timer_.sematex.semaphore, &macos_timer_.sematex.mutex, &ttimeout) == 0);
  }
  pthread_mutex_unlock(&macos_timer_.sematex.mutex);
  return res;
#elif defined LINUX
  bool res;
  struct timespec ttimeout;

  pthread_mutex_lock(&sematex.mutex);
  if (timeout == INT_MAX) {
    res = (pthread_cond_wait(&sematex.semaphore, &sematex.mutex) != 0);
  }
  else {
    CalcTimeout(ttimeout, timeout);
    res = (pthread_cond_timedwait(&sematex.semaphore, &sematex.mutex, &ttimeout) != 0);
  }
  pthread_mutex_unlock(&sematex.mutex);
  return res;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

Event::Event() {
#if defined WINDOWS
  e_ = CreateEvent(NULL, true, false, NULL);
#elif defined LINUX || defined MACOS
  // TODO.
#endif
}

Event::~Event() {
#if defined WINDOWS
  CloseHandle(e_);
#elif defined LINUX || defined MACOS
  // TODO.
#endif
}

void Event::wait() {
#if defined WINDOWS
  WaitForSingleObject(e_, INFINITE);
#elif defined LINUX || defined MACOS
  // TODO.
#endif
}

void Event::fire() {
#if defined WINDOWS
  SetEvent(e_);
#elif defined LINUX || defined MACOS
  // TODO.
#endif
}

void Event::reset() {
#if defined WINDOWS
  ResetEvent(e_);
#elif defined LINUX || defined MACOS
  // TODO.
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

void SignalHandler::Add(signal_handler h) {
#if defined WINDOWS
  if (SetConsoleCtrlHandler(h, true) == 0) {

    int e = GetLastError();
    std::cerr << "Error: " << e << " failed to add signal handler" << std::endl;
    return;
  }
#elif defined LINUX || defined MACOS
  signal(SIGABRT, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGBUS, SIG_IGN);
  //   signal(SIGHUP, h);
  signal(SIGTERM, h);
  signal(SIGINT, h);
  signal(SIGABRT, h);
  //   signal(SIGFPE, h);
  //   signal(SIGILL, h);
  //   signal(SIGSEGV, h);
#endif
}

void SignalHandler::Remove(signal_handler h) {
#if defined WINDOWS
  SetConsoleCtrlHandler(h, false);
#elif defined LINUX || defined MACOS
  signal(SIGABRT, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGBUS, SIG_IGN);
  //   signal(SIGHUP, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);
  signal(SIGABRT, SIG_DFL);
  //   signal(SIGFPE, SIG_DFL);
  //   signal(SIGILL, SIG_DFL);
  //   signal(SIGSEGV, SIG_DFL);
#endif
}

  ////////////////////////////////////////////////////////////////////////////////////////////////

uint8 BSR(word data) {
#if defined WINDOWS
#if defined ARCH_32
  DWORD index;
  _BitScanReverse(&index, data);
  return (uint8)index;
#elif defined ARCH_64
  DWORD index;
  _BitScanReverse64(&index, data);
  return (uint8)index;
#endif
#elif defined LINUX || defined MACOS
#if defined ARCH_32
  return (uint8)(31 - __builtin_clz((uint32_t)data));
#elif defined ARCH_64
  return (uint8)(63 - __builtin_clzll((uint64_t)data));
#endif
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

FastSemaphore::FastSemaphore(uint32 initialCount, uint32 maxCount) : Semaphore(initialCount > 0 ? 1 : 0, 1), count_(initialCount), maxCount_(maxCount) {
}

FastSemaphore::~FastSemaphore() {
}

void FastSemaphore::acquire() {

  int32 c;
  while ((c = --count_) >= maxCount_); // release calls can bring count over maxCount_: acquire has to exhaust these extras
  if (c < 0)
    Semaphore::acquire();
}

void FastSemaphore::release() {

  int32 c = ++count_;
  if (c <= 0)
    Semaphore::release();
}

////////////////////////////////////////////////////////////////////////////////////////////////
/*
  FastMutex::FastMutex(uint32 initialCount):Semaphore(initialCount,1),count_(initialCount){
  }

  FastMutex::~FastMutex(){
  }

  void FastMutex::acquire(){

    int32 former=Atomic::Swap32(&count_,0);
    if(former==0)
      Semaphore::acquire();
  }

  void FastMutex::release(){

    int32 former=Atomic::Swap32(&count_,1);
    if(former==0)
      Semaphore::release();
  }
  */
  ////////////////////////////////////////////////////////////////////////////////////////////////

bool Error::PrintLastOSErrorMessage(const char* title) {
  int32 err = Error::GetLastOSErrorNumber();
  char buf[1024];
  if (!Error::GetOSErrorMessage(buf, 1024, err))
    printf("%s: [%d] (could not get error message)\n", title, err);
  else
    printf("%s: [%d] %s\n", title, err, buf);
  return true;
}

int32 Error::GetLastOSErrorNumber() {
#ifdef WINDOWS
  int32 err = WSAGetLastError();
  WSASetLastError(0);
  return err;
#else
  return (int32)errno;
#endif
}

bool Error::GetOSErrorMessage(char* buffer, uint32 buflen, int32 err) {
  if (buffer == NULL)
    return false;
  if (buflen < 512) {
    strcpy(buffer, "String buffer not large enough");
    return false;
  }
  if (err < 0)
    err = Error::GetLastOSErrorNumber();

#ifdef WINDOWS
  if (err == WSANOTINITIALISED) {
    strcpy(buffer, "Cannot initialize WinSock!");
  }
  else if (err == WSAENETDOWN) {
    strcpy(buffer, "The network subsystem or the associated service provider has failed");
  }
  else if (err == WSAEAFNOSUPPORT) {
    strcpy(buffer, "The specified address family is not supported");
  }
  else if (err == WSAEINPROGRESS) {
    strcpy(buffer, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function");
  }
  else if (err == WSAEMFILE) {
    strcpy(buffer, "No more socket descriptors are available");
  }
  else if (err == WSAENOBUFS) {
    strcpy(buffer, "No buffer space is available. The socket cannot be created");
  }
  else if (err == WSAEPROTONOSUPPORT) {
    strcpy(buffer, "The specified protocol is not supported");
  }
  else if (err == WSAEPROTOTYPE) {
    strcpy(buffer, "The specified protocol is the wrong type for this socket");
  }
  else if (err == WSAESOCKTNOSUPPORT) {
    strcpy(buffer, "The specified socket type is not supported in this address family");
  }
  else if (err == WSAEADDRINUSE) {
    strcpy(buffer, "The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs during execution of the bind function, but could be delayed until this function if the bind was to a partially wildcard address (involving ADDR_ANY) and if a specific address needs to be committed at the time of this function");
  }
  else if (err == WSAEINVAL) {
    strcpy(buffer, "The socket has not been bound with bind");
  }
  else if (err == WSAEISCONN) {
    strcpy(buffer, "The socket is already connected");
  }
  else if (err == WSAENOTSOCK) {
    strcpy(buffer, "The descriptor is not a socket");
  }
  else if (err == WSAEOPNOTSUPP) {
    strcpy(buffer, "The referenced socket is not of a type that supports the listen operation");
  }
  else if (err == WSAEADDRNOTAVAIL) {
    strcpy(buffer, "The specified address is not a valid address for this machine");
  }
  else if (err == WSAEFAULT) {
    strcpy(buffer, "The name or namelen parameter is not a valid part of the user address space, the namelen parameter is too small, the name parameter contains an incorrect address format for the associated address family, or the first two bytes of the memory block specified by name does not match the address family associated with the socket descriptor s");
  }
  else if (err == WSAEMFILE) {
    strcpy(buffer, "The queue is nonempty upon entry to accept and there are no descriptors available");
  }
  else if (err == SOCKETWOULDBLOCK) {
    strcpy(buffer, "The socket is marked as nonblocking and no connections are present to be accepted");
  }
  else if (err == WSAETIMEDOUT) {
    strcpy(buffer, "Attempt to connect timed out without establishing a connection");
  }
  else if (err == WSAENETUNREACH) {
    strcpy(buffer, "The network cannot be reached from this host at this time");
  }
  else if (err == WSAEISCONN) {
    strcpy(buffer, "The socket is already connected (connection-oriented sockets only)");
  }
  else if (err == WSAECONNREFUSED) {
    strcpy(buffer, "The attempt to connect was forcefully rejected");
  }
  else if (err == WSAEAFNOSUPPORT) {
    strcpy(buffer, "Addresses in the specified family cannot be used with this socket");
  }
  else if (err == WSAEADDRNOTAVAIL) {
    strcpy(buffer, "The remote address is not a valid address (such as ADDR_ANY)");
  }
  else if (err == WSAEALREADY) {
    strcpy(buffer, "A nonblocking connect call is in progress on the specified socket");
  }
  else if (err == WSAECONNRESET) {
    strcpy(buffer, "Connection was reset");
  }
  else if (err == WSAECONNABORTED) {
    strcpy(buffer, "Software caused connection abort");
  }
  else {
    strcpy(buffer, "Socket error with no description");
  }

#else
  strcpy(buffer, strerror(err));
#endif

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////

bool WaitForSocketReadability(socket s, int32 timeout) {

  int maxfd = 0;

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  fd_set rdds;
  // create a list of sockets to check for activity
  FD_ZERO(&rdds);
  // specify mySocket
  FD_SET(s, &rdds);

#ifdef WINDOWS
#else
  maxfd = s + 1;
#endif

  if (timeout > 0) {
    ldiv_t d = ldiv(timeout * 1000, 1000000);
    tv.tv_sec = d.quot;
    tv.tv_usec = d.rem;
  }

  // Check for readability
  int ret = select(maxfd, &rdds, NULL, NULL, &tv);
  return(ret > 0);
}

bool WaitForSocketWriteability(socket s, int32 timeout) {

  int maxfd = 0;

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  fd_set wds;
  // create a list of sockets to check for activity
  FD_ZERO(&wds);
  // specify mySocket
  FD_SET(s, &wds);

#ifdef WINDOWS
#else
  maxfd = s + 1;
#endif

  if (timeout > 0) {
    ldiv_t d = ldiv(timeout * 1000, 1000000);
    tv.tv_sec = d.quot;
    tv.tv_usec = d.rem;
  }

  // Check for readability
  return(select(maxfd, NULL, &wds, NULL, &tv) > 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////

int32 String::StartsWith(const std::string &s, const std::string &str) {
  std::string::size_type pos = s.find_first_of(str);
  if (pos == 0)
    return 0;
  else
    return -1;
}

int32 String::EndsWith(const std::string &s, const std::string &str) {
  std::string::size_type pos = s.find_last_of(str);
  if (pos == s.size() - str.size())
    return pos;
  else
    return -1;
}

void String::MakeUpper(std::string &str)
{
  std::transform(str.begin(), str.end(), str.begin(), toupper);
}

void String::MakeLower(std::string &str)
{
  std::transform(str.begin(), str.end(), str.begin(), tolower);
}

void String::Trim(std::string& str, const char* chars2remove)
{
  TrimLeft(str, chars2remove);
  TrimRight(str, chars2remove);
}

void String::TrimLeft(std::string& str, const char* chars2remove)
{
  if (!str.empty())
  {
    std::string::size_type pos = str.find_first_not_of(chars2remove);

    if (pos != std::string::npos)
      str.erase(0, pos);
    else
      str.erase(str.begin(), str.end()); // make empty
  }
}

void String::TrimRight(std::string& str, const char* chars2remove)
{
  if (!str.empty())
  {
    std::string::size_type pos = str.find_last_not_of(chars2remove);

    if (pos != std::string::npos)
      str.erase(pos + 1);
    else
      str.erase(str.begin(), str.end()); // make empty
  }
}


void String::ReplaceLeading(std::string& str, const char* chars2replace, char c)
{
  if (!str.empty())
  {
    std::string::size_type pos = str.find_first_not_of(chars2replace);

    if (pos != std::string::npos)
      str.replace(0, pos, pos, c);
    else
    {
      int n = str.size();
      str.replace(str.begin(), str.end() - 1, n - 1, c);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////

int32 Random::r250_index_;
int32 Random::r521_index_;
uint32 Random::r250_buffer_[R250_LEN];
uint32 Random::r521_buffer_[R521_LEN];

void Random::Init() {

  int32 i = R521_LEN;
  uint32 mask1 = 1;
  uint32 mask2 = 0xFFFFFFFF;

  while (i-- > R250_LEN)
    r521_buffer_[i] = rand();
  while (i-- > 31) {

    r250_buffer_[i] = rand();
    r521_buffer_[i] = rand();
  }

  // Establish linear independence of the bit columns
  // by setting the diagonal bits and clearing all bits above
  while (i-- > 0) {

    r250_buffer_[i] = (rand() | mask1) & mask2;
    r521_buffer_[i] = (rand() | mask1) & mask2;
    mask2 ^= mask1;
    mask1 >>= 1;
  }
  r250_buffer_[0] = mask1;
  r521_buffer_[0] = mask2;
  r250_index_ = 0;
  r521_index_ = 0;
}

float32 Random::operator ()(uint32 range) {
  /*
  I prescale the indices by sizeof(unsigned long) to eliminate
  four shlwi instructions in the compiled code.  This minor optimization
  increased perf by about 12%.

  I also carefully arrange index increments and comparisons to minimize
  instructions.  gcc 3.3 seems a bit weak on instruction reordering. The
  j1/j2 branches are mispredicted, but nevertheless these optimizations
  increased perf by another 10%.
  */

  int32 i1 = r250_index_;
  int32 i2 = r521_index_;
  uint8 *b1 = (uint8 *)r250_buffer_;
  uint8 *b2 = (uint8 *)r521_buffer_;
  uint32 *tmp1, *tmp2;
  uint32 r, s;
  int32 j1, j2;

  j1 = i1 - R250_IB;
  if (j1 < 0)
    j1 = i1 + R250_IA;
  j2 = i2 - R521_IB;
  if (j2 < 0)
    j2 = i2 + R521_IA;

  tmp1 = (uint32 *)(b1 + i1);
  r = (*(uint32 *)(b1 + j1)) ^ (*tmp1);
  *tmp1 = r;
  tmp2 = (uint32 *)(b2 + i2);
  s = (*(uint32 *)(b2 + j2)) ^ (*tmp2);
  *tmp2 = s;

  i1 = (i1 != sizeof(uint32)*(R250_LEN - 1)) ? (i1 + sizeof(uint32)) : 0;
  r250_index_ = i1;
  i2 = (i2 != sizeof(uint32)*(R521_LEN - 1)) ? (i2 + sizeof(uint32)) : 0;
  r521_index_ = i2;

  float32 _r = r ^ s;
  //return range*(_r/((float32)ULONG_MAX));
  return _r;
}
}
