/*
 *  The contents of this file are subject to the Initial
 *  Developer's Public License Version 1.0 (the "License");
 *  you may not use this file except in compliance with the
 *  License. You may obtain a copy of the License at
 *  http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl.
 *
 *  Software distributed under the License is distributed AS IS,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *  See the License for the specific language governing rights
 *  and limitations under the License.
 *
 *  The Original Code was created by Adriano dos Santos Fernandes
 *  for the Firebird Open Source RDBMS project.
 *
 *  Copyright (c) 2008 Adriano dos Santos Fernandes <adrianosf@uol.com.br>
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
 */

#include "firebird.h"
#include "../jrd/ibase.h"
#include "firebird/UdrEngine.h"
#include "firebird/UdrCppEngine.h"
#include "firebird/Interface.h"
#include "../common/classes/alloc.h"
#include "../common/classes/array.h"
#include "../common/classes/init.h"
#include "../common/classes/fb_string.h"
#include "../common/classes/GenericMap.h"
#include "../common/classes/objects_array.h"
#include "../common/os/mod_loader.h"
#include "../common/os/path_utils.h"
#include "../common/classes/ImplementHelper.h"
#include "../common/StatusHolder.h"


namespace Firebird
{
	namespace Udr
	{
//------------------------------------------------------------------------------


struct Node
{
	Node()
		: name(*getDefaultMemoryPool()),
		  module(*getDefaultMemoryPool())
	{
	}

	string name;
	PathName module;
};

struct FunctionNode : public Node
{
	FunctionFactory* factory;
	FunctionNode* next;
};

struct ProcedureNode : public Node
{
	ProcedureFactory* factory;
	ProcedureNode* next;
};

struct TriggerNode : public Node
{
	TriggerFactory* factory;
	TriggerNode* next;
};


static GlobalPtr<ObjectsArray<PathName> > paths;

class Engine : public StdPlugin<Api::ExternalEngineImpl<Engine> >
{
public:
	explicit Engine(IPluginConfig* par)
		: functions(getPool()),
		  procedures(getPool()),
		  triggers(getPool())
	{
		LocalStatus s;
		RefPtr<IConfig> defaultConfig(REF_NO_INCR, par->getDefaultConfig(&s));
		check(&s);

		if (defaultConfig)
		{
			// this plugin is not ready to support different configurations
			// therefore keep legacy approach

			RefPtr<IConfigEntry> icp;

			for (int n = 0; icp.assignRefNoIncr(defaultConfig->findPos(&s, "path", n)); ++n)
			{
				check(&s);

				PathName newPath(icp->getValue());
				bool found = false;

				for (ObjectsArray<PathName>::iterator i = paths->begin(); i != paths->end(); ++i)
				{
					if (*i == newPath)
					{
						found = true;
						break;
					}
				}

				if (!found)
					paths->add(newPath);
			}
		}
	}

	int release()
	{
		if (--refCounter == 0)
		{
			delete this;
			return 0;
		}

		return 1;
	}

public:
	void loadModule(IRoutineMetadata* metadata, PathName* moduleName, string* entryPoint);
	template <typename NodeType, typename ObjType, typename SharedObjType> ObjType* getChild(
		IStatus* status, GenericMap<Pair<NonPooled<IExternalContext*, ObjType*> > >& children,
		SharedObjType* sharedObj, IExternalContext* context, NodeType* nodes,
		SortedArray<SharedObjType*>& sharedObjs, const PathName& moduleName);
	template <typename ObjType> void deleteChildren(
		GenericMap<Pair<NonPooled<IExternalContext*, ObjType*> > >& children);

	template <typename T> T* findNode(T* nodes, const PathName& moduleName,
		const string& entryPoint);

private:
	template <typename T, typename T2> T2* getNode(IStatus* status, T* nodes,
		const PathName& moduleName, IExternalContext* context, IRoutineMetadata* metadata,
		const string& entryPoint);

public:
	void open(IStatus* status, IExternalContext* context, Utf8* name, unsigned nameSize);
	void openAttachment(IStatus* status, IExternalContext* context);
	void closeAttachment(IStatus* status, IExternalContext* context);
	IExternalFunction* makeFunction(IStatus* status, IExternalContext* context,
		IRoutineMetadata* metadata, IMetadataBuilder* inBuilder, IMetadataBuilder* outBuilder);
	IExternalProcedure* makeProcedure(IStatus* status, IExternalContext* context,
		IRoutineMetadata* metadata, IMetadataBuilder* inBuilder, IMetadataBuilder* outBuilder);
	IExternalTrigger* makeTrigger(IStatus* status, IExternalContext* context,
		IRoutineMetadata* metadata, IMetadataBuilder* fieldsBuilder);

public:
	void dispose();

private:
	Mutex childrenMutex;

public:
	SortedArray<class SharedFunction*> functions;
	SortedArray<class SharedProcedure*> procedures;
	SortedArray<class SharedTrigger*> triggers;
};


class ModulesMap : public GenericMap<Pair<Left<PathName, ModuleLoader::Module*> > >
{
public:
	explicit ModulesMap(MemoryPool& p)
		: GenericMap<Pair<Left<PathName, ModuleLoader::Module*> > >(p)
	{
	}

	~ModulesMap();
};


//--------------------------------------


static GlobalPtr<Mutex> modulesMutex;
static GlobalPtr<ModulesMap> modules;

static InitInstance<PathName> loadingModule;
static FunctionNode* registeredFunctions = NULL;
static ProcedureNode* registeredProcedures = NULL;
static TriggerNode* registeredTriggers = NULL;


//--------------------------------------


class SharedFunction : public DisposeIface<Api::ExternalFunctionImpl<SharedFunction> >
{
public:
	SharedFunction(IStatus* status, Engine* aEngine, IExternalContext* context,
				IRoutineMetadata* aMetadata,
				IMetadataBuilder* inBuilder, IMetadataBuilder* outBuilder)
		: engine(aEngine),
		  metadata(aMetadata),
		  moduleName(*getDefaultMemoryPool()),
		  entryPoint(*getDefaultMemoryPool()),
		  info(*getDefaultMemoryPool()),
		  children(*getDefaultMemoryPool())
	{
		engine->loadModule(metadata, &moduleName, &entryPoint);
		FunctionNode* node = engine->findNode<FunctionNode>(
			registeredFunctions, moduleName, entryPoint);
		node->factory->setup(status, context, metadata, inBuilder, outBuilder);
	}

	~SharedFunction()
	{
		engine->deleteChildren(children);
	}

public:
	void dispose()
	{
		delete this;
	}

public:
	void getCharSet(IStatus* status, IExternalContext* context,
		Utf8* name, unsigned nameSize)
	{
		strncpy(name, context->getClientCharSet(), nameSize);

		try
		{
			IExternalFunction* function = engine->getChild<FunctionNode, ExternalFunction>(status,
				children, this, context, registeredFunctions, engine->functions, moduleName);
			if (function)
				function->getCharSet(status, context, name, nameSize);
		}
		catch (const StatusException& e)
		{
			e.stuff(status);
		}
	}

	void execute(IStatus* status, IExternalContext* context, void* inMsg, void* outMsg)
	{
		IExternalFunction* function = engine->getChild<FunctionNode, ExternalFunction>(status,
			children, this, context, registeredFunctions, engine->functions, moduleName);
		if (function)
			function->execute(status, context, inMsg, outMsg);
	}

public:
	Engine* engine;
	IRoutineMetadata* metadata;
	PathName moduleName;
	string entryPoint;
	string info;
	GenericMap<Pair<NonPooled<IExternalContext*, IExternalFunction*> > > children;
};


//--------------------------------------


class SharedProcedure : public DisposeIface<Api::ExternalProcedureImpl<SharedProcedure> >
{
public:
	SharedProcedure(IStatus* status, Engine* aEngine, IExternalContext* context,
				IRoutineMetadata* aMetadata,
				IMetadataBuilder* inBuilder, IMetadataBuilder* outBuilder)
		: engine(aEngine),
		  metadata(aMetadata),
		  moduleName(*getDefaultMemoryPool()),
		  entryPoint(*getDefaultMemoryPool()),
		  info(*getDefaultMemoryPool()),
		  children(*getDefaultMemoryPool())
	{
		engine->loadModule(metadata, &moduleName, &entryPoint);
		ProcedureNode* node = engine->findNode<ProcedureNode>(
			registeredProcedures, moduleName, entryPoint);
		node->factory->setup(status, context, metadata, inBuilder, outBuilder);
	}

	~SharedProcedure()
	{
		engine->deleteChildren(children);
	}

public:
	void dispose()
	{
		delete this;
	}

public:
	void getCharSet(IStatus* status, IExternalContext* context,
		Utf8* name, unsigned nameSize)
	{
		strncpy(name, context->getClientCharSet(), nameSize);

		try
		{
			IExternalProcedure* procedure = engine->getChild<ProcedureNode, ExternalProcedure>(status,
				children, this, context, registeredProcedures, engine->procedures, moduleName);
			if (procedure)
				procedure->getCharSet(status, context, name, nameSize);
		}
		catch (const StatusException& e)
		{
			e.stuff(status);
		}
	}

	IExternalResultSet* open(IStatus* status, IExternalContext* context,
		void* inMsg, void* outMsg)
	{
		try
		{
			IExternalProcedure* procedure = engine->getChild<ProcedureNode, ExternalProcedure>(status,
				children, this, context, registeredProcedures, engine->procedures, moduleName);
			return procedure ? procedure->open(status, context, inMsg, outMsg) : NULL;
		}
		catch (const StatusException& e)
		{
			e.stuff(status);
			return NULL;
		}
	}

public:
	Engine* engine;
	IRoutineMetadata* metadata;
	PathName moduleName;
	string entryPoint;
	string info;
	GenericMap<Pair<NonPooled<IExternalContext*, IExternalProcedure*> > > children;
};


//--------------------------------------


class SharedTrigger : public DisposeIface<Api::ExternalTriggerImpl<SharedTrigger> >
{
public:
	SharedTrigger(IStatus* status, Engine* aEngine, IExternalContext* context,
				IRoutineMetadata* aMetadata, IMetadataBuilder* fieldsBuilder)
		: engine(aEngine),
		  metadata(aMetadata),
		  moduleName(*getDefaultMemoryPool()),
		  entryPoint(*getDefaultMemoryPool()),
		  info(*getDefaultMemoryPool()),
		  children(*getDefaultMemoryPool())
	{
		engine->loadModule(metadata, &moduleName, &entryPoint);
		TriggerNode* node = engine->findNode<TriggerNode>(registeredTriggers, moduleName, entryPoint);
		node->factory->setup(status, context, metadata, fieldsBuilder);
	}

	~SharedTrigger()
	{
		engine->deleteChildren(children);
	}

public:
	void dispose()
	{
		delete this;
	}

public:
	void getCharSet(IStatus* status, IExternalContext* context,
		Utf8* name, unsigned nameSize)
	{
		strncpy(name, context->getClientCharSet(), nameSize);

		try
		{
			IExternalTrigger* trigger = engine->getChild<TriggerNode, ExternalTrigger>(status,
				children, this, context, registeredTriggers, engine->triggers, moduleName);
			if (trigger)
				trigger->getCharSet(status, context, name, nameSize);
		}
		catch (const StatusException& e)
		{
			e.stuff(status);
		}
	}

	void execute(IStatus* status, IExternalContext* context,
		unsigned action, void* oldMsg, void* newMsg)
	{
		IExternalTrigger* trigger = engine->getChild<TriggerNode, ExternalTrigger>(status,
			children, this, context, registeredTriggers, engine->triggers, moduleName);
		if (trigger)
			trigger->execute(status, context, action, oldMsg, newMsg);
	}

public:
	Engine* engine;
	IRoutineMetadata* metadata;
	PathName moduleName;
	string entryPoint;
	string info;
	GenericMap<Pair<NonPooled<IExternalContext*, IExternalTrigger*> > > children;
};


//--------------------------------------


extern "C" void FB_EXPORTED fbUdrRegFunction(const char* name, FunctionFactory* factory)
{
	FunctionNode* node = new FunctionNode();
	node->name = name;
	node->module = loadingModule();
	node->factory = factory;
	node->next = registeredFunctions;
	registeredFunctions = node;
}


extern "C" void FB_EXPORTED fbUdrRegProcedure(const char* name, ProcedureFactory* factory)
{
	ProcedureNode* node = new ProcedureNode();
	node->name = name;
	node->module = loadingModule();
	node->factory = factory;
	node->next = registeredProcedures;
	registeredProcedures = node;
}


extern "C" void FB_EXPORTED fbUdrRegTrigger(const char* name, TriggerFactory* factory)
{
	TriggerNode* node = new TriggerNode();
	node->name = name;
	node->module = loadingModule();
	node->factory = factory;
	node->next = registeredTriggers;
	registeredTriggers = node;
}


ModulesMap::~ModulesMap()
{
	while (registeredFunctions)
	{
		FunctionNode* del = registeredFunctions;
		registeredFunctions = registeredFunctions->next;
		delete del;
	}

	while (registeredProcedures)
	{
		ProcedureNode* del = registeredProcedures;
		registeredProcedures = registeredProcedures->next;
		delete del;
	}

	while (registeredTriggers)
	{
		TriggerNode* del = registeredTriggers;
		registeredTriggers = registeredTriggers->next;
		delete del;
	}

	Accessor accessor(this);
	for (bool cont = accessor.getFirst(); cont; cont = accessor.getNext())
		delete accessor.current()->second;
}


//--------------------------------------


void Engine::loadModule(IRoutineMetadata* metadata, PathName* moduleName, string* entryPoint)
{
	LocalStatus status;
	const string str(metadata->getEntryPoint(&status));
	StatusException::check(status.getErrors());

	const string::size_type pos = str.find('!');
	if (pos == string::npos)
	{
		static const ISC_STATUS statusVector[] = {
			isc_arg_gds,
			isc_random,
			isc_arg_string, (ISC_STATUS) "Invalid entry point",
			isc_arg_end
		};

		StatusException::check(statusVector);
	}

	*moduleName = PathName(str.substr(0, pos).c_str());
	// Do not allow module names with directory separators as a security measure.
	if (moduleName->find_first_of("/\\") != string::npos)
	{
		static const ISC_STATUS statusVector[] = {
			isc_arg_gds,
			isc_random,
			isc_arg_string, (ISC_STATUS) "Invalid module name",
			isc_arg_end
		};

		StatusException::check(statusVector);
	}

	*entryPoint = str.substr(pos + 1);

	string::size_type n = entryPoint->find('!');
	*entryPoint = (n == string::npos ? *entryPoint : entryPoint->substr(0, n));

	MutexLockGuard guard(modulesMutex, FB_FUNCTION);

	if (modules->exist(*moduleName))
		return;

	loadingModule() = *moduleName;

	for (ObjectsArray<PathName>::iterator i = paths->begin(); i != paths->end(); ++i)
	{
		PathName path;
		PathUtils::concatPath(path, *i, *moduleName);

		ModuleLoader::Module* module = ModuleLoader::fixAndLoadModule(path);

		if (module)
		{
			modules->put(*moduleName, module);
			break;
		}
		else
		{
			static const ISC_STATUS statusVector[] = {
				isc_arg_gds,
				isc_random,
				isc_arg_string, (ISC_STATUS) "Module not found",
				isc_arg_end
			};

			StatusException::check(statusVector);
		}
	}
}


template <typename NodeType, typename ObjType, typename SharedObjType> ObjType* Engine::getChild(
	IStatus* status, GenericMap<Pair<NonPooled<IExternalContext*, ObjType*> > >& children,
	SharedObjType* sharedObj, IExternalContext* context, NodeType* nodes,
	SortedArray<SharedObjType*>& sharedObjs, const PathName& moduleName)
{
	MutexLockGuard guard(childrenMutex, FB_FUNCTION);

	if (!sharedObjs.exist(sharedObj))
		sharedObjs.add(sharedObj);

	ObjType* obj;
	if (!children.get(context, obj))
	{
		obj = getNode<NodeType, ObjType>(status, nodes, moduleName, context, sharedObj->metadata,
			sharedObj->entryPoint);

		if (obj)
			children.put(context, obj);
	}

	return obj;
}


template <typename ObjType> void Engine::deleteChildren(
	GenericMap<Pair<NonPooled<IExternalContext*, ObjType*> > >& children)
{
	// No need to lock childrenMutex as if there are more threads simultaneously accessing
	// these children in this moment there will be a memory corruption anyway.

	typedef typename GenericMap<Pair<NonPooled<IExternalContext*, ObjType*> > >::Accessor ChildrenAccessor;
	ChildrenAccessor accessor(&children);
	for (bool found = accessor.getFirst(); found; found = accessor.getNext())
		accessor.current()->second->dispose();
}


template <typename T> T* Engine::findNode(T* nodes, const PathName& moduleName,
		const string& entryPoint)
{
	for (T* node = nodes; node; node = node->next)
	{
		if (node->module == moduleName && entryPoint == node->name)
			return node;
	}

	static const ISC_STATUS statusVector[] = {
		isc_arg_gds,
		isc_random,
		isc_arg_string, (ISC_STATUS) "Entry point not found",
		isc_arg_end
	};

	StatusException::check(statusVector);

	return NULL;
}


template <typename T, typename T2> T2* Engine::getNode(IStatus* status, T* nodes,
		const PathName& moduleName, IExternalContext* context, IRoutineMetadata* metadata,
		const string& entryPoint)
{
	T* node = findNode<T>(nodes, moduleName, entryPoint);
	return node->factory->newItem(status, context, metadata);
}


void Engine::open(IStatus* /*status*/, IExternalContext* /*context*/, Utf8* name, unsigned nameSize)
{
	strncpy(name, "UTF-8", nameSize);
}


void Engine::openAttachment(IStatus* /*status*/, IExternalContext* /*context*/)
{
}


void Engine::closeAttachment(IStatus* status, IExternalContext* context)
{
	MutexLockGuard guard(childrenMutex, FB_FUNCTION);

	for (SortedArray<SharedFunction*>::iterator i = functions.begin(); i != functions.end(); ++i)
	{
		IExternalFunction* function;
		if ((*i)->children.get(context, function))
		{
			function->dispose();
			(*i)->children.remove(context);
		}
	}

	for (SortedArray<SharedProcedure*>::iterator i = procedures.begin(); i != procedures.end(); ++i)
	{
		IExternalProcedure* procedure;
		if ((*i)->children.get(context, procedure))
		{
			procedure->dispose();
			(*i)->children.remove(context);
		}
	}

	for (SortedArray<SharedTrigger*>::iterator i = triggers.begin(); i != triggers.end(); ++i)
	{
		IExternalTrigger* trigger;
		if ((*i)->children.get(context, trigger))
		{
			trigger->dispose();
			(*i)->children.remove(context);
		}
	}
}


IExternalFunction* Engine::makeFunction(IStatus* status, IExternalContext* context,
	IRoutineMetadata* metadata, IMetadataBuilder* inBuilder, IMetadataBuilder* outBuilder)
{
	try
	{
		return new SharedFunction(status, this, context, metadata, inBuilder, outBuilder);
	}
	catch (const StatusException& e)
	{
		e.stuff(status);
		return NULL;
	}
}


IExternalProcedure* Engine::makeProcedure(IStatus* status, IExternalContext* context,
	IRoutineMetadata* metadata, IMetadataBuilder* inBuilder, IMetadataBuilder* outBuilder)
{
	try
	{
		return new SharedProcedure(status, this, context, metadata, inBuilder, outBuilder);
	}
	catch (const StatusException& e)
	{
		e.stuff(status);
		return NULL;
	}
}


IExternalTrigger* Engine::makeTrigger(IStatus* status, IExternalContext* context,
	IRoutineMetadata* metadata, IMetadataBuilder* fieldsBuilder)
{
	try
	{
		return new SharedTrigger(status, this, context, metadata, fieldsBuilder);
	}
	catch (const StatusException& e)
	{
		e.stuff(status);
		return NULL;
	}
}


void Engine::dispose()
{
	delete this;
}


//--------------------------------------


class IExternalEngineFactoryImpl : public SimpleFactory<Engine>
{
} factory;

extern "C" void FB_EXPORTED FB_PLUGIN_ENTRY_POINT(IMaster* master)
{
	CachedMasterInterface::set(master);

	PluginManagerInterfacePtr pi;
	pi->registerPluginFactory(IPluginManager::ExternalEngine, "UDR", &factory);
	getUnloadDetector()->registerMe();
}


//------------------------------------------------------------------------------
	}	// namespace Udr
}	// namespace Firebird
