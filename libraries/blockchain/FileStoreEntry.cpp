#include <blockchain/FileStoreEntry.hpp>
#include <blockchain/ChainInterface.hpp>
#include <fc/crypto/base58.hpp>
namespace TiValue {
	namespace blockchain {
		oUploadRequestEntry UploadRequestEntry::lookup(const ChainInterface &db , const FileIdType &id)
		{
			try
			{
				return db.uploadrequest_lookup_by_id(id);
			}FC_CAPTURE_AND_RETHROW((id));
		}
		void UploadRequestEntry::store(ChainInterface &db, const FileIdType &id, const UploadRequestEntry &entry)
		{
			try
			{
				db.uploadrequest_insert_into_id_map(id, entry);
			}FC_CAPTURE_AND_RETHROW((id));
		}
		void UploadRequestEntry::remove(ChainInterface &db, const FileIdType &id)
		{
			try
			{
				db.uploadrequest_remove_by_id(id);
			}FC_CAPTURE_AND_RETHROW((id));
		}
		oPieceSavedEntry PieceSavedEntry::lookup(const ChainInterface &db, const FilePieceIdType &id)
		{
			try
			{
				return db.piecesaved_lookup_by_id(id);
			}FC_CAPTURE_AND_RETHROW((id));
		}
		void PieceSavedEntry::store(ChainInterface &db, const FilePieceIdType &id, const PieceSavedEntry &entry)
		{
			try
			{
				db.piecesaved_insert_into_id_map(id, entry);
			}FC_CAPTURE_AND_RETHROW((id));
		}
		void PieceSavedEntry::remove(ChainInterface &db, const FilePieceIdType &id)
		{
			try
			{
				db.piecesaved_remove_by_id(id);
			}FC_CAPTURE_AND_RETHROW((id));
		}
		oFileSavedEntry FileSavedEntry::lookup(const ChainInterface &db, const FileIdType &id)
		{
			try
			{
				return db.filesaved_lookup_by_id(id);
			}FC_CAPTURE_AND_RETHROW((id));
		}
		void FileSavedEntry::store(ChainInterface &db, const FileIdType &id, const FileSavedEntry &entry)
		{
			try
			{
				db.filesaved_insert_into_id_map(id, entry);
			}FC_CAPTURE_AND_RETHROW((id));
		}
		void FileSavedEntry::remove(ChainInterface &db, const FileIdType &id)
		{
			try {
				db.filesaved_remove_by_id(id);
			}FC_CAPTURE_AND_RETHROW((id));
		}
		FileIdType::FileIdType() 
		{}
		FileIdType::FileIdType(const string id)
		{
			try {
				size_t pos = 0;
				do {
					pos = id.find(TIV_ADDRESS_PREFIX, pos);
					if (pos == string::npos)
						throw fc::exception();
					try {
						FileContentIdType content_id(id.substr(0, pos));
						PublicKeyType pk(id.substr(pos));
						this->file_id = content_id;
						this->uploader = pk;
						break;
					}
					catch (...) {}
				} while (1);
			}
			catch (...)
			{
				FC_CAPTURE_AND_THROW(invalid_file_id, (id));
			}
		}
		StoreNodeInfo::StoreNodeInfo() 
		{}
		StoreNodeInfo::StoreNodeInfo(const NodeIdType & node, const PublicKeyType & key) :node(node), key(key)
		{}
		bool StoreNodeInfo::operator<(const StoreNodeInfo & info) const
		{
			if (node < info.node)
				return true;
			return key < info.key;
		}
		bool StoreNodeInfo::operator==(const StoreNodeInfo & info) const
		{
			return (key==info.key)&&(node==info.node);
		}
		FileContentIdType GetFileInfo(std::string fileinfo, vector<PieceUploadInfo>& infos, ShareType num_of_pieces, double Price, uint32_t filesize)
		{
			infos.clear();
			FileContentIdType result;
			std::string info_parser;
			auto pos = fileinfo.find_first_of(";");
			if (pos == string::npos)
				FC_CAPTURE_AND_THROW(read_file_info_error, (fileinfo));
			info_parser = fileinfo.substr(0, pos);
			result = info_parser;
			fileinfo = fileinfo.substr(pos + 1);
			int piece_count = 0;
			string buf;
			uint32_t size_count = 0;
			while (pos = fileinfo.find_first_of(";"), pos != string::npos)
			{
				PieceUploadInfo info;
				info_parser = fileinfo.substr(0, pos);
				int pos2 = fileinfo.find_first_of(",");
				buf = info_parser.substr(0, pos2);
				info.pieceid = buf;
				buf = info_parser.substr(pos2 + 1);
				sscanf(buf.c_str(), "%lld", &(info.piece_size));
				size_count += info.piece_size;
				info.price = Price * info.piece_size / filesize;
				infos.push_back(info);
				fileinfo = fileinfo.substr(pos + 1);
			}
			if(size_count != filesize)
				FC_CAPTURE_AND_THROW(read_file_info_error, (fileinfo));
			return result;
		}




		oPieceSavedDeclEntry PieceSavedDeclEntry::lookup(const ChainInterface & db, const FilePieceIdType & id)
		{
			try
			{
				return db.savedecl_lookup_by_id(id);
			}FC_CAPTURE_AND_RETHROW((id));

		}

		void PieceSavedDeclEntry::store(ChainInterface & db, const FilePieceIdType & id, const PieceSavedDeclEntry & entry)
		{
			try
			{
				return db.savedecl_insert_into_id_map(id,entry);
			}FC_CAPTURE_AND_RETHROW((id));

		}

		void PieceSavedDeclEntry::remove(ChainInterface & db, const FilePieceIdType & id)
		{
			try
			{
				return db.savedecl_remove_by_id(id);
			}FC_CAPTURE_AND_RETHROW((id));
		}

		bool PieceStoreInfo::operator<(const PieceStoreInfo & info) const
		{
			if (file_id < info.file_id)
				return true;
			return piece_id < info.piece_id;
		}

		bool PieceStoreInfo::operator==(const PieceStoreInfo & info) const
		{
			if (file_id == info.file_id&&piece_id == info.piece_id)
				return true;
			return false;
		}


  }
}
