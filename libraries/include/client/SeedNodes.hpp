#pragma once

namespace TiValue {
    namespace client {

#ifdef TIV_TEST_NETWORK
#ifndef YQR_TEST
        static const std::vector<std::string> SeedNodes
        {
			"47.94.109.209:64696",
			"101.200.53.4:64696",
			"39.108.8.87:64696",
			"106.14.172.170:64696"
        };
#else 
		static const std::vector<std::string> SeedNodes
		{
			"116.62.224.237:64696",
			"116.62.228.10:64696",
			"116.62.224.190:64696",
		};
#endif
#elif TIV_TEST_NETWORK2
      static const std::vector<std::string> SeedNodes
      {
        "47.94.109.209:64696",
        "101.200.53.4:64696",
        "39.108.8.87:64696",
        "106.14.172.170:64696"
    };
#else
        static const std::vector<std::string> SeedNodes
        {
			//"47.94.109.209:63696",
			//"101.200.53.4:63696",
			//"39.108.8.87:63696",
			//"106.14.172.170:63696"
          "47.104.199.127:63696",
          "47.96.157.181:63696",
          "39.104.92.249:63696",
          "106.14.172.170:63696"
        };
#endif

    }
} // TiValue::client
