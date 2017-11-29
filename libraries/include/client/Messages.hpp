#pragma once
#include <blockchain/Block.hpp>
#include <client/Client.hpp>

namespace TiValue {
    namespace client {

        enum MessageTypeEnum
        {
            trx_message_type = 1000,
            block_message_type = 1001,
            batch_trx_message_type = 1002
        };

        struct TrxMessage
        {
            static const MessageTypeEnum type;

            TiValue::blockchain::SignedTransaction trx;
            TrxMessage() {}
            TrxMessage(TiValue::blockchain::SignedTransaction transaction) :
                trx(std::move(transaction))
            {}
        };

        struct BatchTrxMessage
        {
            static const MessageTypeEnum type;
            std::vector<TiValue::blockchain::SignedTransaction> trx_vec;
            BatchTrxMessage() {}
            BatchTrxMessage(std::vector<TiValue::blockchain::SignedTransaction> transactions) :
                trx_vec(std::move(transactions))
            {}
        };

        struct BlockMessage
        {
            static const MessageTypeEnum type;

            BlockMessage(){}
            BlockMessage(const TiValue::blockchain::FullBlock& blk)
                :block(blk), block_id(blk.id()){}

            TiValue::blockchain::FullBlock    block;
            TiValue::blockchain::BlockIdType block_id;

        };

    }
} // TiValue::client

FC_REFLECT_ENUM(TiValue::client::MessageTypeEnum, (trx_message_type)(block_message_type)(batch_trx_message_type))
FC_REFLECT(TiValue::client::TrxMessage, (trx))
FC_REFLECT(TiValue::client::BatchTrxMessage, (trx_vec))
FC_REFLECT(TiValue::client::BlockMessage, (block)(block_id))
