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

#include <memory.h>


namespace core {

#ifdef PIPE_1
template<typename T, uint32 _S> Pipe11<T, _S>::Pipe11() : Semaphore(0, 65535) {

  head_ = tail_ = -1;
  first_ = last_ = new Block(NULL);
  spare_ = NULL;
}

template<typename T, uint32 _S> Pipe11<T, _S>::~Pipe11() {

  delete first_;
  if (spare_)
    delete spare_;
}

template<typename T, uint32 _S> inline void Pipe11<T, _S>::_clear() { // leaves spare_ as is

  enter();
  reset();
  if (first_->next_)
    delete first_->next_;
  first_->next_ = NULL;
  head_ = tail_ = -1;
  leave();
}

template<typename T, uint32 _S> inline T Pipe11<T, _S>::_pop() {

  T t = first_->buffer_[head_];
  if (++head_ == _S) {

    enter();
    if (first_ == last_)
      head_ = tail_ = -1; // stay in the same block; next push will reset head_ and tail_ to 0
    else {

      if (!spare_) {

        spare_ = first_;
        first_ = first_->next_;
        spare_->next_ = NULL;
      } else {

        Block *b = first_->next_;
        first_->next_ = NULL;
        delete first_;
        first_ = b;
      }
      head_ = 0;
    }
    leave();
  }
  return t;
}

template<typename T, uint32 _S> inline void Pipe11<T, _S>::push(T &t) {

  enter();
  if (++tail_ == 0)
    head_ = 0;
  uint32 index = tail_;
  if (tail_ == _S) {

    if (spare_) {

      last_->next_ = spare_;
      last_ = spare_;
      last_->next_ = NULL;
      spare_ = NULL;
    } else
      last_ = new Block(last_);
    tail_ = 0;
    index = tail_;
  }
  leave();

  last_->buffer_[index] = t;
  release();
}

template<typename T, uint32 _S> inline T Pipe11<T, _S>::pop() {

  Semaphore::acquire();
  return _pop();
}

template<typename T, uint32 _S> inline void Pipe11<T, _S>::clear() {

  _clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S> Pipe1N<T, _S>::Pipe1N() {
}

template<typename T, uint32 _S> Pipe1N<T, _S>::~Pipe1N() {
}

template<typename T, uint32 _S> void Pipe1N<T, _S>::clear() {

  popCS_.enter();
  Pipe11<T, _S>::_clear();
  popCS_.leave();
}

template<typename T, uint32 _S> T Pipe1N<T, _S>::pop() {

  Semaphore::acquire();
  popCS_.enter();
  T t = Pipe11<T, _S>::_pop();
  popCS_.leave();
  return t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S> PipeN1<T, _S>::PipeN1() {
}

template<typename T, uint32 _S> PipeN1<T, _S>::~PipeN1() {
}

template<typename T, uint32 _S> void PipeN1<T, _S>::clear() {

  pushCS_.enter();
  Pipe11<T, _S>::_clear();
  pushCS_.leave();
}

template<typename T, uint32 _S> void PipeN1<T, _S>::push(T &t) {

  pushCS_.enter();
  Pipe11<T, _S>::push(t);
  pushCS_.leave();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S> PipeNN<T, _S>::PipeNN() {
}

template<typename T, uint32 _S> PipeNN<T, _S>::~PipeNN() {
}

template<typename T, uint32 _S> void PipeNN<T, _S>::clear() {

  pushCS_.enter();
  popCS_.enter();
  Pipe11<T, _S>::_clear();
  popCS_.leave();
  pushCS_.leave();
}

template<typename T, uint32 _S> void PipeNN<T, _S>::push(T &t) {

  pushCS_.enter();
  Pipe11<T, _S>::push(t);
  pushCS_.leave();
}

template<typename T, uint32 _S> T PipeNN<T, _S>::pop(bool waitForItem) {

  if (waitForItem)
    Semaphore::acquire();
  else {
    // Use 0 timeout.
    if (Semaphore::acquire(0))
      // A timeout means there are no items.
      return NULL;
  }
  popCS_.enter();
  T t = Pipe11<T, _S>::_pop();
  popCS_.leave();
  return t;
}
#elif defined PIPE_2
template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> Pipe<T, _S, Head, Tail, P, Push, Pop>::Pipe() : Semaphore(0, 1) {

  head_ = -1;
  tail_ = 0;

  waitingList_ = 0;

  first_ = last_ = new Block(NULL);
  spare_ = new Block(NULL);

  _push = new Push<T, _S, P>(*(P *)this);
  _pop = new Pop<T, _S, P>(*(P *)this);
}

template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> Pipe<T, _S, Head, Tail, P, Push, Pop>::~Pipe() {

  delete first_;
  if (spare_)
    delete spare_;
  delete _push;
  delete _pop;
}

template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> inline void Pipe<T, _S, Head, Tail, P, Push, Pop>::shrink() {

  if (!spare_) {

    spare_ = first_;
    first_ = first_->next_;
    spare_->next_ = NULL;
  } else {

    Block *b = first_->next_;
    first_->next_ = NULL;
    delete first_;
    first_ = b;
  }
  head_ = -1;
}

template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> inline void Pipe<T, _S, Head, Tail, P, Push, Pop>::grow() {

  if (spare_) {

    last_->next_ = spare_;
    last_ = spare_;
    last_->next_ = NULL;
    spare_ = NULL;
  } else
    last_ = new Block(last_);
  tail_ = 0;
}

template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> inline void Pipe<T, _S, Head, Tail, P, Push, Pop>::push(T &t) {

  (*_push)(t);
}

template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> inline T Pipe<T, _S, Head, Tail, P, Push, Pop>::pop() {

  return (*_pop)();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class Pipe> PipeFunctor<Pipe>::PipeFunctor(Pipe &p) : pipe(p) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S, class Pipe> Push1<T, _S, Pipe>::Push1(Pipe &p) : PipeFunctor<Pipe>(p) {
}

template<typename T, uint32 _S, class Pipe> void Push1<T, _S, Pipe>::operator ()(T &t) {

  pipe.last_->buffer_[pipe.tail_] = t;

  if (++pipe.tail_ == (int32)_S)
    pipe.grow();

  int32 count = Atomic::Decrement32(&pipe.waitingList_);
  if (count >= 0) // at least one reader is waiting
    pipe.release(); // unlock one reader
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S, class Pipe> PushN<T, _S, Pipe>::PushN(Pipe &p) : PipeFunctor<Pipe>(p), Semaphore(0, 1) {
}

template<typename T, uint32 _S, class Pipe> void PushN<T, _S, Pipe>::operator ()(T &t) {

check_tail: int32 tail = Atomic::Increment32(&pipe.tail_) - 1;

  if (tail < (int32)_S)
    pipe.last_->buffer_[tail] = t;
  else if (tail == (int32)_S) {

    pipe.grow(); // tail set to 0

    pipe.last_->buffer_[pipe.tail_++] = t;

    release(); // unlock writers
    acquire(); // make sure the sem falls back to 0
  } else { // tail>_S: pipe.last_ and pipe.tail_ are being changed

    acquire(); // wait
    release(); // unlock other writers
    goto check_tail;
  }

  int32 count = Atomic::Decrement32(&pipe.waitingList_);
  if (count >= 0) // at least one reader is waiting
    pipe.release(); // unlock one reader
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S, class Pipe> Pop1<T, _S, Pipe>::Pop1(Pipe &p) : PipeFunctor<Pipe>(p) {
}

template<typename T, uint32 _S, class Pipe> T Pop1<T, _S, Pipe>::operator ()() {

  int32 count = Atomic::Increment32(&pipe.waitingList_);
  if (count > 0) // no free lunch
    pipe.acquire(); // wait for a push

  if (pipe.head_ == (int32)_S - 1)
    pipe.shrink();

  T t = pipe.first_->buffer_[++pipe.head_];

  return t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S, class Pipe> PopN<T, _S, Pipe>::PopN(Pipe &p) : PipeFunctor<Pipe>(p), Semaphore(0, 1) {
}

template<typename T, uint32 _S, class Pipe> T PopN<T, _S, Pipe>::operator ()() {

  int32 count = Atomic::Increment32(&pipe.waitingList_);
  if (count > 0) // no free lunch
    pipe.acquire(); // wait for a push

check_head: int32 head = Atomic::Increment32(&pipe.head_);
  if (head < (int32)_S)
    return pipe.first_->buffer_[head];

  if (head == (int32)_S) {

    pipe.shrink(); // head set to -1

    release(); // unlock readers
    acquire(); // make sure the sem falls back to 0

    return pipe.first_->buffer_[++pipe.head_];
  } else { // head>_S: pipe.first_ and pipe.head_ are being changed

    acquire(); // wait
    release(); // unlock other readers
    goto check_head;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 S> Pipe11<T, S>::Pipe11() : Pipe<T, S, int32, int32, Pipe11<T, S>, Push1, Pop1>() {
}

template<typename T, uint32 S> Pipe11<T, S>::~Pipe11() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 S> Pipe1N<T, S>::Pipe1N() : Pipe<T, S, int32, int32 volatile, Pipe1N, Push1, PopN>() {
}

template<typename T, uint32 S> Pipe1N<T, S>::~Pipe1N() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 S> PipeN1<T, S>::PipeN1() : Pipe<T, S, int32 volatile, int32, PipeN1, PushN, Pop1>() {
}

template<typename T, uint32 S> PipeN1<T, S>::~PipeN1() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 S> PipeNN<T, S>::PipeNN() : Pipe<T, S, int32 volatile, int32 volatile, PipeNN, PushN, PopN>() {
}

template<typename T, uint32 S> PipeNN<T, S>::~PipeNN() {
}
#endif
}
