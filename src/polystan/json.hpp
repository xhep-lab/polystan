#ifndef POLYSTAN_JSON_HPP_
#define POLYSTAN_JSON_HPP_

#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <string>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

namespace polystan {
namespace json {

namespace rj = rapidjson;
typedef rapidjson::MemoryPoolAllocator<> Alloc;

Alloc& get_allocator() {
  static rj::GenericDocument<rj::UTF8<>> doc;
  return doc.GetAllocator();
}

template <typename T>
rj::Value Vector(const std::vector<T>& vec, Alloc& alloc) {
  rj::Value value(rj::kArrayType);
  value.Reserve(vec.size(), alloc);
  for (const T& elem : vec) {
    value.PushBack(elem, alloc);
  }
  return value;
}

rj::Value String(const std::string& str, Alloc& alloc) {
  rj::Value value(rj::kStringType);
  value.SetString(str.c_str(), str.size(), alloc);
  return value;
}

class Object {
 public:
  Object() : value(rj::kObjectType), alloc(get_allocator()) {}

  template <typename T>
  void add(const std::string& name, const std::vector<T>& data) {
    value.AddMember(String(name, alloc).Move(), Vector(data, alloc).Move(),
                    alloc);
  }

  template <typename T>
  void add(const std::string& name, T data) {
    value.AddMember(String(name, alloc).Move(), data, alloc);
  }

  void add(const std::string& name, const char* data) {
    value.AddMember(String(name, alloc).Move(), String(data, alloc).Move(),
                    alloc);
  }

  void add(const std::string& name, const std::string& data) {
    add(name, data.c_str());
  }

  template <typename T>
  void add(std::vector<std::string> names, std::vector<T> data) {
    for (int i = 0; i < names.size(); i++) {
      add(names[i], data[i]);
    }
  }

  void add(const std::string& name, Object& child) {
    value.AddMember(String(name, alloc).Move(), child.value.Move(), alloc);
  }

  template <typename T>
  void set(T value_) {
    value = value_;
  }

  void set(const std::string& value_) {
    value.SetString(value_.c_str(), value_.size(), alloc);
  }

  void set(const char* value_) { set(std::string(value_)); }

  void write(const std::string& json_file_name) {
    std::ofstream ofs(json_file_name);
    rj::OStreamWrapper osw(ofs);
    rj::PrettyWriter<rj::OStreamWrapper> writer(osw);
    value.Accept(writer);
  }

 private:
  Alloc& alloc;
  rj::Value value;
};

}  // end namespace json
}  // end namespace polystan

#endif  // POLYSTAN_JSON_HPP_
