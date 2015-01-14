/*
 * The contents of this file are subject to the Interbase Public
 * License Version 1.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy
 * of the License at http://www.Inprise.com/IPL.html
 *
 * Software distributed under the License is distributed on an
 * "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express
 * or implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code was created by Inprise Corporation
 * and its predecessors. Portions created by Inprise Corporation are
 * Copyright (C) Inprise Corporation.
 *
 * All Rights Reserved.
 * Contributor(s): ______________________________________.
 *
 * Dmitry Yemanov
 * Sean Leyne
 * Alex Peshkoff
 * Adriano dos Santos Fernandes
 *
 */

#ifndef YVALVE_Y_OBJECTS_H
#define YVALVE_Y_OBJECTS_H

#include "firebird.h"
#include "firebird/Interface.h"
#include "gen/iberror.h"
#include "../common/StatusHolder.h"
#include "../common/classes/alloc.h"
#include "../common/classes/array.h"
#include "../common/MsgMetadata.h"

namespace Firebird
{
	class ClumpletWriter;
}

namespace Why
{


class YAttachment;
class YBlob;
class YRequest;
class YResultSet;
class YService;
class YStatement;
class YTransaction;

class YObject
{
public:
	YObject()
		: handle(0)
	{
	}

protected:
	FB_API_HANDLE handle;
};

class CleanupCallback
{
public:
	virtual void cleanupCallbackFunction() = 0;
	virtual ~CleanupCallback() { }
};

template <typename T>
class HandleArray
{
public:
	explicit HandleArray(Firebird::MemoryPool& pool)
		: array(pool)
	{
	}

	void add(T* obj)
	{
		Firebird::MutexLockGuard guard(mtx, FB_FUNCTION);

		array.add(obj);
	}

	void remove(T* obj)
	{
		Firebird::MutexLockGuard guard(mtx, FB_FUNCTION);
		FB_SIZE_T pos;

		if (array.find(obj, pos))
			array.remove(pos);
	}

	void destroy(unsigned dstrFlags)
	{
		Firebird::MutexLockGuard guard(mtx, FB_FUNCTION);
		FB_SIZE_T i;

		while ((i = array.getCount()) > 0)
			array[i - 1]->destroy(dstrFlags);
	}

	void assign(HandleArray& from)
	{
		clear();
		array.assign(from.array);
	}

	void clear()
	{
		array.clear();
	}

private:
	Firebird::Mutex mtx;
	Firebird::SortedArray<T*> array;
};

template <typename Impl, typename Intf>
class YHelper : public Firebird::StdPlugin<Intf>, public YObject
{
public:
	typedef typename Intf::Declaration NextInterface;
	typedef YAttachment YRef;

	static const unsigned DF_RELEASE =		0x1;

	explicit YHelper(NextInterface* aNext);

	int release()
	{
		if (--this->refCounter == 0)
		{
			Impl* impl = static_cast<Impl*>(this);

			if (next)
			{
				impl->destroy(0);
			}

			delete impl; 	// call correct destructor
			return 0;
		}

		return 1;
	}

	void destroy2(unsigned dstrFlags)
	{
		RefDeb(Firebird::DEB_RLS_JATT, "YValve/destroy2");
		next = NULL;

		if (dstrFlags & DF_RELEASE)
		{
			RefDeb(Firebird::DEB_RLS_YATT, "destroy2");
			release();
		}
	}

	Firebird::RefPtr<NextInterface> next;
};

class YEvents FB_FINAL : public YHelper<YEvents, Firebird::IEventsImpl<YEvents, Firebird::CheckStatusWrapper> >
{
public:
	static const ISC_STATUS ERROR_CODE = isc_bad_events_handle;

	YEvents(YAttachment* aAttachment, Firebird::IEvents* aNext, Firebird::IEventCallback* aCallback);

	void destroy(unsigned dstrFlags);
	FB_API_HANDLE& getHandle();

	// IEvents implementation
	void cancel(Firebird::CheckStatusWrapper* status);

public:
	YAttachment* attachment;
	Firebird::RefPtr<Firebird::IEventCallback> callback;
};

class YRequest FB_FINAL : public YHelper<YRequest, Firebird::IRequestImpl<YRequest, Firebird::CheckStatusWrapper> >
{
public:
	static const ISC_STATUS ERROR_CODE = isc_bad_req_handle;

	YRequest(YAttachment* aAttachment, Firebird::IRequest* aNext);

	void destroy(unsigned dstrFlags);
	FB_API_HANDLE& getHandle();

	// IRequest implementation
	void receive(Firebird::CheckStatusWrapper* status, int level, unsigned int msgType,
		unsigned int length, unsigned char* message);
	void send(Firebird::CheckStatusWrapper* status, int level, unsigned int msgType,
		unsigned int length, const unsigned char* message);
	void getInfo(Firebird::CheckStatusWrapper* status, int level, unsigned int itemsLength,
		const unsigned char* items, unsigned int bufferLength, unsigned char* buffer);
	void start(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction, int level);
	void startAndSend(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction, int level,
		unsigned int msgType, unsigned int length, const unsigned char* message);
	void unwind(Firebird::CheckStatusWrapper* status, int level);
	void free(Firebird::CheckStatusWrapper* status);

public:
	YAttachment* attachment;
	FB_API_HANDLE* userHandle;
};

class YTransaction FB_FINAL : public YHelper<YTransaction, Firebird::ITransactionImpl<YTransaction, Firebird::CheckStatusWrapper> >
{
public:
	static const ISC_STATUS ERROR_CODE = isc_bad_trans_handle;

	YTransaction(YAttachment* aAttachment, Firebird::ITransaction* aNext);

	void destroy(unsigned dstrFlags);
	FB_API_HANDLE& getHandle();

	// ITransaction implementation
	void getInfo(Firebird::CheckStatusWrapper* status, unsigned int itemsLength,
		const unsigned char* items, unsigned int bufferLength, unsigned char* buffer);
	void prepare(Firebird::CheckStatusWrapper* status, unsigned int msgLength,
		const unsigned char* message);
	void commit(Firebird::CheckStatusWrapper* status);
	void commitRetaining(Firebird::CheckStatusWrapper* status);
	void rollback(Firebird::CheckStatusWrapper* status);
	void rollbackRetaining(Firebird::CheckStatusWrapper* status);
	void disconnect(Firebird::CheckStatusWrapper* status);
	Firebird::ITransaction* join(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction);
	Firebird::ITransaction* validate(Firebird::CheckStatusWrapper* status, Firebird::IAttachment* testAtt);
	YTransaction* enterDtc(Firebird::CheckStatusWrapper* status);

	void addCleanupHandler(Firebird::CheckStatusWrapper* status, CleanupCallback* callback);
	void selfCheck();

public:
	YAttachment* attachment;
	HandleArray<YBlob> childBlobs;
	HandleArray<YResultSet> childCursors;
	Firebird::Array<CleanupCallback*> cleanupHandlers;

private:
	YTransaction(YTransaction* from)
		: YHelper(from->next),
		  attachment(from->attachment),
		  childBlobs(getPool()),
		  childCursors(getPool()),
		  cleanupHandlers(getPool())
	{
		childBlobs.assign(from->childBlobs);
		from->childBlobs.clear();
		childCursors.assign(from->childCursors);
		from->childCursors.clear();
		cleanupHandlers.assign(from->cleanupHandlers);
		from->cleanupHandlers.clear();
	}
};

typedef Firebird::RefPtr<Firebird::ITransaction> NextTransaction;

class YBlob FB_FINAL : public YHelper<YBlob, Firebird::IBlobImpl<YBlob, Firebird::CheckStatusWrapper> >
{
public:
	static const ISC_STATUS ERROR_CODE = isc_bad_segstr_handle;

	YBlob(YAttachment* aAttachment, YTransaction* aTransaction, Firebird::IBlob* aNext);

	void destroy(unsigned dstrFlags);
	FB_API_HANDLE& getHandle();

	// IBlob implementation
	void getInfo(Firebird::CheckStatusWrapper* status, unsigned int itemsLength,
		const unsigned char* items, unsigned int bufferLength, unsigned char* buffer);
	int getSegment(Firebird::CheckStatusWrapper* status, unsigned int length, void* buffer,
								   unsigned int* segmentLength);
	void putSegment(Firebird::CheckStatusWrapper* status, unsigned int length, const void* buffer);
	void cancel(Firebird::CheckStatusWrapper* status);
	void close(Firebird::CheckStatusWrapper* status);
	int seek(Firebird::CheckStatusWrapper* status, int mode, int offset);

public:
	YAttachment* attachment;
	YTransaction* transaction;
};

class YResultSet FB_FINAL : public YHelper<YResultSet, Firebird::IResultSetImpl<YResultSet, Firebird::CheckStatusWrapper> >
{
public:
	static const ISC_STATUS ERROR_CODE = isc_bad_result_set;

	YResultSet(YAttachment* anAttachment, YTransaction* aTransaction, Firebird::IResultSet* aNext);
	YResultSet(YAttachment* anAttachment, YTransaction* aTransaction, YStatement* aStatement,
		Firebird::IResultSet* aNext);

	void destroy(unsigned dstrFlags);

	// IResultSet implementation
	int fetchNext(Firebird::CheckStatusWrapper* status, void* message);
	int fetchPrior(Firebird::CheckStatusWrapper* status, void* message);
	int fetchFirst(Firebird::CheckStatusWrapper* status, void* message);
	int fetchLast(Firebird::CheckStatusWrapper* status, void* message);
	int fetchAbsolute(Firebird::CheckStatusWrapper* status, unsigned int position, void* message);
	int fetchRelative(Firebird::CheckStatusWrapper* status, int offset, void* message);
	FB_BOOLEAN isEof(Firebird::CheckStatusWrapper* status);
	FB_BOOLEAN isBof(Firebird::CheckStatusWrapper* status);
	Firebird::IMessageMetadata* getMetadata(Firebird::CheckStatusWrapper* status);
	void close(Firebird::CheckStatusWrapper* status);
	void setDelayedOutputFormat(Firebird::CheckStatusWrapper* status, Firebird::IMessageMetadata* format);

public:
	YAttachment* attachment;
	YTransaction* transaction;
	YStatement* statement;
};

class YMetadata
{
public:
	explicit YMetadata(bool in)
		: flag(false), input(in)
	{ }

	Firebird::IMessageMetadata* get(Firebird::IStatement* next, YStatement* statement);

private:
	Firebird::RefPtr<Firebird::MsgMetadata> metadata;
	volatile bool flag;
	bool input;
};

class YStatement FB_FINAL : public YHelper<YStatement, Firebird::IStatementImpl<YStatement, Firebird::CheckStatusWrapper> >
{
public:
	static const ISC_STATUS ERROR_CODE = isc_bad_stmt_handle;

	YStatement(YAttachment* aAttachment, Firebird::IStatement* aNext);

	void destroy(unsigned dstrFlags);

	// IStatement implementation
	void getInfo(Firebird::CheckStatusWrapper* status,
		unsigned int itemsLength, const unsigned char* items,
		unsigned int bufferLength, unsigned char* buffer);
	unsigned getType(Firebird::CheckStatusWrapper* status);
	const char* getPlan(Firebird::CheckStatusWrapper* status, FB_BOOLEAN detailed);
	ISC_UINT64 getAffectedRecords(Firebird::CheckStatusWrapper* status);
	Firebird::IMessageMetadata* getInputMetadata(Firebird::CheckStatusWrapper* status);
	Firebird::IMessageMetadata* getOutputMetadata(Firebird::CheckStatusWrapper* status);
	Firebird::ITransaction* execute(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction,
		Firebird::IMessageMetadata* inMetadata, void* inBuffer,
		Firebird::IMessageMetadata* outMetadata, void* outBuffer);
	Firebird::IResultSet* openCursor(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction,
		Firebird::IMessageMetadata* inMetadata, void* inBuffer, Firebird::IMessageMetadata* outMetadata);
	void setCursorName(Firebird::CheckStatusWrapper* status, const char* name);
	void free(Firebird::CheckStatusWrapper* status);
	unsigned getFlags(Firebird::CheckStatusWrapper* status);

public:
	Firebird::Mutex statementMutex;
	YAttachment* attachment;
	YResultSet* cursor;

	Firebird::IMessageMetadata* getMetadata(bool in, Firebird::IStatement* next);

private:
	YMetadata input, output;
};

class EnterCount
{
public:
	EnterCount()
		: enterCount(0)
	{}

	~EnterCount()
	{
		fb_assert(enterCount == 0);
	}

	int enterCount;
	Firebird::Mutex enterMutex;
};

class YAttachment FB_FINAL : public YHelper<YAttachment, Firebird::IAttachmentImpl<YAttachment, Firebird::CheckStatusWrapper> >,
	public EnterCount
{
public:
	static const ISC_STATUS ERROR_CODE = isc_bad_db_handle;

	YAttachment(Firebird::IProvider* aProvider, Firebird::IAttachment* aNext,
		const Firebird::PathName& aDbPath);
	~YAttachment();

	void destroy(unsigned dstrFlags);
	void shutdown();
	FB_API_HANDLE& getHandle();

	// IAttachment implementation
	void getInfo(Firebird::CheckStatusWrapper* status, unsigned int itemsLength,
		const unsigned char* items, unsigned int bufferLength, unsigned char* buffer);
	YTransaction* startTransaction(Firebird::CheckStatusWrapper* status, unsigned int tpbLength,
		const unsigned char* tpb);
	YTransaction* reconnectTransaction(Firebird::CheckStatusWrapper* status, unsigned int length,
		const unsigned char* id);
	YRequest* compileRequest(Firebird::CheckStatusWrapper* status, unsigned int blrLength,
		const unsigned char* blr);
	void transactRequest(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction,
		unsigned int blrLength, const unsigned char* blr, unsigned int inMsgLength,
		const unsigned char* inMsg, unsigned int outMsgLength, unsigned char* outMsg);
	YBlob* createBlob(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction, ISC_QUAD* id,
		unsigned int bpbLength, const unsigned char* bpb);
	YBlob* openBlob(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction, ISC_QUAD* id,
		unsigned int bpbLength, const unsigned char* bpb);
	int getSlice(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction, ISC_QUAD* id,
		unsigned int sdlLength, const unsigned char* sdl, unsigned int paramLength,
		const unsigned char* param, int sliceLength, unsigned char* slice);
	void putSlice(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction, ISC_QUAD* id,
		unsigned int sdlLength, const unsigned char* sdl, unsigned int paramLength,
		const unsigned char* param, int sliceLength, unsigned char* slice);
	void executeDyn(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction, unsigned int length,
		const unsigned char* dyn);
	YStatement* prepare(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* tra,
		unsigned int stmtLength, const char* sqlStmt, unsigned int dialect, unsigned int flags);
	Firebird::ITransaction* execute(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction,
		unsigned int stmtLength, const char* sqlStmt, unsigned int dialect,
		Firebird::IMessageMetadata* inMetadata, void* inBuffer,
		Firebird::IMessageMetadata* outMetadata, void* outBuffer);
	Firebird::IResultSet* openCursor(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* transaction,
		unsigned int stmtLength, const char* sqlStmt, unsigned int dialect,
		Firebird::IMessageMetadata* inMetadata, void* inBuffer, Firebird::IMessageMetadata* outMetadata,
		const char* cursorName);
	YEvents* queEvents(Firebird::CheckStatusWrapper* status, Firebird::IEventCallback* callback,
		unsigned int length, const unsigned char* eventsData);
	void cancelOperation(Firebird::CheckStatusWrapper* status, int option);
	void ping(Firebird::CheckStatusWrapper* status);
	void detach(Firebird::CheckStatusWrapper* status);
	void dropDatabase(Firebird::CheckStatusWrapper* status);

	void addCleanupHandler(Firebird::CheckStatusWrapper* status, CleanupCallback* callback);
	YTransaction* getTransaction(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* tra);
	void getNextTransaction(Firebird::CheckStatusWrapper* status, Firebird::ITransaction* tra, NextTransaction& next);
	void execute(Firebird::CheckStatusWrapper* status, FB_API_HANDLE* traHandle,
		unsigned int stmtLength, const char* sqlStmt, unsigned int dialect,
		Firebird::IMessageMetadata* inMetadata, void* inBuffer,
		Firebird::IMessageMetadata* outMetadata, void* outBuffer);

public:
	Firebird::IProvider* provider;
	Firebird::PathName dbPath;
	HandleArray<YBlob> childBlobs;
	HandleArray<YEvents> childEvents;
	HandleArray<YRequest> childRequests;
	HandleArray<YStatement> childStatements;
	HandleArray<YTransaction> childTransactions;
	Firebird::Array<CleanupCallback*> cleanupHandlers;
	Firebird::StatusHolder savedStatus;	// Do not use raise() method of this class in yValve.
};

class YService FB_FINAL : public YHelper<YService, Firebird::IServiceImpl<YService, Firebird::CheckStatusWrapper> >,
	public EnterCount
{
public:
	static const ISC_STATUS ERROR_CODE = isc_bad_svc_handle;

	YService(Firebird::IProvider* aProvider, Firebird::IService* aNext, bool utf8);
	~YService();

	void shutdown();
	void destroy(unsigned dstrFlags);
	FB_API_HANDLE& getHandle();

	// IService implementation
	void detach(Firebird::CheckStatusWrapper* status);
	void query(Firebird::CheckStatusWrapper* status,
		unsigned int sendLength, const unsigned char* sendItems,
		unsigned int receiveLength, const unsigned char* receiveItems,
		unsigned int bufferLength, unsigned char* buffer);
	void start(Firebird::CheckStatusWrapper* status,
		unsigned int spbLength, const unsigned char* spb);

public:
	typedef Firebird::IService NextInterface;
	typedef YService YRef;

private:
	Firebird::IProvider* provider;
	bool utf8Connection;		// Client talks to us using UTF8, else - system default charset
};

class Dispatcher FB_FINAL : public Firebird::StdPlugin<Firebird::IProviderImpl<Dispatcher, Firebird::CheckStatusWrapper> >
{
public:
	Dispatcher()
		: cryptCallback(NULL)
	{ }

	// IProvider implementation
	YAttachment* attachDatabase(Firebird::CheckStatusWrapper* status, const char* filename,
		unsigned int dpbLength, const unsigned char* dpb);
	YAttachment* createDatabase(Firebird::CheckStatusWrapper* status, const char* filename,
		unsigned int dpbLength, const unsigned char* dpb);
	YService* attachServiceManager(Firebird::CheckStatusWrapper* status, const char* serviceName,
		unsigned int spbLength, const unsigned char* spb);
	void shutdown(Firebird::CheckStatusWrapper* status, unsigned int timeout, const int reason);
	void setDbCryptCallback(Firebird::CheckStatusWrapper* status,
		Firebird::ICryptKeyCallback* cryptCallback);

	int release()
	{
		if (--refCounter == 0)
		{
			delete this;
			return 0;
		}

		return 1;
	}

private:
	YAttachment* attachOrCreateDatabase(Firebird::CheckStatusWrapper* status, bool createFlag,
		const char* filename, unsigned int dpbLength, const unsigned char* dpb);

	Firebird::ICryptKeyCallback* cryptCallback;
};

class UtilInterface FB_FINAL : public Firebird::AutoIface<Firebird::IUtilImpl<UtilInterface, Firebird::CheckStatusWrapper> >
{
	// IUtil implementation
public:
	void getFbVersion(Firebird::CheckStatusWrapper* status, Firebird::IAttachment* att,
		Firebird::IVersionCallback* callback);
	void loadBlob(Firebird::CheckStatusWrapper* status, ISC_QUAD* blobId, Firebird::IAttachment* att,
		Firebird::ITransaction* tra, const char* file, FB_BOOLEAN txt);
	void dumpBlob(Firebird::CheckStatusWrapper* status, ISC_QUAD* blobId, Firebird::IAttachment* att,
		Firebird::ITransaction* tra, const char* file, FB_BOOLEAN txt);
	void getPerfCounters(Firebird::CheckStatusWrapper* status, Firebird::IAttachment* att,
		const char* countersSet, ISC_INT64* counters);			// in perf.cpp
	YAttachment* executeCreateDatabase(Firebird::CheckStatusWrapper* status,
		unsigned stmtLength, const char* creatDBstatement, unsigned dialect,
		FB_BOOLEAN* stmtIsCreateDb = NULL);
};

}	// namespace Why

#endif	// YVALVE_Y_OBJECTS_H
