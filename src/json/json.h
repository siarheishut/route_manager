#ifndef ROOT_MANAGER_SRC_JSON_JSON_PARSER_H_
#define ROOT_MANAGER_SRC_JSON_JSON_PARSER_H_

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace rm::json {
class Node;
using Dict = std::map<std::string, Node>;
using List = std::vector<Node>;

std::ostream &operator<<(std::ostream &out, const Node &node);
std::istream &operator>>(std::istream &in, Node &node);

class Node final : std::variant<std::monostate,
                                List,
                                Dict,
                                double,
                                bool,
                                int,
                                std::string> {
 public:
  using variant::variant;

  inline const variant &GetBase() const { return *this; }

  inline bool IsArray() const {
    return std::holds_alternative<std::vector<Node>>(*this);
  }
  inline bool IsMap() const {
    return std::holds_alternative<Dict>(*this);
  }
  inline bool IsInt() const {
    return std::holds_alternative<int>(*this);
  }
  inline bool IsDouble() const {
    return std::holds_alternative<double>(*this)
        || std::holds_alternative<int>(*this);
  }
  inline bool IsBool() const {
    return std::holds_alternative<bool>(*this);
  }
  inline bool IsString() const {
    return std::holds_alternative<std::string>(*this);
  }

  inline const List &AsArray() const {
    return std::get<std::vector<Node>>(*this);
  }
  inline const Dict &AsMap() const {
    return std::get<Dict>(*this);
  }
  inline int AsInt() const {
    return std::get<int>(*this);
  }
  inline double AsDouble() const {
    return std::holds_alternative<double>(*this) ? std::get<double>(*this) :
           std::get<int>(*this);
  }
  inline bool AsBool() const {
    return std::get<bool>(*this);
  }
  inline const std::string &AsString() const {
    return std::get<std::string>(*this);
  }
  inline friend bool operator==(const Node &l, const Node &r) {
    return l.GetBase() == r.GetBase();
  };
  inline friend bool operator!=(const Node &l, const Node &r) {
    return l.GetBase() != r.GetBase();
  };
};
}

#endif // ROOT_MANAGER_SRC_JSON_JSON_PARSER_H_
