/*******************************************************************************************
 * Copyright (c) 2013, Huang Li <lihuang55555@gmail.com>, IIPL <gitl.sysu.edu.cn>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, this list
 *   of conditions and the following disclaimer in the documentation and/or other
 *   materials provided with the distribution.
 * * Neither the name of the IIPL nor the names of its contributors may be used
 *   to endorse or promote products derived from this software without specific prior
 *   written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************************/
#ifndef GITLDEF_H
#define GITLDEF_H
#include <functional>
#include <QMutex>
#include <QMutexLocker>

/*! concatenating multiple args into one*/
#define CONCATE(...) __VA_ARGS__


/*! Q_PROPERTY getter and setter generator*/
#define ADD_QPROP_RW_INIT(type, name, getter, setter, init) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name READ getter WRITE setter NOTIFY name##Changed) \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
        void setter(type name) { m_##name = name; emit name##Changed(name);} \
        Q_SIGNAL void name##Changed(type& name); \
    private: \
        type m_##name = init;

#define ADD_QPROP_RW(type, name, getter, setter) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name READ getter WRITE setter NOTIFY name##Changed) \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
        void setter(type name) { m_##name = name; emit name##Changed(name);} \
        Q_SIGNAL void name##Changed(type& name); \
    private: \
        type m_##name;


#define ADD_QPROP_RO_INIT(type, name, getter, init) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name READ getter) \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
    private: \
        type m_##name = init;

#define ADD_QPROP_RO(type, name, getter) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name READ getter) \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
    private: \
        type m_##name;

#define ADD_QPROP_PR_INIT(type, name, init) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name) \
    private: \
        type m_##name = init;

#define ADD_QPROP_PR(type, name) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name) \
    private: \
        type m_##name;



/*! getter and setter generator for class memeber */
#define ADD_FIELD_INIT(type, name, getter, setter, init) \
    public: \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
        void setter(type name) { m_##name = name; } \
    private: \
        type m_##name = init;

#define ADD_FIELD(type, name, getter, setter) ADD_CLASS_FIELD(type, name, getter, setter)
#define ADD_CLASS_FIELD(type, name, getter, setter) \
    public: \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
        void setter(type name) { m_##name = name; } \
    private: \
        type m_##name;

#define ADD_FIELD_NOSETTER_INIT(type, name, getter, init) \
    public: \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
    private: \
        type m_##name = init;

#define ADD_FIELD_NOSETTER(type, name, getter) ADD_CLASS_FIELD_NOSETTER(type, name, getter)
#define ADD_CLASS_FIELD_NOSETTER(type, name, getter) \
    public: \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
    private: \
        type m_##name;

#define ADD_FIELD_PRIVATE_INIT(type, name, init ) \
    private: \
        type m_##name = init;

#define ADD_FIELD_PRIVATE(type, name ) ADD_CLASS_FIELD_PRIVATE(type, name )
#define ADD_CLASS_FIELD_PRIVATE(type, name ) \
    private: \
        type m_##name;


/*!  SINGLETON DESIGN PATTERN (Thread Safe) */
#define SINGLETON_PATTERN_DECLARE(classname)\
    public: \
        static classname* getInstance() { QMutexLocker cLocker(&m_cGetInstanceMutex); if(m_instance==NULL) m_instance=new classname(); return m_instance; } \
    private: \
        static classname* m_instance; \
        static QMutex m_cGetInstanceMutex;

#define SINGLETON_PATTERN_IMPLIMENT(classname)\
    classname* classname::m_instance = NULL; \
    QMutex classname::m_cGetInstanceMutex;

/*! PROTOTYPE PATTERN */
#define CLONABLE(classname)\
    public:\
        virtual classname* clone() const { return new classname(*this); }


/* CLIP c BETWEEN a AND b */
#define VALUE_CLIP(min,max,value) ( ((value)>(max))?(max):((value)<(min))?(min):(value) )

/* SCOPE GUARD C++11 Required*/
template <typename F>
struct ScopeExit {
    ScopeExit(F f) : f(f) {}
    ~ScopeExit() { f(); }
    F f;
};
template <typename F>
ScopeExit<F> MakeScopeExit(F f) {
    return ScopeExit<F>(f);
}
#define SCOPE_EXIT(code) \
    auto scope_exit_##__LINE__ = MakeScopeExit([=](){code;})


/*make callback functor*/
#define MAKE_CALLBACK(memberFunc) \
    std::bind( &memberFunc, this, std::placeholders::_1 )

#define MAKE_CALLBACK_OBJ(object, memberFunc) \
    std::bind( &memberFunc, &(object), std::placeholders::_1 )



#endif
