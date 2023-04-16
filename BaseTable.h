//GNU AFFERO 3.0 Useop GIm April 16. 2023
/*
This function header file has been designed to store and manage relational data, 
similar to NoSQL databases, providing a flexible table style for complex data structures.
*/
#ifndef _BaseTable_H_
#define _BaseTable_H_

#include <stdio.h>
#include <string>
#include <map>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <iostream>

#include <any>
#include <assert.h>
#include <memory>
#include <typeinfo>
#include <typeindex>

using tabledata = std::pair<std::string, std::any>;
using Attribute = 

struct Attribute {
    std::unordered_map<std::string, std::pair<std::type_index, std::shared_ptr<void>>> property;

    template<typename T>
    T& get(std::string name) {
        auto it = property.find(name);
        if (it != property.end()) {
            return *std::static_pointer_cast<T>(it->second.second);
        }
        else {
            throw std::runtime_error("Key not found");
        }
    }

    template<typename T>
    std::string str(std::string name) {
        auto it = property.find(name);
        if (it != property.end()) {
            if (it->second.first == typeid(int)) {
                auto value_ptr = static_cast<int*>(it->second.second.get());
                return std::to_string(*value_ptr);
            }
            else if (it->second.first == typeid(float)) {
                auto value_ptr = static_cast<double*>(it->second.second.get());
                return std::to_string(*value_ptr);
            }
            else if (it->second.first == typeid(double)) {
                auto value_ptr = static_cast<double*>(it->second.second.get());
                return std::to_string(*value_ptr);
            }
            else if (it->second.first == typeid(std::string)) {
                auto value_ptr = static_cast<std::string*>(it->second.second.get());
                return *value_ptr;
            }
        }
        else {
            throw std::runtime_error("Key not found");
        }
    }
};

struct DATA {
    std::vector<tabledata> morderd_map;
    void operator += (tabledata data) {
        morderd_map.emplace_back(std::make_pair(data.first, data.second));
    }
};

class BaseTable {
private:
    template<typename _Type, typename ...Args>
    void _extend(size_t& index, std::vector<tabledata>& data, size_t sz) {
        auto iter = data.begin();
        std::advance(iter, sz);
        _Type value = std::any_cast<_Type>(iter->second);

        std::shared_ptr<_Type> data_ptr = std::make_shared<_Type>(value);
        _Table[index].property.emplace(
            iter->first, //column name
            std::make_pair(
                std::type_index(typeid(_Type)),
                std::static_pointer_cast<void>(data_ptr)));

        if constexpr (sizeof...(Args) > 0) {
            _extend<Args...>(index, data, sz + 1);
        }
    }

public:
    std::unordered_map<size_t, Attribute> _Table;

    std::vector<Attribute*> operator[] (std::vector<std::size_t> index) {
        std::vector<Attribute*> _sub;
        for (auto& iter : _Table) {
            _sub.emplace_back(&iter.second);
        }
        return _sub;
    }

    std::vector<Attribute*> operator[] (std::initializer_list<std::size_t> index) {
        std::vector<Attribute*> _sub;
        for (auto& iter : _Table) {
            _sub.emplace_back(&iter.second);
        }
        return _sub;
    }

    std::vector<size_t> operator[] (std::initializer_list<std::string> filter) {
        std::vector<int> a = { 1, 2, 3 };
        std::vector<size_t> indices;
        for (const auto& row : _Table) {
            bool check = [&filter, &row] {
                for (const auto& name : filter)
                    if (row.second.property.find(name) == row.second.property.end())
                        return false;
                return true;
            }();
            if (check) indices.emplace_back(row.first);
        }
        return indices;
    }

    template<typename ...Args>
    void Add(DATA& data) {
        auto index = _Table.size();
        _extend<Args...>(index, data.morderd_map, 0);
    }

    template<typename T>
    void Append(size_t index, T data) {
        std::shared_ptr<void> data_ptr = std::make_shared<T>(data);
        _Table[index].property.emplace(
            typeid(T).name(),
            std::make_pair(
                std::type_index(typeid(T)),
                std::static_pointer_cast<void>(data_ptr)));
    }

    void ShowTable() {
        for (auto& row : _Table) {
            for (auto& col : row.second.property) {
                auto colum = col.first.c_str();
                fprintf(stdout, "[%s]", colum);
            }
            std::cout << "\n";
            for (auto& col : row.second.property) {
                auto& type_idx = col.second.first;
                auto& data_ptr = col.second.second;

                if (type_idx == typeid(int)) {
                    auto value_ptr = static_cast<int*>(data_ptr.get());
                    std::cout << std::to_string(*value_ptr) << " ";
                }
                else if (type_idx == typeid(float)) {
                    auto value_ptr = static_cast<double*>(data_ptr.get());
                    std::cout << std::to_string(*value_ptr) << " ";
                }
                else if (type_idx == typeid(double)) {
                    auto value_ptr = static_cast<double*>(data_ptr.get());
                    std::cout << std::to_string(*value_ptr) << " ";
                }
                else if (type_idx == typeid(std::string)) {
                    auto value_ptr = static_cast<std::string*>(data_ptr.get());
                    std::cout << *value_ptr << " ";
                }
                // Add more types as needed
            }
            fprintf(stdout, "\n");
        }
    }
};

#endif