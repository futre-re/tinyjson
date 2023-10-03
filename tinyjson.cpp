#include <vcruntime.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <list>
#include <string>

// 声明区
struct Json_Object;
struct Json_Value;
struct Json_Array;
Json_Object* prase_object(std::string& json, size_t& start);
Json_Array* prase_array(std::string& json, size_t& start);
//
enum True_False_Null { TRUE_VALUE, FALSE_VALUE, NULL_VALUE };
struct Json_Value {
  size_t Number;
  True_False_Null true_flase_null;
  std::string String;
  void* var_value;
  Json_Value() {}
  Json_Value(std::basic_string<char>& value) { String = value; }
  Json_Value(std::basic_string<char>&& value) { String = value; }
  Json_Value(size_t value) { Number = value; }
  Json_Value(True_False_Null& value) { true_flase_null = value; }
  Json_Value(True_False_Null&& value) { true_flase_null = value; }
  Json_Value(Json_Object& json_object) { var_value = &json_object; }
  Json_Value(Json_Object&& json_object) { var_value = &json_object; }
  Json_Value(Json_Array& json_array) { var_value = &json_array; }
  Json_Value(Json_Array&& json_object) { var_value = &json_object; }
};
struct Json_Array {
  std::list<Json_Value> data;
};
struct Key_Value {
  std::string key;
  Json_Value value;
  Key_Value() {}
  Key_Value(std::basic_string<char>& Key) { key = Key; }
  Key_Value(std::basic_string<char>&& Key) { key = Key; }
  Key_Value(std::basic_string<char>& Key, Json_Value& val) {
    key = Key, value = val;
  }
  Key_Value(std::basic_string<char>&& Key, Json_Value&& val) {
    key = Key, value = val;
  }
};
struct Json_Object {
  std::list<Key_Value> data;
  void insert(Key_Value& key_value);
  void insert(Key_Value&& key_value);
  void erase(Key_Value& key_value);
};
std::string trim(const std::string& str) {
  auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
  auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
  return (start < end) ? std::string(start, end) : std::string();
}

// 返回一个string切片,并且移动start_pos到切片结束的下一个位置
auto slice_string(std::string& json, size_t& start_pos, size_t end_pos) {
  auto read_return = json.substr(start_pos, end_pos - start_pos + 1);
  start_pos = end_pos + 1;
  return read_return;
}

// 移动处理到处理的结尾.并返回start_pos到下一处理之间的string
auto move_task_next(std::string& json, size_t& start_pos, char task_value) {
  size_t temp = start_pos;
  start_pos = json.find(task_value, start_pos);
  auto return_val = slice_string(json, temp, start_pos);
  return return_val;
}

// 移动到下一个非空格的位置
auto move_not_empty(std::string& json, size_t& start_pos) {
  return std::distance(
      std::find_if_not(json.begin() + start_pos, json.end(), ' '),
      json.begin());
}

// 处理数字（当且只写到了处理整数)
auto prase_num(std::string& json, size_t& start_pos) {
  auto end_pos =
      std::find_if_not(json.begin() + start_pos, json.end(), ::isdigit) -
      json.begin();
  std::string num_str = json.substr(start_pos, end_pos - start_pos);
  start_pos = end_pos;
  return std::stoi(trim(num_str));
}

// 处理字符串
auto prase_string(std::string& json, size_t& start_pos) {
  return move_task_next(json, start_pos, '"');
}

auto prase_true_false_null(std::string& json, size_t& strart_pos) {
  switch (std::tolower(json[strart_pos])) {
    case 'f':
      strart_pos += 4;
      return FALSE_VALUE;
    case 't':
      strart_pos += 5;
      return TRUE_VALUE;
    case 'n':
      strart_pos += 4;
      return NULL_VALUE;
  }
  // 不执行
  return NULL_VALUE;
}

// 处理数组
Json_Array* prase_array(std::string& json, size_t& start) {
  Json_Array* json_array = new Json_Array();
  auto end_pos = json.find(']', start);
  while (start < end_pos) {
    auto value_start_pos = (size_t)move_not_empty(json, start);
    if (json[value_start_pos] == '"') {
      // 字符串处理
      value_start_pos += 1;
      auto value = prase_string(json, value_start_pos);
      json_array->data.emplace_back(Json_Value(value));
      start = value_start_pos + 1;
    } else if (isdigit(json[value_start_pos])) {
      json_array->data.emplace_back(
          Json_Value(prase_num(json, value_start_pos)));
      value_start_pos += 1;
    } else if (std::tolower(json[value_start_pos]) == 't' ||
               std::tolower(json[value_start_pos]) == 'f' ||
               std::tolower(json[value_start_pos]) == 'n') {
      json_array->data.emplace_back(
          Json_Value(prase_true_false_null(json, value_start_pos)));
      value_start_pos += 1;
    } else if (json[value_start_pos] == '[') {
      json_array->data.emplace_back(
          Json_Value(*prase_array(json, value_start_pos)));
    } else if (json[value_start_pos] == '{') {
      json_array->data.emplace_back(
          Json_Value(*prase_object(json, value_start_pos)));
    }
    start = value_start_pos + 1;
  }
  return json_array;
}

// 处理对象
Json_Object* prase_object(std::string& json, size_t& start) {
  Json_Object* json_object = new Json_Object();
  auto end_pos = json.find('}', start);
  while (start < end_pos) {
    // auto key_start = json.find('"', start);
    move_task_next(json, start, '"');
    start += 1;
    // auto key_end = json.find('"', key_start + 1);
    auto key = move_task_next(json, start, '"');
    auto temp_key_value = Key_Value(key);
    start += 1;
    // std::string key = json.substr(key_start + 1, key_end - key_start - 1);
    // start = key_end + 1;
    // auto value_start = json.find(':', start) + 1;
    auto value_start = move_task_next(json, start, ':');
    start += 1;
    auto value_start_pos = (size_t)move_not_empty(json, start);
    if (json[value_start_pos] == '"') {
      // 字符串处理
      //  auto value_end = json.find('"', value_start + 1);
      //   std::string value =
      //       json.substr(value_start + 1, value_end - value_start - 1);
      value_start_pos += 1;
      auto value = prase_string(json, value_start_pos);
      temp_key_value.value = Json_Value(value);
      // json_object->data.emplace_back(temp_key_value);
      start = value_start_pos + 1;
    } else if (isdigit(json[value_start_pos])) {
      temp_key_value.value = Json_Value(prase_num(json, value_start_pos));
      value_start_pos += 1;
    } else if (std::tolower(json[value_start_pos]) == 't' ||
               std::tolower(json[value_start_pos]) == 'f' ||
               std::tolower(json[value_start_pos]) == 'n') {
      temp_key_value.value =
          Json_Value(prase_true_false_null(json, value_start_pos));
      value_start_pos += 1;
    } else if (json[value_start_pos] == '[') {
      temp_key_value.value = Json_Value(*prase_array(json, value_start_pos));
    } else if (json[value_start_pos] == '{') {
      temp_key_value.value = Json_Value(*prase_object(json, value_start_pos));
    }
    json_object->data.emplace_back(temp_key_value);
    start = value_start_pos + 1;
  }
  return json_object;
}

int main() {
  std::string json = R"({
    "name": "张三",
    "age": 30,
    "isMarried": true,
    "hobbies": ["读书", "旅游", "音乐"],
    "address": {
        "street": "南京路100号",
        "city": "上海",
        "country": "中国"
    }
})";
  size_t start = 0;
  auto temp = prase_object(json, start);
}
