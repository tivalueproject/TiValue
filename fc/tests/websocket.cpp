#include <fc/network/http/websocket.hpp>
#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>
#include <fc/rpc/websocket_api.hpp>

using namespace fc::http;

class echo_session : public fc::http::websocket_session 
{
   public:
     echo_session( const websocket_connection_ptr c ):fc::http::websocket_session(c){}
     void on_message( const std::string& message )
     {
        idump((message));
        if( message.size() < 64 )
           send_message( "echo " + message );
     }
};


int main( int argc, char** argv )
{ 
   try {
      auto create_session = [&]( const websocket_connection_ptr& c ){
                            return std::make_shared<echo_session>(c);
                            };
      fc::http::websocket_server server;
      server.on_connection(create_session);

      server.listen( 8090 );
      server.start_accept();

      fc::http::websocket_client client;
      auto session = client.connect( "ws://localhost:8090", create_session );
      wlog( "connected" );
      session->send_message( "hello world" );

      fc::usleep( fc::seconds(2) );
      return 0;
   } 
   /*
   catch ( const websocketpp::lib::error_code& e )
   {
      edump( (e.message()) );
   }
   */
    catch ( const fc::exception& e )
   {
      edump((e.to_detail_string()));
   }
}
