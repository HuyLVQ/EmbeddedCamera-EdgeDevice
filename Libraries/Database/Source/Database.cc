#include "Database.h"

Database::Database (const std::string& p_databaseFilePath, struct m_Tag p_tag) {
    m_fileHandler = std::ifstream(p_databaseFilePath);
}

void Database::loadupDatabase() {
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

std::shared_ptr<Database> Database::getInstance(const std::string& p_databaseFilePath) {
    if (s_databaseInstCompilation.find(p_databaseFilePath) == s_databaseInstCompilation.end()) {
        auto newAssigningInstance = std::make_shared<Database>(p_databaseFilePath, m_Tag{});
        s_databaseInstCompilation[p_databaseFilePath] = newAssigningInstance;

        newAssigningInstance->loadupDatabase();

        return newAssigningInstance;
    }

    return s_databaseInstCompilation[p_databaseFilePath];
}     

std::variant<int, float, std::string> Database::getValueFromKey(const std::string& p_key) const {
    return m_dictionaryMapping.at(p_key);
}