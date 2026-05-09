#pragma once

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>
#include <memory>
#include <string>
#include <variant>

#include "Exception.h"

class Database {
    private:
        static std::unordered_map<std::string, std::shared_ptr<Database>> s_databaseInstCompilation;

        std::map<std::string, std::variant<int, float, std::string>> m_dictionaryMapping;
        std::ifstream m_fileHandler;

        struct m_Tag {};

    public:
        Database (const std::string& p_databaseFilePath, struct m_Tag p_tag) {
            m_fileHandler = std::ifstream(p_databaseFilePath);
        }

        void loadupDatabase() {
            std::string currentLineReadout;
            while(std::getline(m_fileHandler, currentLineReadout)) {
                std::istringstream currentLineStream(currentLineReadout);
                std::string key, type, value;

                currentLineStream >> key;
                currentLineStream >> type;

                std::ostringstream oss;
                oss << currentLineStream.rdbuf();
                value = oss.str();
                
                if (type == "int") {
                    m_dictionaryMapping[key] = std::stoi(value);
                } else if (type == "float") {
                    m_dictionaryMapping[key] = std::stof(value);
                } else {
                    m_dictionaryMapping[key] = value;
                }
            }
        }

        static std::shared_ptr<Database> getInstance(const std::string& p_databaseFilePath) {
            if (s_databaseInstCompilation.find(p_databaseFilePath) == s_databaseInstCompilation.end()) {
                auto newAssigningInstance = std::make_shared<Database>(p_databaseFilePath, m_Tag{});
                s_databaseInstCompilation[p_databaseFilePath] = newAssigningInstance;

                newAssigningInstance->loadupDatabase();

                return newAssigningInstance;
            }

            return s_databaseInstCompilation[p_databaseFilePath];
        }        

        std::variant<int, float, std::string> getValueFromKey(const std::string& p_key) const {
            return m_dictionaryMapping.at(p_key);
        }
};