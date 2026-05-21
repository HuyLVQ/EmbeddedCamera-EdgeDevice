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
        static inline std::unordered_map<std::string, std::shared_ptr<Database>> s_databaseInstCompilation;

        std::map<std::string, std::variant<int, float, std::string>> m_dictionaryMapping;
        std::ifstream m_fileHandler;

        struct m_Tag {};

        void loadupDatabase();

    public:
        Database (const std::string& p_databaseFilePath, struct m_Tag p_tag);
        static std::shared_ptr<Database> getInstance(const std::string& p_databaseFilePath);
        std::variant<int, float, std::string> getValueFromKey(const std::string& p_key) const;
};