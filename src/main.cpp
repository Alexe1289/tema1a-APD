#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <pthread.h>
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <atomic>

using namespace std;

class MapReduce {
    public:
        MapReduce(int num_mappers, int num_reducers, int nr_of_files, vector<string> files)
        : num_mappers(num_mappers), num_reducers(num_reducers), nr_of_files(nr_of_files) {
            pthread_mutex_init(&inputVariable, NULL);
            pthread_barrier_init(&barrier, NULL, num_mappers + num_reducers);
            thread_data = (ThreadData*) malloc(sizeof(ThreadData) * (num_mappers + num_reducers));
            threads = (pthread_t*) malloc(sizeof(pthread_t) * (num_mappers + num_reducers));
            aggregated_list.resize(nr_of_files);
            letter_list.resize(LETTERS);
            for (int i = 0; i < LETTERS; i++) {
                pthread_mutex_t mutex;
                pthread_mutex_init(&mutex, NULL);
                mutex_vec.push_back(mutex);
            }
            files_list = files;
            for (int i = 0; i < num_mappers + num_reducers; i++) {
                thread_data[i].id = i;
                thread_data[i].instance = this;
                int r = pthread_create(&threads[i], NULL, thread_func, &thread_data[i]);
                if (r) {
                    cout << "Eroare la crearea thread-ului " << i << endl;
                    exit(-1);
                }
            }

            void* status;
            for (int i = 0; i < num_mappers + num_reducers; i++) {
                int r = pthread_join(threads[i], &status);

                if (r) {
                    cout << "Eroare la thread-ul " << i << endl;
                    exit(-1);
                }
            }
            
        }
        ~MapReduce() {
            free(thread_data);
            free(threads);
            pthread_barrier_destroy(&barrier);
            pthread_mutex_destroy(&inputVariable);
            for (auto i : mutex_vec) {
                pthread_mutex_destroy(&i);
            }
        }
        struct ThreadData {
            int id;
            MapReduce* instance;
        };
        
    private:

        static bool sort_func(const pair<string, set<int>>& a, const pair<string, set<int>>& b) {
            if (a.second.size() != b.second.size()) {
                return a.second.size() > b.second.size();
            }
            return a.first < b.first;
        }

        static void* thread_func(void* args) {
            ThreadData t_data = *(ThreadData*) args;
            MapReduce* instance = t_data.instance;
            if (t_data.id < instance->num_mappers) {
                string file_name;
                while(instance->files_list.size() > 0) {

                    pthread_mutex_lock(&instance->inputVariable);
                    if (instance->files_list.size() <= 0) {
                        pthread_mutex_unlock(&instance->inputVariable);
                        break;
                    }
                    instance->file_idx++;
                    int hold_idx = instance->file_idx; //holds the current file index
                    file_name = instance->files_list.front();
                    instance->files_list.erase(instance->files_list.begin());
                    pthread_mutex_unlock(&instance->inputVariable);
                    if (file_name.empty()) {
                        return NULL;
                    }
                    ifstream file(file_name);
                    if (!file) {
                        cout << "Error reading at thread " << t_data.id << endl;
                    }
                    string line;
                    while (getline(file, line)) {
                        line.erase(remove_if(line.begin(), line.end(), [](unsigned char c) {
                            return !isalpha(c) && !isspace(c);
                        }), line.end());

                        line.erase(unique(line.begin(), line.end(), [](char a, char b) {
                            return a == ' ' && b == ' ';
                        }), line.end());

                        transform(line.begin(), line.end(), line.begin(), [](unsigned char c) {
                            return tolower(c);
                        });
                        // cout << line << endl;
                        stringstream ss(line);
                        unordered_map<string, int>& words_list = instance->aggregated_list[hold_idx - 1];
                        while(getline(ss, line, ' ')) {
                            if (line[0] == 0) {
                                continue;
                            }
                            words_list[line] = hold_idx;
                        }
                        // for (auto& x: words_list) {
                        //     std::cout << x.first << ": " << x.second << endl;
                        // }
                            

                    }
                    file.close();
                }
            }
            pthread_barrier_wait(&instance->barrier);
            
            if (t_data.id >= instance->num_mappers) {
                while (instance->start < instance->LETTERS) {
                    pthread_mutex_lock(&instance->inputVariable);
                    if (instance->start >= instance->LETTERS) {
                        pthread_mutex_unlock(&instance->inputVariable);
                        break;
                    }
                    int hold_idx = instance->start;
                    unordered_map<string, set<int>>& letter_idx_map = instance->letter_list[hold_idx];
                    instance->start++;
                    pthread_mutex_unlock(&instance->inputVariable);
                    for (int i = 0; i < instance->nr_of_files; i++) {
                        for (auto& pair : instance->aggregated_list[i]) {

                            if (pair.first[0] == hold_idx + 97) {
                                letter_idx_map[pair.first].insert(pair.second);
                            }
                        }
                    }
                    vector<pair<string, set<int>>> sorted_words(letter_idx_map.begin(), letter_idx_map.end());
                    sort(sorted_words.begin(), sorted_words.end(), sort_func);
                    pthread_mutex_lock(&instance->inputVariable);
                    string file(1, (char) hold_idx + 97);
                    ofstream fout(file + ".txt");
                    for (auto& entry : sorted_words) {
                        fout << entry.first << ":[";
                        auto last = prev(entry.second.end());
                        for(auto it = entry.second.begin(); it != entry.second.end(); it++) {
                            if (it == last) {
                                fout << *it << "]" << endl;
                            } else {
                                fout << *it << " ";
                            }
                        }
                    }
                    pthread_mutex_unlock(&instance->inputVariable);
                }
            }

            return NULL;
        }
        int num_mappers, num_reducers;
        vector<string> files_list;
        vector<unordered_map<string, int>> aggregated_list;
        vector<pthread_mutex_t> mutex_vec;
        vector<unordered_map<string, set<int>>> letter_list;
        // vector<vector<pair<string, set<int>>>> final_list;
        pthread_mutex_t inputVariable;
        pthread_barrier_t barrier;
        ThreadData* thread_data;
        pthread_t* threads;
        int file_idx = 0;
        int start = 0;
        int nr_of_files;
        const int LETTERS = 26;

};

int main(int argc, char **argv)
{
    ifstream in(argv[3]);
    if (!in) {
        cout << "Error reading at thread ";
        return 0;
    }
    int nr_of_files;
    vector<string> files;
    in >> nr_of_files;
    for (int i = 0; i < nr_of_files; i++) {
        string file;
        in >> file;
        files.push_back(file);
    }
    string nr_mapers = string(argv[1]);
    string nr_reducers = string(argv[2]);
    MapReduce mapR = MapReduce(stoi(nr_mapers), stoi(nr_reducers), nr_of_files, files);
    return 0;
}