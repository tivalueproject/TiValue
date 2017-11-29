#include <fc/network/ntp.hpp>
#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>

int main( int argc, char** argv )
{
   fc::ntp ntp_service;
   ntp_service.set_request_interval(5);
   fc::usleep(fc::seconds(4) );
   auto time = ntp_service.get_time();
   if( time )
   {
      auto ntp_time = *time;
      auto delta = ntp_time - fc::time_point::now();
      auto minutes = delta.count() / 1000000 / 60;
      auto hours = delta.count() / 1000000 / 60 / 60;
      auto seconds = delta.count() / 1000000;
      auto msec= delta.count() / 1000;
      idump( (fc::time_point::now() ) );
      idump( (ntp_time)(delta)(msec)(seconds)(minutes)(hours) );
   }
   else
   {
      elog( "no response" );
   }

   return 0;
}
