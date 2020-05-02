#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <deque>

using namespace std;


struct Email {
  string from;
  string to;
  string body;
};


ostream& operator<<(ostream& out, const Email& m){
    out << m.from << "\n"
        << m.to << "\n"
        << m.body << "\n";
}



class Worker {
public:
    virtual ~Worker() = default;
    Worker() = default;
//    {
//        vector<Node*> to_clear;
//        Node* it_link = cur_element;
//        to_clear.push_back(it_link);
//        while(it_link->next){
//            to_clear.push_back(it_link->next);
//            it_link = it_link->next;
//        }
//        for(auto el : to_clear){
//            delete el;
//        }
//    }

  virtual void Process(unique_ptr<Email> email) = 0;
  virtual void Run() {
    // только первому worker-у в пайплайне нужно это имплементировать
    throw logic_error("Unimplemented");
  }

protected:
    // реализации должны вызывать PassOn, чтобы передать объект дальше
    // по цепочке обработчиков
    void PassOn(unique_ptr<Email> email) const{
        if(next_){
            next_->Process(move(email));
        }
    }

public:

    void SetNext(unique_ptr<Worker> next){
        next_ = move(next);
//        if(this->next_){
//            this->next_->SetNext(move(next));
//        } else {
//            this->next_ = move(next);
//        }
    }
      //      Node* it_link = cur_element->next;
//      while(it_link!=nullptr){
//          it_link = it_link->next;
//      }
//      Node* el = new Node;
//      el->worker = move(next);
//      el->next = nullptr;
//      it_link->next = el;



private:
//  struct Node{
//      unique_ptr<Worker> worker;
//      Node* next;
//  };
  unique_ptr<Worker> next_;
};


class Reader : public Worker {
public:
    explicit Reader(istream& input):input_(input){}

    void Run(){
        string buf;
        while(getline(input_, buf, '\n')){
            if(!buf.empty()){
                Email tmp;
                tmp.from = move(buf);
                getline(input_, tmp.to, '\n');
                getline(input_, tmp.body, '\n');
                if(!tmp.to.empty() && !tmp.from.empty() && !tmp.body.empty()){
                    unique_ptr<Email> further = make_unique<Email>(tmp);
                    PassOn(move(further));
                }
            }
        }
    }

    void Process(unique_ptr<Email> email) {}

private:
    istream& input_;
};


class Filter : public Worker {
public:
  using Function = function<bool(const Email&)>;

public:
    explicit Filter(Function f): func_(f) {}

    void Process(unique_ptr<Email> email) {
        if(func_(*email)){
            PassOn(move(email));
        }
    }
private:
    Function func_;
};


class Copier : public Worker {
public:
  explicit Copier(const string& rec): reciever(rec) {}

    void Process(unique_ptr<Email> email) {
        if(reciever!=email->to){
            unique_ptr<Email> copy_mail = make_unique<Email>(*email);
            copy_mail->to = reciever;
            PassOn(move(email));
            PassOn(move(copy_mail));
        } else {
            PassOn(move(email));
        }
    }

    private:
        string reciever;
};


class Sender : public Worker {
public:
    explicit Sender(ostream& out): output(out) {}

    void Process(unique_ptr<Email> email) {
        output << *email;
        PassOn(move(email));
    }
private:
    ostream& output;
};

/*
// реализуйте класс
class PipelineBuilder {
public:
  // добавляет в качестве первого обработчика Reader
    explicit PipelineBuilder(istream& in):pipe(){
        pipe->SetNext(make_unique<Reader>(in));
    }

  // добавляет новый обработчик Filter
    PipelineBuilder& FilterBy(Filter::Function filter){
        pipe->SetNext(make_unique<Filter>(filter));
        return *this;
    }

  // добавляет новый обработчик Copier
    PipelineBuilder& CopyTo(string recipient){
        pipe->SetNext(make_unique<Copier>(recipient));
        return *this;
    }

  // добавляет новый обработчик Sender
    PipelineBuilder& Send(ostream& out){
        pipe->SetNext(make_unique<Sender>(out));
        return *this;
    }

  // возвращает готовую цепочку обработчиков
    unique_ptr<Worker> Build() {
        pipe->SetNext(nullptr);
        return move(pipe);
    }
private:
  unique_ptr<Worker> pipe;
};
*/

class PipelineBuilder {
public:
    explicit PipelineBuilder(istream& in) {
        workers_.push_back(make_unique<Reader>(in));
    }

    PipelineBuilder& FilterBy(Filter::Function filter) {
        workers_.push_back(make_unique<Filter>(move(filter)));
        return *this;
    }

    PipelineBuilder& CopyTo(string recipient) {
        workers_.push_back(make_unique<Copier>(move(recipient)));
        return *this;
    }

    PipelineBuilder& Send(ostream& out) {
        workers_.push_back(make_unique<Sender>(out));
        return *this;
    }

    unique_ptr<Worker> Build() {
        for (size_t i = workers_.size() - 1; i > 0; --i) {
            workers_[i - 1]->SetNext(move(workers_[i]));
        }
        return move(workers_[0]);
    }

private:
    vector<unique_ptr<Worker>> workers_;
};


void TestSanity() {
  string input = (
    "erich@example.com\n"
    "richard@example.com\n"
    "Hello there\n"

    "erich@example.com\n"
    "ralph@example.com\n"
    "Are you sure you pressed the right button?\n"

    "ralph@example.com\n"
    "erich@example.com\n"
    "I do not make mistakes of that kind\n"
  );
  istringstream inStream(input);
  ostringstream outStream;

  PipelineBuilder builder(inStream);
  builder.FilterBy([](const Email& email) {
    return email.from == "erich@example.com";
  });
  builder.CopyTo("richard@example.com");
  builder.Send(outStream);
  auto pipeline = builder.Build();

  pipeline->Run();

  string expectedOutput = (
    "erich@example.com\n"
    "richard@example.com\n"
    "Hello there\n"

    "erich@example.com\n"
    "ralph@example.com\n"
    "Are you sure you pressed the right button?\n"

    "erich@example.com\n"
    "richard@example.com\n"
    "Are you sure you pressed the right button?\n"
  );

  ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() {
/*
    string input = (
      "erich@example.com\n"
      "richard@example.com\n"
      "Hello there\n"

      "erich@example.com\n"
      "ralph@example.com\n"
      "Are you sure you pressed the right button?\n"

      "ralph@example.com\n"
      "erich@example.com\n"
      "I do not make mistakes of that kind\n"
    );
    istringstream inStream(input);
    Reader rd(inStream);
    rd.Run();
    vector<Email> mails = rd.GetMails();
    Filter fl([](const Email& email) {
        return email.from == "erich@example.com";
      });
    for(auto& m : mails){
        unique_ptr<Email> tmp = make_unique<Email>();
        *tmp = m;
        fl.Process(move(tmp));
    }
*/
  TestRunner tr;
  RUN_TEST(tr, TestSanity);
  return 0;
}
