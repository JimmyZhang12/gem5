#include <vector>
#include <string>

class SaveData {

    public:
        SaveData();
        void save_data(double data);
        void data_to_file(std::string write_path, std::string data_name);


    private:
        static std::vector<double> data;
        void write_file();
};