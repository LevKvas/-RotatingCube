#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>
#include <QString>

struct Vector3 {
    float x, y, z;
};


class MeshObjFile{
public:
    MeshObjFile(const std::string& _filename){
        filename = _filename;
        loadFile();
    }

    const std::vector<Vector3>& getVertices()const{
        return vertices;
    }

    const std::vector<unsigned int>& getIndices()const{
        return indices;
    }

    void loadFile(){
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Не удалось открыть файл");
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string type;
            ss >> type;

            if (type == "v") {
                // read coors
                Vector3 v;
                ss >> v.x >> v.y >> v.z;
                vertices.push_back(v);
            }
            else if (type == "f") {
                std::string v1, v2, v3;
                ss >> v1 >> v2 >> v3;

                // Функция, которая вытаскивает только первое число до слэша
                auto parseIdx = [](const std::string& s) -> unsigned int {
                    std::stringstream temp(s);
                    std::string firstPart;
                    std::getline(temp, firstPart, '/'); // Читаем только до первого '/'
                    return std::stoul(firstPart);
                };

                try {
                    unsigned int i1 = parseIdx(v1);
                    unsigned int i2 = parseIdx(v2);
                    unsigned int i3 = parseIdx(v3);

                    indices.push_back(i1 - 1);
                    indices.push_back(i2 - 1);
                    indices.push_back(i3 - 1);
                } catch (...) {
                    // Пропускаем битые строки
                }
            }
        }
        file.close();
    }


private:
    std::string filename{};
    std::vector<Vector3> vertices;
    std::vector<unsigned int> indices;
};
