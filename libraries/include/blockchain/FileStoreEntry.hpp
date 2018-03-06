#pragma once

#include <blockchain/Types.hpp>
#include <blockchain/Exceptions.hpp>
#include <map>
namespace TiValue {
	namespace blockchain {
		class ChainInterface;
		struct PieceUploadInfo
		{
			FilePieceIdType pieceid;
			size_t piece_size;
			double price;
		};
		struct UploadRequestEntry;
		typedef optional<UploadRequestEntry> oUploadRequestEntry;

		struct FileIdType
		{
			FileContentIdType file_id;
			PublicKeyType uploader;
			FileIdType();
			FileIdType(const string id);
			operator string() const
			{
				return file_id + string(uploader);
			}
		};
		inline bool operator == (const FileIdType& id_1, const FileIdType& id_2)
		{
			return std::tie(id_1.file_id, id_1.uploader) == std::tie(id_2.file_id, id_2.uploader);
		}
		inline bool operator<(const FileIdType& id_1, const FileIdType& id_2)
		{
			if (id_1.file_id == id_2.file_id)
				return id_1.uploader < id_2.uploader;
			return id_1.file_id < id_2.file_id;
		}



		struct UploadRequestEntry
		{
			FileIdType id;
			vector<PieceUploadInfo> pieces;
			size_t num_of_copys;
			//ContractIdType authenticating_contract;
			NodeIdType node_id;
			string filename;
			string description;
			// database related functions
			static oUploadRequestEntry lookup(const ChainInterface&, const FileIdType&);
			static void store(ChainInterface&, const FileIdType&, const UploadRequestEntry&);
			static void remove(ChainInterface&, const FileIdType&);
		};

		inline bool operator == (const UploadRequestEntry& entry_1, const UploadRequestEntry& entry_2)
		{
			return entry_1.id == entry_2.id;
		}
		inline bool operator<(const  UploadRequestEntry& entry_1, const UploadRequestEntry& entry_2)
		{
			return entry_1.id < entry_1.id;
		}

		struct PieceSavedEntry;
		typedef optional<PieceSavedEntry> oPieceSavedEntry;
		struct PieceSavedEntry
		{
			FilePieceIdType piece_id;
			std::set<NodeIdType> storageNode;
			static oPieceSavedEntry lookup(const ChainInterface&, const FilePieceIdType&);
			static void store(ChainInterface&, const FilePieceIdType&, const PieceSavedEntry&);
			static void remove(ChainInterface&, const FilePieceIdType&);
		};
		struct FileSavedEntry;
		typedef optional<FileSavedEntry> oFileSavedEntry;
		struct FileSavedEntry
		{
			FileIdType file_id;
      FilePieceIdType piece_id;
			static oFileSavedEntry lookup(const ChainInterface&, const FileIdType&);
			static void store(ChainInterface&, const FileIdType&, const FileSavedEntry&);
			static void remove(ChainInterface&, const FileIdType&);
		};




		struct StoreNodeInfo
		{
			NodeIdType node;
			PublicKeyType key;
			StoreNodeInfo();
			StoreNodeInfo(const NodeIdType& node, const PublicKeyType& key);
			bool operator <(const StoreNodeInfo& info) const;
			bool operator ==(const StoreNodeInfo& info) const;
		};
		struct PieceStoreInfo
		{
			FileIdType file_id;
			FilePieceIdType piece_id;
			set<StoreNodeInfo> nodes;
			bool operator <(const PieceStoreInfo& info) const;
			bool operator ==(const PieceStoreInfo& info) const;
		};
		struct PieceSavedDeclEntry;
		typedef optional<PieceSavedDeclEntry> oPieceSavedDeclEntry;
		struct PieceSavedDeclEntry
		{
			FilePieceIdType piece_id;
			set<PieceStoreInfo> store_info;
			static oPieceSavedDeclEntry lookup(const ChainInterface& db, const FilePieceIdType& id);
			static void store(ChainInterface& db, const FilePieceIdType&id, const PieceSavedDeclEntry& entry);
			static void remove(ChainInterface& db, const FilePieceIdType&id);
		};
		class FileStoreDbInterface
		{
			friend struct UploadRequestEntry;
			friend struct PieceSavedEntry;
			friend struct FileSavedEntry;
			friend struct PieceSavedDeclEntry;

			//insert related
			virtual void uploadrequest_insert_into_id_map(const FileIdType& file_id, const UploadRequestEntry& entry) = 0;
			virtual void piecesaved_insert_into_id_map(const FilePieceIdType& piece_id,const PieceSavedEntry& entry)=0;
			virtual void filesaved_insert_into_id_map(const FileIdType& piece_id, const FileSavedEntry& entry) = 0;
			virtual void savedecl_insert_into_id_map(const FilePieceIdType& file_id, const PieceSavedDeclEntry& entry) = 0;

			//lookup related
			virtual oUploadRequestEntry uploadrequest_lookup_by_id(const FileIdType& file_id) const = 0;
			virtual oPieceSavedEntry piecesaved_lookup_by_id(const FilePieceIdType& file_id)const = 0;
			virtual oFileSavedEntry filesaved_lookup_by_id(const FileIdType& file_id)const = 0;
			virtual oPieceSavedDeclEntry savedecl_lookup_by_id(const FilePieceIdType& file_id)const = 0;

			//remove related			
			virtual void uploadrequest_remove_by_id(const FileIdType& file_id) = 0;
			virtual void piecesaved_remove_by_id(const FilePieceIdType& file_id) = 0;
			virtual void filesaved_remove_by_id(const FileIdType& file_id) = 0;
			virtual void savedecl_remove_by_id(const FilePieceIdType& file_id) = 0;

		};
		struct PieceSaveInfo
		{
			string filename;
			string piece_id;
			size_t size;
			std::set<NodeIdType> nodes;
		};
		struct FileSaveInfo
		{
			string file_id;
			string c_id;
			string filename;
			vector<PieceSaveInfo> pieces;
		};
		struct FilePieceInfo
		{
			string file_id;
			string piece_id;
		};

		FileContentIdType GetFileInfo(std::string filename, vector<PieceUploadInfo>& infos, ShareType num_of_pieces, double Price,uint32_t filesize);

    struct StoreNodeInfoPlus {
      NodeIdType node;
      PublicKeyType key;
      bool is_confirmed;
      StoreNodeInfoPlus(){}
      StoreNodeInfoPlus(NodeIdType node, PublicKeyType key, bool is_confirmed) :node(node), key(key), is_confirmed(is_confirmed){}
    };

    struct UploadRequestEntryPlus {
      FileIdType file_id;
      PieceUploadInfo piece;
      vector<StoreNodeInfoPlus> storers;
      size_t num_of_copy;
      size_t num_of_declared;
      size_t num_of_confirmed;
      NodeIdType node_id;
      string file_name;
      string description;
    };

    struct CanApplyEntry {
      FileIdType file_id;
      PieceUploadInfo piece;
      size_t num_of_copy;
      //size_t num_of_declared;
      size_t num_of_confirmed;
      NodeIdType node_id;
      string file_name;
      string description;
    };

    struct HaveAppliedFileEntry{
      FileIdType file_id;
      PieceUploadInfo piece;
      bool is_confirmed;
      size_t num_of_copy;
      size_t num_of_declared;
      size_t num_of_confirmed;
      NodeIdType node_id;
      string file_name;
      string description;
    };

	}
}
namespace std
{
	template<>
	struct hash < TiValue::blockchain::FileIdType >
	{
		size_t operator()(const TiValue::blockchain::FileIdType& s)const
		{
			return  *((size_t*)&(s.file_id));
		}
	};
}
FC_REFLECT(TiValue::blockchain::FileIdType,
(file_id)
(uploader)
)
FC_REFLECT(TiValue::blockchain::PieceUploadInfo,
	(pieceid)
	(piece_size)
	(price)
	)
	FC_REFLECT(TiValue::blockchain::UploadRequestEntry,
	(id)
	(pieces)
	(num_of_copys)
	(node_id)
	(filename)
	(description)
	)
	FC_REFLECT(TiValue::blockchain::PieceSavedEntry,
	(piece_id)
	(storageNode)
	)
	FC_REFLECT(TiValue::blockchain::FileSavedEntry,
	(file_id)
  (piece_id)
	)	

	FC_REFLECT(TiValue::blockchain::PieceSaveInfo,
		(filename)
		(piece_id)
		(nodes)
		(size)
	)
	FC_REFLECT(TiValue::blockchain::FileSaveInfo,
	(file_id)
		(c_id)
		(filename)
		(pieces)
	)
	FC_REFLECT(TiValue::blockchain::FilePieceInfo,
	(file_id)
	(piece_id)
	)
	FC_REFLECT(TiValue::blockchain::StoreNodeInfo,
		(node)
		(key)
	)


  FC_REFLECT(TiValue::blockchain::PieceStoreInfo,
  (file_id)
  (piece_id)
  (nodes)
	)

	FC_REFLECT(TiValue::blockchain::PieceSavedDeclEntry,
	(piece_id)
	(store_info)
	)



  FC_REFLECT(TiValue::blockchain::UploadRequestEntryPlus,
  (file_id)
  (piece)
  (storers)
  (num_of_copy)
  (num_of_declared)
  (num_of_confirmed)
  (node_id)
  (file_name)
  (description))
  
  FC_REFLECT(TiValue::blockchain::StoreNodeInfoPlus,
  (node)
  (key)
  (is_confirmed))

  FC_REFLECT(TiValue::blockchain::CanApplyEntry,
  (file_id)
  (piece)
  (num_of_copy)
  (num_of_confirmed)
  (node_id)
  (file_name)
  (description))

  FC_REFLECT(TiValue::blockchain::HaveAppliedFileEntry,
  (file_id)
  (piece)
  (is_confirmed)
  (num_of_copy)
  (num_of_declared)
  (num_of_confirmed)
  (node_id)
  (file_name)
  (description))

