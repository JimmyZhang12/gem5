#include "cpu/power/write_data.hh"
#include <fstream>
#include <iostream>

using namespace std;

vector<double> SaveData::data;

SaveData::SaveData(){};

void 
SaveData::save_data(double data){
    this->data.push_back(data);
}

void SaveData::data_to_file(string write_path, string data_name){
    string fullpath = write_path + '/' + data_name + ".bin";
    cout <<"Writing to: " << fullpath << endl;
    ofstream outfile(fullpath, ios::out | ios::binary);

    auto bytes = data.size() * sizeof(data[0]);
    outfile.write(reinterpret_cast<char*>(&data[0]), bytes);
}

 