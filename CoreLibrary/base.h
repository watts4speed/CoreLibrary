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

#ifndef core_base_h
#define core_base_h

//#define WITH_DETAIL_OID // Enable get_detail_oid() in every object.

#include <cstdlib>
#include <atomic>

#include "types.h"

namespace core {

class _Object;

// Smart pointer (ref counting, deallocates when ref count<=0).
// No circular refs (use std c++ ptrs).
// No passing in functions (cast P<C> into C*).
// Cannot be a value returned by a function (return C* instead).
template<class C> class P {
private:
  _Object *object_;
public:
  P();
  P(C *o);
  P(const P<C> &p);
  ~P();
  C *operator ->() const;
  template<class D> operator D *() const {

    return static_cast<D *>((C *)object_);
  }
  bool operator ==(C *c) const;
  bool operator !=(C *c) const;
  bool operator !() const;
  bool operator <(P<C> &p) const       { return (size_t)object_ < (size_t)p.object_; }
  bool operator <(const P<C> &p) const { return (size_t)object_ < (size_t)p.object_; }
  template<class D> bool operator ==(P<D> &p) const;
  template<class D> bool operator !=(P<D> &p) const;
  P<C> &operator =(C *c);
  P<C> &operator =(const  P<C> &p);
  template<class D> P<C> &operator =(const P<D> &p);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Root smart-pointable object class.
class core_dll _Object {
  template<class C> friend class P;
  friend class _P;
protected:
#ifdef ARCH_32
  uint32 __vfptr_padding_Object_;
#endif
  std::atomic_int32_t refCount_;
  _Object();
#ifdef WITH_DETAIL_OID
  uint64 detail_oid_;
#endif
public:
  virtual ~_Object();
  void incRef();
  virtual void decRef();
#ifdef WITH_DETAIL_OID
  uint64 get_detail_oid() const { return detail_oid_; }

  /**
   * Set this object's detail OID and also set the static last_detail_oid
   * so that the next detail OID will be higher than this one.
   * \param detail_oid The detail OID.
   */
  void set_detail_oid(uint64 detail_oid);
#endif
};

// Template version of the well-known DP. Adapts C to _Object.
template<class C> class _ObjectAdapter :
  public C,
  public _Object {
protected:
  _ObjectAdapter();
};
}


#include "base.tpl.cpp"


#endif
