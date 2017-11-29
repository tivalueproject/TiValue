#pragma once

#include <net/Node.hpp>
#include <wallet/Wallet.hpp>

#include <fc/filesystem.hpp>
#include <fc/network/ip.hpp>

#include <memory>

namespace TiValue {
    namespace rpc {
        namespace detail { class RpcClientImpl; }

        using namespace TiValue::blockchain;
        using namespace TiValue::wallet;

        typedef vector<std::pair<ShareType, string> > Balances;

        enum GenerateTransactionFlag
        {
            sign_and_broadcast = 0,
            do_not_broadcast = 1,
            do_not_sign = 2
        };

    }
} // TiValue::rpc
FC_REFLECT_ENUM(TiValue::rpc::GenerateTransactionFlag, (do_not_broadcast)(do_not_sign)(sign_and_broadcast))
