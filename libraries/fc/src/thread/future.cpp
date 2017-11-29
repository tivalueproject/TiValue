#include <fc/thread/future.hpp>
#include <fc/thread/spin_yield_lock.hpp>
#include <fc/thread/thread.hpp>
#include <fc/thread/unique_lock.hpp>
#include <fc/exception/exception.hpp>

#include <boost/assert.hpp>


namespace fc {

  promise_base::promise_base( const char* desc )
  :_ready(false),
   _blocked_thread(nullptr),
   _blocked_fiber_count(0),
   _timeout(time_point::maximum()),
   _canceled(false),
#ifndef NDEBUG
   _cancellation_reason(nullptr),
#endif
   _desc(desc),
   _compl(nullptr)
  { }

  const char* promise_base::get_desc()const{
    return _desc; 
  }
               
  void promise_base::cancel(const char* reason /* = nullptr */){
//      wlog("${desc} canceled!", ("desc", _desc? _desc : ""));
      _canceled = true;
#ifndef NDEBUG
      _cancellation_reason = reason;
#endif
    }
  bool promise_base::ready()const {
    return _ready;
  }
  bool promise_base::error()const {
    { synchronized(_spin_yield) 
      return _exceptp != nullptr;
    }
  }

  void promise_base::set_exception( const fc::exception_ptr& e ){
    _exceptp = e;
    _set_value(nullptr);
  }

  void promise_base::_wait( const microseconds& timeout_us ){
     if( timeout_us == microseconds::maximum() ) 
       _wait_until( time_point::maximum() );
     else 
       _wait_until( time_point::now() + timeout_us );
  }
  void promise_base::_wait_until( const time_point& timeout_us ){
    { synchronized(_spin_yield) 
      if( _ready ) {
        if( _exceptp ) 
          _exceptp->dynamic_rethrow_exception();
        return;
      }
      _enqueue_thread();
    }
    try
    {
      thread::current().wait_until( ptr(this,true), timeout_us );
    }
    catch (...)
    {
      _dequeue_thread();
      throw;
    }
    _dequeue_thread();
    if( _ready ) 
    {
       if( _exceptp ) 
         _exceptp->dynamic_rethrow_exception();
       return; 
    }
    FC_THROW_EXCEPTION( timeout_exception, "" );
  }
  void promise_base::_enqueue_thread(){
     ++_blocked_fiber_count;
     // only one thread can wait on a promise at any given time
     assert(!_blocked_thread ||
            _blocked_thread == &thread::current());
     _blocked_thread = &thread::current();
  }
  void promise_base::_dequeue_thread(){ 
    synchronized(_spin_yield)
    if (!--_blocked_fiber_count)
      _blocked_thread = nullptr;
  }
  void promise_base::_notify(){
    // copy _blocked_thread into a local so that if the thread unblocks (e.g., 
    // because of a timeout) before we get a chance to notify it, we won't be
    // calling notify on a null pointer
    thread* blocked_thread;
    { synchronized(_spin_yield)
      blocked_thread = _blocked_thread;
    }
    if( blocked_thread ) 
      blocked_thread->notify(ptr(this,true));
  }
  promise_base::~promise_base() { }
  void promise_base::_set_timeout(){
    if( _ready ) 
      return;
    set_exception( std::make_shared<fc::timeout_exception>() );
  }
  void promise_base::_set_value(const void* s){
 //   slog( "%p == %d", &_ready, int(_ready));
//    BOOST_ASSERT( !_ready );
    { synchronized(_spin_yield) 
      if (_ready) //don't allow promise to be set more than once
        return;
      _ready = true;
    }
    _notify();
    if( nullptr != _compl ) {
      _compl->on_complete(s,_exceptp);
    }
  }
  void promise_base::_on_complete( detail::completion_handler* c ) {
    { synchronized(_spin_yield) 
        delete _compl; 
        _compl = c;
    }
  }
}

