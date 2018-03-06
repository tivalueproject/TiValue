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
			std::vector<TiValue::blockchain::UploadRequestEntry> ClientImpl::wallet_get_my_upload_requests()
			{
				return _wallet->get_my_upload_requests();
			}
      //added on 02/08/2018
      std::vector<TiValue::blockchain::HaveAppliedFileEntry> ClientImpl::wallet_list_my_declared_file(const std::string& account)
      {
        std::vector<HaveAppliedFileEntry> res;
        PublicKeyType public_key = _wallet->get_owner_public_key(account);
        std::vector<TiValue::blockchain::UploadRequestEntry> upload_reqeusts = _chain_db->list_upload_requests();

        for (auto itr = upload_reqeusts.begin(); itr != upload_reqeusts.end(); itr++) {
          if (public_key != itr->id.uploader) {
            HaveAppliedFileEntry applied_entry;
            applied_entry.file_id = itr->id;
            applied_entry.piece = itr->pieces[0];
            applied_entry.description = itr->description;
            applied_entry.file_name = itr->filename;
            applied_entry.is_confirmed = false;
            applied_entry.node_id = itr->node_id;
            applied_entry.num_of_copy = itr->num_of_copys;
            applied_entry.num_of_confirmed = 0;
            applied_entry.num_of_declared = 0;
            oPieceSavedDeclEntry o_decl_entry = _chain_db->get_save_decl_entry(itr->pieces[0].pieceid);
            oPieceSavedEntry o_piece_saved_entry = _chain_db->get_piece_saved_entry(itr->pieces[0].pieceid);
            PieceSavedEntry piece_saved_entry;
            if (o_piece_saved_entry.valid()) {
              piece_saved_entry = *o_piece_saved_entry;
            }
            if (o_decl_entry.valid()) {
              PieceSavedDeclEntry piece_save_decl_entry = *o_decl_entry;
              set<PieceStoreInfo> store_info = piece_save_decl_entry.store_info;
              PieceStoreInfo temp_info;
              temp_info.file_id = itr->id;
              temp_info.piece_id = itr->pieces[0].pieceid;
              auto search = store_info.find(temp_info);
              if (search != store_info.end()) {
                
                applied_entry.num_of_declared = search->nodes.size();
                for (auto itr2 = search->nodes.begin(); itr2 != search->nodes.end(); itr2++) {
                  if (itr2->key == public_key) {
                    if (piece_saved_entry.storageNode.find(itr2->node) != piece_saved_entry.storageNode.end()) {
                      applied_entry.is_confirmed = true;
                      applied_entry.num_of_confirmed++;
                    }
                    res.push_back(applied_entry);
                  }
                }
              }
            }

          }     
        }
        return res;
      }
      //added on 02/07/2018
      std::vector<TiValue::blockchain::UploadRequestEntryPlus> ClientImpl::wallet_list_my_upload_requests(const std::string& account) {
        std::vector<TiValue::blockchain::UploadRequestEntryPlus> res;
        std::vector<TiValue::blockchain::UploadRequestEntry> upload_reqeusts = _chain_db->list_upload_requests();
        PublicKeyType public_key = _wallet->get_owner_public_key(account);
        for (auto itr = upload_reqeusts.begin(); itr != upload_reqeusts.end(); itr++) {
          if (public_key == itr->id.uploader)  {
            UploadRequestEntryPlus upload_plus;
            upload_plus.file_id          = itr->id;
            upload_plus.piece            = itr->pieces[0];   
            upload_plus.node_id          = itr->node_id;
            upload_plus.file_name        = itr->filename;
            upload_plus.description      = itr->description;
            upload_plus.num_of_copy      = itr->num_of_copys;
            upload_plus.num_of_declared  = 0;
            upload_plus.num_of_confirmed = 0;

            oPieceSavedDeclEntry o_save_decl_entry = _chain_db->get_save_decl_entry(itr->pieces[0].pieceid);      
            oPieceSavedEntry o_piece_saved_entry = _chain_db->get_piece_saved_entry(itr->pieces[0].pieceid);
            PieceSavedEntry piece_saved_entry;
            if (o_piece_saved_entry.valid()) {
              piece_saved_entry = *o_piece_saved_entry;
            }
            
            if (o_save_decl_entry.valid()) {
              PieceSavedDeclEntry piece_save_decl_entry = *o_save_decl_entry;
              set<PieceStoreInfo> store_info = piece_save_decl_entry.store_info;
              PieceStoreInfo temp_info;
              temp_info.file_id = itr->id;
              temp_info.piece_id = itr->pieces[0].pieceid;
              auto search = store_info.find(temp_info);
              if (search != store_info.end()) {
                upload_plus.num_of_declared = search->nodes.size();
                for (auto itr2 = search->nodes.begin(); itr2 != search->nodes.end(); itr2++) {
                  if (piece_saved_entry.storageNode.find(itr2->node) != piece_saved_entry.storageNode.end()) {
                    upload_plus.storers.push_back(StoreNodeInfoPlus(itr2->node, itr2->key, true));
                    upload_plus.num_of_confirmed++;
                  } else {
                    upload_plus.storers.push_back(StoreNodeInfoPlus(itr2->node, itr2->key, false));
                  }
                }
              }
            }
            res.push_back(upload_plus);
          }
        }       
        return res;
      }

			
      std::vector<std::string> ClientImpl::blockchain_list_file_saved()
			{
				vector<std::string> res;
				//auto file_ids=_chain_db->get_file_saved();
				//for (auto file_id : file_ids)
				//	res.push_back(file_id);
        auto entries = _chain_db->get_file_saved();
        for (auto entry : entries) {
          res.push_back(std::string(entry.file_id) + "," + std::string(entry.piece_id));
        }
				return res;
			}
			
      void ClientImpl::wallet_set_node_id(const std::string& node_id)
			{
				_wallet->set_node_id(node_id);
			}
			
      TiValue::blockchain::UploadRequestEntry ClientImpl::store_file_to_network(const std::string& owner, const TiValue::blockchain::FilePath& filename, uint32_t filesize,  const std::string& description,const std::string& piecesinfo, const std::string& asset_symbol, double price, uint32_t numofcopy, uint32_t numofpiece, uint32_t payterm, const std::string& node_id, double exec_limit)
			{
				//ContractIdType cid(AuthorizatingContractId, AddressType::contract_address);
				//auto contract_entry = _chain_db->get_contract_entry(cid);
				//if (!contract_entry.valid())
				//	FC_CAPTURE_AND_THROW(authorazing_contract_not_exsited, (cid));
				//if (contract_entry->code.abi.count(TIVALUE_GETACCESS_CONTRACT_INTERFACE) < 1)
				//	FC_CAPTURE_AND_THROW(invalid_authorazing_contract, (cid));
				if (asset_symbol != TIV_BLOCKCHAIN_SYMBOL)
					FC_CAPTURE_AND_THROW(invalid_asset_symbol,(asset_symbol));
				if (exec_limit <= 0)
					FC_CAPTURE_AND_THROW(zero_amount, (exec_limit));
				auto res= _wallet->store_file_to_network(owner, filename, filesize, description, piecesinfo, asset_symbol, price, numofcopy, numofpiece, payterm, node_id, exec_limit);
				_wallet->cache_transaction(res.second, false);
				network_broadcast_transaction(res.second.trx);
				return res.first;
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
			
      std::vector<TiValue::blockchain::UploadRequestEntry> ClientImpl::blockchain_get_upload_requests()
			{
				return _chain_db->list_upload_requests();
			}

      //added on 02/08/2018
      std::vector<TiValue::blockchain::UploadRequestEntry> ClientImpl::blockchain_list_file_saved_info() const {
        std::vector<TiValue::blockchain::UploadRequestEntry> upload_requests = _chain_db->list_upload_requests();
        std::vector<TiValue::blockchain::UploadRequestEntry> res;
        for (std::vector<UploadRequestEntry>::iterator itr = upload_requests.begin(); itr != upload_requests.end(); itr++) {
          oPieceSavedEntry o_piece_saved_entry = _chain_db->get_piece_saved_entry(itr->pieces[0].pieceid);
          if (o_piece_saved_entry.valid()) {
            res.push_back(*itr);
          }
        }
        return res;
      }

      //added on 02/08/2018
      std::vector<TiValue::blockchain::CanApplyEntry> ClientImpl::blockchain_list_can_apply_file() const
      {
        std::vector<TiValue::blockchain::UploadRequestEntry> upload_requests = _chain_db->list_upload_requests();
        std::vector<TiValue::blockchain::CanApplyEntry> res;
        for (std::vector<UploadRequestEntry>::iterator itr = upload_requests.begin(); itr != upload_requests.end(); itr++) {
          CanApplyEntry apply_entry;
          apply_entry.file_id          = itr->id;
          apply_entry.file_name        = itr->filename;
          apply_entry.description      = itr->description;
          apply_entry.node_id          = itr->node_id;
          apply_entry.num_of_copy      = itr->num_of_copys;
          apply_entry.piece            = itr->pieces[0];
          apply_entry.num_of_confirmed = 0;
          size_t num_of_confirmed = 0;
          oPieceSavedEntry o_piece_saved_entry = _chain_db->get_piece_saved_entry(itr->pieces[0].pieceid);  
          if (o_piece_saved_entry.valid()) {
            PieceSavedEntry piece_saved_entry = *o_piece_saved_entry;
            size_t num_of_confirmed = piece_saved_entry.storageNode.size();
            apply_entry.num_of_confirmed = num_of_confirmed;
          }
          if (apply_entry.num_of_confirmed < apply_entry.num_of_copy) {
            res.push_back(apply_entry);
          }
        }
        return res;
      }

			TiValue::wallet::WalletTransactionEntry ClientImpl::confirm_piece_saved(const std::string& confirmer, const std::string& file_id, const std::string& file_piece_id, const std::string& Storage, double exec_limit)
			{
				auto entry = _wallet->confirm_piece_saved(confirmer, file_id, file_piece_id, Storage, exec_limit);
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
      
      TiValue::wallet::WalletTransactionEntry ClientImpl::declare_piece_saved(const std::string& file_id, const std::string& piece_id, const std::string& storer, const std::string& node_id)
			{
				//auto sr_entry=_chain_db->get_store_request_entry(piece_id);
        oUploadRequestEntry sr_entry = _chain_db->get_upload_request(file_id);
				//auto acc_entry=_wallet->get_account(storer);
				//if (!sr_entry.valid())
				//	FC_CAPTURE_AND_THROW(store_request_not_exsited,(piece_id)(storer));
        if (!sr_entry.valid()) {
          FC_CAPTURE_AND_THROW(upload_request_not_exsited, (file_id)(storer));
        }
				bool found = false;
				//for (auto reqit = sr_entry->store_request.begin(); reqit != sr_entry->store_request.end(); reqit++)
				//{
				//	if (acc_entry.owner_key == reqit->second)
				//	{
				//		found = true;
				//		break;
				//	}	
				//}
				//if (!found)
				//	FC_CAPTURE_AND_THROW(store_request_not_exsited, (piece_id)(storer));
        if (sr_entry->pieces[0].pieceid == piece_id) {
          found = true;
        } else {
          FC_CAPTURE_AND_THROW(upload_request_not_exsited, (piece_id)(storer));
        }

				WalletTransactionEntry entry = _wallet->declare_piece_saved(file_id, piece_id, storer, node_id);
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
					auto decl_entry = _chain_db->get_save_decl_entry(piece_entry.pieceid);
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
