#include <api/GlobalApiLogger.hpp>
#include <client/ClientImpl.hpp>

#include <fc/io/json.hpp>
#include <fc/thread/scoped_lock.hpp>

#include <api/CommonApi.hpp>
#include <client/ApiLogger.hpp>
#include <fc/crypto/base58.hpp>
namespace TiValue {
	namespace client {
		namespace detail
		{
			TiValue::blockchain::FileSaveInfo ClientImpl::blockchain_get_file_save_node(const std::string& file_id)
			{

				FileIdType fileid(file_id);
				FileSaveInfo result;

				result.file_id = fileid;
				auto filesave=_chain_db->get_file_saved_entry(file_id);
				
				if (!filesave.valid())
					FC_CAPTURE_AND_THROW(file_not_exsited,(file_id));
				auto uploadrequest = _chain_db->get_upload_request(file_id);
				if(!uploadrequest.valid())
					FC_CAPTURE_AND_THROW(upload_request_not_exsited, (file_id));
				result.filename = uploadrequest->filename;
				result.c_id = filesave->file_id.file_id;
				auto& pieces=uploadrequest->pieces;
				for (auto& piece:pieces)
				{
					auto save_entry=_chain_db->get_piece_saved_entry(piece.pieceid);
					if(!save_entry.valid())
                        FC_CAPTURE_AND_THROW(file_piece_missing, (piece.pieceid));
					PieceSaveInfo info;
					info.piece_id = piece.pieceid;
					info.nodes = save_entry->storageNode;
					info.size = piece.piece_size;
					info.filename = uploadrequest->filename;
					result.pieces.push_back(info);
				}
				return result;
			}
			vector<FilePieceInfo> ClientImpl::wallet_get_my_store_rejected()
			{
				return _wallet->get_my_store_rejected();
			}
			std::vector<string> ClientImpl::wallet_get_my_store_confirmed()
			{
				return _wallet->get_my_store_confirmed();
			}
			std::vector<LocalStoreRequestInfo> ClientImpl::wallet_get_my_store_request()
			{
				 auto store_req_res=  _wallet->get_local_store_requests();
				 auto upload_req_res = _chain_db->list_upload_requests();
				 for (auto& sit : store_req_res)
				 {
					 for (auto uit : upload_req_res)
					 {
						 if (uit.id == sit.file_id)
						 {
							 for (auto piece_it : uit.pieces)
							 {
								 if (piece_it.pieceid == sit.piece_id)
								 {
									 sit.piece_size = piece_it.piece_size;
									 break;
								 }
							 }
						 }
					 } 
				 }
				 return store_req_res;
			}	
			std::vector<TiValue::blockchain::FileAccessInfo> ClientImpl::wallet_get_my_access()
			{
				return _wallet->get_my_access();
			}
			std::vector<TiValue::blockchain::UploadRequestEntry> ClientImpl::wallet_get_my_upload_requests()
			{
				return _wallet->get_my_upload_requests();
			}
			std::vector<TiValue::blockchain::StoreRequestInfo> ClientImpl::wallet_list_store_request_for_my_file(const std::string& file_id)
			{
				return _wallet->list_store_request_for_my_file(file_id);
			}
			std::string ClientImpl::blockchain_get_file_authorizing_contract(const std::string& file_id)
			{
				FileIdType id(file_id);
				auto entry=_chain_db->get_upload_request(id);
				if (!entry.valid())
					FC_CAPTURE_AND_THROW(upload_request_not_exsited,(file_id));
				return entry->authenticating_contract.AddressToString(AddressType::contract_address);
			}
			std::vector<std::string> ClientImpl::blockchain_list_file_saved()
			{
				vector<std::string> res;
				auto file_ids=_chain_db->get_file_saved();
				for (auto file_id : file_ids)
					res.push_back(file_id);
				return res;
			}
			void ClientImpl::wallet_set_node_id(const std::string& node_id)
			{
				_wallet->set_node_id(node_id);
			}
			TiValue::blockchain::UploadRequestEntry ClientImpl::store_file_to_network(const std::string& owner, const std::string& AuthorizatingContractId, 
				const TiValue::blockchain::FilePath& filename, uint32_t filesize, const std::string& description,
				const std::string& piecesinfo, const std::string& asset_symbol, double price, uint32_t numofcopy,
				uint32_t numofpiece, uint32_t payterm, const std::string& node_id, double exec_limit)
			{
				ContractIdType cid(AuthorizatingContractId,AddressType::contract_address);
				auto contract_entry=_chain_db->get_contract_entry(cid);
				if (!contract_entry.valid())
					FC_CAPTURE_AND_THROW(authorazing_contract_not_exsited,(cid));
				if(contract_entry->code.abi.count(TIVALUE_GETACCESS_CONTRACT_INTERFACE)<1)
					FC_CAPTURE_AND_THROW(invalid_authorazing_contract, (cid));
				if (asset_symbol != TIV_BLOCKCHAIN_SYMBOL)
					FC_CAPTURE_AND_THROW(invalid_asset_symbol,(asset_symbol));
				if (exec_limit <= 0)
					FC_CAPTURE_AND_THROW(zero_amount, (exec_limit));
				auto res= _wallet->store_file_to_network(owner, AuthorizatingContractId, filename, filesize, description, piecesinfo, asset_symbol, price, numofcopy
				, numofpiece, payterm, node_id, exec_limit);
				_wallet->cache_transaction(res.second, false);
				network_broadcast_transaction(res.second.trx);
				return res.first;
			}
			TiValue::wallet::WalletTransactionEntry ClientImpl::store_reject(const std::string& file_id, const std::string& file_piece_id, const std::string& node_id,double exec_limit)
			{
				auto entry= _wallet->store_reject(file_id, file_piece_id, node_id, exec_limit);
				_wallet->cache_transaction(entry, false);
				network_broadcast_transaction(entry.trx);
				return entry;
			}
			TiValue::wallet::WalletTransactionEntry ClientImpl::get_file_access(const std::string& requester, const std::string& file_id, double exec_limit)
			{
				auto entry = _wallet->get_file_access(requester, file_id, exec_limit);
				_wallet->cache_transaction(entry, false);
				network_broadcast_transaction(entry.trx);
				return entry;
			}
			TiValue::wallet::WalletTransactionEntry  ClientImpl::store_file_piece(const std::string& requester, const std::string& file_id, const std::string& file_piece_id, const std::string& node_id, double exec_limit)
			{
				auto entry = _wallet->store_file_piece(requester, file_id, file_piece_id,node_id, exec_limit);
				_wallet->cache_transaction(entry, false);
				network_broadcast_transaction(entry.trx);
				return entry;
			}
			bool ClientImpl::blockchain_check_signature(const std::string& origin_data, const std::string& signature, const std::string& key)
			{
				PublicKeyType pkey(key);
				//fc::ecc::compact_signature sig=;
				//fc::sha256 ori(origin_data);
				//
				//	fc::ecc::public_key(sig, trx_digest, _enforce_canonical_signatures).serialize();
				return true;
			}
			std::vector<TiValue::blockchain::UploadRequestEntry> ClientImpl::blockchain_get__upload_requests()
			{
				return _chain_db->list_upload_requests();
			}
			TiValue::wallet::WalletTransactionEntry ClientImpl::confirm_piece_saved(const std::string& confirmer, const std::string& file_id, const std::string& file_piece_id, const std::string& Storage, double exec_limit)
			{
				auto entry = _wallet->confirm_piece_saved(confirmer, file_id,file_piece_id,Storage,  exec_limit);
				_wallet->cache_transaction(entry, false);
				network_broadcast_transaction(entry.trx);
				return entry;
			}
			bool inline check_upload_request(oUploadRequestEntry entry, string piece_id)
			{
				if (!entry.valid())
					return false;
				for (auto piece : entry->pieces)
				{
					if (piece.pieceid == piece_id)
					{
						return true;
					}
				}
				return false;
			}
            void ClientImpl::wallet_allow_store_request(const std::string& file_id, const std::string& piece_id, const std::string& storer)
            {
                _wallet->allow_store(file_id,piece_id,storer);
            }

			bool ClientImpl::download_validation(const std::string& file_id, const std::string& authentication)
			{
				string fid = file_id;
				bool got_piece = false;
				int copy = 0;
				auto reqs=_chain_db->list_upload_requests();
				for (auto req : reqs)
				{
					for (auto it : req.pieces)
					{
						if (fid == it.pieceid)
						{
							fid = req.id;
							got_piece = true;
							copy = req.num_of_copys;
							break;
						}
					}
					if (got_piece)
						break;
				}
				
				auto au_info=fc::from_base58(authentication);
				fc::ecc::compact_signature sig;
				for (int i = 0; i < au_info.size(); i++)
				{
					sig.at(i) = au_info[i];
				}
				auto digest = sha256::hash(fid);
				auto key = fc::ecc::public_key(sig, digest, false).serialize();

				PublicKeyType pkey(key);
				printf("file_id=%s\n,au=%s\n,key=", fid.c_str(), authentication.c_str(), pkey.operator fc::string().c_str());
					auto files = _chain_db->get_file_saved();
					for (auto file : files)
					{
						if (file == fid)
						{
							auto access_enable = _chain_db->get_enable_access_entry(file);
							if (!access_enable.valid())
								continue;
							for (auto fe : access_enable->fetcher)
								printf("%s\n",fe.operator fc::string().c_str());
							if (access_enable->fetcher.find(pkey) != access_enable->fetcher.end())
								return true;
						}
						/*
						else if (check_upload_request(_chain_db->get_upload_request(file), file_id))
						{
							auto req = _chain_db->get_store_request(file_id);
							if (!req.valid())
								continue;
							auto it = req->store_request.begin();
							while (it != req->store_request.end())
							{
								if (it->second == pkey)
									return true;
								it++;
							}
						}
						*/
					}
					//校验是否时存储请求
					auto reqs_for_my_file=_wallet->list_store_request_for_my_file(fid);
                    
					auto sd_entry=_chain_db->get_save_decl_entry(file_id);
					if (sd_entry.valid())
					{
						for (auto& it : sd_entry->store_info)
						{
							if (it.file_id == fid)
								copy--;
						}
						if (copy <= 0)
							return false;
					}
					for (auto req_for_my_file : reqs_for_my_file)
					{
						
						if (req_for_my_file.piece_id == file_id)
						{
							for (auto reqer : req_for_my_file.requestors)
							{
                                if (reqer.key == pkey)
                                {
                                    if(_wallet->check_store_allowed( fid,file_id, pkey))
                                        return true;
                                    return false;
                                }
							}
						}
					}
					return false;
			}
			std::string ClientImpl::generate_download_validation(const std::string& file_id)
			{
				auto accesss=_wallet->get_my_access();
				for (auto accessinfo : accesss)
				{
					for (auto file: accessinfo.file_id)
					{
						if (file != file_id&&!check_upload_request(_chain_db->get_upload_request(file), file_id))
							continue;
						auto entry = _wallet->get_account(accessinfo.account);
						auto private_key=_wallet->get_private_key(entry.owner_address());
						auto sig = private_key.sign_compact(sha256::hash(file));
						return fc::to_base58((char*)&(sig.data), sig.size());
					}
				}
				auto storerequestss = _wallet->get_local_store_requests();

				auto storerequests = _wallet->get_my_store_requests();
				for (auto req = storerequests.begin(); req != storerequests.end(); req++)
				{
					if (req->second.find(file_id)!=req->second.end())
					{
						auto store_req = _chain_db->get_store_request(req->first);
						if (!store_req.valid())
							continue;
						auto req_it = store_req->store_request.begin();
						while (req_it != store_req->store_request.end())
						{
							if (_wallet->is_my_public_key(req_it->second))
							{
								auto private_key = _wallet->get_private_key(Address(req_it->second));
								auto sig = private_key.sign_compact(sha256::hash(file_id));
								string res;
								return fc::to_base58((char*)&(sig.data), sig.size());
							}
							req_it++;
						}
					}
				}
				FC_CAPTURE_AND_THROW(access_unauthorized,(file_id));
			}
			TiValue::wallet::WalletTransactionEntry ClientImpl::declare_piece_saved(const std::string& file_id, const std::string& piece_id, const std::string& storer)
			{
				auto sr_entry=_chain_db->get_store_request_entry(piece_id);
				auto acc_entry=_wallet->get_account(storer);
				if (!sr_entry.valid())
					FC_CAPTURE_AND_THROW(store_request_not_exsited,(piece_id)(storer));
				bool found = false;
				for (auto reqit = sr_entry->store_request.begin(); reqit != sr_entry->store_request.end(); reqit++)
				{
					if (acc_entry.owner_key == reqit->second)
					{
						found = true;
						break;
					}	
				}
				if (!found)
					FC_CAPTURE_AND_THROW(store_request_not_exsited, (piece_id)(storer));
				WalletTransactionEntry entry = _wallet->declare_piece_saved(file_id,piece_id,storer);
				_wallet->cache_transaction(entry, false);
				network_broadcast_transaction(entry.trx);
				return entry;
			}
			std::set<TiValue::blockchain::PieceStoreInfo> ClientImpl::blockchain_list_file_save_declare(const std::string& file_id)
			{
				std::set<TiValue::blockchain::PieceStoreInfo> res;
				FileIdType fid(file_id);
				auto upload_entry = _chain_db->get_upload_request(fid);
				if (!upload_entry.valid())
					FC_CAPTURE_AND_THROW(upload_request_not_exsited);
				for (auto& piece_entry : upload_entry->pieces)
				{
					auto decl_entry=_chain_db->get_save_decl_entry(piece_entry.pieceid);
					if (!decl_entry.valid())
						continue;
					for (auto entry : decl_entry->store_info)
					{
						if (fid == entry.file_id)
							res.insert(entry);
					}
				}
				return res;
			}
		}
	}
} 
