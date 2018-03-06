#include <blockchain/FileStoreOperations.hpp>
#include <blockchain/ChainInterface.hpp>
#include <blockchain/TransactionEvaluationState.hpp>
#include <blockchain/Exceptions.hpp>
namespace TiValue
{
	namespace blockchain
	{
		UploadRequestOperation::UploadRequestOperation(const FileIdType& file_id, 
      const PublicKeyType& requestor,
			const vector<PieceUploadInfo>& pieces,
			ShareType num_of_copys,
			ShareType payterm,
			const string& filename,
			const string& description,
			const string& node_id) :
			file_id(file_id), requestor(requestor), pieces(pieces), /*authentication(authentication),*/ num_of_copys(num_of_copys), payterm(payterm),filename(filename),description(description),node_id(node_id)
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
					//entry.authenticating_contract = authentication;
					entry.description = description;
					entry.node_id =		node_id;
					entry.filename =	filename;
					eval_state._current_state->store_upload_request(entry);
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
			//if (!store_request_entry.valid() ||
			//	store_request_entry->store_request.find(this->Node) == store_request_entry->store_request.end())
			//	FC_CAPTURE_AND_THROW(store_request_not_exsited, ((this->piece_id, Node)));

      //added on 1/16/2018
      oPieceSavedDeclEntry decl_entry = _cur_state->get_save_decl_entry(piece_id);
      if (!decl_entry.valid()) {
        FC_CAPTURE_AND_THROW(piece_id_not_existed, (piece_id));
      }

      for (set<PieceStoreInfo>::iterator itr1 = decl_entry->store_info.begin(); itr1 != decl_entry->store_info.end(); itr1++) {
        if (itr1->file_id == file_id && itr1->piece_id == piece_id) {
          for (set<StoreNodeInfo>::iterator itr2 = itr1->nodes.begin(); itr2 != itr1->nodes.end(); itr2++) {
            if (itr2->node == Node) {
              break;
            }
            if (itr2 == itr1->nodes.end()) {
              FC_CAPTURE_AND_THROW(node_id_not_existed, (Node));
            }
          }
        }
      }
    
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
			//auto reject_entry = _cur_state->get_reject_store_entry(this->piece_id);
			//if (reject_entry.valid())
			//{
			//	auto info_range = reject_entry->info.equal_range(Node);
			//	auto info_it = info_range.first;
			//	while (info_it != info_range.second)
			//	{
			//		if (info_it->second == file_id)
			//		{
			//			reject_entry->info.erase(info_it);
			//			break;
			//		}
			//	}
			//	if (reject_entry->info.size() == 0)
			//		_cur_state->remove_reject_store_entry(reject_entry->piece_id);
			//	else
			//		_cur_state->store_store_reject(*reject_entry);
			//}
			for (auto piece : pieces)
			{
				if (piece.pieceid == piece_id)
					continue;
				if (!_cur_state->get_piece_saved_entry(piece.pieceid).valid())
					return;
			}
			FileSavedEntry file_store_entry;
			file_store_entry.file_id = file_id;
      file_store_entry.piece_id = piece_id;
			_cur_state->store_file_saved_entry(file_store_entry);

			}
			FC_CAPTURE_AND_RETHROW((*this))
		}






		PieceSavedDeclareOperation::PieceSavedDeclareOperation(const FileIdType & file_id, const FilePieceIdType & piece_id, 
			const NodeIdType & node_id, const PublicKeyType & key):file_id(file_id),piece_id(piece_id),node_id(node_id),key(key)
		{

		}

		void PieceSavedDeclareOperation::evaluate(TransactionEvaluationState & eval_state) const
		{
			try {
        if (!eval_state.check_signature(Address(key)))
          FC_CAPTURE_AND_THROW(missing_signature, (key));
        //auto entry = eval_state._current_state->get_store_request_entry(piece_id);
        //if (!entry.valid())
        //  FC_CAPTURE_AND_THROW(store_request_not_exsited, (piece_id));
        //if (entry->file_id.find(file_id) == entry->file_id.end())
        //  FC_CAPTURE_AND_THROW(file_piece_upload_request_not_exsited, (piece_id));
        //auto node_and_key = entry->store_request.find(node_id);
        //if (node_and_key == entry->store_request.end() || node_and_key->second != key)
        //  FC_CAPTURE_AND_THROW(store_request_not_exsited, (""));
        oUploadRequestEntry uploadEntry = eval_state._current_state->get_upload_request(file_id);
        if (!uploadEntry.valid()) {
          FC_CAPTURE_AND_THROW(upload_request_not_exsited, (file_id));
        }
        if (uploadEntry->pieces[0].pieceid != piece_id) {
          FC_CAPTURE_AND_THROW(piece_id_not_existed, (piece_id));
        }

        oPieceSavedDeclEntry decl_entry = eval_state._current_state->get_save_decl_entry(piece_id);
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
            info.nodes.insert(StoreNodeInfo(node_id, key));
            decl_entry->store_info.insert(info);
          }
          else
          {
            if (it->nodes.find(StoreNodeInfo(node_id, key)) != it->nodes.end()) {
              FC_CAPTURE_AND_THROW(save_decl_exsited, (node_id));
            }         
            StoreNodeInfo node_info = StoreNodeInfo(node_id, key);
            PieceStoreInfo info;
            info.file_id = file_id;
            info.piece_id = piece_id;
            info.nodes = it->nodes;
            info.nodes.insert(StoreNodeInfo(node_id, key));
            decl_entry->store_info.erase(info);
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
