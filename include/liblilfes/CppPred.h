/*
 * $Id: CppPred.h,v 1.3 2008-07-21 06:19:39 mkurita Exp $
 *
 *		some copyright notice should come here
 *
 * CppPred.h:
 *		A set of classes to define your own predicate and load it through 
 *	a shared object file. See the examle code at the bottom of this file 
 * 	for the usage.
 *
 */

#ifndef CppPred_h__
#define CppPred_h__

#include "lconfig.h"
#include "ftype.h"
#include "builtin.h"
#include "option.h"

#include <cassert>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace lilfes {

//------------------------------------------------------------------------------
// Helper classes: Don't use what are defined inside of this namespace directly
//------------------------------------------------------------------------------
namespace CppPredImpl {

	//--------------------------------------------------------------------------
	// A template trick to put the arguments on the stack.
	// (Replacement of NGETARGS macro in builtin.h)
	//--------------------------------------------------------------------------
	template<unsigned NumArg, unsigned ArgIx>
	struct GetArgsLoop {
		static void go(lilfes::machine &m, lilfes::FSP argv[])
		{
#ifdef DOASSERT
			m.SetRP(ArgIx + 1);
#endif
			argv[ArgIx] = lilfes::FSP(m, c2PTR(m.ReadLocal(ArgIx)));
			GetArgsLoop<NumArg, ArgIx + 1>::go(m, argv);
		}
	};

	template<unsigned NumArg>
	struct GetArgsLoop<NumArg, NumArg> {
		static void go(lilfes::machine &, lilfes::FSP []) {}
	};
	
	template<unsigned NumArg>
	inline
	void getArgs(lilfes::machine &m, lilfes::FSP argv[])
	{
		GetArgsLoop<NumArg, 0>::go(m, argv);
	}
	
	//--------------------------------------------------------------------------
	// Base class of the predicate class
	//--------------------------------------------------------------------------
	template<unsigned Arity>
	class PredBase : public lilfes::procthunk {

	private:
		const std::string _predName;
		const std::string _className;

	protected: /// You cannot instantiate this class directly
		PredBase(const std::string predName)
			: procthunk(Arity)
			, _predName(predName)
			, _className("builtin(" + predName + ")")
		{}
	
	public:
		const char *ClassName(void) { return _className.c_str(); }
		const char *PredName(void) { return _predName.c_str(); }
	};

} // namespace CppPredImpl


//------------------------------------------------------------------------------
// Base predicate classes
//
// LIMITATION:
//		You cannot define a set of predicates which have the same name but
//  take different number of arguments, like some_pred/1 and some_pred/2.
//  Use LILFES_BUILTIN_PRED_OVERLOAD_* macros in builtin.h for such purpose.
//------------------------------------------------------------------------------
template<unsigned Arity>
class Pred : public CppPredImpl::PredBase<Arity> {

protected: /// You cannot instantiate this class directly
	Pred(const std::string &name) : CppPredImpl::PredBase<Arity>(name) {}

public:
	/// Override this function in your derived class.
	/// You can assume the number of arguments passed via argv matches 
	/// the template parameter 'Arity'.
	virtual bool exec(lilfes::machine &mach, lilfes::FSP *argv) = 0;

	/// Inherited from lilfes::procthunk
	/// Don't override this function in your derived class
	void execwork(lilfes::machine &mach)
	{
		P2(ClassName());
		lilfes::FSP argv[Arity];
		CppPredImpl::getArgs<Arity>(mach, argv);
		if (! exec(mach, argv)) {
			mach.Fail();
		}
	}
};

template<>
class Pred<0> : public CppPredImpl::PredBase<0> {

protected: /// You cannot instantiate this class directly
	Pred<0>(const std::string &name) : CppPredImpl::PredBase<0>(name) {}

public:
	/// Override this function
	virtual bool exec(lilfes::machine &mach) = 0;

	/// Inherited from lilfes::procthunk
	/// Don't override this function in your derived class
	void execwork(lilfes::machine &mach)
	{
		P2(ClassName());
		if (! exec(mach)) {
			mach.Fail();
		}
	}
};

//------------------------------------------------------------------------------
// Module declaration class:
//  It does (approximately) what a ":- module" directive does in lilfes code
//------------------------------------------------------------------------------
class ModuleDecl {
public:
	ModuleDecl(const std::string &moduleName)
		: _firstLoad(false)
	{
		using namespace lilfes;

		if (module::CurrentModule()->IsRegistered()) {
			throw std::runtime_error(
				"ModuleDecl objects ("
				+ moduleName 
				+ ") were instatiated twice in a .so module");
		}

		if (moduleName.empty()) {
			throw std::runtime_error(
				"ModuleDecl: module name shold not be an empty string");
		}

		module *curr = module::CurrentModule();
		module *loaded = module::SearchModule(moduleName.c_str());

		if (loaded) {

			_firstLoad = false;

			if (moduleName == "core" || moduleName == "builtin") {
				throw std::runtime_error(
					"Built-in module "
					+ moduleName
					+ " cannot be redifined");
			}

			int warnFlag
				= lilfes_flag::Search("warn-module-override")->GetIntVal();
			if (loaded->GetDefName() != curr->GetDefName() && warnFlag) {
				inwarn(0)
					<< "Same module name \""
					<< moduleName
					<< " with the different paths; "
					   "the old module will be invalidated" << endmsg;
				inwarn(0) << loaded->GetDefName() << endmsg;
				inwarn(0) << curr->GetDefName() << endmsg;
			}
			else {
				module *prevModule = curr->GetPrevModule();
				if (prevModule) {
					prevModule->Import(loaded);
					curr->SetPrevModule(0); /// avoid re-import in Finish()
				}
			}
		}
		else {
			_firstLoad = true;
			curr->Register(moduleName.c_str());
		}
	}

	bool firstLoad(void) const { return _firstLoad; }

private:
	bool _firstLoad;
};

//------------------------------------------------------------------------------
// Module Initializer:
//   You can do what you want to do, such as type definitions, in the exec() 
//  function of your derived class.
//------------------------------------------------------------------------------
class ModuleInitializer {
public:
	virtual ~ModuleInitializer(void) {}
	virtual void exec(lilfes::module *module) = 0;
};

//------------------------------------------------------------------------------
// Registrator class
//------------------------------------------------------------------------------
class Registrator {
public:
	template<unsigned Arity>
	Registrator(const ModuleDecl &mdecl, CppPredImpl::PredBase<Arity> *pred)
	{
		using namespace lilfes;

		if (! mdecl.firstLoad()) {
			return;
		}

		module::UseCurrentModule();
		module::CurrentModule()->SetInterfaceMode(true); /// we need this?

		type *t = new type(pred->PredName(), module::CurrentModule());
		t->SetAsChildOf(t_pred[Arity]);
		t->Fix();
		proc_builtin *proc = new proc_builtin(t, Arity, pred);
		proc = 0; /// dummy to stop warning from compliers
	}

	Registrator(const ModuleDecl &mdecl, ModuleInitializer *initer)
	{
		using namespace lilfes;

		if (! mdecl.firstLoad()) {
			return;
		}

		module::UseCurrentModule();
		module::CurrentModule()->SetInterfaceMode(true); /// we need this?

		initer->exec(module::CurrentModule());
	}
};

} // namespace lilfes

#ifdef CPP_PRED_SAMPLE_CODE
////////////////////////////////////////////////////////////////////////////////
/// A sample of the usage of CppPred.h
///
/// <How to use this example>
/// 1. Make a .cpp file which contains the follows 2 lines:
///
///		#define CPP_PRED_SAMPLE_CODE
///		#include <liblilfes/CppPred.h>
///
///	2. Complie it (my_module.cpp) and create a .so file:
///
///		$ g++ -shared -fPIC -o my_module.so my_module.cpp
///
/// 3. Load the .so from your lilfes code/prompt with load_so/1 directive:
///
///     > :- load_so("my_module").
///
/// 4. Use the predicates and types:
///
///		> ?- hello($X).
///		$X: "hello from hello/1"
///
///		> ?- hello0.
///		hello from hello0/0
///
///		> ?- $X = my_type.
///		$X: my_type
///
////////////////////////////////////////////////////////////////////////////////

// Of course you'll need this in your own code
/* #include <liblilfes/CppPred.h> */

/// Derive your class from Pred<n> template base class for your n-ary predicate
class HelloPred : public lilfes::Pred<1> {
public:

	/// Pass the name of your predicate to the constructor of the base class
	HelloPred(void) : lilfes::Pred<1>("hello") {}

	bool exec(lilfes::machine &, lilfes::FSP argv[])
	{
		/// Do what you like here. The return value specifies the 
		/// success/failure of your predicate.
		return argv[0].Unify("hello from hello/1");
	}
};

/// Derive your class from Pred<0> template base class for your 0-ary predicate
class HelloPred0 : public lilfes::Pred<0> {
public:

	/// Pass the name of your predicate to the constructor of the base class
	HelloPred0(void) : lilfes::Pred<0>("hello0") {}

	/// What you need to define for 0-ary predicate is 
	/// exec(lilfes::machine&), not exec(lilfes::machine&, lilfes::FSP argv[])
	bool exec(lilfes::machine &)
	{
		std::cout << "hello from hello0/0" << std::endl;
		return true;
	}
};

static const lilfes::type *my_type = 0;

/// Derive your initializer class from  ModuleInitializer (optional)
class MyModuleInit : public lilfes::ModuleInitializer {
public:

	/// The module you declared will be passed as 'module' here
	void exec(lilfes::module *module)
	{
		lilfes::type *t = new lilfes::type("my_type", module);
		t->SetAsChildOf(lilfes::bot);
		t->Fix();

		/// This function will be called only once.
		/// So you can safely initialize static variables here.
		assert(my_type == 0);
		my_type = t;
	}
};

/// Create a static ModuleDecl object here
static lilfes::ModuleDecl mod("my_module");

/// Pass the ModuleDecl object and an instance of your initializer/predicate
/// objects to static Registrator objects.
static lilfes::Registrator regInit(mod, new MyModuleInit());
static lilfes::Registrator regHello(mod, new HelloPred());
static lilfes::Registrator regHello0(mod, new HelloPred0());

/* end of exapmle */

#endif // CPP_PRED_SAMPLE_CODE

#endif // CppPred_h__
