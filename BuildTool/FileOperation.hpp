#pragma once
#include <algorithm>
#include <functional>
#include <regex>
#include <set>
#include <string>
#include <vector>
#include <stdexcept>
#include <io.h>

class Directory {
	typedef std::function<std::string(std::string&, std::string)> sss;

private:
	std::string path;

	void findExt(std::string format, std::set<std::string>& files,
		sss findDir = nullptr, sss findFile = nullptr, bool iter = true) {
		using namespace std;

		intptr_t  hFile = 0;//�ļ����  64λ��long ��Ϊ intptr_t
		struct _finddata_t fileinfo;//�ļ���Ϣ 
		string p;
		if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) //�ļ�����
		{
			do {
				if (fileinfo.name[0] == '.') continue;
				if ((fileinfo.attrib & _A_SUBDIR)) {							//�ж��Ƿ�Ϊ�ļ���
				
					if (findDir) files.insert(findDir(path, fileinfo.name));	//�����ļ�����
					if (!iter) continue;
					Directory tmp(path + "\\" + fileinfo.name);
					tmp.findExt(format, files, findDir, findFile);				//�ݹ�����ļ���
					
				}
				else
				{
					if (format.empty() || regex_search(fileinfo.name, regex(format + "$"))) {
						if (findFile)
							files.insert(findFile(path, fileinfo.name));			//��������ļ��У������ļ���
					}
				}
			} while (_findnext(hFile, &fileinfo) == 0);
			_findclose(hFile);
		}
	}

	
public:
	Directory subDir(std::string name) {
		return Directory(path + "\\" + name);
	}
	Directory(std::string path): path(path) {
		if (_access(path.c_str(), 0) == -1) {
			throw std::runtime_error("file not found: " + path);
		}
	}

	
	void getPackage_byBinary(std::set<std::string>& packages, std::string binRoot) {
		using namespace std;
		set<string> files;

		findExt(".class", files, nullptr, [](string& path, string name) -> string{
			return path + "\\" + name.erase(name.size() - 6);
		});

		for (string i : files) {
			string r = regex_replace(i, regex(binRoot), "");
			packages.insert(dir2package(r));
		}
	}

	void getPackage_bySource(std::set<std::string>& packages, std::string srcRoot) {
		using namespace std;
		set<string> files;

		findExt(".java", files, nullptr, [](string & path, string name) -> string {
			return path + "\\" + name.erase(name.size() - 5);
		});

		for (string i : files) {
			packages.insert(dir2package(regex_replace(i, regex(srcRoot), "")));
		}
	}

	void getCurrentStructure(std::set<std::string>& files) {
		auto fullpath = [](std::string & path, std::string name) {return path + "\\" + name; };
		findExt("", files, fullpath, fullpath, false);
	}

	void getCurrentSubDir(std::set<std::string>& dirs) {
		findExt("", dirs, [](std::string & path, std::string name) {return name; }, nullptr, false);
	}

	void findAllExt(std::string format, std::vector<std::string>& files) {
		std::set<std::string> tmp;
		findExt(format, tmp, nullptr, [](std::string & path, std::string name) {return path + "\\" + name; });
		files.resize(tmp.size());
		std::transform(tmp.begin(), tmp.end(), files.begin(), [](const std::string x) -> std::string {return x; });
	}

	static std::string dir2package(std::string dir) {
		dir = std::regex_replace(dir, std::regex("\\.class$"), "");

		if (dir.empty()) return "defalutPackage";
		std::string r;

		for (char i : dir) {
			if (i == '\\' || i == '/') r.push_back('.');
			else r.push_back(i);
		}

		return r;
	}

	static std::string package2dir(std::string package) {
		std::string r;
		for (char i : package) {
			if (i == '.') r.push_back('\\');
			else r.push_back(i);
		}

		r += ".class";
		return r;
	}
};