#ifndef PYXIS__PIPE__PYXNET_CHANNEL_H
#define PYXIS__PIPE__PYXNET_CHANNEL_H
/******************************************************************************
pyxnet_channel.h

begin		: July 13, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
// pyxlib includes
#include "pyxlib.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/wire_buffer.h"
#include "pyxis/utility/thread_pool.h"

///////////////////////////////////////////////////////////////////////////////
// PYXNETChannelKeyProvider
///////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXNETChannelKeyProvider : public PYXObject
{
public:
	virtual std::string getKey(const std::string & key)
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}
};

///////////////////////////////////////////////////////////////////////////////
// PYXNETChannel
///////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXNETChannel : public PYXObject
{
public:
	typedef PYXTaskSourceWithResult< PYXPointer<PYXConstBuffer> > GetKeyTaskResult;

	class Handle : public PYXObject
	{
		friend class PYXNETChannelProvider;
		friend class PYXNETChannel;

	public:
		static PYXPointer<Handle> create(const ProcRef & processProcRef, const std::string & code)
		{
			return PYXNEW(Handle,processProcRef,code);
		}

		Handle(const ProcRef & processProcRef, const std::string & code);		

	private:
		bool m_foundRemotely;
		bool m_published;

		boost::detail::atomic_count m_refCount;

		ProcRef m_procRef;
		std::string m_dataCode;
		int m_handle;

		typedef std::multimap< std::string, PYXPointer<GetKeyTaskResult> > PendingTaskMap;
		PendingTaskMap m_pendingTasks;

		boost::recursive_mutex m_mutex;
	};

	friend class PYXNETChannelProvider;

private:
	static PYXPointer<PYXNETChannel> create(const ProcRef & processProcRef, const std::string & code);	
	static PYXPointer<PYXNETChannel> create(const PYXPointer<Handle> & handle);	

	PYXNETChannel(const ProcRef & processProcRef, const std::string &code);
	PYXNETChannel(const PYXPointer<Handle> & handle);	

public:
	virtual ~PYXNETChannel();


	void publish();
	void unpublish();

	//TODO: this is not implemented, no one is setting m_published
	bool isPublished() { return m_handle->m_published; }

	//TODO: this is not implemented, no one is setting m_published
	bool wasFoundRemotely() { return m_handle->m_foundRemotely; }

	const ProcRef & getProcRef() const;
	const std::string & getDataCode() const;
	int getHandle() const;

public:
	std::auto_ptr<PYXConstWireBuffer> getKey(const std::string & key)
	{
		return std::auto_ptr<PYXConstWireBuffer>(new PYXConstWireBuffer(requestKey(key)->getResult()));
	}

	PYXPointer<GetKeyTaskResult> requestKey(const std::string & key);

	void attachLocalProvider(const PYXPointer<PYXNETChannelKeyProvider> & provider);

protected:
	void keyProvided(const std::string & key,const std::string & value);
	void keyProvidedFailed(const std::string & key);

	static void addPendingTask(PYXPointer<PYXNETChannel::Handle> handle, const std::string & key,PYXPointer<GetKeyTaskResult> task);
	static void removePendingTask(PYXPointer<PYXNETChannel::Handle> handle, std::string key,PYXPointer<GetKeyTaskResult> task);

private:
	void channelWasPublished();
	void channelFoundRemotely();

private:
	PYXPointer<Handle> m_handle;
};

///////////////////////////////////////////////////////////////////////////////
// PYXNETChannelProvider
///////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXNETChannelProvider : public PYXObject
{
	friend class PYXNETChannel;

private:
	typedef std::map< int,PYXPointer<PYXNETChannel::Handle> > ChannelsMapByHandle;	

	ChannelsMapByHandle m_channelByHandle;	
	boost::recursive_mutex m_mutex;

public:
	PYXPointer<PYXNETChannel> getOrCreateChannel(const ProcRef & processProcRef, const std::string & code );

private:
	PYXPointer<PYXNETChannel> getChannel(int channelId);
	void releaseChannel(int channelId);

//SWIG Director API...
protected: 
	virtual int createChannel(const ProcRef & processProcRef,const std::string & code);
	virtual void removeChannel(int channelId);

	virtual void requestKey(int channelId, const std::string & key);
	virtual void cancelRequestKey(int channelId, const std::string & key);

	virtual void addLocalProvider(int channelId, PYXPointer<PYXNETChannelKeyProvider> provider);

	virtual void publishChannel(int channelId);
	virtual void unpublishChannel(int channelId);

	void onChannelFoundRemotely(int channelId);
	void onChannelPublished(int channelId);

public:
	void keyProvidedFailed(int channelId, const std::string & key);
	void keyProvided(int channelId, const std::string & key,const std::string & dataBase64);

// SWIG doesn't know about addRef and release, since they are defined in 
// the opaque PYXObject.  Add them here so they get director'ed.
public:

	virtual long release() const
	{
		return PYXObject::release();
	}

	virtual long addRef() const
	{
		return PYXObject::addRef();
	}

public:

	//! Virtual destructor.
	virtual ~PYXNETChannelProvider()
	{}

private:

	static PYXPointer<PYXNETChannelProvider> m_spProvider;

public:

	static PYXPointer<PYXNETChannelProvider> getInstance();
	static void setInstance( PYXPointer<PYXNETChannelProvider> spProvider);
};

#endif // guard
