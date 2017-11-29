#include <contract_engine/glua_contract_engine.hpp>
#include <blockchain/Exceptions.hpp>

namespace glua
{
	GluaContractEngine::GluaContractEngine(bool use_contract)
	{
		_scope = std::make_shared<lua::lib::GluaStateScope>(use_contract);
	}
	GluaContractEngine::~GluaContractEngine()
	{

	}

	bool GluaContractEngine::has_gas_limit() const
	{
		return _scope->get_instructions_limit() >= 0;
	}
	int64_t GluaContractEngine::gas_limit() const {
		return _scope->get_instructions_limit();
	}
	int64_t GluaContractEngine::gas_used() const
	{
		return _scope->get_instructions_executed_count();
	}
	void GluaContractEngine::set_gas_limit(int64_t gas_limit)
	{
		_scope->set_instructions_limit(gas_limit);
	}
	void GluaContractEngine::set_no_gas_limit()
	{
		set_gas_limit(-1);
	}
	void GluaContractEngine::set_gas_used(int64_t gas_used)
	{
		int *insts_executed_count = lua::lib::get_lua_state_value(_scope->L(), INSTRUCTIONS_EXECUTED_COUNT_LUA_STATE_MAP_KEY).int_pointer_value;
		if (insts_executed_count)
		{
			*insts_executed_count = gas_used;
		}
		else
		{
			// 还没执行过vm指令，这时候不允许修改gas_used
		}
	}
	void GluaContractEngine::add_gas_used(int64_t delta_used)
	{
		set_gas_used(gas_used() + delta_used);
	}

	void GluaContractEngine::stop()
	{
		_scope->notify_stop();
	}

	// @throws glua::core::GluaException
	void GluaContractEngine::compilefile_to_stream(std::string filename, void *stream)
	{
		char err_msg[LUA_EXCEPTION_MULTILINE_STRNG_MAX_LENGTH] = "\0";
		if (!lua::lib::compilefile_to_stream(_scope->L(), filename.c_str(), (GluaModuleByteStream*)stream, err_msg, USE_TYPE_CHECK))
		{
			throw glua::core::GluaException(err_msg);
		}
	}

	void GluaContractEngine::add_global_bool_variable(std::string name, bool value)
	{
		_scope->add_global_bool_variable(name.c_str(), value);
	}
	void GluaContractEngine::add_global_int_variable(std::string name, int64_t value)
	{
		_scope->add_global_int_variable(name.c_str(), value);
	}
	void GluaContractEngine::add_global_string_variable(std::string name, std::string value)
	{
		_scope->add_global_string_variable(name.c_str(), value.c_str());
	}

	void GluaContractEngine::set_caller(std::string caller, std::string caller_address)
	{
		add_global_string_variable("caller", caller);
		add_global_string_variable("caller_address", caller_address);
	}

	void GluaContractEngine::set_state_pointer_value(std::string name, void *addr)
	{
		GluaStateValue statevalue;
		statevalue.pointer_value = addr;
		lua::lib::set_lua_state_value(_scope->L(), name.c_str(), statevalue, GluaStateValueType::LUA_STATE_VALUE_POINTER);
	}

	void GluaContractEngine::clear_exceptions()
	{
		lua::api::global_glua_chain_api->clear_exceptions(_scope->L());
	}

	void GluaContractEngine::execute_contract_api_by_address(std::string contract_id, std::string method, std::string argument, std::string *result_json_string)
	{
		lua::lib::execute_contract_api_by_address(_scope->L(), contract_id.c_str(), method.c_str(), argument.c_str(), result_json_string);
		if (_scope->L()->force_stopping == true && _scope->L()->exit_code == LUA_API_INTERNAL_ERROR)
			FC_CAPTURE_AND_THROW(TiValue::blockchain::lua_executor_internal_error, (""));
		int exception_code = lua::lib::get_lua_state_value(_scope->L(), "exception_code").int_value;
		char* exception_msg = (char*)lua::lib::get_lua_state_value(_scope->L(), "exception_msg").string_value;
		if (exception_code > 0)
		{
			if (exception_code == tichain_API_LVM_LIMIT_OVER_ERROR)
				FC_CAPTURE_AND_THROW(TiValue::blockchain::contract_run_out_of_money);
			else
			{
				FC_CAPTURE_AND_THROW(TiValue::blockchain::lua_executor_internal_error, (exception_msg));
			}
		}
	}

	void GluaContractEngine::execute_contract_init_by_address(std::string contract_id, std::string argument, std::string *result_json_string)
	{
		lua::lib::execute_contract_init_by_address(_scope->L(), contract_id.c_str(), argument.c_str(), result_json_string);
		if (_scope->L()->force_stopping == true && _scope->L()->exit_code == LUA_API_INTERNAL_ERROR)
			FC_CAPTURE_AND_THROW(TiValue::blockchain::lua_executor_internal_error, (""));
		int exception_code = lua::lib::get_lua_state_value(_scope->L(), "exception_code").int_value;
		char* exception_msg = (char*)lua::lib::get_lua_state_value(_scope->L(), "exception_msg").string_value;
		if (exception_code > 0)
		{
			if (exception_code == tichain_API_LVM_LIMIT_OVER_ERROR)
				FC_CAPTURE_AND_THROW(TiValue::blockchain::contract_run_out_of_money);
			else
			{
				FC_CAPTURE_AND_THROW(TiValue::blockchain::lua_executor_internal_error, (exception_msg));
			}
		}
	}

	void GluaContractEngine::load_and_run_stream(void *stream)
	{
		lua::lib::run_compiled_bytestream(_scope->L(), (GluaModuleByteStream*)stream);
	}

	std::shared_ptr<::blockchain::contract_engine::VMModuleByteStream> GluaContractEngine::get_bytestream_from_code(const TiValue::blockchain::Code& code)
	{
		auto code_stream = lua::api::global_glua_chain_api->get_bytestream_from_code(_scope->L(), code);
		return code_stream;
	}

}
