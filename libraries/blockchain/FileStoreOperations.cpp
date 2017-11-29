#include <blockchain/FileStoreOperations.hpp>
#include <blockchain/ChainInterface.hpp>
#include <blockchain/TransactionEvaluationState.hpp>
#include <blockchain/Exceptions.hpp>
namespace TiValue
{
	namespace blockchain
	{
		UploadRequestOperation::UploadRequestOperation(const FileIdType& file_id, const PublicKeyType& requestor,
			const vector<PieceUploadInfo>& pieces,
			const ContractIdType& authentication,
			ShareType num_of_copys,
			ShareType payterm,
			const string& filename,
			const string& description,
			const string& node_id) :
			file_id(file_id), requestor(requestor), pieces(pieces), authentication(authentication), num_of_copys(num_of_copys), payterm(payterm),filename(filename),description(description),node_id(node_id)
		{
			num_of_pieces = pieces.size();
		}

		void UploadRequestOperation::evaluate(TransactionEvaluationState & eval_state) const
		{
			try {
				if (!eval_state.evaluate_contract_result)
					FC_CAPTURE_AND_THROW(in_result_of_execute, ("UploadRequestOperation can only in result transaction"));
				if (!eval_state._current_state->get_upload_request(file_id).valid())
				{
					UploadRequestEntry entry;
					entry.id = file_id;
					entry.num_of_copys = num_of_copys;
					entry.pieces = pieces;
					entry.authenticating_contract = authentication;
					entry.description = description;
					entry.node_id =		node_id;
					entry.filename =	filename;
					eval_state._current_state->store_upload_request(entry);
				}
			}
			FC_CAPTURE_AND_RETHROW((*this))
		}

		StoreRequestOperation::StoreRequestOperation(const FileIdType& file_id, const FilePieceIdType & piece_id, const PublicKeyType & requester, const NodeIdType & node_id) :piece_id(piece_id), requester(requester), node_id(node_id), file_id(file_id)
		{

		}

		void StoreRequestOperation::evaluate(TransactionEvaluationState & eval_state) const
		{
			try {
				if (!eval_state.evaluate_contract_result)
					FC_CAPTURE_AND_THROW(in_result_of_execute, ("StoreRequestOperation can only in result transaction"));

				oUploadRequestEntry uploadrequest_entry = eval_state._current_state->get_upload_request(file_id);
				if (!uploadrequest_entry.valid())
					FC_CAPTURE_AND_THROW(upload_request_not_exsited, (file_id));
				auto search_end = uploadrequest_entry->pieces.end();
				auto piece = uploadrequest_entry->pieces.begin();
				for (; piece != search_end; piece++)
				{
					if (piece->pieceid == piece_id)
						break;
				}
				if (piece == search_end)
					FC_CAPTURE_AND_THROW(file_piece_upload_request_not_exsited, ((file_id, piece_id)));
				oStoreRequestEntry storeRequest = eval_state._current_state->get_store_request(piece_id);
				if (storeRequest.valid())
				{
					storeRequest->store_request.insert(std::make_pair(node_id, requester));

					storeRequest->file_id.insert(file_id);
					eval_state._current_state->store_store_request(*storeRequest);
				}
				else
				{
					StoreRequestEntry entry;
					entry.file_id.insert(file_id);
					entry.piece_id = piece_id;
					entry.store_request.insert(std::make_pair(node_id, requester));
					eval_state._current_state->store_store_request(entry);

				}
			}
			FC_CAPTURE_AND_RETHROW((*this))
		}

		PieceSavedOperation::PieceSavedOperation(const FileIdType & file_id, const FilePieceIdType & piece_id, const NodeIdType & Node):file_id(file_id), piece_id(piece_id), Node(Node)
		{

		}

		void PieceSavedOperation::evaluate(TransactionEvaluationState & eval_state) const
		{
			try {
			auto _cur_state = eval_state._current_state;
			if (!eval_state.evaluate_contract_result)
				FC_CAPTURE_AND_THROW(in_result_of_execute, ("PieceSavedOperation can only in result transaction"));
			oStoreRequestEntry store_request_entry= _cur_state->get_store_request_entry(piece_id);
			if (!store_request_entry.valid() ||
				store_request_entry->store_request.find(this->Node) == store_request_entry->store_request.end())
				FC_CAPTURE_AND_THROW(store_request_not_exsited, ((this->piece_id, Node)));
			auto piece_saved_entry=_cur_state->get_piece_saved_entry(piece_id);

			if (piece_saved_entry.valid())
			{
				piece_saved_entry->storageNode.insert(Node);
				_cur_state->store_piece_saved_entry(*piece_saved_entry);
			}
			else 
			{
				PieceSavedEntry new_entry;
				new_entry.piece_id = piece_id;
				new_entry.storageNode.insert(Node);
				_cur_state->store_piece_saved_entry(new_entry);
			};
			auto upload_request = _cur_state->get_upload_request(file_id);
			auto& pieces = upload_request->pieces;
			auto reject_entry = _cur_state->get_reject_store_entry(this->piece_id);
			if (reject_entry.valid())
			{
				auto info_range = reject_entry->info.equal_range(Node);
				auto info_it = info_range.first;
				while (info_it != info_range.second)
				{
					if (info_it->second == file_id)
					{
						reject_entry->info.erase(info_it);
						break;
					}
				}
				if (reject_entry->info.size() == 0)
					_cur_state->remove_reject_store_entry(reject_entry->piece_id);
				else
					_cur_state->store_store_reject(*reject_entry);
			}
			for (auto piece : pieces)
			{
				if (piece.pieceid == piece_id)
					continue;
				if (!_cur_state->get_piece_saved_entry(piece.pieceid).valid())
					return;
			}
			FileSavedEntry file_store_entry;
			file_store_entry.file_id = file_id;
			_cur_state->store_file_saved_entry(file_store_entry);

			}
			FC_CAPTURE_AND_RETHROW((*this))
		}

		EnableAccessOperation::EnableAccessOperation(const FileIdType & file_id, const PublicKeyType & requester) :file_id(file_id), requester(requester) 
		{}

		void EnableAccessOperation::evaluate(TransactionEvaluationState & eval_state) const
		{
			try{
			auto _cur_state = eval_state._current_state;
			if (!eval_state.evaluate_contract_result)
				FC_CAPTURE_AND_THROW(in_result_of_execute, ("EnableAccessOperation can only in result transaction"));
			auto enabe_access= _cur_state->get_enable_access_entry(this->file_id);
			if (enabe_access.valid())
			{
				enabe_access->fetcher.insert(requester);
				_cur_state->store_enable_access_entry(*enabe_access);
			}
			else
			{
				EnableAccessEntry new_entry;
				new_entry.file_id = file_id;
				new_entry.fetcher.insert(requester);

				_cur_state->store_enable_access_entry(new_entry);
			}
			}
			FC_CAPTURE_AND_RETHROW((*this))
		}

		StoreRejectOperation::StoreRejectOperation(const FileIdType & file_id, const FilePieceIdType & piece_id, const NodeIdType & node_id) :file_id(file_id), piece_id(piece_id), node_id(node_id) {}

		void StoreRejectOperation::evaluate(TransactionEvaluationState & eval_state) const
		{
			try{
			auto _cur_state = eval_state._current_state;
			if (!eval_state.evaluate_contract_result)
				FC_CAPTURE_AND_THROW(in_result_of_execute, ("EnableAccessOperation can only in result transaction"));
			auto upload_request=_cur_state->get_upload_request(file_id);
			if (!upload_request.valid())
				FC_CAPTURE_AND_THROW(upload_request_not_exsited, ((file_id)));
			auto search_end = upload_request->pieces.end();
			auto piece = upload_request->pieces.begin();
			for (; piece != search_end; piece++)
			{
				if (piece->pieceid == piece_id)
					break;
			}
			if (piece == search_end)
				FC_CAPTURE_AND_THROW(file_piece_upload_request_not_exsited, ((file_id, piece_id)));
			auto piece_saved=_cur_state->get_piece_saved_entry(piece_id);
			if(!piece_saved.valid())
				FC_CAPTURE_AND_THROW(piece_not_saved, ((file_id, piece_id)));
			if(piece_saved->storageNode.erase(node_id)==0)
				FC_CAPTURE_AND_THROW(piece_not_saved_by_this_node, ((file_id, piece_id,node_id)));
			if (piece_saved->storageNode.size() > 0)
				_cur_state->store_piece_saved_entry(*piece_saved);
			else
			{
				_cur_state->remove_piece_saved_entry(piece_id);
				_cur_state->remove_file_saved_entry(file_id);
			}
			auto store_rejection = _cur_state->get_reject_store_entry(piece_id);
			if (store_rejection.valid())
			{
				store_rejection->info.insert(std::make_pair(node_id, file_id));
				_cur_state->store_store_reject(*store_rejection);
			}
			else
			{
				StoreRejectEntry new_entry;
				new_entry.piece_id = piece_id;
				new_entry.info.insert(std::make_pair(node_id,file_id));
				_cur_state->store_store_reject(new_entry);
			}
			}
			FC_CAPTURE_AND_RETHROW((*this))
		}

		PieceSavedDeclareOperation::PieceSavedDeclareOperation(const FileIdType & file_id, const FilePieceIdType & piece_id, 
			const NodeIdType & node_id, const PublicKeyType & key):file_id(file_id),piece_id(piece_id),node_id(node_id),key(key)
		{

		}

		void PieceSavedDeclareOperation::evaluate(TransactionEvaluationState & eval_state) const
		{
			try{
			if (!eval_state.check_signature(Address(key)))
				FC_CAPTURE_AND_THROW(missing_signature, (key));
			auto entry= eval_state._current_state->get_store_request_entry(piece_id);
			if (!entry.valid())
				FC_CAPTURE_AND_THROW(store_request_not_exsited, (piece_id));
			if(entry->file_id.find(file_id)== entry->file_id.end())
				FC_CAPTURE_AND_THROW(file_piece_upload_request_not_exsited, (piece_id));
			auto node_and_key =entry->store_request.find(node_id);
			if (node_and_key == entry->store_request.end() || node_and_key->second != key)
				FC_CAPTURE_AND_THROW(store_request_not_exsited, (""));
			auto decl_entry=eval_state._current_state->get_save_decl_entry(piece_id);
			if (decl_entry.valid())
			{
				PieceStoreInfo temp;
				temp.file_id = file_id;
				temp.piece_id = piece_id;
				set<PieceStoreInfo>::iterator it = decl_entry->store_info.find(temp);

				if (it == decl_entry->store_info.end())
				{
					PieceStoreInfo info;
					info.file_id = file_id;
					info.piece_id = piece_id;
					info.nodes.insert(StoreNodeInfo(this->node_id, this->key));
					decl_entry->store_info.insert(info);
				}
				else
				{
					if (it->nodes.find(StoreNodeInfo(this->node_id, this->key)) != it->nodes.end())
						FC_CAPTURE_AND_THROW(save_decl_exsited,(this->node_id));
					StoreNodeInfo node_info= StoreNodeInfo(this->node_id, this->key);
					PieceStoreInfo info;
					info.file_id = file_id;
					info.piece_id = piece_id;
					info.nodes = it->nodes;
					info.nodes.insert(StoreNodeInfo(this->node_id, this->key));
					decl_entry->store_info.insert(info);
				}
				eval_state._current_state->store_save_decl_entry(*decl_entry);
			}
			else
			{
				PieceSavedDeclEntry decl_entry;
				decl_entry.piece_id = piece_id;
				PieceStoreInfo info;
				info.file_id = file_id;
				info.piece_id = piece_id;
				info.nodes.insert(StoreNodeInfo(this->node_id, this->key));
				decl_entry.store_info.insert(info);
				eval_state._current_state->store_save_decl_entry(decl_entry);
			}
		}FC_CAPTURE_AND_RETHROW((*this))
		}
		
}
}
