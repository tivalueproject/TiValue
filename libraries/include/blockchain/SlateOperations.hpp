#pragma once

#include <blockchain/Operations.hpp>

namespace TiValue {
    namespace blockchain {

        struct DefineSlateOperation
        {
            static const OperationTypeEnum type;

            vector<signed_int> slate;

            void evaluate(TransactionEvaluationState& eval_state)const;
        };

    }
} // TiValue::blockchain

FC_REFLECT(TiValue::blockchain::DefineSlateOperation, (slate))
