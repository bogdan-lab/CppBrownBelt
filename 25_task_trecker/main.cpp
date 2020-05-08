#include <map>
#include <tuple>
#include <iostream>
#include <string>
#include <unordered_map>
//#include "stuff.h"
using namespace std;



enum class TaskStatus {
  NEW,          // новая
  IN_PROGRESS,  // в разработке
  TESTING,      // на тестировании
  DONE          // завершена
};

TaskStatus Next(TaskStatus mode){
    return static_cast<TaskStatus>(static_cast<int>(mode) + 1);
}

using TasksInfo = map<TaskStatus, int>;

class TeamTasks {
private:
    unordered_map<string, TasksInfo> data_;

    void MergeTaskInfos(TasksInfo upd, const string& person){
        TaskStatus stat = TaskStatus::NEW;
        while(stat!=TaskStatus::DONE){
            data_[person][stat] -= upd[Next(stat)];
            data_[person][Next(stat)] += upd[Next(stat)];
            stat = Next(stat);
        }
    }

    TasksInfo GetUntouched(TasksInfo upd, const string& person) const {
        TaskStatus stat = TaskStatus::NEW;
        TasksInfo untouched;
        while(stat!=TaskStatus::DONE){
            if(data_.at(person).count(stat)>0 &&
                    (data_.at(person).at(stat)>upd[Next(stat)])){
                untouched[stat] = data_.at(person).at(stat) - upd[Next(stat)];
            }
            stat = Next(stat);
        }
        return untouched;
    }

public:
  // Получить статистику по статусам задач конкретного разработчика
    const TasksInfo& GetPersonTasksInfo(const string& person) const{
        return data_.at(person);
    }

    void AddNewTask(const string& person){
        data_[person][TaskStatus::NEW]++;
    }

  tuple<TasksInfo, TasksInfo> PerformPersonTasks(
          const string& person, int task_count){
      TasksInfo updated;
      const TasksInfo& current = data_[person];
      TaskStatus stat = TaskStatus::NEW;
      while(task_count>0 && stat!=TaskStatus::DONE){
          if(current.count(stat)>0){
              updated[Next(stat)] = min(current.at(stat), task_count);
              task_count-=current.at(stat);
          }
          stat = Next(stat);
      }
    TasksInfo untouched = GetUntouched(updated, person);
      MergeTaskInfos(updated, person);
    return tie(updated, untouched);
  }
};



// Принимаем словарь по значению, чтобы иметь возможность
// обращаться к отсутствующим ключам с помощью [] и получать 0,
// не меняя при этом исходный словарь
void PrintTasksInfo(TasksInfo tasks_info) {
  cout << tasks_info[TaskStatus::NEW] << " new tasks" <<
      ", " << tasks_info[TaskStatus::IN_PROGRESS] << " tasks in progress" <<
      ", " << tasks_info[TaskStatus::TESTING] << " tasks are being tested" <<
      ", " << tasks_info[TaskStatus::DONE] << " tasks are done" << endl;
}

int main() {
  TeamTasks tasks;
  tasks.AddNewTask("Ilia");
  for (int i = 0; i < 3; ++i) {
    tasks.AddNewTask("Ivan");
  }
  cout << "Ilia's tasks: ";
  PrintTasksInfo(tasks.GetPersonTasksInfo("Ilia"));
  cout << endl;
  cout << "Ivan's tasks: ";
  PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"));

  TasksInfo updated_tasks, untouched_tasks;

  tie(updated_tasks, untouched_tasks) =
      tasks.PerformPersonTasks("Ivan", 2);
  cout << endl;
  cout << "Updated Ivan's tasks: ";
  PrintTasksInfo(updated_tasks);
  cout << endl;
  cout << "Untouched Ivan's tasks: ";
  PrintTasksInfo(untouched_tasks);
  cout << "Ivan's tasks: ";
  PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"));


  cout << endl;
  tie(updated_tasks, untouched_tasks) =
      tasks.PerformPersonTasks("Ivan", 2);
  cout << "Updated Ivan's tasks: ";
  PrintTasksInfo(updated_tasks);
  cout << endl;
  cout << "Untouched Ivan's tasks: ";
  PrintTasksInfo(untouched_tasks);
  cout << endl;
  return 0;
}
