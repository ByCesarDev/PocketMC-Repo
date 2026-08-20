#pragma once
#include <string>
#include <vector>

struct PanoramaInfo {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    std::string folderPath;
    std::string iconPath;
};

class PanoramaRegistry {
public:
    static std::vector<PanoramaInfo> getAllPanoramas();
    static PanoramaInfo getPanoramaById(const std::string& id);
    static PanoramaInfo getDefaultPanorama();
};
