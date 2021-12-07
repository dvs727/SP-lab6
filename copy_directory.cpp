/*
Написать две программы. Первая - копирует файл. Вторая - копирует содержимое
директории пофайлово с помощью первой программы в отдельных процессах.
*/
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <mutex>
#include <utility>

using namespace std;

#define DIRECTORY 4
#define FILE 8

#define HANDLERS_COUNT 4

void create_copy_tasks(string path_to_source, string path_to_destination);
void handle_copy_tasks();
void copy_file(string path_to_source, string path_to_destination);

vector<pair<string, string>> copy_tasks;
mutex copy_tasks_mutex;

bool can_finish = false;
mutex can_finish_mutex;


int main(int argc, char const *argv[]){
	if (argc != 3){
		cout << "Usage: " << argv[0] << " <source_directory> <destination_directory>" << endl;
		exit(0);
	}

	string path_to_source = argv[1];
	string path_to_destination = argv[2];

	thread th(
		create_copy_tasks,
		path_to_source,
		path_to_destination
	);

	vector<thread> thread_handlers;

	for (int i = 0; i < HANDLERS_COUNT; ++i)
		thread_handlers.push_back(thread(handle_copy_tasks));


	for (int i = 0; i < HANDLERS_COUNT; ++i)
		thread_handlers[i].join();

	cout << "Copy task handlers done their work" << endl;

	th.join();

	cout << "Finish program, copy_tasks count: " << copy_tasks.size() << endl;

	return 0;
}

void create_copy_tasks(string path_to_source, string path_to_destination) {
	DIR *dir;
	struct dirent *ent;
	dir = opendir(path_to_source.c_str());
	

	while ((ent = readdir (dir)) != NULL){
		string name(ent->d_name); 
		if (name == "." || name == "..")
			continue;

		int type = ent->d_type;
		if (type & DIRECTORY)
			continue; // maybe recursive copy_directory?
		
		string from = path_to_source + "/" + name;
		string to = path_to_destination + "/" + name;

		copy_tasks_mutex.lock();
		copy_tasks.push_back(pair<string, string>(from, to));
		cout << "Task created for copying " << from << " to " << to << endl;
		copy_tasks_mutex.unlock();
	
	}

	closedir (dir);

	can_finish_mutex.lock();
	can_finish = true;
	can_finish_mutex.unlock();
}


void handle_copy_tasks() {
	bool can_finish_here = false;
	int copy_tasks_size = 1;


	while (true) {
		can_finish_mutex.lock();
		bool can_finish_here = can_finish;
		can_finish_mutex.unlock();

		copy_tasks_mutex.lock();
		int copy_tasks_size = copy_tasks.size();
		copy_tasks_mutex.unlock();

		if(can_finish_here && copy_tasks_size == 0)
			break;


		cout << "Try to get task" << endl;
	

		copy_tasks_mutex.lock();

		if(copy_tasks.size() == 0){
	
			cout << "Copy tasks empty" << endl;
	
			copy_tasks_mutex.unlock();
			continue;
		}

		pair<string, string> copy_task = copy_tasks.back();

		copy_tasks.pop_back();
		copy_tasks_mutex.unlock();

		string path_to_source = copy_task.first;
		string path_to_destination = copy_task.second;


		cout << "Got task: " << path_to_source << " to " << path_to_destination << endl;

		
		if (path_to_source != "" && path_to_destination != "")
			copy_file(path_to_source, path_to_destination);
	}
}


void copy_file(string path_to_source, string path_to_destination) {
	cout << "Copying " << path_to_source << " to " << path_to_destination << endl;

	string line;
    ifstream inp_file(path_to_source);
    ofstream out_file(path_to_destination);
 
    if(inp_file && out_file)
        while(getline(inp_file,line))
            out_file << line << "\n";
    
    inp_file.close();
    out_file.close(); 
}