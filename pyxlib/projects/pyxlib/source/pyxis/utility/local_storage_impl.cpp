/******************************************************************************
local_storage.cpp

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "local_storage_impl.h"
#include "tester.h"
#include "exceptions.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/ssl_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/profile.h"
#include "pyxis/utility/cache_map.h"
#include "pyxis/utility/app_services.h"

#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/functional/hash.hpp"

#include "sqlite3.h"

#include <cpprest/http_client.h>
#include <cpprest/interopstream.h>

// standard includes
#include <cassert>


///////////////////////////////////////////////////////////////////////////////
// PYXProcessLocalStorageSqlite
///////////////////////////////////////////////////////////////////////////////

class PYXLocalStorageSqlite : public PYXLocalStorage
{
private:	
	class PYXLocalStorageSqliteImpl
	{
	private:
		std::string m_dbPath;
		boost::recursive_mutex m_dbMutex;

		sqlite3 * m_handle;

		sqlite3_stmt * m_getStmt;
		sqlite3_stmt * m_upsertStmt;
		sqlite3_stmt * m_deleteStmt;

	public:
		static PYXLocalStorageSqliteImpl * create(const std::string & dbPath) 
		{
			sqlite3 * handle;
			int retval = sqlite3_open(dbPath.c_str(),&handle);

			// If connection failed, handle returns NULL
			if(retval)
			{
				TRACE_ERROR("Failed to open process local storage sqlite database: " << dbPath);
				return 0;
			}
			return new PYXLocalStorageSqliteImpl(dbPath,handle);
		}

	private:
		PYXLocalStorageSqliteImpl(const std::string & dbPath,sqlite3 * handle) : m_dbPath(dbPath), m_handle(handle)
		{
			TRACE_INFO("open sqltile db: " << m_dbPath);
		}

	public:
		virtual ~PYXLocalStorageSqliteImpl()
		{
			sqlite3_finalize(m_getStmt);
			sqlite3_finalize(m_upsertStmt);
			sqlite3_finalize(m_deleteStmt);

			sqlite3_close(m_handle);

			TRACE_INFO("closing sqltile db: " << m_dbPath);
		}

	public:
		void initalize()
		{
			int retval = sqlite3_exec(m_handle,"CREATE TABLE IF NOT EXISTS data (key TEXT PRIMARY KEY,data BLOB NOT NULL)",0,0,0);

			if(retval)
			{
				PYXTHROW(PYXException,"Failed to create data table in db " << m_dbPath);
			}

			retval = sqlite3_prepare_v2(m_handle,"SELECT data FROM data WHERE key = ?",-1,&m_getStmt,0);

			if(retval)
			{
				PYXTHROW(PYXException,"Failed to create get statement in db " << m_dbPath);
			}

			retval = sqlite3_prepare_v2(m_handle,"INSERT OR REPLACE INTO data VALUES (?,?)",-1,&m_upsertStmt,0);

			if(retval)
			{
				PYXTHROW(PYXException,"Failed to create upsert statement in db " << m_dbPath);
			}

			retval = sqlite3_prepare_v2(m_handle,"DELETE FROM data WHERE key = ?",-1,&m_deleteStmt,0);

			if(retval)
			{
				PYXTHROW(PYXException,"Failed to create delete statement in db " << m_dbPath);
			}
		}

		std::auto_ptr<PYXConstWireBuffer> get(const std::string & key)
		{
			boost::recursive_mutex::scoped_lock lock(m_dbMutex);

			sqlite3_reset(m_getStmt);
			sqlite3_bind_text(m_getStmt,1,key.c_str(),key.length(), SQLITE_STATIC);
			int retval = sqlite3_step(m_getStmt);

			if (retval == SQLITE_ROW)
			{
				int blobSize = sqlite3_column_bytes(m_getStmt,0);
				return std::auto_ptr<PYXConstWireBuffer>(new PYXConstWireBuffer(( char *)sqlite3_column_blob(m_getStmt,0),blobSize));
			}
			else if (retval == SQLITE_DONE)
			{
				return std::auto_ptr<PYXConstWireBuffer>();
			}
			else {
				PYXTHROW(PYXException,"Failed to get the requested key " << key << " on db " << m_dbPath);
			}
		}

		void set(const std::string &key,PYXWireBuffer & data)
		{
			boost::recursive_mutex::scoped_lock lock(m_dbMutex);

			PYXPointer<PYXConstBufferSlice> buffer = data.getBuffer();

			sqlite3_reset(m_upsertStmt);
			sqlite3_bind_text(m_upsertStmt,1,key.c_str(),key.length(), SQLITE_STATIC);
			sqlite3_bind_blob(m_upsertStmt,2,buffer->begin(),buffer->size(), SQLITE_STATIC);
			int retval = sqlite3_step(m_upsertStmt);

			if (retval == SQLITE_DONE)
			{
				return;
			}
			else
			{
				PYXTHROW(PYXException,"Failed to update the requested key " << key << " on db " << m_dbPath);
			}
		}

		void setMany(const std::map<std::string,PYXConstBufferSlice> & data) 
		{
			boost::recursive_mutex::scoped_lock lock(m_dbMutex);

			sqlite3_reset(m_getStmt);
			sqlite3_reset(m_upsertStmt);
			sqlite3_reset(m_deleteStmt);

			bool failed = false;

			try
			{
				sqlite3_exec(m_handle,"begin",0,0,0);

				for(std::map<std::string,PYXConstBufferSlice>::const_iterator it = data.begin();it!= data.end();++it)
				{
					sqlite3_reset(m_upsertStmt);
					sqlite3_bind_text(m_upsertStmt,1,it->first.c_str(),it->first.length(), SQLITE_STATIC);
					sqlite3_bind_blob(m_upsertStmt,2,it->second.begin(),it->second.size(), SQLITE_STATIC);
					
					int retval = sqlite3_step(m_upsertStmt);

					if (retval != SQLITE_DONE)
					{
						TRACE_ERROR("Failed to upsert many (return code was: " << retval << ", message: " << sqlite3_errmsg(m_handle) << ") - rolling back");
						failed = true;
						break;
					}
				}

				sqlite3_reset(m_upsertStmt);

				if (!failed)
				{
					//try commit if everything is ok
					int retval = sqlite3_exec(m_handle,"commit",0,0,0);

					if (retval != SQLITE_OK)
					{
						failed = true;
						TRACE_ERROR("Failed to commit a transacation (return code was: " << retval << ", message: " << sqlite3_errmsg(m_handle) << ") - roolling back");						
					}
				}
			}
			catch(...)
			{
				TRACE_ERROR("Failed to upsert many due to an exception - roolling back");
				failed = true;				
			}

			if (failed)
			{
				int retval = sqlite3_exec(m_handle,"rollback",0,0,0);
				if (retval != SQLITE_OK)
				{
					TRACE_ERROR("Failed to rollback a transacation (return code was: " << retval << ", message: " << sqlite3_errmsg(m_handle) << ")");
				}
				PYXTHROW(PYXException,"Failed to update the db " << m_dbPath);
			}
		}

		void remove(const std::string &key)
		{
			boost::recursive_mutex::scoped_lock lock(m_dbMutex);

			sqlite3_reset(m_deleteStmt);
			sqlite3_bind_text(m_deleteStmt,1,key.c_str(),key.length(), SQLITE_STATIC);
			int retval = sqlite3_step(m_deleteStmt);

			if (retval == SQLITE_DONE)
			{
				return;
			}
			else 
			{
				PYXTHROW(PYXException,"Failed to delete the requested key " << key << " on db " << m_dbPath);
			}
		}

		virtual void removeAll()
		{
			boost::recursive_mutex::scoped_lock lock(m_dbMutex);

			int retval = sqlite3_exec(m_handle,"DELETE FROM data",0,0,0);

			if(retval)
			{
				PYXTHROW(PYXException,"Failed to delete all data in db " << m_dbPath);
			}
		}

	};

	typedef std::map<std::string,boost::weak_ptr<PYXLocalStorageSqliteImpl>> InstanceMap; 
	static InstanceMap m_instances;
	static boost::recursive_mutex m_instancesMutex;

	std::string m_dbPath;
	boost::shared_ptr<PYXLocalStorageSqliteImpl> m_instance;

	PYXLocalStorageSqlite(const std::string & dbPath) : m_dbPath(dbPath)
	{
		boost::recursive_mutex::scoped_lock lock(m_instancesMutex);

		InstanceMap::iterator it = m_instances.find(dbPath);

		if (it != m_instances.end())
		{
			m_instance = it->second.lock();
		} else {
			m_instance.reset(PYXLocalStorageSqliteImpl::create(m_dbPath));
			m_instance->initalize();
			m_instances[m_dbPath] = boost::weak_ptr<PYXLocalStorageSqliteImpl>(m_instance);
		}
	}

public:
	static PYXPointer<PYXLocalStorage> create(const std::string & dbPath)
	{
		return PYXNEW(PYXLocalStorageSqlite,dbPath);
	}

	virtual ~PYXLocalStorageSqlite()
	{
		boost::recursive_mutex::scoped_lock lock(m_instancesMutex);

		m_instance.reset();

		InstanceMap::iterator it = m_instances.find(m_dbPath);

		if (it != m_instances.end())
		{
			if (it->second.expired())
			{
				m_instances.erase(it);
			}
		}
	}

public:

	virtual std::auto_ptr<PYXConstWireBuffer> get(const std::string & key)
	{
		return m_instance->get(key);
	}

	virtual void set(const std::string &key,PYXWireBuffer & data)
	{
		return m_instance->set(key,data);
	}

	virtual void setMany(const std::map<std::string,PYXConstBufferSlice> & data) 
	{
		return m_instance->setMany(data);
	}

	virtual void remove(const std::string &key)
	{
		return m_instance->remove(key);
	}

	virtual void removeAll()
	{
		return m_instance->removeAll();
	}

	virtual void applyChanges(const std::vector<PYXPointer<PYXLocalStorageChange>> & changes)
	{
		std::map<std::string,PYXConstBufferSlice> sets;

		for(auto & change : changes) 
		{
			switch(change->getChangeType()) 
			{
			case PYXLocalStorageChange::knSet:
				sets[change->getKey()] = *change->getData();
				break;
			case PYXLocalStorageChange::knRemove:
				if (sets.size()>0)
				{

					auto it = sets.find(change->getKey());
					if (it != sets.end())
					{
						sets.erase(it);
					}
					m_instance->setMany(sets);
					sets.clear();
				}
				m_instance->remove(change->getKey());
				break;
			case PYXLocalStorageChange::knRemoveAll:
				if (sets.size()>0)
				{
					sets.clear();
				}
				m_instance->removeAll();
				break;
			default:
				PYXTHROW(PYXException,"unknown change type");
			}
		}

		if (sets.size()>0)
		{
			m_instance->setMany(sets);
			sets.clear();
		}
	}
};

PYXLocalStorageSqlite::InstanceMap PYXLocalStorageSqlite::m_instances;
boost::recursive_mutex PYXLocalStorageSqlite::m_instancesMutex;

#define START_MEASURE_TIME \
	PYXHighQualityTimer timer;\
	static int totalCalls = 0;\
	static double totalTime = 0;\
	timer.start();\

#define END_MEASURE_TIME(name) \
	timer.stop(); \
	totalTime += timer.getTime(); \
	totalCalls++; \
	\
	if (totalCalls%100 == 0) \
	{ \
		TRACE_INFO(name << "avarage time " << (int)(1000 * totalTime / totalCalls) << "ms"); \
	} \

/*!

HashedKeyValueChunk is a very simple format that keeps small number of key value pairs.

in order to reduce space and solve file upper to lower case confusions, 
we are not storing the keys at all - but storing the key suffix and hash value

The chunk format is:

[16 bytes header]

[16 bytes record] * number of records

[X bytes value]
[Y bytes value] 
...
[Z bytes value ]

Header is:
4 Bytes - magic, should be equal to "PYX\0"
4 Bytes - version, should be equal to 1
4 Bytes - number of records
4 Bytes - size of header (should be number of records x 16 byte)

we use size of header to enable cases when a file want to 'prep' some free space to add records without rewrite the chunk

Record is:
4 bytes - suffix - last 4 digits of the string (padding with 0 if needed)
4 bytes - hash - first 4 bytes of SHA1 generated from the completed key.
4 bytes - value offset - offset is calculated from beginning of file as : "sizeof(header) + header.headerSize + offset"
4 bytes - value size - number of bytes for the value

*/
class HashedKeyValueChunk : public PYXObject
{
public:
	struct Header
	{
		char magic[4];
		long version;
		long recordsCount;
		long headerSize;
 	};

	struct RecordId
	{
		char postFix[4];
		long hash;

	public:
		RecordId()
		{
		}

		RecordId(const std::string & key)
		{
			setFrom(key);
		}

		void setFrom(const std::string & key) 
		{
			SSLUtils::Checksum checksum("SHA1");
			checksum.generate(key);

			hash = (checksum.getByte(0) << 24) + (checksum.getByte(1) << 16) + (checksum.getByte(2) << 8) + checksum.getByte(4);
			if (key.size() >= 4) 
			{
				memcpy_s(postFix,sizeof(postFix),key.c_str() + key.size() - 4 ,4);
			}
			else 
			{
				memset(postFix,0,sizeof(postFix));
				memcpy_s(postFix,sizeof(postFix),key.c_str(),key.size());
			}
		}

		bool equals(const RecordId & other) const {
			return memcmp(this,&other,sizeof(RecordId)) == 0;
		}

		bool operator==(const RecordId & other) const  {
			return equals(other);
		}

		bool operator!=(const RecordId & other) const {
			return !equals(other);
		}

		bool operator<(const RecordId & other) const {
			return memcmp(this,&other,sizeof(RecordId)) < 0;
		}

		bool operator>(const RecordId & other) const {
			return memcmp(this,&other,sizeof(RecordId)) > 0;
		}
	};

	struct Record
	{
		RecordId id;
		long valueOffset;
		long valueSize;
	};

private:
	mutable boost::recursive_mutex m_chunkMutex;
	mutable std::ifstream m_stream;

	//we hope all our headers would be smaller than 65K
	static const int MAX_RECORDS = 4096; //65K header size

	//we throw exception when it get to 256K
	static const int MAX_RECORDS_LIMIT = 4 * 4096; //256K headers
	static const char * s_magic;
	Header m_header;
	std::vector<Record> m_records;

public:
	static PYXPointer<HashedKeyValueChunk> create(const std::string & file)
	{
		return PYXNEW(HashedKeyValueChunk,file);
	}

	//empty
	HashedKeyValueChunk() 
	{
	}

	//open a chunk file
	HashedKeyValueChunk(const std::string & file)
	{
		m_stream.open(file, std::ios_base::in | std::ios_base::binary);
		m_stream.read((char*)&m_header,sizeof(Header));

		if (memcmp(m_header.magic,s_magic,sizeof(m_header.magic)) != 0) 
		{
			PYXTHROW(PYXException,"Wrong magic of HashedKeyValueChunk");
		}

		if (m_header.version != 1) 
		{
			PYXTHROW(PYXException,"Wrong version of HashedKeyValueChunk");
		}

		if (m_header.recordsCount > MAX_RECORDS_LIMIT) 
		{
			PYXTHROW(PYXException,"Invalid records count");
		}

		if (m_header.headerSize > MAX_RECORDS_LIMIT * sizeof(Record))
		{
			PYXTHROW(PYXException,"Invalid header size");
		}

		m_records.resize(m_header.headerSize / sizeof(Record));
		m_stream.read((char *)&m_records[0],m_records.size() * sizeof(Record));
	}

	bool has(const std::string & key) const
	{
		RecordId id(key);
		for(auto & record : m_records) 
		{
			if (id == record.id)
			{
				return true;
			}
		}

		return false;
	}

	std::auto_ptr<PYXConstWireBuffer> get(const std::string & key) const
	{
		RecordId id(key);
		for(auto & record : m_records) 
		{
			if (id != record.id)
			{
				continue;
			}

			{
				boost::recursive_mutex::scoped_lock lock(m_chunkMutex);
				PYXPointer<PYXConstBuffer> buffer = PYXConstBuffer::create(record.valueSize);			
				m_stream.seekg(sizeof(Header) + m_header.headerSize + record.valueOffset, std::ifstream::beg);
				m_stream.read((char *)const_cast<unsigned char*>(buffer->begin()), record.valueSize);
				return std::auto_ptr<PYXConstWireBuffer>(new PYXConstWireBuffer(buffer));
			}
		}
		return std::auto_ptr<PYXConstWireBuffer>();
	}

	class Modifier
	{
	private:
		mutable boost::recursive_mutex m_modifierMutex;
		std::map<HashedKeyValueChunk::RecordId,PYXConstBufferSlice> m_records;

	public:
		void load(HashedKeyValueChunk & chunk)
		{
			boost::recursive_mutex::scoped_lock lock(m_modifierMutex);
			boost::recursive_mutex::scoped_lock chunkLock(chunk.m_chunkMutex);

			for(auto & record : chunk.m_records)
			{
				PYXPointer<PYXConstBuffer> buffer = PYXConstBuffer::create(record.valueSize);			
				chunk.m_stream.seekg(sizeof(Header) + chunk.m_header.headerSize + record.valueOffset, std::ifstream::beg);
				chunk.m_stream.read((char *)const_cast<unsigned char*>(buffer->begin()), record.valueSize);
				m_records[record.id] = buffer;
			}
		}

		void save(const std::string & file)
		{
			boost::recursive_mutex::scoped_lock lock(m_modifierMutex);

			Header newHeader;
			memcpy(newHeader.magic,s_magic,sizeof(newHeader.magic));
			newHeader.version = 1;
			newHeader.recordsCount = m_records.size();

			std::vector<Record> newRecords;
			newRecords.reserve(m_records.size());

			long valueOffset = 0;
			Record newRecord;

			for(auto & keyValue : m_records) 
			{
				memcpy(&newRecord.id,&keyValue.first,sizeof(RecordId));
				newRecord.valueSize = keyValue.second.size();
				newRecord.valueOffset = valueOffset;
				valueOffset += newRecord.valueSize;
				newRecords.push_back(newRecord);
			}

			if (m_records.size() > MAX_RECORDS_LIMIT) 
			{
				PYXTHROW(PYXException,"Hashed keys can only have " << MAX_RECORDS_LIMIT << " records");
			} 
			else if (m_records.size() > MAX_RECORDS) 
			{
				TRACE_INFO("Large header with " << m_records.size() << " records");
			}
	
			newHeader.headerSize = newRecords.size() * sizeof(Record);

			std::ofstream output(file,std::ios_base::out | std::ios_base::binary);

			output.write((char *)&newHeader,sizeof(newHeader));
			output.write((char *)&newRecords[0],sizeof(Record) * newRecords.size());

			//append new values
			for(auto & keyValue : m_records) 
			{
				output.write((char *)keyValue.second.begin(),keyValue.second.size());
			}
		}

		void set(const std::string & key,const PYXConstBufferSlice & slice) 
		{
			boost::recursive_mutex::scoped_lock lock(m_modifierMutex);
			RecordId id(key);
			m_records[id] = slice;
		}

		void remove(const std::string & key)
		{
			boost::recursive_mutex::scoped_lock lock(m_modifierMutex);
			RecordId id(key);
			m_records.erase(id);
		}

		bool empty() const {
			boost::recursive_mutex::scoped_lock lock(m_modifierMutex);
			return m_records.empty();
		}

		size_t size() const {
			boost::recursive_mutex::scoped_lock lock(m_modifierMutex);
			return m_records.size();
		}

	public:

		Modifier()
		{
		}

		Modifier(HashedKeyValueChunk & chunk)
		{
			load(chunk);
		}
	};
};

const char * HashedKeyValueChunk::s_magic = "PYX";

class VerySimpleFileBasedLocalStorage : public PYXLocalStorage
{
private:
	std::string m_path;

protected:
	static boost::recursive_mutex s_liveChunksMutex;
	static CacheMap<std::string, PYXPointer<HashedKeyValueChunk>> s_liveChunks;

	static PYXPointer<HashedKeyValueChunk> getChunk(const std::string & file) 
	{
		static int requests = 0;
		static int hits = 0;

		boost::recursive_mutex::scoped_lock lock(s_liveChunksMutex);

		if (requests == 1000)
		{
			TRACE_INFO("getChunk hit rate" << (100.0*hits/requests) << "%");
			requests=0;
			hits=0;
		}

		requests++;

		if (s_liveChunks.exists(file)) 
		{
			hits++;
			return s_liveChunks[file];
		}
		auto newChunk = HashedKeyValueChunk::create(file);
		s_liveChunks[file] = newChunk;
		return newChunk;
	}

	static void invalidateChunk(const std::string & file)
	{
		boost::recursive_mutex::scoped_lock lock(s_liveChunksMutex);
		s_liveChunks.erase(file);
	}

	static void invalidateAllChunks()
	{
		boost::recursive_mutex::scoped_lock lock(s_liveChunksMutex);
		s_liveChunks.clear();
	}

	VerySimpleFileBasedLocalStorage(const std::string & path) : m_path(path)
	{
		TRACE_INFO("open simple file db: " << path);
	}

public:
	static PYXPointer<VerySimpleFileBasedLocalStorage> create(const std::string & path)
	{
		return PYXNEW(VerySimpleFileBasedLocalStorage,path);
	}

private:
	std::string encodeString(const std::string & str) 
	{
		std::string result;
		result.reserve(str.size());
		bool lastCharWasDivider = true;

		for (const auto c : str)
		{
			switch(c) 
			{
				case '/':	
				case '\\':	
				case ':':	
					if (!lastCharWasDivider)
					{
						result += '\\';
					}
					lastCharWasDivider = true;
					break;
				case '?':
				case '%':
				case '*':
				case '|':
				case '"':
				case '\'':
				case '<':
				case '>':
				case '.':
				case ' ':
					result += '_';
					lastCharWasDivider = false;
					break;
				default:
					result += c;
					lastCharWasDivider = false;
					break;
			}
		}
		return result;
	}

	/*
		please note this implementation treat ':','/','\' as partition markers.

		assuming a key have N partitions markers:
		1) directory name is made from [part0]/[part1]/[partN-2]
		2) file name is made from [partN-1].
		3) the last [partN] is used key record inside the chunk.

		example: 
		- key "hist:0:1-21312" is split into /hist/0/1-21312, and the chunk file name will be "/hist/0.part".
		- key "tree:version" is split into /tree/version, and the chunk file name will be "tree.part"
		- key "group:2-321231:4567 is split into /group/2-321231/4567, and the chunk file name will be "/group/2-321231.part"
	*/

	std::string keyToFileName(const std::string & key) {

		std::string result = encodeString(key);
		auto pos = result.find_last_of("\\");
		if (pos != std::string::npos) 
		{
			result.resize(pos);
		}
		return "\\" + result + ".part";
	}

public:
	virtual std::auto_ptr<PYXConstWireBuffer> get(const std::string & key) 
	{
		//START_MEASURE_TIME;

		auto keyFileStr = m_path + keyToFileName(key);
		auto keyFile = FileUtils::stringToPath(keyFileStr );

		if (!FileUtils::exists(keyFile))
		{
			return std::auto_ptr<PYXConstWireBuffer>();
		}

		auto result = getChunk(keyFileStr)->get(key);

		//END_MEASURE_TIME("VerySimpleFileBasedLocalStorage::get");

		return result;
	}

	void set(const std::string &key, const PYXConstBufferSlice & data) {
		
		//START_MEASURE_TIME;

		auto keyFileStr = m_path + keyToFileName(key);
		auto tmpFileName = keyFileStr  +  ".tmp";

		auto keyFile = FileUtils::stringToPath(keyFileStr );
		auto keyDir = keyFile.branch_path();

		if (!FileUtils::exists(keyDir))
		{
			boost::filesystem::create_directories(keyDir);
		}

		HashedKeyValueChunk::Modifier modifier;

		if (FileUtils::exists(keyFile))
		{
			modifier.load(*getChunk(keyFileStr));
		}		

		modifier.set(key,data);

		if (FileUtils::exists(tmpFileName))
		{
			boost::filesystem::remove(tmpFileName);
		}
		
		modifier.save(tmpFileName);

		// making sure the file and tmp file do not exist
		if (FileUtils::exists(keyFile))
		{
			boost::filesystem::remove(keyFile);
		}

		// rename the tmp file to actual file
		std::rename(tmpFileName.c_str(), keyFileStr.c_str());

		invalidateChunk(keyFileStr);

		//END_MEASURE_TIME("VerySimpleFileBasedLocalStorage::set");
	}

	virtual void set(const std::string &key, PYXWireBuffer & data) {
		set(key,*data.getBuffer());
	}

	virtual void setMany(const std::map<std::string,PYXConstBufferSlice> & data)
	{
		for(auto & entry : data)
		{
			set(entry.first,entry.second);
		}
	}
	
	virtual void remove(const std::string & key) 
	{
		auto keyFileStr = m_path + keyToFileName(key);
		auto keyFile = FileUtils::stringToPath(keyFileStr );

		if (!FileUtils::exists(keyFile))
		{
			return;
		}

		auto chunk = getChunk(keyFileStr);

		if (chunk->has(key)) 
		{
			HashedKeyValueChunk::Modifier modifier(*chunk);

			modifier.remove(key);

			invalidateChunk(keyFileStr);

			if (modifier.empty()) 
			{
				boost::filesystem::remove(keyFile);		
			}
			else
			{
				auto tmpFileName = keyFileStr  +  ".tmp";

				if (FileUtils::exists(tmpFileName))
				{
					boost::filesystem::remove(tmpFileName);
				}
		
				modifier.save(tmpFileName);

				// making sure the file and tmp file do not exist
				if (FileUtils::exists(keyFile))
				{
					boost::filesystem::remove(keyFile);
				}

				// rename the tmp file to actual file
				std::rename(tmpFileName.c_str(), keyFileStr.c_str());
			}
		}
	}

	virtual void removeAll()
	{
		invalidateAllChunks();
		boost::filesystem::remove_all(m_path);
	}

	virtual void applyChanges(const std::vector<PYXPointer<PYXLocalStorageChange>> & changes)
	{
		PYXHighQualityTimer timer;
		timer.start();

		std::map<std::string,std::vector<PYXPointer<PYXLocalStorageChange>>> m_changesPerChunks;
		std::string chunkFromKey;

		for(auto & change : changes) 
		{
			switch(change->getChangeType()) 
			{
			case PYXLocalStorageChange::knSet:
			case PYXLocalStorageChange::knRemove:
				chunkFromKey = keyToFileName(change->getKey());
				m_changesPerChunks[chunkFromKey].push_back(change);
				break;
			case PYXLocalStorageChange::knRemoveAll:
				//we can forget all previous changes.
				m_changesPerChunks.clear();
				removeAll();
				break;
			default:
				PYXTHROW(PYXException,"unknown change type");
			}
		}

		for(auto & changesForChunk : m_changesPerChunks) 
		{
			auto keyFileStr = m_path + changesForChunk.first;
			auto tmpFileName = keyFileStr  +  ".tmp";

			auto keyFile = FileUtils::stringToPath(keyFileStr );
			auto keyDir = keyFile.branch_path();

			if (!FileUtils::exists(keyDir))
			{
				boost::filesystem::create_directories(keyDir);
			}

			HashedKeyValueChunk::Modifier modifier;

			if (FileUtils::exists(keyFile))
			{
				modifier.load(*getChunk(keyFileStr));
			}		

			for(auto & change : changesForChunk.second) 
			{
				switch(change->getChangeType()) 
				{
				case PYXLocalStorageChange::knSet:
					modifier.set(change->getKey(),*change->getData());
					break;
				case PYXLocalStorageChange::knRemove:
					modifier.remove(change->getKey());
					break;
				default:
					PYXTHROW(PYXException,"unknown change type");
				}
			}

			invalidateChunk(keyFileStr);

			if (modifier.empty())
			{
				if (FileUtils::exists(keyFile))
				{
					boost::filesystem::remove(keyFile);
				}
			}
			else 
			{
				if (FileUtils::exists(tmpFileName))
				{
					boost::filesystem::remove(tmpFileName);
				}
		
				modifier.save(tmpFileName);

				// making sure the file and tmp file do not exist
				if (FileUtils::exists(keyFile))
				{
					boost::filesystem::remove(keyFile);
				}

				// rename the tmp file to actual file
				std::rename(tmpFileName.c_str(), keyFileStr.c_str());
			}
		}

		auto total = timer.tick();

		TRACE_INFO("commit took " << total << " [sec], updated " << m_changesPerChunks.size() << " chunks ( based on " << changes.size() << " updates ");
	}
};

boost::recursive_mutex VerySimpleFileBasedLocalStorage::s_liveChunksMutex;
CacheMap<std::string, PYXPointer<HashedKeyValueChunk>> VerySimpleFileBasedLocalStorage::s_liveChunks(256);


class RESTLocalStorage : public PYXLocalStorage
{
private:
	web::http::client::http_client m_client;
	std::string m_partition;

public:
	RESTLocalStorage(const std::string & partition) : m_client(U("http://localhost:12345/api/v1/cache")), m_partition(partition)
	{
		
	}

	static PYXPointer<RESTLocalStorage> create(const std::string & partition)
	{
		return PYXNEW(RESTLocalStorage,partition);
	}


private:
	pplx::task<bool> set(const std::string & key,const PYXConstBufferSlice & slice)
	{
		std::vector<unsigned char> data(slice.begin(),slice.end());

		web::uri_builder builder;
		builder.append_path(utility::conversions::to_string_t(m_partition));
		builder.append_path(utility::conversions::to_string_t(key),true);

		web::http::http_request request(web::http::methods::POST);
		request.set_request_uri(builder.to_uri());
		request.set_body(data);

		auto task = m_client.request(request)
		.then([&key](pplx::task<web::http::http_response> previousTask) -> bool {
			auto response = previousTask.get();
			auto code = response.status_code();
			if (code == web::http::status_codes::OK || code == web::http::status_codes::Created)
			{
				return true;
			}

			PYXTHROW(PYXHttpException, "Unable to set key " << key << ", HTTP code: " << response.status_code());
		});

		return task;
	}

public:
	virtual std::auto_ptr<PYXConstWireBuffer> get(const std::string & key)
	{
		web::uri_builder builder;
		builder.append_path(utility::conversions::to_string_t(m_partition));
		builder.append_path(utility::conversions::to_string_t(key),true);

		web::http::http_request request(web::http::methods::GET);
		request.set_request_uri(builder.to_uri());

		std::auto_ptr<PYXConstWireBuffer> result;

		auto task = m_client.request(request)
		.then([&result,&key](pplx::task<web::http::http_response> previousTask) -> bool {
			auto response = previousTask.get();
			if (response.status_code() == web::http::status_codes::OK)
			{
				auto responseBody = response.extract_vector().get();

				PYXPointer<PYXConstBuffer> buffer = PYXConstBuffer::create(reinterpret_cast<char *>(&responseBody[0]),responseBody.size());
				result = std::auto_ptr<PYXConstWireBuffer>(new PYXConstWireBuffer(buffer));
				return true;
			} 
			else if (response.status_code() == web::http::status_codes::NotFound)
			{
				return false;
			}

			PYXTHROW(PYXHttpException, "Unable to get key " << key << ", HTTP code: " << response.status_code());
		});

		task.wait();
		return result;
	}

	virtual void set(const std::string &key, PYXWireBuffer & data)
	{
		set(key,*data.getBuffer()).wait();
	}

	virtual void setMany(const std::map<std::string,PYXConstBufferSlice> & data)
	{
		std::vector<pplx::task<bool>> tasks;
		tasks.reserve(10);

		for(auto & entry : data)
		{
			tasks.push_back(set(entry.first,entry.second));
			if (tasks.size() == 10)
			{
				pplx::when_all(tasks.begin(),tasks.end()).wait();
				tasks.clear();
			}
		}

		if (tasks.size()>0)
		{
			pplx::when_all(tasks.begin(),tasks.end()).wait();
		}
	}

	virtual void remove(const std::string & key)
	{
		//TODO
	}

	virtual void applyChanges(const std::vector<PYXPointer<PYXLocalStorageChange>> & changes)
	{
		std::map<std::string,PYXConstBufferSlice> sets;

		for(auto & change : changes) 
		{
			if (change->getChangeType() == PYXLocalStorageChange::knSet)
			{
				sets[change->getKey()] = *change->getData();
			}
		}

		setMany(sets);

		for(auto & change : changes) 
		{
			switch(change->getChangeType()) 
			{
			case PYXLocalStorageChange::knSet:
				//do nothing this time around
				break;
			case PYXLocalStorageChange::knRemove:
				remove(change->getKey());
				break;
			case PYXLocalStorageChange::knRemoveAll:
				removeAll();
				break;
			default:
				PYXTHROW(PYXException,"unknown change type");
			}
		}
	};

	virtual void removeAll()
	{
		//TODO
	};
};


PYXPointer<PYXLocalStorage> PYXLocalStorageFactory::createSqlite(const std::string & file)
{
	return PYXLocalStorageSqlite::create(file); 
}

PYXPointer<PYXLocalStorage> PYXLocalStorageFactory::createREST(const std::string & partition)
{
	return RESTLocalStorage::create(partition); 
}


PYXPointer<PYXLocalStorage> PYXLocalStorageFactory::create(const std::string & dbPath)
{
	if (AppServices::getConfiguration(AppServicesConfiguration::localStorageFormat) == AppServicesConfiguration::localStorageFormat_files)
	{
		return VerySimpleFileBasedLocalStorage::create(dbPath + "\\keys" );
	}
	else
	{
		return PYXLocalStorageSqlite::create(dbPath + "\\processLocalData.sqlite3" );
	}
}
