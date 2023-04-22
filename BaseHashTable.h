#ifndef _BASE_HASH_TABLE_H_
#define _BASE_HASH_TABLE_H_
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include <utility>
#include <unordered_map>
#include <iostream>
#include <any>

#define _usg_type_cast template<typename _T1>
#define _usg_type_cast_args template<typename _T1, typename ...Args>
#define _usg_ptr_sz_ 4
#define _usg_cast_type_(_VAR_,_Type) =static_cast<_Type*>(_VAR_)
#define _usg_foward_ptr_(_VAR_) =static_cast<void*>(reinterpret_cast<char*>(_VAR_) + _usg_ptr_sz_)

class HASH_TABLE{
    std::unordered_map<std::string, std::unordered_map<std::type_index, std::any>> bucket_list;
    _usg_type_cast void _Add(std::string key, _T1 _obj1){
        bucket_list[key][typeid(_T1)] =_obj1;
    }
    _usg_type_cast_args void _Add(std::string key, _T1 _obj1, Args ...args){
        bucket_list[key][typeid(_T1)] = _obj1;
        _Add<Args...>(key, args...);
    }

public:

    template<typename... Args>
    void Add(std::string key, Args... args){
        _Add<Args...>(key, args...);
        printf("Add is finish\n");
    }
    void Delete(std::string key){
        bucket_list.erase(key);
    }
    template<typename T>
    T* Get(std::string key){
        assert(bucket_list.count(key) > 0);
        assert(bucket_list.at(key).count(typeid(T)) > 0);
        return &std::any_cast<T>(bucket_list.at(key).at(typeid(T)));
    }
};

#endif
