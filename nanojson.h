#ifndef __NANOJSON_H__
#define __NANOJSON_H__

#include "picojson.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>

namespace nanojson
{
    struct _member_info;
    typedef std::vector<_member_info *> _pos_list;

    typedef const char *(*_f_get_name)();
    typedef void (*_set_value)(void *, const void *);
    typedef void (*_array_ctor)(void *, picojson::array &);

    namespace _json_values
    {
        enum type
        {
            null_type,
            boolean_type,
            int_type,
            double_type,
            string_type,
            array_type,
            object_type,
            error_type
        };
    }

    class exception : public std::exception
    {
    private:
        const std::string message;
        const char *fileName;
        const char *funcName;
        int line;
    public:
        exception() { }
        exception(const char *message, const char *fileName, const char *funcName, const int line) :
            message(message), fileName(fileName), funcName(funcName), line(line) { }

        inline const char *getMessage() const { return message.c_str(); }
        inline const char *getFileName() const { return fileName; }
        inline const char *getFuncName() const { return funcName; }
        inline int getLine() const { return line; }
    };

#define __exception(MSG) exception(MSG, __FILE__, __FUNCTION__, __LINE__)

    struct _member_info
    {
        _f_get_name n;
        union
        {
            _set_value s;
            _pos_list *list;
            _array_ctor ctor;
        };
        size_t pos;
        _json_values::type type;
    };

    namespace _type_checker
    {
        struct _false_type { static const bool value = false; };
        struct _true_type { static const bool value = true; };

        template<bool B>
        struct _enable { typedef void type; };

        template<>
        struct _enable<false> { };

        template<typename>
        struct _ignore { typedef void type; };

        template<typename T>
        struct _is_void : public _false_type { };

        template<>
        struct _is_void<void> : public _true_type { };

        template<_json_values::type T>
        struct _elem
        {
            typedef _elem<T> value;
            static const _json_values::type val = T;
        };

        template<bool C, typename T, typename F>
        struct _if { typedef typename T::value value; };

        template<typename T, typename F>
        struct _if<false, T, F> { typedef typename F::value value; };

        template <typename, typename>
        struct _is_same : public _false_type { };

        template <typename T>
        struct _is_same<T, T> : public _true_type { };

        template<typename T>
        struct _is_pointer : public _false_type { };

        template<typename T>
        struct _is_pointer<T *> : public _true_type { };

        template<typename T>
        struct _is_vector : public _false_type { };

        template<typename T, typename A>
        struct _is_vector<std::vector<T, A> > : public _true_type { };

        template<typename T, typename U = void>
        struct _has_self_type : public _false_type { };

        template<typename T>
        struct _has_self_type<T, typename _type_checker::_ignore<typename T::self_type>::type> : public _true_type { };

        template<typename T>
        struct get_type
        {
             static const _json_values::type value = _if<
                _is_pointer<T>::value,
                _elem<_json_values::null_type>,
                _if<
                    _is_same<T, bool>::value,
                    _elem<_json_values::boolean_type>,
                    _if<
                        std::numeric_limits<T>::is_integer,
                        _elem<_json_values::int_type>,
                        _if<
                            std::numeric_limits<T>::is_iec559,
                            _elem<_json_values::double_type>,
                            _if<
                                _is_same<T, std::string>::value,
                                _elem<_json_values::string_type>,
                                _if<
                                    _is_vector<T>::value,
                                    _elem<_json_values::array_type>,
                                    _if<
                                        _has_self_type<T>::value,
                                        _elem<_json_values::object_type>,
                                        _elem<_json_values::error_type>
                                    >
                                >
                            >
                        >
                    >
                >
            >::value::val;
        };
    };

    namespace _parser_funcs
    {
        /* parser functions */
        void parse(void *result, _pos_list *list, picojson::object &obj);

        void parse(void *o, _member_info *info, picojson::value &value)
        {
            switch(info->type)
            {
                case _json_values::null_type:
                    if(!value.is<picojson::null>())
                        throw __exception("pointer value must be null");
                    info->s(o, 0);
                    break;
                case _json_values::boolean_type:
                    {
                        const bool b = value.get<bool>();
                        info->s(o, &b);
                    }
                    break;
                case _json_values::int_type:
                    {
                        const int i = static_cast<int>(value.get<double>());
                        info->s(o, &i);
                    }
                    break;
                case _json_values::double_type:
                    {
                        const double d = value.get<double>();
                        info->s(o, &d);
                    }
                    break;
                case _json_values::string_type:
                    {
                        const std::string &s = value.get<std::string>();
                        info->s(o, &s);
                    }
                    break;
                case _json_values::array_type:
                    info->ctor(o, value.get<picojson::array>());
                    break;
                case _json_values::object_type:
                    {
                        picojson::object &children = value.get<picojson::object>();
                        parse(o, info->list, children);
                    }
                    break;
                case _json_values::error_type:
                    throw __exception("error invalid member type");
                    break;
            }     
        }

        void parse(void *result, _pos_list *list, picojson::object &obj)
        {
            for(_pos_list::iterator it = list->begin(); it != list->end(); ++it)
            {
                _member_info *info = *it;
                parse(
                    static_cast<char *>(result) + info->pos,
                    info,
                    obj[info->n()]
                );
            }
        }

        template<typename T>
        inline void parse(T &result, picojson::object &obj) { parse(&result, &T::_pos, obj); }

        /* functions to assign value to vector */
        template<typename T>
        inline void assign(
            void *v,
            picojson::array &list,
            typename _type_checker::_enable<
                !std::numeric_limits<T>::is_integer &&
                !_type_checker::_has_self_type<T>::value
            >::type* = 0)
        {
            for(picojson::array::iterator it = list.begin(); it != list.end(); ++it)
                static_cast<std::vector<T> *>(v)->push_back(it->get<T>());
        }

        template<typename T>
        inline void assign(
            void *v,
            picojson::array &list,
            typename _type_checker::_enable<_type_checker::_has_self_type<T>::value>::type* = 0
        )
        {
            for(picojson::array::iterator it = list.begin(); it != list.end(); ++it)
            {
                T elem;
                parse(elem, it->get<picojson::object>());
                static_cast<std::vector<T> *>(v)->push_back(elem);
            }
        }

        template<typename T>
        inline void assign(
            void *v,
            picojson::array &list,
            typename _type_checker::_enable<std::numeric_limits<T>::is_integer>::type* = 0
        )
        {
            for(picojson::array::iterator it = list.begin(); it != list.end(); ++it)
                static_cast<std::vector<T> *>(v)->push_back(it->get<double>());
        }

        template<typename T>
        inline void assign_bridge(void *v, picojson::array &list) { assign<T>(v, list); }
    }

    template<typename C, typename T>
    class json_element
    {
        template<typename S>
        inline static void set_vparam(
            _member_info &mi,
            typename _type_checker::_enable<_type_checker::_has_self_type<S>::value>::type* = 0
        ) { mi.list = &T::_pos; }

        template<typename S>
        inline static void set_vparam(
            _member_info &mi,
            typename _type_checker::_enable<_type_checker::_is_vector<S>::value>::type* = 0
        ) { mi.ctor = _parser_funcs::assign_bridge<typename S::value_type>; }

        template<typename S>
        inline static void set_vparam(
            _member_info &mi,
            typename _type_checker::_enable<
                !_type_checker::_has_self_type<S>::value &&
                !_type_checker::_is_vector<S>::value
            >::type* = 0
        ) { mi.s = set; }

        struct __initializer
        {
            __initializer()
            {
                _meminfo.n = C::_get_name;
                set_vparam<T>(_meminfo);
                C::_register(&_meminfo);
            }
        };
    private:
        static _member_info _meminfo;

        static void set(void *o, const void *v) { *static_cast<T *>(o) = *static_cast<const T *>(v); }
    public:
        json_element() { static __initializer _init; }
    };

    template<typename C, typename T>
    _member_info json_element<C, T>::_meminfo;

    template<typename S>
    class object
    {
    public:
        typedef S self_type;
        static _pos_list _pos;

        static inline void _register(_member_info *mi, const size_t s, const _json_values::type type)
        {
            mi->pos = s;
            mi->type = type;
            _pos.push_back(mi);
        }
    };

    template<typename T>
    _pos_list object<T>::_pos;

    class reader
    {
    private:
        const char *filename;
    public:
        reader() { }
        reader(const char *filename) : filename(filename) { }
        ~reader() { }

        bool load(const char *filename)
        {
            this->filename = filename;
            return false;
        }

        template<typename T>
        T parse(const char *str, const size_t len)
        {
            T result;
            picojson::value val;
            std::string err;
            picojson::parse(val, str, str + len, &err);

            if(!err.empty())
                throw __exception("json parse error.");

            if(!val.is<picojson::object>())
                throw __exception("root element must be object.");

            _parser_funcs::parse<T>(result, val.get<picojson::object>());

            return result;
        }

        template<typename T>
        inline T parse(const char *str) { return parse<T>(str, strlen(str)); }

        template<typename T>
        inline T parse()
        {
            std::ifstream ifs(filename, std::ios::in);
            assert(ifs);
            std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            return parse<T>(str.c_str(), str.size());
        }
    };
}
#define def(T, NAME)    \
    struct type_ ## NAME : public nanojson::json_element<type_ ## NAME, T> {  \
        static const char *_get_name() { return # NAME; }    \
        static void _register(nanojson::_member_info *_mi)  \
        {   \
            self_type::_register(   \
                _mi, offsetof(self_type, NAME),         \
                nanojson::_type_checker::get_type<T>::value     \
            );      \
        } \
    } type_ ## NAME;      \
    T NAME;
#undef __exception

#endif
