#include <variant>
#include "json.h"

using namespace std;

namespace Json {

  Document::Document(Node root) : root(move(root)) {
  }

  const Node& Document::GetRoot() const {
    return root;
  }

  Node LoadArray(istream& input) {
    vector<Node> result;

    for (char c; input >> c && c != ']'; ) {
      if (c != ',') {
        input.putback(c);
      }
      result.push_back(LoadNode(input));
    }

    return Node(move(result));
  }

  Node LoadNum(istream& input) {
    string line;
    getline(input, line);
    return Node(stod(line));
   }

  Node LoadString(istream& input) {
    string line;
    getline(input, line, '"');
    return Node(move(line));
  }

  Node LoadDict(istream& input) {
    map<string, Node> result;

    for (char c; input >> c && c != '}'; ) {
      if (c == ',') {
        input >> c;
      }

      string key = LoadString(input).AsString();
      input >> c;
      result.emplace(move(key), LoadNode(input));
    }

    return Node(move(result));
  }



  Node LoadBool(istream &input){
      string line;
      getline(input, line);
      if(line[0]=='t'){
          return Node(true);
      }
      return Node(false);
  }

  Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
      return LoadArray(input);
    } else if (c == '{') {
      return LoadDict(input);
    } else if (c == '"') {
      return LoadString(input);
    } else if (isdigit(c)) {
      input.putback(c);
      return LoadNum(input);
    }
    else {
        input.putback(c);
        return LoadBool(input);
    }
  }

  Document Load(istream& input) {
    return Document{LoadNode(input)};
  }



  void PrintNode(const Node& node, ostream& output){
      if(node.HasVector()){
          PrintNodeVector(node, output);
      }
      else if (node.HasMap()){
          PrintNodeMap(node, output);
      }
      else if (node.HasString()) {
          PrintNodeString(node, output);
      }
      else if (node.HasDouble()){
          PrintNodeDouble(node, output);
      }
      else if (node.HasInt()){
          PrintNodeInt(node, output);
      }
      else {
          throw invalid_argument("Node does not contain any value");
      }
  }

  void PrintNodeVector(const Node& node, ostream& output){
     vector<Node> vec_node = node.AsArray();
     output << "[\n";
     for(const auto& el : vec_node){
         PrintNode(el, output);
     }
     output << "]\n";
  }

  void PrintNodeMap(const Node& node, ostream& output){
      map<string, Node> map_node = node.AsMap();
      output << "{\n";
      for(const auto& el : map_node){
          output << "\"" << el.first << "\": ";
          PrintNode(el.second, output);
      }
      output << "}\n";
  }

  void PrintNodeString(const Node& node, ostream& output){
      output << "\"" << node.AsString() << "\"";
  }

  void PrintNodeInt(const Node& node, ostream& output){
      output << node.AsInt();
  }

  void PrintNodeDouble(const Node& node, ostream& output){
      output << node.AsDouble();
  }
}
