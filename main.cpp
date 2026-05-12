#include <dirent.h>
#include <sys/stat.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

static const std::unordered_map<std::string, std::string> EXT_CATEGORY = {
    {".mp3", "audio"}, {".wav", "audio"}, {".aac", "audio"}, {".flac", "audio"},
    {".ogg", "audio"}, {".wma", "audio"}, {".m4a", "audio"},
    {".mp4", "video"}, {".mkv", "video"}, {".avi", "video"}, {".mov", "video"},
    {".wmv", "video"}, {".flv", "video"}, {".webm", "video"}, {".mpg", "video"},
    {".mpeg", "video"}, {".m4v", "video"},
    {".jpg", "images"}, {".jpeg", "images"}, {".png", "images"}, {".gif", "images"},
    {".bmp", "images"}, {".tiff", "images"}, {".tif", "images"}, {".webp", "images"},
    {".svg", "images"}, {".ico", "images"},
};

static std::string toLower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

static std::string getExtension(const std::string& name) {
    auto pos = name.rfind('.');
    if (pos == std::string::npos) return "";
    return toLower(name.substr(pos));
}

static std::string escapeJson(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

static void scan(const std::string& dir,
                 std::unordered_map<std::string, std::vector<std::string>>& media) {
    DIR* d = opendir(dir.c_str());
    if (!d) return;

    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        std::string fullPath = dir + "/" + name;

        struct stat st;
        if (stat(fullPath.c_str(), &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            scan(fullPath, media);
        } else if (S_ISREG(st.st_mode)) {
            std::string ext = getExtension(name);
            auto it = EXT_CATEGORY.find(ext);
            if (it != EXT_CATEGORY.end())
                media[it->second].push_back(name);
        }
    }
    closedir(d);
}

static void runScan(const std::string& home) {
    std::unordered_map<std::string, std::vector<std::string>> media;
    media["audio"];
    media["video"];
    media["images"];

    scan(home, media);

    std::ofstream out(home + "/.media_files/media.json");
    out << "{\n";
    const std::vector<std::string> order = {"audio", "video", "images"};
    for (std::size_t i = 0; i < order.size(); ++i) {
        const auto& cat = order[i];
        out << "  \"" << cat << "\": [";
        const auto& files = media[cat];
        for (std::size_t j = 0; j < files.size(); ++j) {
            out << " \"" << escapeJson(files[j]) << "\"";
            if (j + 1 < files.size()) out << ",";
        }
        out << " ]";
        if (i + 1 < order.size()) out << ",";
        out << "\n";
    }
    out << "}\n";

    std::cout << "Scan done."
              << "  audio: "  << media["audio"].size()
              << "  video: "  << media["video"].size()
              << "  images: " << media["images"].size()
              << "\n";
}

static int parseInterval(const std::string& arg) {
    if (arg.empty()) return -1;
    char unit = arg.back();
    int multiplier = 1;
    std::string digits = arg;
    if (unit == 's' || unit == 'm' || unit == 'h') {
        if (unit == 'm') multiplier = 60;
        else if (unit == 'h') multiplier = 3600;
        digits = arg.substr(0, arg.size() - 1);
    }
    try {
        int n = std::stoi(digits);
        if (n <= 0) return -1;
        return n * multiplier;
    } catch (...) {
        return -1;
    }
}

int main() {
    std::string input;
    int seconds = -1;
    while (seconds < 0) {
        std::cout << "Enter scan interval (e.g. 30, 5m, 1h): ";
        if (!(std::cin >> input)) { std::cerr << "Input error\n"; return 1; }
        seconds = parseInterval(input);
        if (seconds < 0)
            std::cerr << "Invalid interval, try again.\n";
    }

    const char* home = std::getenv("USERPROFILE");
    if (!home) home = std::getenv("HOME");
    if (!home) { std::cerr << "Cannot find home directory\n"; return 1; }

    std::cout << "Scanning every " << seconds << "s. Press Ctrl+C to stop.\n";

    while (true) {
        runScan(home);
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
    }
}
