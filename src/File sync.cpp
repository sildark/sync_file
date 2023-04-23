
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>


namespace fs = std::filesystem;

void read_directory(const fs::path& path, std::vector<fs::path>& files)
{
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_directory())
        {
            read_directory(entry.path(), files);
        }
    }
    for (const auto& entry : fs::directory_iterator(path))
    { if (entry.is_regular_file())
        {
            files.push_back(entry.path());
        }
    }
}
bool compare_files(const std::string& file1_path, const std::string& file2_path)
{
    std::ifstream file1(file1_path, std::ios::binary);
    std::ifstream file2(file2_path, std::ios::binary);

    if (!file1 || !file2) {
        return false; 
    }

    std::vector<char> buffer1(std::istreambuf_iterator<char>(file1), {});
    std::vector<char> buffer2(std::istreambuf_iterator<char>(file2), {});

    return buffer1 == buffer2;
}

void sync_folders(const std::vector<fs::path>& folder1_files, const std::vector<fs::path>& folder2_files, const std::string& folder1_path, const std::string& folder2_path, const std::string& log_path)
{
    time_t now = time(0);
    tm local_time;
    localtime_s(&local_time, &now);

    std::ofstream log_file(log_path);
    

    // Iterate over all files in the first folder
    for (const auto& file1_path : folder1_files) {
        std::string file2_path = folder2_path + file1_path.string().substr(folder1_path.size());

        // If the corresponding file exists in the second folder, compare the contents of the two files
        if (std::find(folder2_files.begin(), folder2_files.end(), file2_path) != folder2_files.end()) {
            if (!compare_files(file1_path.string(), file2_path)) {
                fs::remove(file2_path);
                fs::copy_file(file1_path, file2_path);
                log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Updated " << file2_path << " to " << folder2_path << std::endl;
                std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Updated " << file2_path << " to " << folder2_path << std::endl;
            }
        }
        // If the file does not exist in the second folder, create a copy of the file from the first folder in the second folder
        else {
            fs::create_directories(fs::path(file2_path).parent_path());
            fs::copy_file(file1_path, file2_path);
            log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Created" << file2_path << " to " << folder2_path << std::endl;
            std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Created" << file2_path << " to " << folder2_path << std::endl;
        }
    }

    // Iterate over all files in the second folder
    for (const auto& file2_path : folder2_files) {
        std::string file1_path = folder1_path + file2_path.string().substr(folder2_path.size());

        // If the corresponding file does not exist in the first folder, delete the file from the second folder
        if (std::find(folder1_files.begin(), folder1_files.end(), file1_path) == folder1_files.end()) {
            fs::remove(file2_path);
            log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << std::endl;
            std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <original_folder> <sync_folder> <log_file>\n";
        return 1;
    }
    const std::string path = argv[1];
    const std::string dest_dir = argv[2];
    const std::string log_file = argv[3];
    std::ofstream sync_file_stream(dest_dir);
    if (!sync_file_stream) 
    {
        std::cerr << "Sync file already exists. Using folder.\n";
    }
    sync_file_stream.close();
    std::ofstream log_file_stream(log_file);
    if (!log_file_stream) 
    {
        std::cerr << "Log file already exists. Using file.\n";        
    }
    log_file_stream.close();

    char timestamp[32];
    //std::string log_file = "C:/Users/Silviu/Desktop/sol/Log.txt";
    //std::string path = "C:/Users/Silviu/Desktop/sol/original";
    //std::string dest_dir = "C:/Users/Silviu/Desktop/sol/destination";
    std::vector<fs::path> Sync;
    std::vector<fs::path> files;
    
    read_directory(dest_dir, Sync);
    read_directory(path, files);

    std::ofstream log(log_file, std::ios::app);
    while (true)
    {
        std::string input;
        std::getline(std::cin, input);

        // Check if the user entered "end"
        if (input == "end") {
            break; // Exit the loop
        }
        auto current_time = std::chrono::system_clock::now();
        std::time_t current_time_t = std::chrono::system_clock::to_time_t(current_time);
        ctime_s(timestamp, sizeof(timestamp), &current_time_t);

        std::cout << "Sync completed at " << timestamp;
       
        sync_folders(files,Sync,path ,dest_dir,log_file);

        // wait for 5 minutes before the next sync
        std::this_thread::sleep_for(std::chrono::minutes(5));
    }

    
  
    return 0;
}

