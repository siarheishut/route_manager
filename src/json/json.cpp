#include "json.h"

#include <vector>
#include <string>
#include <optional>
#include <sstream>
#include <ios>
#include <cctype>

namespace rm::json {
std::istream &ReadArray(std::istream &input, Node &arr) {
  char c;
  if (!(input >> c)) return input;
  if (c == ']') {
    arr = List{};
    return input;
  } else {
    input.putback(c);
  }

  List result;
  do {
    Node node;
    if (!(input >> node >> c)) return input;
    result.push_back(std::move(node));
    if (c == ']') {
      arr = std::move(result);
      return input;
    }
  } while (c == ',');
  input.setstate(std::ios_base::failbit);
  return input;
}

std::istream &ReadNumber(std::istream &input, Node &num) {
  double value = 0.0, int_part = 0.0;
  input >> value;
  if (!input) return input;
  if (modf(value, &int_part) == 0.0) {
    num = static_cast<int>(int_part);
  } else {
    num = value;
  }
  return input;
}

std::istream &ReadString(std::istream &input, Node &str) {
  std::string line;
  if (!getline(input, line, '"')) {
    return input;
  }
  if (input.eof()) {
    input.setstate(std::ios_base::failbit);
    return input;
  }
  str = std::move(line);
  return input;
}

std::istream &ReadBool(std::istream &input, Node &out) {
  bool b;
  input >> std::boolalpha >> b;
  if (!input) return input;
  out = Node{b};
  return input;
}

std::istream &ReadNull(std::istream &input, Node &null) {
  char n, u, l1, l2;
  if (!input.get(n).get(u).get(l1).get(l2)) {
    return input;
  }
  if (n != 'n' || u != 'u' || l1 != 'l' || l2 != 'l') {
    input.setstate(std::ios_base::failbit);
    return input;
  }
  null = Node{std::monostate{}};
  return input;
}

std::istream &ReadDict(std::istream &input, Node &dict) {
  char c;
  if (!(input >> c)) return input;
  if (c == '}') {
    dict = Dict{};
    return input;
  } else {
    input.putback(c);
  }

  Dict result;

  char c1, c2;
  do {
    Node key, value;
    input >> key >> c1 >> value >> c2;
    if (!input) return input;
    if (!(key.IsString()) || c1 != ':') {
      input.setstate(std::ios_base::failbit);
      return input;
    }
    result.emplace(key.AsString(), value);

    if (c2 == '}') {
      dict = std::move(result);
      return input;
    }
  } while (c2 == ',');
  input.setstate(std::ios_base::failbit);
  return input;
}

std::istream &operator>>(std::istream &input, Node &node) {
  char c;
  if (!(input >> c)) return input;

  if (c == '[') {
    ReadArray(input, node);
  } else if (c == '{') {
    ReadDict(input, node);
  } else if (c == '"') {
    ReadString(input, node);
  } else if (c == 't' || c == 'f') {
    input.putback(c);
    ReadBool(input, node);
  } else if (isdigit(c) || c == '-') {
    input.putback(c);
    ReadNumber(input, node);
  } else if (c == 'n') {
    input.putback(c);
    ReadNull(input, node);
  } else {
    input.setstate(std::ios_base::failbit);
  }
  return input;
}

void Write(std::ostream &out, std::monostate) {
  out << "null";
}

void Write(std::ostream &out, bool value) {
  out << std::boolalpha << value;
}

template<typename T>
void Write(std::ostream &out, const T &value) {
  out << value;
}

void Write(std::ostream &out, const List &list) {
  out << '[';
  bool first = true;
  for (auto &item : list) {
    if (!first) out << ",";
    first = false;
    Write(out, item);
  }
  out << ']';
}

void Write(std::ostream &out, const rm::json::Dict &list) {
  out << '{';
  bool first = true;
  for (auto &[key, value] : list) {
    if (!first) out << ",";
    first = false;
    out << key << ":";
    Write(out, value);
  }
  out << '}';
}

std::ostream &operator<<(std::ostream &out, const Node &node) {
  std::visit([&](auto &&val) { Write(out, val); }, node.GetBase());
  return out;
}
}
