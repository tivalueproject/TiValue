#pragma once

#define TIV_WALLET_VERSION                                  uint32_t( 109 )

#define TIV_WALLET_MIN_PASSWORD_LENGTH                      8
#define TIV_WALLET_MIN_BRAINKEY_LENGTH                      32

#define TIV_WALLET_DEFAULT_UNLOCK_TIME_SEC                  ( 60 * 60 )

#define TIV_WALLET_DEFAULT_TRANSACTION_FEE                  1000 // XTS

#define TIV_WALLET_DEFAULT_TRANSACTION_FEE_RATE             0.002
#define TIV_WALLET_LEAST_TRANSACTION_FEE                    10000

#define TIV_WALLET_DEFAULT_TRANSACTION_EXPIRATION_SEC       ( 60 * 60 )

#define WALLET_DEFAULT_MARKET_TRANSACTION_EXPIRATION_SEC    ( 60 * 10 )
