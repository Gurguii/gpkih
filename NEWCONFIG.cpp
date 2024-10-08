#include <iostream>
#include <istream>
#include <string>
#include <string_view>
#include <fstream>
#include <unordered_map>
#include <cstring>
#include <sstream>
#include <algorithm>

bool compare(std::string_view st0, std::string_view st1, size_t nchars){
	for(int i = 0; i < nchars; ++i){
		if(st0[i] != st1[i]){
			return false;
		}
	}
	return true;
}

class Config{
	using ConfigMap = std::unordered_map<std::string,std::unordered_map<std::string,std::string>>;
private:
	/* BEG - Config properties */
	char commentChar;
	char keyValDelimiter;
	ConfigMap mappedConfiguration;

	char sectionOpenDelimiter;
	char sectionCloseDelimiter;
	/* END - Config properties */

	bool compare(std::string& st0, std::string &st1, size_t nChars){
		for(int i = 0; i < nChars; ++i){
			if(st0[i] != st1[i]){
				return false;
			}
		}
		return true;
	}

	void skipUntilMatch(std::ifstream &file, std::string &match){
		std::basic_ifstream<char>::pos_type currentLine = file.tellg();
		std::string line;
		while(getline(file, line)){
			if(compare(line, match, match.size())){
				file.seekg(currentLine);
				return;
			}
			currentLine = file.tellg();
		}
	}

	void skipUntilMatch(std::ifstream &file, char &match){
		std::basic_ifstream<char>::pos_type currentLine = file.tellg();
		std::string line;
		while(getline(file, line)){
			if(line[0] == match){
				file.seekg(currentLine);
				return;
			}
			currentLine = file.tellg();
		}
	}


public:
	Config() = delete;

	Config( 
		ConfigMap configTemplate, 
		char sectionOpenDelimiter = '[', 
		char sectionCloseDelimiter = ']', 
		char commentChar = '#',
		char keyValDelimiter = '='
	):mappedConfiguration(configTemplate),sectionOpenDelimiter(sectionOpenDelimiter),sectionCloseDelimiter(sectionCloseDelimiter),commentChar(commentChar),keyValDelimiter(keyValDelimiter){}

	int load(std::string_view filePath){
		std::string line;
		std::ifstream file(filePath.data());
		
		if(!file.is_open()){
			return -1;
		}

		while(getline(file, line)){
			if(line[0] != sectionOpenDelimiter){
				continue;
			}

			line.erase(std::remove_if(line.begin(), line.end(), [&](const char &c){return (c == ' ' || c == sectionCloseDelimiter || c == sectionOpenDelimiter);}), line.end());

			if(mappedConfiguration.find(line) == mappedConfiguration.end()){
				skipUntilMatch(file, sectionOpenDelimiter);
				continue;
			}

			printf("section: %s\n", line.c_str());

			std::unordered_map<std::string, std::string> &section = mappedConfiguration[line];
			std::string keyval{};
			
			auto pos = file.tellg();

			while(getline(file, keyval)){
				
				if(keyval[0] == commentChar || keyval.empty()){
					continue;
				}
				
				if(keyval[0] == sectionOpenDelimiter){
					file.seekg(pos);
					break;
				}

				pos = file.tellg();
				std::string key;
				std::string value;
				std::stringstream ss{keyval};
				getline(ss, key, keyValDelimiter);
				getline(ss, value);

				if(section.find(key) == section.end()){
					continue;
				}
			
				section[std::move(key)] = std::move(value);
				
			}
		}

		return 0;
	}

	void print(){
		for(const auto &[sectionName, section] : mappedConfiguration){
			printf("== %s ==\n", sectionName.c_str());
			for(const auto &[key, val] : section){
				std::cout << key << "=" << val << "\n";
			}
			printf("\n\n");
		}
	}

	ConfigMap* const getConfig(){
		return &mappedConfiguration;
	};

	std::unordered_map<std::string, std::string> *const getSection(std::string_view section){
		if(mappedConfiguration.find(section.data()) != mappedConfiguration.end()){
			return &mappedConfiguration[section.data()];
		}else{
			return nullptr;
		}
	};

	std::string *const getKey(std::string_view section, std::string_view key){
		if(mappedConfiguration.find(section.data()) != mappedConfiguration.end()){
			if(mappedConfiguration[section.data()].find(key.data()) != mappedConfiguration[section.data()].end()){
				return &mappedConfiguration[section.data()][key.data()];
			}
		}
		return nullptr;
	}

	int sync(){
		return 0;
	}

};

int main()
{
	std::unordered_map<std::string, std::unordered_map<std::string,std::string>> _template{
  		{
  			"behaviour",{
  			  {"headers",""},
  			  {"print_generated_certificate",""},
  			  {"prompt",""},
  			  {"autoanswer",""}
  		}},{
  			"logs",{
  			  {"includedFormatFields",""},
  			  {"includedLevels",""},
  			  {"maxSize",""}
  		}},
  			{"cli",{
  			  {"customPS",""}
  		}},
  			{"quiero",{
  			  {"morir",""}
  			},
  		}
	};

	Config config(_template);
	config.load("test.conf");
	config.print();

	auto a = config.getKey("behaviour", "prompt");
	printf("behaviour.prompt = %s\n", a->c_str());

	return 0;
}