#pragma once
#include <memory>
#include <string>
#include <stdint.h>

#include <fc/time.hpp>

namespace TiValue {
    namespace client {
        namespace detail {
            class TivGntpNotifierImpl;
        }

        class TvGntpNotifier {
        public:
            TvGntpNotifier(const std::string& host_to_notify = "127.0.0.1", uint16_t port = 23053,
                const std::string& tiv_instance_identifier = "Tiv",
                const fc::optional<std::string>& password = fc::optional<std::string>());
            ~TvGntpNotifier();

            void client_is_shutting_down();
            void notify_connection_count_changed(uint32_t new_connection_count);
            void notify_client_exiting_unexpectedly();
            void notify_head_block_too_old(const fc::time_point_sec head_block_age);
        private:
            std::unique_ptr<detail::TivGntpNotifierImpl> my;
        };
        typedef std::shared_ptr<TvGntpNotifier> TvGntpNotifierPtr;

    }
} // end namespace TiValue::client
