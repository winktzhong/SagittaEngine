#ifndef _SINGLETON_H_INCLUDED_
#define _SINGLETON_H_INCLUDED_

// The following code is adapted from boost 1.33.1,
// the original file is boost/pool/detail/singleton.hpp
//
// usage of this template class is, for example:
//
// class ABC_Manager
//     : public Singleton<ABC_Manager>
// ABC_Manager.instance().foo();
//


// IMPORANT NOTE: 
// class "T" in template must be no-throw default constructible 
// and no-throw destructible




// Copyright (C) 2000 Stephen Cleary
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org for updates, documentation, and revision history.


//
// The following helper classes are placeholders for a generic "singleton"
//  class.  The classes below support usage of singletons, including use in
//  program startup/shutdown code, AS LONG AS there is only one thread
//  running before main() begins, and only one thread running after main()
//  exits.
//
// This class is also limited in that it can only provide singleton usage for
//  classes with default constructors.
//

// The design of this class is somewhat twisted, but can be followed by the
//  calling inheritance.  Let us assume that there is some user code that
//  calls "singleton_default<T>::instance()".  The following (convoluted)
//  sequence ensures that the same function will be called before main():
//    instance() contains a call to create_object.do_nothing()
//    Thus, object_creator is implicitly instantiated, and create_object
//      must exist.
//    Since create_object is a static member, its constructor must be
//      called before main().
//    The constructor contains a call to instance(), thus ensuring that
//      instance() will be called before main().
//    The first time instance() is called (i.e., before main()) is the
//      latest point in program execution where the object of type T
//      can be created.
//    Thus, any call to instance() will auto-magically result in a call to
//      instance() before main(), unless already present.
//  Furthermore, since the instance() function contains the object, instead
//  of the singleton_default class containing a static instance of the
//  object, that object is guaranteed to be constructed (at the latest) in
//  the first call to instance().  This permits calls to instance() from
//  static code, even if that code is called before the file-scope objects
//  in this file have been initialized.

#include <cassert>

namespace Sagitta{

	// T must be: no-throw default constructible and no-throw destructible
	template <typename T>
	class Singleton
	{
	private:
		struct object_creator
		{
			// This constructor does nothing more than ensure that instance()
			//  is called before main() begins, thus creating the static
			//  T object before multithreading race issues can come up.
			object_creator() { Singleton<T>::instance(); }
			inline void do_nothing() const { }
		};
		static object_creator create_object;

	private:
		Singleton(const Singleton&);
		Singleton& operator=(const Singleton&);

	protected:
		Singleton() {}
		virtual ~Singleton() {}

	public:
		typedef T object_type;

		// If, at any point (in user code), singleton_default<T>::instance()
		//  is called, then the following function is instantiated.
		static object_type & instance()
		{
			// This is the object that we return a reference to.
			// It is guaranteed to be created before main() begins because of
			//  the next line.
			static object_type obj;

			// The following line does nothing else than force the instantiation
			//  of singleton_default<T>::create_object, whose constructor is
			//  called before main() begins.
			create_object.do_nothing();

			return obj;
		}
	};

	template <typename T>
	typename Singleton<T>::object_creator
		Singleton<T>::create_object;
    
    
/*
 In many cases, we want to manage the life time of a singleton,
 so we create and destroy them by ourselves.
 */

    template<class T>
    class sgManuallySingleton
    {
    private:
        typedef T obj_type;
        static obj_type *obj;
        
        sgManuallySingleton(const sgManuallySingleton&);
        sgManuallySingleton &operator=(const sgManuallySingleton&);
    protected:
        sgManuallySingleton(void){}
        virtual ~sgManuallySingleton(void){}
        
    public:
        static void CreateInstance(void)
        {
            if(!obj)
                obj = new T();
        }
        
        static void DestroyInstance(void)
        {
            if(obj)
                delete obj;
        }
        
        static T *instance(void)
        {
            assert(obj);
            return obj;
        }
    };
    
    template <typename T>
	typename sgManuallySingleton<T>::obj_type *sgManuallySingleton<T>::obj;
    
    

}; // namespace Sagitta

#endif //_SINGLETON_H_INCLUDED_

