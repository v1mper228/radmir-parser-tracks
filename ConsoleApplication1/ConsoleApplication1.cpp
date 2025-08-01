#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <random>

using json = nlohmann::json;

struct Track {
    int id;
    std::string name;
    std::vector<int> category;
    int duration;
    long long released;
};

size_t CallBack(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
std::string get_random_user_agent() {
    static const std::vector<std::string> user_agents = {
        // Chrome
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36",

        // Firefox
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/120.0",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:109.0) Gecko/20100101 Firefox/119.0",
        "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/118.0",

        // Safari
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 14_1) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Safari/605.1.15",
        "Mozilla/5.0 (iPad; CPU OS 17_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Mobile/15E148 Safari/604.1",

        // Edge
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0",

        // Opera
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36 OPR/105.0.0.0",

        // Mobile
        "Mozilla/5.0 (iPhone; CPU iPhone OS 17_1_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Mobile/15E148 Safari/604.1",
        "Mozilla/5.0 (Linux; Android 14; SM-S918B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.6099.43 Mobile Safari/537.36"
    }; // анти жоский детект by radmir ( хотя и с musicparser парсило, но малоли idk )

    static std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<size_t> dist(0, user_agents.size() - 1);
    return user_agents[dist(rng)];
}
std::string download_json(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return "[smile_dev] -> ti lox ebaniy";

    std::string read_buffer;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CallBack);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, get_random_user_agent().c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }
    curl_easy_cleanup(curl);
    return read_buffer;
}

int main() {
   
    setlocale(LC_ALL, "");

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::ifstream input_file("input.json");
    if (!input_file.is_open()) {
        return 1;
    }

    json input;
    try {
        input_file >> input;
    }
    catch (const json::exception& e) {
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::vector<Track> all_tracks;
    const std::string base_url = "https://music.radmir.online/";
    size_t total_files = 0;


    if (!input.is_array() || input.size() < 2 || !input[1].is_array()) {
        return 1;
    }

    for (const auto& genre_list : input[1]) {
        if (!genre_list.is_array()) continue;

        for (const auto& filename : genre_list) {
            if (!filename.is_string()) continue;

            std::string file_url = base_url + filename.get<std::string>();
            std::cout << "[smile_dev] -> " << file_url << std::endl;

            std::string json_data = download_json(file_url);
            total_files++;

            if (json_data.empty()) continue;

            try {
                json tracks = json::parse(json_data);

                for (const auto& track_data : tracks) {
                    Track track;
                    track.id = track_data["id"].get<int>();
                    track.name = track_data["name"].get<std::string>();

                    for (const auto& cat : track_data["category"]) {
                        track.category.push_back(cat.get<int>());
                    }

                    track.duration = track_data["duration"].get<int>();
                    track.released = track_data["released"].get<long long>();
                    all_tracks.push_back(track);
                }
            }
            catch (const json::exception& e) {
                
                // ne eby che pisat, no bez nego v momente krashit ( radmir pidarasi )
                std::cout << "[smile_dev] govno_kot founded-> 1488 swaga 0 " << std::endl;
            }
        }
    }

    curl_global_cleanup();


    std::ofstream out_file("all_tracks.json");
    json output_json = json::array();
    for (const auto& track : all_tracks) {
        output_json.push_back({
            {"id", track.id},
            {"name", track.name},
            {"category", track.category},
            {"duration", track.duration},
            {"released", track.released},
            {"smile_studio", "1488->swaga?"}
            });
    }
    out_file << output_json.dump(2);
    std::cout << "[smile_dev] -> dumped succesful-> swaga - 100" << std::endl;
    std::getchar();
    

    return 0;
}