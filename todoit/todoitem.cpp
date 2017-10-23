#include "stdafx.h"
#include "todoitem.h"
#include <string>
#include <iostream>
#include <regex>

TodoItem::TodoItem(const std::string &todo_line) :
  raw_line(todo_line)
{
//  std::cout << "=== construct regular " << raw_line << std::endl;
}

TodoItem &TodoItem::operator=(const TodoItem &obj) {
//  std::cout << "=== operator= " << obj.AssembleTodo() << std::endl;
  this->done_ = false;
  this->priority_ = "";
  this->dateDone_ = "";
  this->dateCreation_ = "";
  this->contexts_ = std::vector<std::string>();
  this->projects_ = std::vector<std::string>();
  this->raw_line = obj.AssembleTodo();
  this->DecodeLine();
  return *this;
}

TodoItem TodoItem::init(const std::string &todoLine) {
  TodoItem item(todoLine);
  item.DecodeLine();
  return item;
}

bool TodoItem::operator ==(const TodoItem &obj) {
  return this->AssembleTodo() == obj.AssembleTodo();
}

TodoItem::TodoItem(const TodoItem &obj) :
  raw_line(obj.AssembleTodo())
{
//  std::cout << "=== construct copy " << raw_line << std::endl;
  DecodeLine();
}

/**
 * Initial decoding of the todo-line
 * @brief TodoItem::DecodeLine
 */
void TodoItem::DecodeLine() {
  done_ = IsDone();
  priority_ = GetPriority();
  dateDone_ = GetDateDone();
  dateCreation_ = GetDateCreation();
  contexts_ = GetContexts();
  projects_ = GetProjects();
  todo_ = GetTodo();
}

/**
 * Find all matches of pattern in todoline
 * @brief TodoItem::RegexFind
 * @param pattern
 * @return std::deque<std::string> queue of found strings
 */
std::deque<std::string> TodoItem::RegexFind(std::string pattern) {
  int cutPos = 0;
  std::deque<std::string> findings;

  if (done_) cutPos += PADDING_DONE;
  if (priority_.length() > 0) cutPos += PADDING_PRIORITY;
  if (dateCreation_.length() > 0) cutPos += PADDING_DATE;
  if (dateDone_.length() > 0) cutPos += PADDING_DATE;

  std::string search_string = raw_line.substr(cutPos, raw_line.length());

  std::regex rgx(pattern);
  auto begin = std::sregex_iterator(search_string.begin(), search_string.end(), rgx);
  auto end = std::sregex_iterator();

  for (std::sregex_iterator i = begin; i != end; ++i) {
    std::smatch match = *i;
    std::string match_str = match.str().substr(1);
    findings.push_back(match_str);
  }

  return findings;
}

/**
 * Check if todo is done
 * @brief TodoItem::IsDone
 * @return bool if done
 */
bool TodoItem::IsDone(){
  if (raw_line.substr(0,2) == "x ")
    return true;
  else
    return false;
}

/**
 * Get the priorty of the task
 * @brief TodoItem::GetPriority
 * @return string proirity
 */
std::string TodoItem::GetPriority() {
  int priority_start = 0;

  if (done_) priority_start = 2;

  if (raw_line.substr(priority_start, 1) == "(" &&
      raw_line.substr(priority_start + 2, 1) == ")" &&
      raw_line.substr(priority_start + 3, 1) == " " &&
      isupper(raw_line.substr(priority_start + 1, 1)[0])) {

    return raw_line.substr(priority_start + 1, 1);
  } else {
    return "";
  }
}

/**
 * @brief TodoItem::GetDateCreation
 * @return std::string creation date or empty string
 */
std::string TodoItem::GetDateCreation() {
  int start_pos = 0;

  if (done_) start_pos += PADDING_DONE;
  if (priority_.length() > 0) start_pos += PADDING_PRIORITY;
  if (dateDone_.length() > 0) start_pos += PADDING_DATE;

  std::string possible_date = raw_line.length()< start_pos + 10?"":raw_line.substr(start_pos, 10);
  if (regex_match(possible_date, std::regex(REGEX_DATE)) && raw_line.substr(start_pos+10, 1) == " ")
    return possible_date;
  else
    return "";
}

/**
 * @brief TodoItem::GetDateDone
 * @return std::string done date or empty string
 */
std::string TodoItem::GetDateDone() {
  if (!done_) return "";

  int start_pos = PADDING_DONE;
  if (priority_.length() > 0) start_pos += PADDING_PRIORITY;

  std::string possible_date = raw_line.length() < start_pos + 10 ? "" : raw_line.substr(start_pos, 10);
  if (regex_match(possible_date, std::regex(REGEX_DATE)) &&
      raw_line.substr(start_pos + 10, 1) == " ")
    return possible_date;
  else
    return "";
}

std::vector<std::string> TodoItem::GetContexts() {
  std::deque<std::string> finds = RegexFind(REGEX_CONTEXT);
  std::vector<std::string> contexts;

  for (size_t i = finds.size(); i>0; --i){
    contexts.push_back(finds.front());
    finds.pop_front();
  }

  return contexts;
}

std::vector<std::string> TodoItem::GetProjects() {
  std::deque<std::string> finds = RegexFind(REGEX_PROJECT);
  std::vector<std::string> projects;

  for (size_t i = finds.size(); i>0; --i){
    projects.push_back(finds.front());
    finds.pop_front();
  }

  return projects;
}

std::string TodoItem::GetTodo() {
  size_t partsToStripOf = contexts_.size();
  partsToStripOf += projects_.size();

  int startPos = 0;
  if (done_) startPos += 2;
  if (dateDone_.length() > 0) startPos += 11;
  if (dateCreation_.length() > 0) startPos += 11;
  if (priority_.length() > 0) startPos += 4;


  std::string line = raw_line.substr(startPos, raw_line.length());

  return line;
}

/**
 * Get all key->value pairs in todo-line
 * @brief TodoItem::GetKeyValuePairs
 * @return vector<TodoItem::ItemKVPair_t> Vector of kev->values pairs
 */
std::vector<TodoItem::ItemKVPair_t> TodoItem::GetKeyValuePairs() {
  std::deque<std::string> kvs = RegexFind(REGEX_KEYVALUE);
  std::vector<TodoItem::ItemKVPair_t> keyValues;

  for (size_t i = kvs.size(); i>0; --i){
    std::string part = kvs.back();
    std::string key = part.substr(0,part.find(":"));
    std::string value = part.substr(part.find(":")+1, part.length());
    keyValues.push_back({key, value});
    kvs.pop_back();
  }

  return keyValues;
}

/**
 * Get the value of specifyed key
 * @brief TodoItem::GetKeyValue
 * @param key
 * @return
 */
std::string TodoItem::GetKeyValue(std::string key) {
  std::vector<TodoItem::ItemKVPair_t> keyValues = GetKeyValuePairs();

  for (size_t i =  keyValues.size(); i>0; --i){
    if (key == keyValues.back().key)
      return keyValues.back().value;

    keyValues.pop_back();
  }

  return "";
}

std::string TodoItem::AssembleTodo() const {
  std::string bakedTodo;

  if (done_) bakedTodo = "x ";
  if (priority_.length()>0) bakedTodo += "("+ priority_+") ";
  if (dateDone_.length()>0) bakedTodo += dateDone_+" ";
  if (dateCreation_.length()>0) bakedTodo += dateCreation_+" ";
  bakedTodo += todo_;

  return bakedTodo;
}

void TodoItem::SetDone(bool isDone) {
  done_ = isDone;
}

void TodoItem::SetPriority(std::string priority) {
  priority_ = priority;
}

void TodoItem::SetDateCreation(std::string creationDate) {
  if (creationDate.length() > 0 && !regex_match(creationDate, std::regex(REGEX_DATE)))
    throw std::invalid_argument("Invalid date format, should be YYYY-MM-DD");
  dateCreation_ = creationDate;
}

void TodoItem::SetDateDone(std::string doneDate) {
  if (doneDate.length() > 0 && !regex_match(doneDate, std::regex(REGEX_DATE)))
    throw std::invalid_argument("Invalid date format, should be YYYY-MM-DD");
  dateDone_ = doneDate;
}

void TodoItem::SetTodo(std::string todoLine) {
  todo_ = todoLine;
}
