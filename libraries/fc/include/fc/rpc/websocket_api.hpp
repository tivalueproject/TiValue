#pragma once
#include <fc/rpc/api_connection.hpp>
#include <fc/rpc/state.hpp>
#include <fc/network/http/websocket.hpp>
#include <fc/io/json.hpp>
#include <fc/reflect/variant.hpp>

namespace fc { namespace rpc {

   class websocket_api_connection : public api_connection
   {
      public:
         ~websocket_api_connection()
         {
         }

         websocket_api_connection( fc::http::websocket_connection& c )
         :_connection(c)
         {
            _rpc_state.add_method( "call", [this]( const variants& args ) -> variant {
                      FC_ASSERT( args.size() == 3 && args[2].is_array() );
                      return this->receive_call( args[0].as_uint64(),
                                                         args[1].as_string(),
                                                         args[2].get_array() );
                                  });

            _rpc_state.add_method( "notice", [this]( const variants& args ) -> variant {
                      FC_ASSERT( args.size() == 2 && args[1].is_array() );
                      this->receive_notice( args[0].as_uint64(), args[1].get_array() );
                      return variant();
                                  });

            _rpc_state.add_method( "callback", [this]( const variants& args ) -> variant {
                      FC_ASSERT( args.size() == 2 && args[1].is_array() );
                      this->receive_callback( args[0].as_uint64(), args[1].get_array() );
                      return variant();
                                  });

            _rpc_state.on_unhandled( [&]( const std::string& method_name, const variants& args ){
                       return this->receive_call( 0, method_name, args );
                                   }); 

            _connection.on_message_handler( [&]( const std::string& msg ){ on_message(msg,true); } );
            _connection.on_http_handler( [&]( const std::string& msg ){ return on_message(msg,false); } );
            _connection.closed.connect( [this](){ closed(); } );
         }

         virtual variant send_call( api_id_type api_id, 
                                    string method_name, 
                                    variants args = variants() ) override
         {
            auto request = _rpc_state.start_remote_call(  "call", {api_id, std::move(method_name), std::move(args) } );
            _connection.send_message( fc::json::to_string(request) );
            return _rpc_state.wait_for_response( *request.id );
         }
         virtual variant send_callback( uint64_t callback_id, variants args = variants() ) override
         {
            auto request = _rpc_state.start_remote_call( "callback", {callback_id, std::move(args) } );
            _connection.send_message( fc::json::to_string(request) );
            return _rpc_state.wait_for_response( *request.id );
         }
         virtual void send_notice( uint64_t callback_id, variants args = variants() ) override
         {
            fc::rpc::request req{ optional<uint64_t>(), "notice", {callback_id, std::move(args)}};  
            _connection.send_message( fc::json::to_string(req) );
         }


      protected:
         std::string on_message( const std::string& message, bool send_message = true )
         { 
            try {
               auto var = fc::json::from_string(message);
               const auto& var_obj = var.get_object();
               if( var_obj.contains( "method" ) )
               {
                  auto call = var.as<fc::rpc::request>();
                  try {
                     auto result = _rpc_state.local_call( call.method, call.params );
                     if( call.id )
                     {
                        auto reply = fc::json::to_string( response( *call.id, result ) );
                        if( send_message )
                           _connection.send_message( reply );
                        return reply;
                     }
                  }
                  catch ( const fc::exception& e )
                  {
                     if( call.id )
                     {
                        auto reply = fc::json::to_string( response( *call.id,  error_object{ 1, e.to_detail_string(), fc::variant(e)}  ) );
                        if( send_message )
                           _connection.send_message( reply );

                        return reply;
                     }
                  }
               }
               else 
               {
                  auto reply = var.as<fc::rpc::response>();
                  _rpc_state.handle_reply( reply );
               }
            } catch ( const fc::exception& e ) {
               wdump((e.to_detail_string()));
               return e.to_detail_string();
            }
            return string();
         }
         fc::http::websocket_connection&  _connection;
         fc::rpc::state                   _rpc_state;
   };

} } // namespace fc::rpc
